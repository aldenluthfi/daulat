//! zarqan.c
//!
//! Zarqan kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Royal Substitution innate, climax, Many-Faced
//! King overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

const Piece      ZARQAN_PIECES[] = { {} };
const Card       ZARQAN_CARDS[]  = { {} };
const BoardTrait ZARQAN_TRAITS[] = { {} };

/// zarqan_innate
///
/// Attaches the Royal Substitution innate to the given side at the
/// given mastery level.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void zarqan_innate(BattleState* battle, Side side, MasteryLevel level) {
    (void) battle;
    (void) side;
    (void) level;
}

/// zarqan_climax
///
/// Fires the Zarqan combo climax for the given side.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void zarqan_climax(BattleState* battle, Side side) {
    (void) battle;
    (void) side;
}

/// zarqan_overseer
///
/// Sets up the Many-Faced King overseer battle with its bespoke army
/// and boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void zarqan_overseer(BattleState* battle) {
    (void) battle;
}

/// zarqan_event
///
/// Resolves a Zarqan narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void zarqan_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
