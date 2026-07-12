//! universal.c
//!
//! Kingdom-agnostic game data. Holds the King, the universal and mastery
//! card pools, the modifier and chain registries, the aggregating
//! registry pointer tables, the event string tables, the kingdom
//! dispatch tables, and the Vorath battle setup.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// king_mv
///
/// Generates the King's movement: one square in any direction, the eight
/// unit vectors of ALL_DIRECTIONS used as leap offsets.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* king_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, false);

    return mg_end();
}

/// king_at
///
/// Generates the King's attack coverage: one square in any direction,
/// the eight unit vectors of ALL_DIRECTIONS used as leap offsets.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* king_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                UNIVERSAL DATA
\*----------------------------------------------------------------------------*/

const Piece UNIVERSAL_PIECES[] = {
    {
        .at      = king_at,
        .mv      = king_mv,
        .name    = "King",
        .desc    = "Moves one square in any direction. Cannot be "
                   "bought. Flipping it loses the battle.",
        .id      = PIECE_KING,
        .kingdom = KINGDOM_NONE,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 10,
    },
};

const Card UNIVERSAL_CARDS[] = { {} };

const BattleModifier MODIFIER_REGISTRY[MODIFIER_COUNT]   = {};
const ChainPenalty   CHAIN_REGISTRY[CHAIN_PENALTY_COUNT] = {};

const Piece* const PIECE_REGISTRY[PIECE_COUNT] = {
    [PIECE_KING]             = &UNIVERSAL_PIECES[0],
    [PIECE_PAWN]             = &CAELAN_PIECES[0],
    [PIECE_KNIGHT]           = &CAELAN_PIECES[1],
    [PIECE_BISHOP]           = &CAELAN_PIECES[2],
    [PIECE_QUEEN]            = &CAELAN_PIECES[3],
    [PIECE_GRYPHON]          = &CAELAN_PIECES[4],
    [PIECE_CHANCELLOR]       = &CAELAN_PIECES[5],
    [PIECE_SOVEREIGN_BANNER] = &CAELAN_PIECES[6],
};

const Card* const       CARD_REGISTRY[CARD_COUNT]         = {};
const BoardTrait* const TRAIT_REGISTRY[BOARD_TRAIT_COUNT] = {};

const KingdomID KINGDOM_ADJACENT[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = KINGDOM_KEWARANI,
    [KINGDOM_KEWARANI]  = KINGDOM_ZARQAN,
    [KINGDOM_ZARQAN]    = KINGDOM_HARUSHIMA,
    [KINGDOM_HARUSHIMA] = KINGDOM_CAELAN,
    [KINGDOM_CAELAN]    = KINGDOM_LONGWEI,
};

const char* const EVENT_NAME[EVENT_COUNT]     = {};
const char* const EVENT_TEXT[EVENT_COUNT]     = {};
const char* const EVENT_OPTION_A[EVENT_COUNT] = {};
const char* const EVENT_OPTION_B[EVENT_COUNT] = {};

/*----------------------------------------------------------------------------*\
                               DISPATCH TABLES
\*----------------------------------------------------------------------------*/

void (*const KINGDOM_INNATE[KINGDOM_COUNT])(BattleState*, Side,
                                            MasteryLevel) = {
    longwei_innate,
    kewarani_innate,
    zarqan_innate,
    harushima_innate,
    caelan_innate,
};

void (*const KINGDOM_CLIMAX[KINGDOM_COUNT])(BattleState*, Side) = {
    longwei_climax,
    kewarani_climax,
    zarqan_climax,
    harushima_climax,
    caelan_climax,
};

void (*const KINGDOM_OVERSEER[KINGDOM_COUNT])(BattleState*) = {
    longwei_overseer,
    kewarani_overseer,
    zarqan_overseer,
    harushima_overseer,
    caelan_overseer,
};

void (*const KINGDOM_EVENT[KINGDOM_COUNT])(EngineState*, EventID,
                                           EventChoice) = {
    longwei_event,
    kewarani_event,
    zarqan_event,
    harushima_event,
    caelan_event,
};

/*----------------------------------------------------------------------------*\
                                    VORATH
\*----------------------------------------------------------------------------*/

/// vorath_setup
///
/// Sets up the final Vorath battle: the twenty by twenty board, the
/// Grand King, his army, and the quadrant tally effects.
///
/// Params:
/// - battle -> battle to set up
///
void vorath_setup(BattleState* battle) {
    (void) battle;
}
