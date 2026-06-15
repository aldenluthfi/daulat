//! main.c
//!
//! Headless engine entry point. Reads frontend verbs from stdin and
//! writes `< ...` lines to stdout via the engine state machine.
//! Long-standing CLI subcommands (--new-profile, --dump-save,
//! --test-save, --debug-map, --debug-jump-results) short-circuit
//! before the main loop and exit directly.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              CLI SUBCOMMANDS
\*--------------------------------------------------------------------------*/

/// Zero-init a Profile and persist it to disk.
static int cli_new_profile(void) {
    Profile profile;
    profile_new(&profile);
    if (!profile_save(&profile)) {
        log_err("--new-profile: profile_save failed");
        return 1;
    }
    const char* path = profile_path();
    log_info("--new-profile: wrote %s", path != NULL ? path : "(null)");
    return 0;
}

/// Pretty-print a save file's header + chunk index.
static int cli_dump_save(const char* path) {
    return save_dump(path);
}

/// Simulate a Vorath defeat (or loss): load profile, build a
/// synthetic run with no chain disqualification, run_finalize, save.
static int cli_debug_jump_results(bool won) {
    Profile profile;
    if (!profile_load(&profile))
        profile_new(&profile);
    RunState run;
    run_init(&run, 0xC1A1C1A1A0A1A1A1ULL);
    run.profile = &profile;
    for (size_t k = 0; k < KINGDOM_COUNT; k++) {
        run.cleared_kingdoms[k]     = true;
        run.mastery_disqualified[k] = false;
    }
    RunEnd outcome = won ? RUN_END_VORATH_WIN : RUN_END_LOSS;
    run_finalize(&run, outcome);
    log_info(
        "--debug-jump-results: prestige=%u defeats=%u "
        "wins=%u losses=%u",
        profile.prestige_tier, profile.vorath_defeat_count,
        profile.total_wins, profile.total_losses
    );
    for (size_t k = 0; k < KINGDOM_COUNT; k++)
        log_info("  mastery[%zu]=%u", k, profile.mastery_levels[k]);
    return 0;
}

/// Generate a map for (kingdom, tier) and print its node table.
static int cli_debug_map(const char* kingdom_arg, const char* tier_arg) {
    static const struct {
        const char* name;
        Kingdom     value;
    } KINGDOMS[] = {
        {"LONGWEI", KINGDOM_LONGWEI},
        {"HARUSHIMA", KINGDOM_HARUSHIMA},
        {"KEWARANI", KINGDOM_KEWARANI},
        {"ZARQAN", KINGDOM_ZARQAN},
        {"CAELAN", KINGDOM_CAELAN},
    };
    static const struct {
        const char* name;
        Tier        value;
    } TIERS[] = {
        {"TOWN", TIER_TOWN},
        {"PROVINCE", TIER_PROVINCE},
        {"COUNTRY", TIER_COUNTRY},
    };
    Kingdom k    = KINGDOM_LONGWEI;
    Tier    tier = TIER_TOWN;
    bool    ok   = false;
    for (size_t i = 0; i < sizeof(KINGDOMS) / sizeof(KINGDOMS[0]); i++)
        if (strcmp(kingdom_arg, KINGDOMS[i].name) == 0) {
            k  = KINGDOMS[i].value;
            ok = true;
            break;
        }
    if (!ok) {
        log_err("--debug-map: unknown kingdom '%s'", kingdom_arg);
        return 1;
    }
    ok = false;
    for (size_t i = 0; i < sizeof(TIERS) / sizeof(TIERS[0]); i++)
        if (strcmp(tier_arg, TIERS[i].name) == 0) {
            tier = TIERS[i].value;
            ok   = true;
            break;
        }
    if (!ok) {
        log_err("--debug-map: unknown tier '%s'", tier_arg);
        return 2;
    }
    MapState map;
    map_generate(&map, k, tier, 0xC0FFEEABCDULL);

    static const char* TYPE_NAMES[] = {
        "BATTLE", "ELITE", "ARCHIVE", "OFFERING",
        "EVENT", "OVERSEER", "LIBERATION"
    };
    log_info(
        "map: %s %s | seed=%llx | nodes=%u",
        kingdom_arg, tier_arg, (unsigned long long)map.seed, map.node_count
    );
    for (uint8_t i = 0; i < map.node_count; i++) {
        const MapNode* node = &map.nodes[i];
        log_info(
            "  [%u] %-10s payload=%u mod=%u trait=%u -> %u",
            node->id,
            TYPE_NAMES[node->type < NODE_TYPE_COUNT ? node->type : 0],
            node->payload_id,
            node->modifier_id,
            node->trait_id,
            node->edge_count > 0 ? node->edges[0] : 0xFFFFu
        );
    }
    return 0;
}

/// Round-trip a synthetic RunState through save+load.
static int cli_test_save(void) {
    RunState before;
    run_init(&before, 0xC0FFEE12345678ULL);
    before.current_kingdom         = KINGDOM_HARUSHIMA;
    before.current_map_tier        = TIER_TOWN;
    before.relic_count             = 0;
    run_add_relic(&before, RELIC_MINTED_COIN);
    run_add_relic(&before, RELIC_IRON_KING);
    before.chain_levels[KINGDOM_LONGWEI] = 2;
    before.subjugated[KINGDOM_ZARQAN]    = true;
    before.vorath_counter                = 7;
    before.vorath_pressure               = 3;
    before.cleared_kingdoms[KINGDOM_CAELAN] = true;
    before.cleared_maps[KINGDOM_HARUSHIMA][TIER_TOWN] = true;
    before.mastery_l3[KINGDOM_HARUSHIMA]  = 1;
    before.revealed_recipes              = 0xDEADBEEFCAFEBABEULL;
    before.forbidden_recipes             = 0x1234567890ABCDEFULL;
    before.flags = RUN_FOREIGN_MARKUP_OFF | RUN_VISION_ENEMY_VALUES;
    before.chain_silver_pending[KINGDOM_LONGWEI] = 2;
    before.chain_silver_pending[KINGDOM_CAELAN]  = 1;

    if (!run_save(&before)) {
        log_err("--test-save: run_save failed");
        return 1;
    }
    RunState after;
    memset(&after, 0xAA, sizeof(after));
    if (!run_load(&after)) {
        log_err("--test-save: run_load failed");
        return 2;
    }
    before.profile = NULL;
    after.profile  = NULL;
    if (memcmp(&before, &after, sizeof(RunState)) != 0) {
        log_err("--test-save: round-trip mismatch");
        return 3;
    }
    log_info("--test-save: round-trip OK");
    run_delete();
    return 0;
}

/*--------------------------------------------------------------------------*\
                              ENTRY
\*--------------------------------------------------------------------------*/

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--new-profile") == 0)
            return cli_new_profile();
        if (strcmp(argv[i], "--dump-save") == 0 && i + 1 < argc)
            return cli_dump_save(argv[i + 1]);
        if (strcmp(argv[i], "--test-save") == 0)
            return cli_test_save();
        if (strcmp(argv[i], "--debug-map") == 0 && i + 2 < argc)
            return cli_debug_map(argv[i + 1], argv[i + 2]);
        if (strcmp(argv[i], "--debug-jump-results") == 0) {
            bool won = true;
            if (i + 1 < argc && strcmp(argv[i + 1], "loss") == 0)
                won = false;
            return cli_debug_jump_results(won);
        }
    }

    EngineState engine;
    if (!engine_init(&engine, stdin, stdout))
        return 1;
    char line[PROTOCOL_LINE_BYTES];
    while (!engine.quitting
           && protocol_read_line(stdin, line, sizeof(line))) {
        engine_handle_line(&engine, line);
    }
    engine_destroy(&engine);
    return 0;
}
