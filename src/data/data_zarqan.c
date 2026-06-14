//! data_zarqan.c
//!
//! Static templates for the Zarqan Sultanate kingdom. Holds both the
//! piece roster (base pieces plus combo results) and the card roster
//! as immutable arrays consumed by the registry.
//!
//! Zarqan follows the Tamerlane / Shatranj tradition: the player
//! sits at the bottom (y=0) and pieces face forward. The Ziraafa
//! family's bespoke movement lives in mg_zarqan.c.
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
                    .base_value = 1,
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
                    .tier = TIER_TOWN,
                    .base_value = 2,
                    .move = {.func = mg_leap_set,
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
                    .base_value = 3,
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
            [PIECE_OLD_KING] =
                {
                    .id = PIECE_OLD_KING,
                    .name = "Old King",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_COUNTRY,
                    .base_value = 5,
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
                    .tier = TIER_CAPSTONE,
                    .base_value = 7,
                    .move = {.func = mg_choice, .params = {}, .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_ZIRAAFA] =
                {
                    .id = PIECE_ZIRAAFA,
                    .name = "Ziraafa",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_PROVINCE,
                    .base_value = 6,
                    .move = {.func = mg_zq_ziraafa,
                             .params = {},
                             .param_count = 0},
                    .threat = {.func = mg_zq_war_elephant,
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
                    .base_value = 8,
                    .move = {.func = mg_zq_ziraafa,
                             .params = {},
                             .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_WAR_ELEPHANT] =
                {
                    .id = PIECE_WAR_ELEPHANT,
                    .name = "War Elephant",
                    .kingdom = KINGDOM_ZARQAN,
                    .tier = TIER_CAPSTONE,
                    .base_value = 6,
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
                                     {.type = EARG_INT, .v.i = 1},
                                     {.type = EARG_INT, .v.i = 1},
                                     {.type = EARG_INT, .v.i = -1},
                                     {.type = EARG_INT, .v.i = 1},
                                     {.type = EARG_INT, .v.i = -1},
                                     {.type = EARG_INT, .v.i = -1},
                                     {.type = EARG_INT, .v.i = 1},
                                     {.type = EARG_INT, .v.i = -1},
                                 },
                             .param_count = 16},
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
            .id = CARD_COUNSEL,
            .name = "Counsel",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_DISTRICT,
            .play_cost = 2,
            .sell_value = 1,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_AMBITION] =
        {
            .id = CARD_AMBITION,
            .name = "Ambition",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_TOWN,
            .play_cost = 3,
            .sell_value = 2,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_CONQUEST] =
        {
            .id = CARD_CONQUEST,
            .name = "Conquest",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_PROVINCE,
            .play_cost = 4,
            .sell_value = 2,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_FORTUNE] =
        {
            .id = CARD_FORTUNE,
            .name = "Fortune",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_COUNTRY,
            .play_cost = 5,
            .sell_value = 3,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_CRUSADE] =
        {
            .id = CARD_CRUSADE,
            .name = "Crusade",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_COUNTRY,
            .play_cost = 6,
            .sell_value = 3,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_DIVINE_RIGHT] =
        {
            .id = CARD_DIVINE_RIGHT,
            .name = "Divine Right",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_DISTRICT,
            .play_cost = 2,
            .sell_value = 1,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_INTRIGUE] =
        {
            .id = CARD_INTRIGUE,
            .name = "Intrigue",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_PROVINCE,
            .play_cost = 3,
            .sell_value = 2,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_TREACHERY] = {
        .id = CARD_TREACHERY,
        .name = "Treachery",
        .kingdom = KINGDOM_ZARQAN,
        .tier = TIER_PROVINCE,
        .play_cost = 3,
        .sell_value = 2,
        .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_ZARQAN_COUNT =
    sizeof(CARDS_ZARQAN) / sizeof(CARDS_ZARQAN[0]);
