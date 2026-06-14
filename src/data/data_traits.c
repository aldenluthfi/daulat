//! data_traits.c
//!
//! Static templates for the ten board traits (two per kingdom) that
//! reshape combat by editing the playable area or modifying movement
//! globally. One trait is drawn alongside the battle modifier on
//! roughly half of each kingdom's nodes.
//!
//! As with battle modifiers, .type is retained next to .id even
//! though they currently match; category-wide effects will read it
//! once the relic synergies that key off kingdom trait categories
//! land.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              TRAITS
\*--------------------------------------------------------------------------*/

/// TRAITS
///
/// Dense board-trait table indexed by TraitId. Iteration order
/// follows the GDD's per-kingdom listing so balancing edits stay
/// easy to scan.
///
const BoardTrait TRAITS[] = {
    {
        .id           = TRAIT_RIVER_CROSSING,
        .name         = "River Crossing",
        .description  = "A river blocks the center.",
        .type         = TRAIT_RIVER_CROSSING,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_PALACE,
        .name         = "Palace",
        .description  = "A grand palace.",
        .type         = TRAIT_PALACE,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_FOG_COAST,
        .name         = "Fog Coast",
        .description  = "Foggy coastal terrain.",
        .type         = TRAIT_FOG_COAST,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_ISLAND_CHAIN,
        .name         = "Island Chain",
        .description  = "Islands across the board.",
        .type         = TRAIT_ISLAND_CHAIN,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_TRADE_ROUTE,
        .name         = "Trade Route",
        .description  = "A busy trade route.",
        .type         = TRAIT_TRADE_ROUTE,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_CONTESTED_MARKET,
        .name         = "Contested Market",
        .description  = "A contested marketplace.",
        .type         = TRAIT_CONTESTED_MARKET,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_SANDSTORM,
        .name         = "Sandstorm",
        .description  = "A desert sandstorm.",
        .type         = TRAIT_SANDSTORM,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_MIRAGE,
        .name         = "Mirage",
        .description  = "An illusory mirage.",
        .type         = TRAIT_MIRAGE,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_CASTLE_CORNERS,
        .name         = "Castle Corners",
        .description  = "Fortified castle corners.",
        .type         = TRAIT_CASTLE_CORNERS,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
    {
        .id           = TRAIT_SIEGE_TRENCH,
        .name         = "Siege Trench",
        .description  = "Trenches from a siege.",
        .type         = TRAIT_SIEGE_TRENCH,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, EFF(TODO, todo)},
        .effect_count = 1,
    },
};

const size_t TRAITS_COUNT = sizeof(TRAITS) / sizeof(TRAITS[0]);
