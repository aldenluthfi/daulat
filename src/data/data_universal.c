//! data_universal.c
//!
//! Static templates for kingdom-agnostic content: the King piece and
//! the universal card pool drawn alongside every kingdom's cards.
//!
//! These templates are always available regardless of which kingdom
//! is being played. PIECE_KING lives in the universal registry for
//! lookup via PIECES_UNIVERSAL[0] without duplication in any kingdom.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECES
\*--------------------------------------------------------------------------*/

/// PIECES_UNIVERSAL
///
/// Single-entry array containing the King template. Both sides spawn
/// from this entry; the registry routes PIECE_KING here regardless
/// of which kingdom the rest of the army comes from.
///
const PieceTemplate PIECES_UNIVERSAL[] = {
    [PIECE_KING] = {
        .id         = PIECE_KING,
        .name       = "King",
        .kingdom    = KINGDOM_NONE,
        .tier       = TIER_KING,
        .base_value = 0,
        .move =
            {.func = mg_step_set,
             .params =
                 {
                     {.type = EARG_INT, .v.i = 1},
                     {.type = EARG_INT, .v.i = 0},
                     {.type = EARG_INT, .v.i = 1},
                     {.type = EARG_INT, .v.i = 1},
                     {.type = EARG_INT, .v.i = 0},
                     {.type = EARG_INT, .v.i = 1},
                     {.type = EARG_INT, .v.i = -1},
                     {.type = EARG_INT, .v.i = 1},
                     {.type = EARG_INT, .v.i = -1},
                     {.type = EARG_INT, .v.i = 0},
                     {.type = EARG_INT, .v.i = -1},
                     {.type = EARG_INT, .v.i = -1},
                     {.type = EARG_INT, .v.i = 0},
                     {.type = EARG_INT, .v.i = -1},
                     {.type = EARG_INT, .v.i = 1},
                     {.type = EARG_INT, .v.i = -1},
                 },
             .param_count = 16},
        .passives      = {},
        .passive_count = 0,
    },
};

const size_t PIECES_UNIVERSAL_COUNT =
    sizeof(PIECES_UNIVERSAL) / sizeof(PIECES_UNIVERSAL[0]);

/*--------------------------------------------------------------------------*\
                              CARDS
\*--------------------------------------------------------------------------*/

/// CARDS_UNIVERSAL
///
/// Dense array indexed by CardId for every cross-kingdom card in the
/// District .. Capstone range. Universal cards mix freely with each
/// kingdom's cards in the active card set.
///
const CardTemplate CARDS_UNIVERSAL[] = {
    [CARD_PAWN_STORM] =
        {
            .id         = CARD_PAWN_STORM,
            .name       = "Pawn Storm",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_DISTRICT,
            .play_cost  = 3,
            .sell_value = 1,
            .on_play =
                {{.trigger = TRIGGER_QUERY_PIECE_COST, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_REVITALIZE] =
        {
            .id         = CARD_REVITALIZE,
            .name       = "Revitalize",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_TOWN,
            .play_cost  = 2,
            .sell_value = 1,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_HOSTAGE] =
        {
            .id         = CARD_HOSTAGE,
            .name       = "Hostage",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_TOWN,
            .play_cost  = 4,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_LAST_STAND] =
        {
            .id         = CARD_LAST_STAND,
            .name       = "Last Stand",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_PROVINCE,
            .play_cost  = 3,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_SACRIFICE] =
        {
            .id         = CARD_SACRIFICE,
            .name       = "Sacrifice",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_PROVINCE,
            .play_cost  = 2,
            .sell_value = 1,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_REFORGE] =
        {
            .id         = CARD_REFORGE,
            .name       = "Reforge",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_PROVINCE,
            .play_cost  = 5,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_MERCY] =
        {
            .id         = CARD_MERCY,
            .name       = "Mercy",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_TOWN,
            .play_cost  = 1,
            .sell_value = 1,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_BLOODLETTING] =
        {
            .id         = CARD_BLOODLETTING,
            .name       = "Bloodletting",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_PROVINCE,
            .play_cost  = 3,
            .sell_value = 2,
            .on_play = {{.trigger = TRIGGER_RESOLVE_ATTACK, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_COUNTER_COUP] =
        {
            .id         = CARD_COUNTER_COUP,
            .name       = "Counter Coup",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_COUNTRY,
            .play_cost  = 4,
            .sell_value = 2,
            .on_play =
                {{.trigger = TRIGGER_RESOLVE_DEFENSE, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_SPITE] =
        {
            .id         = CARD_SPITE,
            .name       = "Spite",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_COUNTRY,
            .play_cost  = 5,
            .sell_value = 3,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CHAIN_BREAK] =
        {
            .id         = CARD_CHAIN_BREAK,
            .name       = "Chain Break",
            .kingdom    = KINGDOM_NONE,
            .tier       = TIER_TOWN,
            .play_cost  = 2,
            .sell_value = 1,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_HYDRA] = {
        .id         = CARD_HYDRA,
        .name       = "Hydra",
        .kingdom    = KINGDOM_NONE,
        .tier       = TIER_CAPSTONE,
        .play_cost  = 7,
        .sell_value = 4,
        .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_UNIVERSAL_COUNT =
    sizeof(CARDS_UNIVERSAL) / sizeof(CARDS_UNIVERSAL[0]);
