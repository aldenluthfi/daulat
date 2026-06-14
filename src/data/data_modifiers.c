//! data_modifiers.c
//!
//! Static templates for the twelve battle modifiers drawn per battle
//! and revealed before entry. Modifiers are grouped into Economy,
//! Meter, Cards, and Board categories per the GDD; each entry shapes
//! that battle's economy, resolve, draw, or board behaviour.
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
/// Dense modifier table indexed by ModifierId. Three entries per
/// category (Economy / Meter / Cards / Board) carry the GDD-named
/// per-battle rule modulators.
///
const Modifier MODIFIERS[] = {
    /* Economy */
    {
        .id           = MODIFIER_LEAN_TIMES,
        .name         = "Lean Times",
        .description  = "Reduced starting centipawns.",
        .type         = MODIFIER_LEAN_TIMES,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_WINDFALL,
        .name         = "Windfall",
        .description  = "Both sides gain bonus centipawns at start.",
        .type         = MODIFIER_WINDFALL,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_OPEN_MARKET,
        .name         = "Open Market",
        .description  = "Foreign-kingdom markup reduced for the battle.",
        .type         = MODIFIER_OPEN_MARKET,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    /* Meter */
    {
        .id           = MODIFIER_GLASS_CANNON,
        .name         = "Glass Cannon",
        .description  = "Meters halved on both sides.",
        .type         = MODIFIER_GLASS_CANNON,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_BLOODBATH,
        .name         = "Bloodbath",
        .description  = "All damage dealt is amplified.",
        .type         = MODIFIER_BLOODBATH,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_IRON_WILL,
        .name         = "Iron Will",
        .description  = "All damage taken is reduced.",
        .type         = MODIFIER_IRON_WILL,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    /* Cards */
    {
        .id           = MODIFIER_RICH_HAND,
        .name         = "Rich Hand",
        .description  = "Draw extra cards each turn.",
        .type         = MODIFIER_RICH_HAND,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_SPARSE_HAND,
        .name         = "Sparse Hand",
        .description  = "Draw fewer cards each turn.",
        .type         = MODIFIER_SPARSE_HAND,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_KINGDOM_PURITY,
        .name         = "Kingdom Purity",
        .description  = "Only same-kingdom cards may be drawn.",
        .type         = MODIFIER_KINGDOM_PURITY,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    /* Board */
    {
        .id           = MODIFIER_FOG_OF_WAR,
        .name         = "Fog of War",
        .description  = "Enemy piece values hidden.",
        .type         = MODIFIER_FOG_OF_WAR,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_DENSE_TERRAIN,
        .name         = "Dense Terrain",
        .description  = "Sliders blocked by extra terrain squares.",
        .type         = MODIFIER_DENSE_TERRAIN,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id           = MODIFIER_EXTENDED_FRONT,
        .name         = "Extended Front",
        .description  = "Board widened; territory baseline shifts.",
        .type         = MODIFIER_EXTENDED_FRONT,
        .effects[0]   = {.trigger = TRIGGER_BATTLE_START, .apply = eff_todo},
        .effect_count = 1,
    },
};

const size_t MODIFIERS_COUNT = sizeof(MODIFIERS) / sizeof(MODIFIERS[0]);
