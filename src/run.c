//! run.c
//!
//! Run-state skeleton for the Regnum battle engine.
//! Full campaign loop is out of scope; this is a minimal stub for battle init.
//! Holds relics, chains, mastery, figurehead powers, and synergies.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"
#include <string.h>

/// run_init
///
/// Zero-initialize a RunState for a new campaign.
///
/// Params:
/// - RunState* run -> run state to initialize
///
void run_init(RunState* run) {
    memset(run, 0, sizeof(*run));
}

/// run_add_relic
///
/// Add a relic to the run's inventory.
///
/// Params:
/// - RunState* run -> run state to modify
/// - const RelicTemplate* rel -> relic to add
///
void run_add_relic(RunState* run, const RelicTemplate* rel) {
    if (run->relic_count >= MAX_RELICS_HELD)
        return;
    run->relics[run->relic_count++] = rel;
}

/// run_remove_relic
///
/// Remove a relic from the run by its id.
///
/// Params:
/// - RunState* run -> run state to modify
/// - uint16_t relic_id -> id of relic to remove
///
void run_remove_relic(RunState* run, uint16_t relic_id) {
    for (uint8_t i = 0; i < run->relic_count; i++) {
        if (run->relics[i] != NULL && run->relics[i]->id == relic_id) {
            run->relic_count--;
            run->relics[i] = run->relics[run->relic_count];
            return;
        }
    }
}

/// run_apply_chain
///
/// Set the penalty chain level for the run.
///
/// Params:
/// - RunState* run -> run state to modify
/// - uint8_t level -> chain level (0=Bronze, 1=Silver, 2=Gold)
///
void run_apply_chain(RunState* run, uint8_t level) {
    run->chain_level = level;
}

/// run_kingdom_cleared
///
/// Check whether a kingdom has been cleared in this run.
///
/// Params:
/// - const RunState* run -> run state to query
/// - Kingdom k -> kingdom to check
///
/// Return:
/// bool -> true if the kingdom has been cleared
///
bool run_kingdom_cleared(const RunState* run, Kingdom k) {
    if (k >= 5)
        return false;
    return run->cleared_kingdoms[k];
}