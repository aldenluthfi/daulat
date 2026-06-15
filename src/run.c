//! run.c
//!
//! Run-state implementation: small in-memory helpers plus the
//! binary save/load codec backed by `save.h`. Relics are stored
//! as ids and rehydrated to templates via the registry; the
//! Profile pointer is not persisted (it is re-attached at runtime).
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include <SDL3/SDL.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              IN-MEMORY HELPERS
\*--------------------------------------------------------------------------*/

void run_init(RunState* run, uint64_t seed) {
    memset(run, 0, sizeof(*run));
    run->run_seed = seed;
}

void run_add_relic(RunState* run, RelicId id) {
    if (run->relic_count >= MAX_RELICS_HELD)
        return;
    for (uint8_t i = 0; i < run->relic_count; i++)
        if (run->relic_ids[i] == id)
            return;
    run->relic_ids[run->relic_count++] = id;
}

void run_remove_relic(RunState* run, RelicId id) {
    for (uint8_t i = 0; i < run->relic_count; i++) {
        if (run->relic_ids[i] == id) {
            run->relic_count--;
            run->relic_ids[i] = run->relic_ids[run->relic_count];
            return;
        }
    }
}

const struct RelicTemplate* run_relic_at(const RunState* run, uint8_t index) {
    if (index >= run->relic_count)
        return NULL;
    return relic_template(run->relic_ids[index]);
}

bool run_kingdom_cleared(const RunState* run, Kingdom k) {
    if ((unsigned)k >= KINGDOM_COUNT)
        return false;
    return run->cleared_kingdoms[k];
}

/*--------------------------------------------------------------------------*\
                              PATH
\*--------------------------------------------------------------------------*/

static char RUN_PATH[1024];
static bool RUN_PATH_READY = false;

const char* run_path(void) {
    if (RUN_PATH_READY)
        return RUN_PATH;
    char* pref = SDL_GetPrefPath("aldenluthfi", "regnum");
    if (pref == NULL) {
        log_err("SDL_GetPrefPath failed: %s", SDL_GetError());
        return NULL;
    }
    SDL_snprintf(RUN_PATH, sizeof(RUN_PATH), "%srun.regsav", pref);
    SDL_free(pref);
    RUN_PATH_READY = true;
    return RUN_PATH;
}

/*--------------------------------------------------------------------------*\
                              CHUNK CODEC
\*--------------------------------------------------------------------------*/

static bool write_run_chunk(SaveWriter* w, const RunState* run) {
    if (!save_write_chunk_begin(w, CHUNK_RUN_META))
        return false;
    if (!save_write_u64(w, run->run_seed))
        return false;
    if (!save_write_u8(w, (uint8_t)run->current_kingdom))
        return false;
    if (!save_write_u8(w, (uint8_t)run->current_map_tier))
        return false;

    if (!save_write_bytes(w, &run->current_map, sizeof(run->current_map)))
        return false;

    if (!save_write_u8(w, run->relic_count))
        return false;
    for (uint8_t i = 0; i < run->relic_count; i++)
        if (!save_write_u16(w, (uint16_t)run->relic_ids[i]))
            return false;

    if (!save_write_bytes(w, run->chain_levels, sizeof(run->chain_levels)))
        return false;
    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        if (!save_write_bool(w, run->subjugated[i]))
            return false;
    if (!save_write_u8(w, run->liberation_respawn_counter))
        return false;

    if (!save_write_u16(w, run->vorath_counter))
        return false;
    if (!save_write_u8(w, run->vorath_pressure))
        return false;

    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        for (size_t t = 0; t < TIER_PER_KINGDOM; t++)
            if (!save_write_bool(w, run->cleared_maps[i][t]))
                return false;
    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        if (!save_write_bool(w, run->cleared_kingdoms[i]))
            return false;
    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        if (!save_write_bool(w, run->mastery_disqualified[i]))
            return false;
    if (!save_write_bytes(w, run->mastery_l3, sizeof(run->mastery_l3)))
        return false;

    if (!save_write_u64(w, run->revealed_recipes))
        return false;
    if (!save_write_u64(w, run->forbidden_recipes))
        return false;

    if (!save_write_u32(w, run->flags))
        return false;
    if (!save_write_bytes(
            w, run->chain_silver_pending, sizeof(run->chain_silver_pending)
        ))
        return false;

    return save_write_chunk_end(w);
}

static bool read_run_chunk(SaveReader* r, RunState* run) {
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    if (!save_read_u64(r, &run->run_seed))
        return false;
    if (!save_read_u8(r, &u8))
        return false;
    run->current_kingdom = (Kingdom)u8;
    if (!save_read_u8(r, &u8))
        return false;
    run->current_map_tier = (Tier)u8;

    if (!save_read_bytes(r, &run->current_map, sizeof(run->current_map)))
        return false;

    if (!save_read_u8(r, &run->relic_count))
        return false;
    if (run->relic_count > MAX_RELICS_HELD) {
        log_err("run_load: relic_count %u exceeds capacity", run->relic_count);
        return false;
    }
    for (uint8_t i = 0; i < run->relic_count; i++) {
        if (!save_read_u16(r, &u16))
            return false;
        run->relic_ids[i] = (RelicId)u16;
    }

    if (!save_read_bytes(r, run->chain_levels, sizeof(run->chain_levels)))
        return false;
    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        if (!save_read_bool(r, &run->subjugated[i]))
            return false;
    if (!save_read_u8(r, &run->liberation_respawn_counter))
        return false;

    if (!save_read_u16(r, &run->vorath_counter))
        return false;
    if (!save_read_u8(r, &run->vorath_pressure))
        return false;

    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        for (size_t t = 0; t < TIER_PER_KINGDOM; t++)
            if (!save_read_bool(r, &run->cleared_maps[i][t]))
                return false;
    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        if (!save_read_bool(r, &run->cleared_kingdoms[i]))
            return false;
    for (size_t i = 0; i < KINGDOM_COUNT; i++)
        if (!save_read_bool(r, &run->mastery_disqualified[i]))
            return false;
    if (!save_read_bytes(r, run->mastery_l3, sizeof(run->mastery_l3)))
        return false;

    if (!save_read_u64(r, &run->revealed_recipes))
        return false;
    if (!save_read_u64(r, &run->forbidden_recipes))
        return false;

    if (!save_read_u32(r, &run->flags))
        return false;
    if (!save_read_bytes(
            r, run->chain_silver_pending, sizeof(run->chain_silver_pending)
        ))
        return false;

    (void)u32;
    (void)u64;
    return true;
}

/*--------------------------------------------------------------------------*\
                              PERSISTENCE
\*--------------------------------------------------------------------------*/

bool run_load(RunState* run) {
    memset(run, 0, sizeof(*run));
    const char* path = run_path();
    if (path == NULL)
        return false;
    SaveReader r;
    if (!save_reader_open(&r, path))
        return false;
    for (uint32_t i = 0; i < r.chunk_count; i++) {
        SaveChunkId id;
        uint32_t    len;
        if (!save_read_chunk_header(&r, &id, &len))
            return false;
        if (id == CHUNK_RUN_META) {
            if (!read_run_chunk(&r, run))
                return false;
        } else {
            if (!save_skip(&r, len))
                return false;
        }
    }
    return true;
}

bool run_save(const RunState* run) {
    const char* path = run_path();
    if (path == NULL)
        return false;
    SaveWriter w;
    save_writer_init(&w);
    if (!save_write_header(&w, 1u))
        return false;
    if (!write_run_chunk(&w, run))
        return false;
    return save_writer_flush(&w, path);
}

void run_delete(void) {
    const char* path = run_path();
    if (path == NULL)
        return;
    if (!SDL_RemovePath(path))
        log_warn("run_delete: SDL_RemovePath failed: %s", SDL_GetError());
}

/*--------------------------------------------------------------------------*\
                              END-OF-RUN
\*--------------------------------------------------------------------------*/

void run_finalize(RunState* run, RunEnd outcome) {
    Profile* profile = run->profile;
    if (profile == NULL) {
        log_warn("run_finalize: no profile attached; skipping pass");
        run_delete();
        return;
    }

    if (outcome == RUN_END_VORATH_WIN) {
        profile->vorath_defeat_count++;
        profile->total_wins++;
        if (profile->prestige_tier < 1)
            profile->prestige_tier = 1;
        for (size_t k = 0; k < KINGDOM_COUNT; k++) {
            if (run->mastery_disqualified[k])
                continue;
            if (profile->mastery_levels[k] < 3)
                profile->mastery_levels[k]++;
        }
    } else {
        profile->total_losses++;
    }

    profile_save(profile);
    run_delete();
}
