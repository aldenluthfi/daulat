//! run.h
//!
//! Run state for the Regnum campaign: identity, relics held, chain
//! progression, vorath escalators, progression flags, and the live
//! pointer to the active Profile. Relics are stored as ids so the
//! struct serializes cleanly; pointers are reseated on demand via
//! the registry.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef RUN_H
#define RUN_H

#include "defs.h"
#include "map.h"

struct Profile;
struct RelicTemplate;

/*--------------------------------------------------------------------------*\
                              RUN STATE
\*--------------------------------------------------------------------------*/

/// RunState
///
/// Per-run game state. Carries everything that needs to round-trip
/// through `run.regsav` plus the live pointer to the player's
/// Profile (re-attached at runtime; not persisted).
///
typedef struct RunState {
    /* Identity */
    uint64_t run_seed;
    Kingdom  current_kingdom;
    Tier     current_map_tier;

    /* Active map */
    MapState current_map;

    /* Relics (ids for serialization; pointers via registry) */
    RelicId relic_ids[MAX_RELICS_HELD];
    uint8_t relic_count;

    /* Penalty chains, per kingdom */
    uint8_t chain_levels[KINGDOM_COUNT];
    bool    subjugated[KINGDOM_COUNT];
    uint8_t liberation_respawn_counter;

    /* Vorath escalators */
    uint16_t vorath_counter;
    uint8_t  vorath_pressure;

    /* Progression flags */
    bool    cleared_maps[KINGDOM_COUNT][TIER_PER_KINGDOM];
    bool    cleared_kingdoms[KINGDOM_COUNT];
    uint8_t mastery_l3[KINGDOM_COUNT];

    /* Recipe state */
    uint64_t revealed_recipes;
    uint64_t forbidden_recipes;

    /* Run-wide flags & overrides */
    uint32_t flags;
    int      reclaim_cost_override;
    int      royal_sub_per_battle;

    /* Live pointer (not persisted) */
    struct Profile* profile;
} RunState;

/*--------------------------------------------------------------------------*\
                              FLAG BITS
\*--------------------------------------------------------------------------*/

#define RUN_FOREIGN_MARKUP_OFF   (1u << 0)
#define RUN_DOUBLE_ARCHIVE       (1u << 1)
#define RUN_VISION_ENEMY_VALUES  (1u << 2)
#define RUN_PREREVEAL_MODIFIER   (1u << 3)

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// run_init
///
/// Zero a RunState and seed its RNG identity from `seed`.
///
/// Params:
/// - RunState* run -> run to initialize
/// - uint64_t  seed -> deterministic run seed
///
void run_init(RunState* run, uint64_t seed);

/// run_add_relic
///
/// Append a relic id to the inventory; no-op if at capacity or if
/// the relic is already held.
///
/// Params:
/// - RunState* run -> run to modify
/// - RelicId   id  -> id to add
///
void run_add_relic(RunState* run, RelicId id);

/// run_remove_relic
///
/// Remove the named relic id from the inventory; no-op if absent.
///
/// Params:
/// - RunState* run -> run to modify
/// - RelicId   id  -> id to remove
///
void run_remove_relic(RunState* run, RelicId id);

/// run_relic_at
///
/// Resolve the held relic template at slot `idx` via the registry.
/// Returns NULL on out-of-range.
///
/// Params:
/// - const RunState* run -> run to query
/// - uint8_t          idx -> slot index
///
/// Return:
/// const RelicTemplate* -> template pointer or NULL
///
const struct RelicTemplate* run_relic_at(const RunState* run, uint8_t idx);

/// run_kingdom_cleared
///
/// Has this run cleared the given kingdom's Country map?
///
/// Params:
/// - const RunState* run -> run to query
/// - Kingdom         k   -> kingdom id
///
/// Return:
/// bool -> true if cleared
///
bool run_kingdom_cleared(const RunState* run, Kingdom k);

/*--------------------------------------------------------------------------*\
                              PERSISTENCE
\*--------------------------------------------------------------------------*/

/// run_path
///
/// Path for `run.regsav` under SDL's pref path.
///
/// Return:
/// const char* -> absolute path, or NULL on SDL error
///
const char* run_path(void);

/// run_load
///
/// Read the active run from disk. Returns false (and leaves `*run`
/// zero-initialized) when the file is missing or corrupt.
///
/// Params:
/// - RunState* run -> destination
///
/// Return:
/// bool -> true on success
///
bool run_load(RunState* run);

/// run_save
///
/// Persist the active run to disk atomically.
///
/// Params:
/// - const RunState* run -> source
///
/// Return:
/// bool -> true on success
///
bool run_save(const RunState* run);

/// run_delete
///
/// Remove `run.regsav` from disk (used at run-end).
///
void run_delete(void);

#endif /* RUN_H */
