//! data_archetypes.c
//!
//! Static templates for the five AI archetypes — one per kingdom —
//! that drive enemy decision-making. Each archetype carries a primary
//! AIWeights profile and a fallback profile activated when the AI's
//! meter is sufficiently behind.
//!
//! The pick function pointer stays NULL for default scoring; specific
//! kingdoms override it once their bespoke decision logic (Reclaimer,
//! Trickster, etc.) is implemented in src/ai.c.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              ARCHETYPES
\*--------------------------------------------------------------------------*/

/// ARCHETYPES
///
/// Linear archetype table. registry.ai_archetype() walks the array
/// and returns the entry whose .kingdom matches; five entries (one
/// per kingdom) does not warrant indexing.
///
const AIArchetype ARCHETYPES[] = {
    {
        .name    = "Siege Engineer",
        .kingdom = KINGDOM_LONGWEI,
        .pick    = NULL,
        .primary =
            {
                .value_diff_w      = 12,
                .territory_w       = 6,
                .aggression_w      = 4,
                .sell_threshold    = 4,
                .save_threshold    = 6,
                .max_piece_cost    = 8,
                .combo_chain_bonus = 5,
                .reclaim_priority  = 0,
            },
        .fallback =
            {
                .value_diff_w      = 6,
                .territory_w       = 9,
                .aggression_w      = 2,
                .sell_threshold    = 3,
                .save_threshold    = 4,
                .max_piece_cost    = 6,
                .combo_chain_bonus = 3,
                .reclaim_priority  = 0,
            },
        .fallback_meter_diff = 5,
    },
    {
        .name    = "Reclaimer",
        .kingdom = KINGDOM_HARUSHIMA,
        .pick    = NULL,
        .primary =
            {
                .value_diff_w      = 8,
                .territory_w       = 4,
                .aggression_w      = 2,
                .sell_threshold    = 2,
                .save_threshold    = 8,
                .max_piece_cost    = 5,
                .combo_chain_bonus = 7,
                .reclaim_priority  = 10,
            },
        .fallback =
            {
                .value_diff_w      = 4,
                .territory_w       = 6,
                .aggression_w      = 1,
                .sell_threshold    = 2,
                .save_threshold    = 5,
                .max_piece_cost    = 4,
                .combo_chain_bonus = 4,
                .reclaim_priority  = 5,
            },
        .fallback_meter_diff = 3,
    },
    {
        .name    = "The Tide",
        .kingdom = KINGDOM_KEWARANI,
        .pick    = NULL,
        .primary =
            {
                .value_diff_w      = 6,
                .territory_w       = 3,
                .aggression_w      = 8,
                .sell_threshold    = 5,
                .save_threshold    = 5,
                .max_piece_cost    = 7,
                .combo_chain_bonus = 4,
                .reclaim_priority  = 0,
            },
        .fallback =
            {
                .value_diff_w      = 3,
                .territory_w       = 5,
                .aggression_w      = 4,
                .sell_threshold    = 3,
                .save_threshold    = 3,
                .max_piece_cost    = 5,
                .combo_chain_bonus = 2,
                .reclaim_priority  = 0,
            },
        .fallback_meter_diff = 4,
    },
    {
        .name    = "Trickster",
        .kingdom = KINGDOM_ZARQAN,
        .pick    = NULL,
        .primary =
            {
                .value_diff_w      = 5,
                .territory_w       = 2,
                .aggression_w      = 12,
                .sell_threshold    = 6,
                .save_threshold    = 3,
                .max_piece_cost    = 9,
                .combo_chain_bonus = 3,
                .reclaim_priority  = 0,
            },
        .fallback =
            {
                .value_diff_w      = 2,
                .territory_w       = 4,
                .aggression_w      = 6,
                .sell_threshold    = 4,
                .save_threshold    = 2,
                .max_piece_cost    = 7,
                .combo_chain_bonus = 2,
                .reclaim_priority  = 0,
            },
        .fallback_meter_diff = 6,
    },
    {
        .name    = "The Hammer",
        .kingdom = KINGDOM_CAELAN,
        .pick    = NULL,
        .primary =
            {
                .value_diff_w      = 9,
                .territory_w       = 8,
                .aggression_w      = 2,
                .sell_threshold    = 3,
                .save_threshold    = 7,
                .max_piece_cost    = 7,
                .combo_chain_bonus = 6,
                .reclaim_priority  = 0,
            },
        .fallback =
            {
                .value_diff_w      = 5,
                .territory_w       = 10,
                .aggression_w      = 1,
                .sell_threshold    = 2,
                .save_threshold    = 5,
                .max_piece_cost    = 5,
                .combo_chain_bonus = 4,
                .reclaim_priority  = 0,
            },
        .fallback_meter_diff = 4,
    },
};

const size_t ARCHETYPES_COUNT = sizeof(ARCHETYPES) / sizeof(ARCHETYPES[0]);
