//!
//! Run-state skeleton for the Regnum battle engine.
//! Full campaign loop is out of scope; this is a minimal stub for battle init.
//! Holds relics, chains, mastery, figurehead powers, and synergies.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"
#include <string.h>

void run_init(RunState* run) {
    memset(run, 0, sizeof(*run));
}

void run_add_relic(RunState* run, const RelicTemplate* rel) {
    if (run->relic_count >= MAX_RELICS_HELD)
        return;
    run->relics[run->relic_count++] = rel;
}

void run_remove_relic(RunState* run, uint16_t relic_id) {
    for (uint8_t i = 0; i < run->relic_count; i++) {
        if (run->relics[i] != NULL && run->relics[i]->id == relic_id) {
            run->relic_count--;
            run->relics[i] = run->relics[run->relic_count];
            return;
        }
    }
}

void run_apply_chain(RunState* run, uint8_t level) {
    run->chain_level = level;
}

bool run_kingdom_cleared(const RunState* run, Kingdom k) {
    if (k >= 5)
        return false;
    return run->cleared_kingdoms[k];
}