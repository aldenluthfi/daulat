//! data_zarqan.c
//!
//! Static templates for the Zarqan Sultanate kingdom. Holds both the
//! piece roster (base pieces plus combo results) and the card roster
//! as immutable arrays consumed by the registry.
//!
//! Zarqan follows the Tamerlane / Shatranj tradition: the player
//! sits at the bottom (y=0) and pieces face forward. The Ziraafa
//! family's bespoke movement lives in mg_zarqan.c. The Kyosha+Wazir
//! combo result (value 50) is exclusively obtainable through a
//! Zarqan recipe and lives here alongside other combo results.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECES
\*--------------------------------------------------------------------------*/

/// PIECES_ZARQAN
///
/// Dense array indexed by PieceId for every Zarqan piece. Ziraafa
/// uses a custom threat MoveGen (war-elephant adjacency) distinct
/// from its move pattern; the registry routes ids inside the
/// PIECE_WAZIR .. PIECE_WAR_ELEPHANT range here.
///
const PieceTemplate
    PIECES_ZARQAN[] =
        {
            [PIECE_WAZIR] =
                {
                    .id = PIECE_WAZIR,
                    .name = "Wazir",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_DISTRICT,
                    .base_value = 15,
                    .move = {.func = mg_step_set,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = -1},
                            },
                        .param_count = 8},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_JAMAL] =
                {
                    .id = PIECE_JAMAL,
                    .name = "Jamal",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_DISTRICT,
                    .base_value = 20,
                    .move =
                        {.func = mg_leap_set,
                            .params =
                                {
                                    {.type = EARG_INT, .v.i = 1},
                                    {.type = EARG_INT, .v.i = 3},
                                    {.type = EARG_INT, .v.i = -1},
                                    {.type = EARG_INT, .v.i = 3},
                                    {.type = EARG_INT, .v.i = 1},
                                    {.type = EARG_INT, .v.i = -3},
                                    {.type = EARG_INT, .v.i = -1},
                                    {.type = EARG_INT, .v.i = -3},
                                    {.type = EARG_INT, .v.i = 3},
                                    {.type = EARG_INT, .v.i = 1},
                                    {.type = EARG_INT, .v.i = -3},
                                    {.type = EARG_INT, .v.i = 1},
                                    {.type = EARG_INT, .v.i = 3},
                                    {.type = EARG_INT, .v.i = -1},
                                    {.type = EARG_INT, .v.i = -3},
                                    {.type = EARG_INT, .v.i = -1},
                                },
                            .param_count = 16},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_TALLIYA] =
                {
                    .id = PIECE_TALLIYA,
                    .name = "Talliya",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_TOWN,
                    .base_value = 30,
                    .move = {.func = mg_slide_dirs,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0xAA},
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = 20},
                            },
                        .param_count = 3},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_ZIRAAFA] =
                {
                    .id = PIECE_ZIRAAFA,
                    .name = "Ziraafa",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_PROVINCE,
                    .base_value = 35,
                    .move = {.func = mg_zq_ziraafa,
                        .params = {},
                        .param_count = 0},
                    .threat =
                        {.func = mg_zq_war_elephant,
                            .params = {},
                            .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_SHAHZADEH] =
                {
                    .id = PIECE_SHAHZADEH,
                    .name = "Shahzadeh",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_CAPSTONE,
                    .base_value = 100,
                    .move = {.func = mg_step_set,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = -1},
                            },
                        .param_count = 8},
                    .passives =
                        {
                            {.trigger = TRIGGER_TURN_START, EFF(TODO, todo)},
                        },
                    .passive_count = 1,
                },
            [PIECE_OLD_KING] =
                {
                    .id = PIECE_OLD_KING,
                    .name = "Old King",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_TOWN,
                    .base_value = 30,
                    .move = {.func = mg_step_set,
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
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_CATAPHRACT] =
                {
                    .id = PIECE_CATAPHRACT,
                    .name = "Cataphract",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_PROVINCE,
                    .base_value = 40,
                    .move = {.func = mg_choice, .params = {}, .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_ROOK] =
                {
                    .id = PIECE_ROOK,
                    .name = "Rook",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_CAPSTONE,
                    .base_value = 50,
                    .move = {.func = mg_slide_dirs,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0x55},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 20},
                            },
                        .param_count = 3},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_WAR_ELEPHANT] =
                {
                    .id = PIECE_WAR_ELEPHANT,
                    .name = "War Elephant",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_COUNTRY,
                    .base_value = 50,
                    .move = {.func = mg_zq_ziraafa,
                        .params = {},
                        .param_count = 0},
                    .threat =
                        {.func = mg_zq_war_elephant,
                            .params = {},
                            .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
};

const size_t PIECES_ZARQAN_COUNT =
    sizeof(PIECES_ZARQAN) / sizeof(PIECES_ZARQAN[0]);

/*--------------------------------------------------------------------------*\
                              CARDS
\*--------------------------------------------------------------------------*/

/// CARDS_ZARQAN
///
/// Dense array indexed by CardId for every Zarqan-kingdom card.
/// Effect handlers default to eff_todo until the corresponding card
/// behaviour is implemented in src/effects.
///
const CardTemplate CARDS_ZARQAN[] = {
    [CARD_COUNSEL] =
        {
            .id         = CARD_COUNSEL,
            .name       = "Counsel",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 15,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_PILLAGE] =
        {
            .id         = CARD_PILLAGE,
            .name       = "Pillage",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 20,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_ROYAL_DECOY] =
        {
            .id         = CARD_ROYAL_DECOY,
            .name       = "Royal Decoy",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 30,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_BAZAAR] =
        {
            .id         = CARD_BAZAAR,
            .name       = "Bazaar",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 35,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_STEPPE_RIDERS] =
        {
            .id         = CARD_STEPPE_RIDERS,
            .name       = "Steppe Riders",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 30,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_AMBITION] =
        {
            .id         = CARD_AMBITION,
            .name       = "Ambition",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 45,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CITADEL] =
        {
            .id         = CARD_CITADEL,
            .name       = "Citadel",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 50,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CONQUEST] = {
        .id         = CARD_CONQUEST,
        .name       = "Conquest",
        .kingdom    = KINGDOM_ZARQAN,
        .tier       = TIER_COUNTRY,
        .play_cost  = 50,
        .sell_value = 80,
        .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_ZARQAN_COUNT =
    sizeof(CARDS_ZARQAN) / sizeof(CARDS_ZARQAN[0]);