//! data_harushima.c
//!
//! Static templates for the Harushima Shogunate kingdom. Holds both
//! the piece roster (base pieces plus combo results) and the card
//! roster as immutable arrays consumed by the registry.
//!
//! Harushima follows the Shogi / Chu Shogi tradition: the player
//! sits at the bottom (y=0) and pieces face forward. Specialised
//! movement patterns live in mg_harushima.c.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECES
\*--------------------------------------------------------------------------*/

/// PIECES_HARUSHIMA
///
/// Dense array indexed by PieceId for every Harushima piece. Combo
/// results sit alongside base pieces; the registry routes ids inside
/// the PIECE_FUHYO .. PIECE_DRAGON range here.
///
const PieceTemplate PIECES_HARUSHIMA[] =
    {
        [PIECE_FUHYO] =
            {
                .id         = PIECE_FUHYO,
                .name       = "Fuhyo",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_DISTRICT,
                .base_value = 10,
                .move =
                    {.func = mg_step,
                     .params =
                         {{.type = EARG_INT, .v.i = 0},
                          {.type = EARG_INT, .v.i = 1}},
                     .param_count = 2},
                .passives      = {},
                .passive_count = 0,
            },
        [PIECE_KYOSHA] =
            {
                .id         = PIECE_KYOSHA,
                .name       = "Kyosha",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_DISTRICT,
                .base_value = 30,
                .move =
                    {.func = mg_slide,
                     .params =
                         {
                             {.type = EARG_INT, .v.i = 0},
                             {.type = EARG_INT, .v.i = 1},
                             {.type = EARG_INT, .v.i = 1},
                             {.type = EARG_INT, .v.i = 20},
                         },
                     .param_count = 4},
                .passives      = {},
                .passive_count = 0,
            },
        [PIECE_GINSHO] =
            {
                .id         = PIECE_GINSHO,
                .name       = "Ginsho",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_TOWN,
                .base_value = 30,
                .move = {.func = mg_hs_ginsho, .params = {}, .param_count = 0},
                .passives      = {},
                .passive_count = 0,
            },
        [PIECE_KINSHO] =
            {
                .id         = PIECE_KINSHO,
                .name       = "Kinsho",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_PROVINCE,
                .base_value = 35,
                .move = {.func = mg_hs_kinsho, .params = {}, .param_count = 0},
                .passives      = {},
                .passive_count = 0,
            },
        [PIECE_SHISHI] =
            {
                .id         = PIECE_SHISHI,
                .name       = "Shishi",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_CAPSTONE,
                .base_value = 100,
                .move =
                    {.func        = mg_hs_shishi,
                     .params      = {{.type = EARG_INT, .v.i = 2}},
                     .param_count = 1},
                .passives      = {},
                .passive_count = 0,
            },
        [PIECE_HONORABLE_HORSE] =
            {
                .id         = PIECE_HONORABLE_HORSE,
                .name       = "Honorable Horse",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_TOWN,
                .base_value = 40,
                .move =
                    {.func        = mg_hs_honorable_horse,
                     .params      = {{.type = EARG_INT, .v.i = 1}},
                     .param_count = 1},
                .passives =
                    {
                        {.trigger = TRIGGER_PIECE_ENTERED_ENEMY_TERR,
                         .apply   = eff_swap_movegen},
                    },
                .passive_count = 1,
            },
        [PIECE_PROMOTED_BISHOP] =
            {
                .id         = PIECE_PROMOTED_BISHOP,
                .name       = "Promoted Bishop",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_PROVINCE,
                .base_value = 50,
                .move = {.func = mg_compound, .params = {}, .param_count = 0},
                .passives      = {},
                .passive_count = 0,
            },
        [PIECE_DAIMYO] =
            {
                .id         = PIECE_DAIMYO,
                .name       = "Daimyo",
                .kingdom    = KINGDOM_HARUSHIMA,
                .tier       = TIER_COUNTRY,
                .base_value = 50,
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
                .passives =
                    {
                        {.trigger = TRIGGER_RESOLVE_FLIP,
                         .apply   = eff_grant_flip_immunity},
                    },
                .passive_count = 1,
            },
        [PIECE_DRAGON] = {
            .id         = PIECE_DRAGON,
            .name       = "Dragon",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_CAPSTONE,
            .base_value = 70,
            .move       = {.func = mg_compound, .params = {}, .param_count = 0},
            .passives   = {},
            .passive_count = 0,
        },
};

const size_t PIECES_HARUSHIMA_COUNT =
    sizeof(PIECES_HARUSHIMA) / sizeof(PIECES_HARUSHIMA[0]);

/*--------------------------------------------------------------------------*\
                              CARDS
\*--------------------------------------------------------------------------*/

/// CARDS_HARUSHIMA
///
/// Dense array indexed by CardId for every Harushima-kingdom card.
/// Each entry records play/sell costs and on-play effect handlers;
/// effects still in design use eff_todo.
///
const CardTemplate CARDS_HARUSHIMA[] = {
    [CARD_RONIN] =
        {
            .id         = CARD_RONIN,
            .name       = "Ronin",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 15,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_RESURRECTION] =
        {
            .id         = CARD_RESURRECTION,
            .name       = "Resurrection",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 25,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_GOLD_STANDARD] =
        {
            .id         = CARD_GOLD_STANDARD,
            .name       = "Gold Standard",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 25,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_PROMOTION] =
        {
            .id         = CARD_PROMOTION,
            .name       = "Promotion",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 30,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_DUAL_DROP] =
        {
            .id         = CARD_DUAL_DROP,
            .name       = "Dual Drop",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_PROVINCE,
            .play_cost  = 30,
            .sell_value = 45,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_FORCE_DROP] =
        {
            .id         = CARD_FORCE_DROP,
            .name       = "Force Drop",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 50,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_BUSHIDO] = {
        .id         = CARD_BUSHIDO,
        .name       = "Bushido",
        .kingdom    = KINGDOM_HARUSHIMA,
        .tier       = TIER_COUNTRY,
        .play_cost  = 0,
        .sell_value = 60,
        .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_HARUSHIMA_COUNT =
    sizeof(CARDS_HARUSHIMA) / sizeof(CARDS_HARUSHIMA[0]);