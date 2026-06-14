//! data_longwei.c
//!
//! Static templates for the Longwei Empire kingdom. Provides both
//! the piece roster (base pieces plus combo results) and the card
//! roster as immutable arrays consumed by the registry.
//!
//! Longwei follows the Xiangqi / Janggi tradition: the player sits
//! at the bottom of the board (y=0) and pieces face forward
//! (positive y). Movement specialities live in mg_longwei.c.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECES
\*--------------------------------------------------------------------------*/

/// PIECES_LONGWEI
///
/// Dense array indexed by PieceId for every Longwei piece. The slot
/// for PIECE_KING is intentionally left to the universal table; the
/// registry routes PIECE_KING to PIECES_UNIVERSAL[0] directly so a
/// duplicate king entry here would be dead weight.
///
const PieceTemplate PIECES_LONGWEI[] =
    {
        [PIECE_BING] =
            {
                .id = PIECE_BING,
                .name = "Bing",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_DISTRICT,
                .base_value = 1,
                .move = {.func = mg_step,
                    .params = {{.type = EARG_INT, .v.i = 0},
                        {.type = EARG_INT, .v.i = 1}},
                    .param_count = 2},
                .passives = {},
                .passive_count = 0,
            },
        [PIECE_XIANG] =
            {
                .id = PIECE_XIANG,
                .name = "Xiang",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_TOWN,
                .base_value = 2,
                .move = {.func = mg_lw_xiang, .params = {}, .param_count = 0},
                .passives = {},
                .passive_count = 0,
            },
        [PIECE_MA] =
            {
                .id = PIECE_MA,
                .name = "Ma",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_TOWN,
                .base_value = 3,
                .move = {.func = mg_lw_ma, .params = {}, .param_count = 0},
                .passives = {},
                .passive_count = 0,
            },
        [PIECE_PAO] =
            {
                .id = PIECE_PAO,
                .name = "Pao",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_PROVINCE,
                .base_value = 4,
                .move = {.func = mg_lw_pao, .params = {}, .param_count = 0},
                .passives = {},
                .passive_count = 0,
            },
        [PIECE_LIUBO_DIVINER] =
            {
                .id = PIECE_LIUBO_DIVINER,
                .name = "Liubo Diviner",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_COUNTRY,
                .base_value = 5,
                .move = {.func = mg_lw_liubo, .params = {}, .param_count = 0},
                .passives = {},
                .passive_count = 0,
            },
        [PIECE_SANG] =
            {
                .id = PIECE_SANG,
                .name = "Sang",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_CAPSTONE,
                .base_value = 6,
                .move = {.func = mg_lw_sang, .params = {}, .param_count = 0},
                .passives = {},
                .passive_count = 0,
            },
        [PIECE_NORTHERN_CAVALRY] =
            {
                .id = PIECE_NORTHERN_CAVALRY,
                .name = "Northern Cavalry",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_CAPSTONE,
                .base_value = 7,
                .move =
                    {.func = mg_leap_set,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = -2},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = -2},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = -2},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = -2},
                            },
                        .param_count = 16},
                .passives =
                    {
                        {.trigger = TRIGGER_RESOLVE_DEFENSE, EFF(TODO, todo)},
                    },
                .passive_count = 1,
            },
        [PIECE_HWACHA] =
            {
                .id = PIECE_HWACHA,
                .name = "Hwacha",
                .kingdom = KINGDOM_LONGWEI,
                .tier = TIER_CAPSTONE,
                .base_value = 8,
                .move = {.func = mg_lw_hwacha, .params = {}, .param_count = 0},
                .passives = {},
                .passive_count = 0,
            },
};

const size_t PIECES_LONGWEI_COUNT =
    sizeof(PIECES_LONGWEI) / sizeof(PIECES_LONGWEI[0]);

/*--------------------------------------------------------------------------*\
                              CARDS
\*--------------------------------------------------------------------------*/

/// CARDS_LONGWEI
///
/// Dense array indexed by CardId for every Longwei-kingdom card. Each
/// entry registers its play/sell costs and the effects fired on play;
/// unimplemented effects use eff_todo so the registry can still hand
/// the card to a UI even before its behaviour exists.
///
const CardTemplate CARDS_LONGWEI[] = {
    [CARD_RIVER_WADE] =
        {
            .id         = CARD_RIVER_WADE,
            .name       = "River Wade",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_DISTRICT,
            .play_cost  = 2,
            .sell_value = 1,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CHARGE] =
        {
            .id         = CARD_CHARGE,
            .name       = "Charge",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_DISTRICT,
            .play_cost  = 2,
            .sell_value = 1,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_FORMATION] =
        {
            .id         = CARD_FORMATION,
            .name       = "Formation",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_TOWN,
            .play_cost  = 3,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_DIVINATION] =
        {
            .id         = CARD_DIVINATION,
            .name       = "Divination",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_TOWN,
            .play_cost  = 3,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CANNON_VOLLEY] =
        {
            .id         = CARD_CANNON_VOLLEY,
            .name       = "Cannon Volley",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_PROVINCE,
            .play_cost  = 4,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_PALACE_DECREE] =
        {
            .id         = CARD_PALACE_DECREE,
            .name       = "Palace Decree",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_PROVINCE,
            .play_cost  = 4,
            .sell_value = 2,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_MANDATE] = {
        .id         = CARD_MANDATE,
        .name       = "Mandate",
        .kingdom    = KINGDOM_LONGWEI,
        .tier       = TIER_COUNTRY,
        .play_cost  = 5,
        .sell_value = 3,
        .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_LONGWEI_COUNT =
    sizeof(CARDS_LONGWEI) / sizeof(CARDS_LONGWEI[0]);
