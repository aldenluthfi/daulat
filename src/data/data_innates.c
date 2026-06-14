//! data_innates.c
//!
//! Static templates for the five kingdom innate powers (Bulwark,
//! Reclaim, Double Time, Royal Substitution, Conqueror's Reward).
//!
//! Each innate is gated on the player entering that kingdom's
//! Province map. Once active, its Effect[] feeds into the EffectBus
//! and shapes resolve, movement, or economy for the rest of the run.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              INNATES
\*--------------------------------------------------------------------------*/

/// INNATES
///
/// Array of innate templates keyed by Kingdom. Lookup is linear
/// because the array has five entries; the registry walks it once
/// at activation time.
///
const InnateTemplate INNATES[] = {
    {
        .kingdom = KINGDOM_LONGWEI,
        .name    = "Bulwark",
        .effects =
            {
                {.trigger = TRIGGER_RESOLVE_DEFENSE, .apply = eff_todo},
            },
        .effect_count = 1,
    },
    {
        .kingdom = KINGDOM_HARUSHIMA,
        .name    = "Reclaim",
        .effects =
            {
                {.trigger = TRIGGER_TURN_END, .apply = eff_todo},
            },
        .effect_count = 1,
    },
    {
        .kingdom = KINGDOM_KEWARANI,
        .name    = "Double Time",
        .effects =
            {
                {.trigger = TRIGGER_TURN_START, .apply = eff_todo},
            },
        .effect_count = 1,
    },
    {
        .kingdom = KINGDOM_ZARQAN,
        .name    = "Royal Substitution",
        .effects =
            {
                {.trigger = TRIGGER_PIECE_ENTERED_ENEMY_TERR,
                 .apply   = eff_todo},
            },
        .effect_count = 1,
    },
    {
        .kingdom = KINGDOM_CAELAN,
        .name    = "Conqueror's Reward",
        .effects =
            {
                {.trigger = TRIGGER_PIECE_FLIPPED, .apply = eff_todo},
            },
        .effect_count = 1,
    },
};

const size_t INNATES_COUNT = sizeof(INNATES) / sizeof(INNATES[0]);
