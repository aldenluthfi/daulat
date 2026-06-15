//! data_caelan.c
//!
//! Static templates for the Caelan Kingdom. Holds both the piece
//! roster (base pieces plus combo results) and the card roster as
//! immutable arrays consumed by the registry.
//!
//! Caelan follows the standard chess (plus Grant Acedrex) tradition:
//! the player sits at the bottom (y=0) and pieces face forward. The
//! Gryphon's two-stage walk lives in mg_caelan.c; everything else
//! resolves to mg_basics primitives directly. Rook is NOT a Caelan
//! piece — per the GDD it is a Zarqan combo result (Kyosha + Wazir).
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECES
\*--------------------------------------------------------------------------*/

/// PIECES_CAELAN
///
/// Dense array indexed by PieceId for every Caelan piece. Bishop and
/// Queen use mg_slide_dirs with direction-bit masks; Chancellor and
/// Sovereign Banner use mg_choice / passive effect.
///
const PieceTemplate
    PIECES_CAELAN[] =
        {
            [PIECE_PAWN] =
                {
                    .id = PIECE_PAWN,
                    .name = "Pawn",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_DISTRICT,
                    .base_value = 10,
                    .move = {.func = mg_step,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0},
                                {.type = EARG_INT, .v.i = 1},
                            },
                        .param_count = 2},
                    .threat = {.func = mg_slide_dirs,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0xAA},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 1},
                            },
                        .param_count = 3},
                    .passives =
                        {
                            {.trigger = TRIGGER_PIECE_PLACED,
                                EFF(TODO, todo)},
                        },
                    .passive_count = 1,
                },
            [PIECE_KNIGHT] =
                {
                    .id = PIECE_KNIGHT,
                    .name = "Knight",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_DISTRICT,
                    .base_value = 30,
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
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_BISHOP] =
                {
                    .id = PIECE_BISHOP,
                    .name = "Bishop",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_TOWN,
                    .base_value = 30,
                    .move = {.func = mg_slide_dirs,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0xAA},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 20},
                            },
                        .param_count = 3},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_QUEEN] =
                {
                    .id = PIECE_QUEEN,
                    .name = "Queen",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_PROVINCE,
                    .base_value = 90,
                    .move = {.func = mg_slide_dirs,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0xFF},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 20},
                            },
                        .param_count = 3},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_GRYPHON] =
                {
                    .id = PIECE_GRYPHON,
                    .name = "Gryphon",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_CAPSTONE,
                    .base_value = 100,
                    .move = {.func = mg_ca_gryphon,
                        .params = {},
                        .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_CHANCELLOR] =
                {
                    .id = PIECE_CHANCELLOR,
                    .name = "Chancellor",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_PROVINCE,
                    .base_value = 70,
                    .move = {.func = mg_choice, .params = {}, .param_count = 0},
                    .passives = {},
                    .passive_count = 0,
                },
            [PIECE_SOVEREIGN_BANNER] =
                {
                    .id = PIECE_SOVEREIGN_BANNER,
                    .name = "Sovereign Banner",
                    .kingdom = KINGDOM_CAELAN,
                    .tier = TIER_PROVINCE,
                    .base_value = 110,
                    .move = {.func = mg_slide_dirs,
                        .params =
                            {
                                {.type = EARG_INT, .v.i = 0xFF},
                                {.type = EARG_INT, .v.i = 1},
                                {.type = EARG_INT, .v.i = 20},
                            },
                        .param_count = 3},
                    .passives =
                        {
                            {.trigger = TRIGGER_TURN_START, EFF(TODO, todo)},
                        },
                    .passive_count = 1,
                },
};

const size_t PIECES_CAELAN_COUNT =
    sizeof(PIECES_CAELAN) / sizeof(PIECES_CAELAN[0]);

/*--------------------------------------------------------------------------*\
                              CARDS
\*--------------------------------------------------------------------------*/

/// CARDS_CAELAN
///
/// Dense array indexed by CardId for every Caelan-kingdom card. The
/// roster matches the GDD: two District, two Town, two Province, two
/// Country. Effect handlers default to eff_todo until per-card
/// behaviour is implemented.
///
const CardTemplate CARDS_CAELAN[] = {
    [CARD_CASTLING] =
        {
            .id         = CARD_CASTLING,
            .name       = "Castling",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 15,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_QUEENS_GAMBIT] =
        {
            .id         = CARD_QUEENS_GAMBIT,
            .name       = "Queen's Gambit",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_DISTRICT,
            .play_cost  = 0,
            .sell_value = 20,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_VENGEANCE] =
        {
            .id         = CARD_VENGEANCE,
            .name       = "Vengeance",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_TOWN,
            .play_cost  = 0,
            .sell_value = 30,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_QUEENS_DECREE] =
        {
            .id         = CARD_QUEENS_DECREE,
            .name       = "Queen's Decree",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 50,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CATHEDRAL] =
        {
            .id         = CARD_CATHEDRAL,
            .name       = "Cathedral",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 45,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CORONATION] =
        {
            .id         = CARD_CORONATION,
            .name       = "Coronation",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 55,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_CRUSADE] =
        {
            .id         = CARD_CRUSADE,
            .name       = "Crusade",
            .kingdom    = KINGDOM_CAELAN,
            .tier       = TIER_COUNTRY,
            .play_cost  = 0,
            .sell_value = 70,
            .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_DIVINE_RIGHT] = {
        .id         = CARD_DIVINE_RIGHT,
        .name       = "Divine Right",
        .kingdom    = KINGDOM_CAELAN,
        .tier       = TIER_COUNTRY,
        .play_cost  = 0,
        .sell_value = 75,
        .on_play    = {{.trigger = TRIGGER_CARD_PLAYED, EFF(TODO, todo)}},
        .play_effect_count = 1,
    },
};

const size_t CARDS_CAELAN_COUNT =
    sizeof(CARDS_CAELAN) / sizeof(CARDS_CAELAN[0]);