//! data_synergies.c
//!
//! Static templates for the five inter-kingdom synergy bonuses
//! granted when a kingdom's Overseer is cleared. Each synergy keeps
//! a single passive effect that activates whenever the player next
//! fights in the lore-adjacent kingdom listed in the GDD.
//!
//! Synergies stack across the run as more Overseers fall, so the
//! lookup walks the array each battle to register everything that
//! applies.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              SYNERGIES
\*--------------------------------------------------------------------------*/

/// SYNERGIES
///
/// Array of synergy templates keyed by .cleared (the kingdom whose
/// Overseer was beaten). registry.synergy_template() does a linear
/// match — five entries is well below the threshold where indexing
/// would be worthwhile.
///
const Synergy SYNERGIES[] = {
    {
        .cleared     = KINGDOM_LONGWEI,
        .name        = "Longwei Synergy",
        .description = "Longwei kingdom bonus.",
        .bonus       = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
    },
    {
        .cleared     = KINGDOM_HARUSHIMA,
        .name        = "Harushima Synergy",
        .description = "Harushima kingdom bonus.",
        .bonus       = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
    },
    {
        .cleared     = KINGDOM_KEWARANI,
        .name        = "Kewarani Synergy",
        .description = "Kewarani kingdom bonus.",
        .bonus       = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
    },
    {
        .cleared     = KINGDOM_ZARQAN,
        .name        = "Zarqan Synergy",
        .description = "Zarqan kingdom bonus.",
        .bonus       = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
    },
    {
        .cleared     = KINGDOM_CAELAN,
        .name        = "Caelan Synergy",
        .description = "Caelan kingdom bonus.",
        .bonus       = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
    },
};

const size_t SYNERGIES_COUNT = sizeof(SYNERGIES) / sizeof(SYNERGIES[0]);
