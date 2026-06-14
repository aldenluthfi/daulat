//!
//! main.c
//!
//! SDL3 entry point. Uses the `SDL_MAIN_USE_CALLBACKS` pattern to
//! hand control to the four `SDL_App*` callbacks. The application
//! state lives in `src/app.c`.
//!
//! Define `SMOKE_TEST` at build time to compile the original two-
//! turn battle-trace demo instead. Useful for engine-only validation
//! without an SDL3 window.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifdef SMOKE_TEST

#include "prelude.h"
#include <stdio.h>

/*--------------------------------------------------------------------------*\
                              PRINT HELPERS
\*--------------------------------------------------------------------------*/

static void print_board(const BattleState* bs) {
    log_info("  Board %dx%d:", bs->board.width, bs->board.height);
    for (int y = bs->board.height - 1; y >= 0; y--) {
        printf("  %2d ", y);
        for (int x = 0; x < bs->board.width; x++) {
            Position    p  = {x, y};
            PieceState* pc = board_at(&bs->board, p);
            if (pc == NULL) {
                printf(". ");
            } else {
                char side = (pc->owner == SIDE_PLAYER) ? 'P' : 'E';
                printf("%c ", side);
            }
        }
        printf("\n");
    }
    printf("    ");
    for (int x = 0; x < bs->board.width; x++)
        printf("%d ", x);
    printf("\n");
}

static void print_pieces(const BattleState* bs) {
    log_info("  Pieces:");
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        const PieceState* piece = &bs->pieces[i];
        const char* side = (piece->owner == SIDE_PLAYER) ? "PLAYER" : "ENEMY ";
        log_info(
            "    [%u] %s (%s) at (%d,%d) val=%d",
            piece->id,
            piece->tmpl->name,
            side,
            piece->pos.x,
            piece->pos.y,
            piece_value(piece)
        );
    }
}

static void print_state(const BattleState* bs) {
    log_info(
        "Turn %u | Active: %s | CP: P=%d E=%d | "
        "Meter: P=%d/%d E=%d/%d",
        bs->turn_no,
        bs->active_side == SIDE_PLAYER ? "PLAYER" : "ENEMY ",
        bs->cp[SIDE_PLAYER],
        bs->cp[SIDE_ENEMY],
        bs->meter[SIDE_PLAYER],
        bs->meter_cap[SIDE_PLAYER],
        bs->meter[SIDE_ENEMY],
        bs->meter_cap[SIDE_ENEMY]
    );
}

static void print_hand(const BattleState* bs, Side side) {
    const char* side_str = (side == SIDE_PLAYER) ? "PLAYER" : "ENEMY ";
    uint8_t     count    = bs->hand_count[side];
    log_info("  %s hand (%d cards):", side_str, count);
    for (uint8_t i = 0; i < count; i++) {
        const CardTemplate* tmpl = bs->hand[side][i].tmpl;
        log_info(
            "    [%u] %s cost=%d sell=%d",
            i,
            tmpl->name,
            tmpl->play_cost,
            tmpl->sell_value
        );
    }
}

/*--------------------------------------------------------------------------*\
                              SMOKE TEST
\*--------------------------------------------------------------------------*/

int main(void) {
    log_info("=== Regnum Battle Boilerplate Demo ===");

    BattleConfig cfg = {
        .width          = 12,
        .height         = 12,
        .max_turns      = 10,
        .starting_cp    = 20,
        .rng_seed       = 0xDEADBEEFULL,
        .player_side    = SIDE_PLAYER,
        .modifiers      = {0},
        .modifier_count = 0,
        .traits         = {0},
        .trait_count    = 0,
        .run            = NULL,
    };

    BattleState bs;
    battle_init(&bs, &cfg);

    Position pk = {5, 0};
    Position ek = {5, 11};
    piece_spawn(&bs, PIECE_KING, pk, SIDE_PLAYER);
    piece_spawn(&bs, PIECE_KING, ek, SIDE_ENEMY);

    Position pp = {3, 1};
    Position ep = {3, 10};
    piece_spawn(&bs, PIECE_PAWN, pp, SIDE_PLAYER);
    piece_spawn(&bs, PIECE_KNIGHT, ep, SIDE_ENEMY);

    log_info("--- Initial State ---");
    print_board(&bs);
    print_pieces(&bs);

    log_info("");
    log_info("--- TURN 1: PLAYER ---");
    battle_turn_start(&bs);
    print_state(&bs);
    print_hand(&bs, SIDE_PLAYER);

    Position pawn_to = {3, 2};
    if (battle_can_move(&bs, bs.pieces[1].id, pawn_to)) {
        battle_action_move(&bs, bs.pieces[1].id, pawn_to);
        log_info("  Moved pawn to (%d,%d)", pawn_to.x, pawn_to.y);
    }

    if (bs.hand_count[SIDE_PLAYER] > 0) {
        battle_sell_card(&bs, 0);
        log_info("  Sold card 0");
    }

    print_state(&bs);
    print_board(&bs);

    battle_turn_end(&bs);

    log_info("");
    log_info("--- TURN 1: ENEMY ---");
    battle_turn_start(&bs);
    print_state(&bs);
    ai_play_turn(&bs);
    battle_turn_end(&bs);

    log_info("");
    log_info("--- TURN 2: PLAYER ---");
    battle_turn_start(&bs);
    print_state(&bs);
    print_hand(&bs, SIDE_PLAYER);

    if (bs.piece_count > 0) {
        MoveList ml = {0};
        battle_legal_moves(&bs, bs.pieces[0].id, &ml);
        if (ml.count > 0) {
            battle_action_move(&bs, bs.pieces[0].id, ml.squares[0]);
            log_info(
                "  Moved piece 0 to (%d,%d)",
                ml.squares[0].x,
                ml.squares[0].y
            );
        }
    }

    print_state(&bs);
    battle_turn_end(&bs);

    log_info("");
    log_info("--- TURN 2: ENEMY ---");
    battle_turn_start(&bs);
    print_state(&bs);
    ai_play_turn(&bs);
    battle_turn_end(&bs);

    log_info("");
    log_info("--- Final State ---");
    BattleResult res        = battle_check_end(&bs);
    const char*  result_str = "UNKNOWN";
    if (res == BATTLE_PLAYER_WON)
        result_str = "PLAYER WON";
    else if (res == BATTLE_ENEMY_WON)
        result_str = "ENEMY WON";
    else if (res == BATTLE_DRAW)
        result_str = "DRAW";
    log_info("Battle result: %s", result_str);
    print_state(&bs);
    print_board(&bs);
    print_pieces(&bs);

    battle_destroy(&bs);
    log_info("");
    log_info("=== Demo complete ===");
    return 0;
}

#else /* !SMOKE_TEST: SDL3 app entry */

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdlib.h>
#include <string.h>

#include "app.h"
#include "log.h"
#include "map.h"
#include "profile.h"
#include "run.h"
#include "save.h"

/*--------------------------------------------------------------------------*\
                              CLI SUBCOMMANDS
\*--------------------------------------------------------------------------*/

/// cli_new_profile
///
/// Zero-init a Profile and persist it to disk. Used by
/// `--new-profile` to bootstrap the pref-path file from a clean
/// shell.
///
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

/// cli_dump_save
///
/// Pretty-print a save file's header + chunk index. Walks every
/// chunk and reports id, length, and offset.
///
static int cli_dump_save(const char* path) {
    return save_dump(path);
}

/// cli_debug_map
///
/// Generate a map for the given (kingdom, tier) pair and print its
/// node table. Used by `--debug-map <KINGDOM> <TIER>` to verify the
/// GDD § 6 layout.
///
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

/// cli_test_save
///
/// Round-trip a synthetic RunState through save+load and verify
/// every field survives memcmp. Exits non-zero on any mismatch.
///
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
    before.reclaim_cost_override         = 10;
    before.royal_sub_per_battle          = 3;

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
    /* profile pointer is intentionally not persisted */
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
                              CALLBACKS
\*--------------------------------------------------------------------------*/

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--new-profile") == 0) {
            int rc = cli_new_profile();
            exit(rc);
        }
        if (strcmp(argv[i], "--dump-save") == 0 && i + 1 < argc) {
            int rc = cli_dump_save(argv[i + 1]);
            exit(rc);
        }
        if (strcmp(argv[i], "--test-save") == 0) {
            int rc = cli_test_save();
            exit(rc);
        }
        if (strcmp(argv[i], "--debug-map") == 0 && i + 2 < argc) {
            int rc = cli_debug_map(argv[i + 1], argv[i + 2]);
            exit(rc);
        }
    }

    App*          app    = NULL;
    SDL_AppResult result = app_init(&app);
    *appstate = app;
    return result;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    return app_iterate((App*)appstate);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return app_event((App*)appstate, event);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    (void)result;
    app_quit((App*)appstate);
}

#endif /* SMOKE_TEST */
