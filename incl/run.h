//! run.h
//!
//! Run state skeleton for the Regnum battle engine.
//! Holds relics, chains, mastery, figurehead powers, synergies.
//! Full campaign loop is out of scope — this is just the container.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef RUN_H
#define RUN_H

#include "types.h"

/*--------------------------------------------------------------------------*\
                              RUN STATE
\*--------------------------------------------------------------------------*/

typedef struct {
    /* Relics */
    const RelicTemplate* relics[MAX_RELICS_HELD];
    uint8_t              relic_count;

    /* Penalty chain */
    uint8_t chain_level;

    /* Figurehead power */
    const FigureheadPower* figurehead;

    /* Innate ability */
    const InnateTemplate* innate;

    /* Mastery hooks */
    const MasteryHook* mastery_hooks[8];
    uint8_t            mastery_hook_count;

    /* Synergies (cleared kingdoms) */
    bool cleared_kingdoms[5];

    /* Player stats */
    int gold;
    int wins;
    int losses;
} RunState;

/*--------------------------------------------------------------------------*\
                              RUN API
\*--------------------------------------------------------------------------*/

/// Initialize a new run.
void run_init(RunState* run);

/// Add a relic to the run.
void run_add_relic(RunState* run, const RelicTemplate* rel);

/// Remove a relic from the run.
void run_remove_relic(RunState* run, uint16_t relic_id);

/// Apply a penalty chain level.
void run_apply_chain(RunState* run, uint8_t level);

/// Check if a kingdom has been cleared.
bool run_kingdom_cleared(const RunState* run, Kingdom k);

#endif /* RUN_H */