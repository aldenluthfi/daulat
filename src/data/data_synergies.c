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
        .name        = "Pao Vanguard",
        .description = "Pao attacks deal +10 damage in Harushima.",
        .bonus =
            {.trigger = TRIGGER_RESOLVE_ATTACK, EFF(SYN_LONGWEI, syn_longwei)},
    },
    {
        .cleared     = KINGDOM_HARUSHIMA,
        .name        = "Reclaimed Honor",
        .description = "Caelan card plays draw 1 in Caelan battles.",
        .bonus =
            {.trigger = TRIGGER_CARD_PLAYED, EFF(SYN_HARUSHIMA, syn_harushima)},
    },
    {
        .cleared     = KINGDOM_KEWARANI,
        .name        = "Caravan Discount",
        .description = "Kewarani pieces cost 10 cp less in Zarqan.",
        .bonus =
            {.trigger = TRIGGER_QUERY_PIECE_COST,
             EFF(SYN_KEWARANI, syn_kewarani)},
    },
    {
        .cleared     = KINGDOM_ZARQAN,
        .name        = "Imperial Tribute",
        .description = "Ziraafa and Talliya gain +5 value in Longwei.",
        .bonus = {.trigger = TRIGGER_BATTLE_START, EFF(SYN_ZARQAN, syn_zarqan)},
    },
    {
        .cleared     = KINGDOM_CAELAN,
        .name        = "Golden Sultanate",
        .description = "Sultan's Gold yields +10 cp in Kewarani.",
        .bonus = {.trigger = TRIGGER_CARD_PLAYED, EFF(SYN_CAELAN, syn_caelan)},
    },
};

const size_t SYNERGIES_COUNT = sizeof(SYNERGIES) / sizeof(SYNERGIES[0]);
