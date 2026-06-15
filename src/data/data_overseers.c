//! data_overseers.c
//!
//! Static templates for the six Overseer fights: one for each kingdom
//! plus Vorath, the Unbroken. An Overseer is the final node of a
//! kingdom's Country map; defeating it unlocks that kingdom's
//! capstone piece and unchains its figurehead.
//!
//! Each entry's Effect[] holds the bespoke mechanic (e.g. Iron
//! Strategist's split-damage rule). Stubs route to eff_todo until the
//! battle-side hooks are implemented.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              OVERSEERS
\*--------------------------------------------------------------------------*/

/// OVERSEERS
///
/// Dense Overseer table indexed by OverseerId. The five kingdom
/// Overseers sit first; Vorath occupies the final slot and the
/// registry exposes him through the same overseer_template() path.
///
const Overseer OVERSEERS[] = {
    {
        .id          = OVERSEER_IRON_STRATEGIST,
        .name        = "Iron Strategist",
        .description = "An unyielding tactician.",
        .effects[0] =
            {.trigger = TRIGGER_BATTLE_START,
             EFF(OVERSEER_IRON_STRATEGIST, overseer_iron_strategist)},
        .effect_count = 1,
    },
    {
        .id          = OVERSEER_ETERNAL_RECURSION,
        .name        = "Eternal Recursion",
        .description = "An endless loop of conflict.",
        .effects[0] =
            {.trigger = TRIGGER_PIECE_FLIPPED,
             EFF(OVERSEER_ETERNAL_RECURSION, overseer_eternal_recursion)},
        .effect_count = 1,
    },
    {
        .id          = OVERSEER_CARAVAN_OF_CONQUEST,
        .name        = "Caravan of Conquest",
        .description = "A traveling army of conquest.",
        .effects[0] =
            {.trigger = TRIGGER_TURN_END,
             EFF(OVERSEER_CARAVAN_OF_CONQUEST, overseer_caravan_of_conquest)},
        .effect_count = 1,
    },
    {
        .id          = OVERSEER_MANY_FACED_KING,
        .name        = "Many-Faced King",
        .description = "A ruler of many masks.",
        .effects[0] =
            {.trigger = TRIGGER_BATTLE_START,
             EFF(OVERSEER_MANY_FACED_KING, overseer_many_faced_king)},
        .effect_count = 1,
    },
    {
        .id          = OVERSEER_CROWNED_HERETIC,
        .name        = "Crowned Heretic",
        .description = "A defiant monarch.",
        .effects[0] =
            {.trigger = TRIGGER_BATTLE_START,
             EFF(OVERSEER_CROWNED_HERETIC, overseer_crowned_heretic)},
        .effect_count = 1,
    },
    {
        .id          = OVERSEER_VORATH,
        .name        = "Vorath",
        .description = "The Grand King.",
        .effects[0] =
            {.trigger = TRIGGER_BATTLE_START,
             EFF(VORATH_MEMORY_APPLY, vorath_memory_apply)},
        .effect_count = 1,
    },
};

const size_t OVERSEERS_COUNT = sizeof(OVERSEERS) / sizeof(OVERSEERS[0]);
