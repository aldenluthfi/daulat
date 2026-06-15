//! data_kewarani.c
//!
//! Static templates for the Kewarani Negusate kingdom. Holds both
//! the piece roster (base pieces plus combo results) and the card
//! roster as immutable arrays consumed by the registry.
//!
//! Kewarani follows the Senterej tradition: the player sits at the
//! top of the board (y=height-1) so forward is delta_y=-1. Splitter
//! pieces use passive effects on TRIGGER_PIECE_FLIPPED rather than
//! engine branches; movement specialities live in mg_kewarani.c.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECES
\*--------------------------------------------------------------------------*/

/// PIECES_KEWARANI
///
/// Dense array indexed by PieceId for every Kewarani piece. The
/// splitter behaviours (Medeq Squad, Sultan's Levy, Negus Guard)
/// hook TRIGGER_PIECE_FLIPPED via passives so the resolve loop never
/// special-cases them.
///
const PieceTemplate
    PIECES_KEWARANI[] =
        {
            [PIECE_MEDEQ] =
                {
                    .id = PIECE_MEDEQ,
                    .name = "Medeq",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_DISTRICT,
                    .base_value = 10,
                    .move = {.func = mg_kw_berolina,
                        .params = {{.type = EARG_INT, .v.i = -1}},
                        .param_count = 1},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_MAKWANAM] =
                {
                    .id = PIECE_MAKWANAM,
                    .name = "Makwanam",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_DISTRICT,
                    .base_value = 15,
                    .move = {.func = mg_step_set,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = -1},
                                {.type = EARG_INT, .v.i = -1},
                            },
                        .param_count = 8},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_SABA] =
                {
                    .id = PIECE_SABA,
                    .name = "Saba",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_TOWN,
                    .base_value = 20,
                    .move =
                        {.func = mg_leap_set,
                            .params =
                                {
                                    {.type = EARG_INT, .v.i = 2},
                                    {.type = EARG_INT, .v.i = 2},
                                    {.type = EARG_INT, .v.i = 2},
                                    {.type = EARG_INT, .v.i = -2},
                                    {.type = EARG_INT, .v.i = -2},
                                    {.type = EARG_INT, .v.i = 2},
                                    {.type = EARG_INT, .v.i = -2},
                                    {.type = EARG_INT, .v.i = -2},
                                },
                            .param_count = 8},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_FARAS] =
                {
                    .id = PIECE_FARAS,
                    .name = "Faras",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_PROVINCE,
                    .base_value = 30,
                    .move = {.func = mg_leap_set,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = -2},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 2},
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = -2},
                            },
                        .param_count = 8},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_NEGUS_GUARD] =
                {
                    .id = PIECE_NEGUS_GUARD,
                    .name = "Negus Guard",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_CAPSTONE,
                    .base_value = 100,
                    .move = {.func = mg_kw_negus_guard,
                        .params = {},
                        .param_count = 0},
                    .passives =
                        {
                            {.trigger = TRIGGER_PIECE_FLIPPED,
                                .apply = eff_splitter_medeq,
                                .args = {{.type = EARG_INT, .v.i = 2}},
                                .arg_count = 1},
                        },
                    .passive_count = 1,
                },
            [PIECE_MEDEQ_SQUAD] =
                {
                    .id = PIECE_MEDEQ_SQUAD,
                    .name = "Medeq Squad",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_TOWN,
                    .base_value = 20,
                    .move = {.func = mg_kw_berolina,
                        .params = {{.type = EARG_INT, .v.i = -1}},
                        .param_count = 1},
                    .passives =
                        {
                            {.trigger = TRIGGER_PIECE_FLIPPED,
                                .apply = eff_splitter_medeq,
                                .args = {{.type = EARG_INT, .v.i = 2}},
                                .arg_count = 1},
                        },
                    .passive_count = 1,
                },
            [PIECE_SULTANS_LEVY] =
                {
                    .id = PIECE_SULTANS_LEVY,
                    .name = "Sultan's Levy",
                    .kingdom = KINGDOM_KEWARANI,
                    .tier = TIER_PROVINCE,
                    .base_value = 110,
                    .move = {.func = mg_kw_negus_guard,
                        .params = {},
                        .param_count = 0},
                    .passives =
                        {
                            {.trigger = TRIGGER_PIECE_FLIPPED,
                                .apply = eff_splitter_medeq,
                                .args = {{.type = EARG_INT, .v.i = 3}},
                                .arg_count = 1},
                        },
                    .passive_count = 1,
                },
};

const size_t PIECES_KEWARANI_COUNT =
    sizeof(PIECES_KEWARANI) / sizeof(PIECES_KEWARANI[0]);

/*--------------------------------------------------------------------------*\
                              CARDS
\*--------------------------------------------------------------------------*/

/// CARDS_KEWARANI
///
/// Dense array indexed by CardId for every Kewarani-kingdom card.
/// Effects share `eff_todo` until their bodies land; the registry
/// still hands valid templates to the UI so card visuals can be
/// developed before behaviour exists.
///
const CardTemplate CARDS_KEWARANI[] = {
    [CARD_SULTANS_GOLD] =
        {
            .id         = CARD_SULTANS_GOLD,
            .name       = "Sultan's Gold",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 20,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_MARCH] =
        {
            .id         = CARD_MARCH,
            .name       = "March",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 15,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_DOUBLE_TIME] =
        {
            .id         = CARD_DOUBLE_TIME,
            .name       = "Double Time",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 30,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_SALT_ROAD] =
        {
            .id         = CARD_SALT_ROAD,
            .name       = "Salt Road",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 35,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CARAVAN] =
        {
            .id         = CARD_CARAVAN,
            .name       = "Caravan",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 45,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_DOUBLESTRIKE] =
        {
            .id         = CARD_DOUBLESTRIKE,
            .name       = "Doublestrike",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 50,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_HAJJ] = {
        .id         = CARD_HAJJ,
        .name       = "Hajj",
        .kingdom    = KINGDOM_KEWARANI,
        .tier       = TIER_COUNTRY,
        .play_cost  = 0,
        .sell_value = 70,
        .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_KEWARANI_COUNT =
    sizeof(CARDS_KEWARANI) / sizeof(CARDS_KEWARANI[0]);