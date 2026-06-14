//! data_modifiers.c
//!
//! Static templates for the eight battle modifiers (Economy, Meter,
//! Cards, Board pairs of boost/drain). One modifier is rolled per
//! battle and revealed before entry.
//!
//! The .type field is intentionally retained alongside .id even
//! though they currently match; the runtime distinguishes "this
//! specific modifier" from "this category of modifier" once
//! category-wide effects (e.g. relic synergies) come online.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              MODIFIERS
\*--------------------------------------------------------------------------*/

/// MODIFIERS
///
/// Dense modifier table indexed by ModifierId. Entries are paired by
/// category (Boost/Drain) so registry.modifier_template() returns
/// the exact entry by id, and category-aware listings can iterate
/// in pairs.
///
const Modifier MODIFIERS[] = {
    {
        .id = MODIFIER_ECONOMY_BOOST,
        .name = "Economy Boost",
        .description = "Boosts economy.",
        .type = MODIFIER_ECONOMY_BOOST,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_ECONOMY_DRAIN,
        .name = "Economy Drain",
        .description = "Drains economy.",
        .type = MODIFIER_ECONOMY_DRAIN,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_METER_BOOST,
        .name = "Meter Boost",
        .description = "Boosts meter.",
        .type = MODIFIER_METER_BOOST,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_METER_DRAIN,
        .name = "Meter Drain",
        .description = "Drains meter.",
        .type = MODIFIER_METER_DRAIN,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_CARDS_BOOST,
        .name = "Cards Boost",
        .description = "Boosts cards.",
        .type = MODIFIER_CARDS_BOOST,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_CARDS_DRAIN,
        .name = "Cards Drain",
        .description = "Drains cards.",
        .type = MODIFIER_CARDS_DRAIN,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_BOARD_BOOST,
        .name = "Board Boost",
        .description = "Boosts board.",
        .type = MODIFIER_BOARD_BOOST,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MODIFIER_BOARD_DRAIN,
        .name = "Board Drain",
        .description = "Drains board.",
        .type = MODIFIER_BOARD_DRAIN,
        .effects[0] = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
};

const size_t MODIFIERS_COUNT = sizeof(MODIFIERS) / sizeof(MODIFIERS[0]);
