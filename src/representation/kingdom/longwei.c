//! longwei.c
//!
//! Longwei kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Bulwark innate, climax, Iron Strategist
//! overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

const Piece      LONGWEI_PIECES[] = { {} };
const Card       LONGWEI_CARDS[]  = { {} };
const BoardTrait LONGWEI_TRAITS[] = { {} };

/// longwei_innate
///
/// Attaches the Bulwark innate to the given side at the given mastery
/// level.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void longwei_innate(BattleState* battle, Side side, MasteryLevel level) {
    (void) battle;
    (void) side;
    (void) level;
}

/// longwei_climax
///
/// Fires the Longwei combo climax for the given side.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void longwei_climax(BattleState* battle, Side side) {
    (void) battle;
    (void) side;
}

/// longwei_overseer
///
/// Sets up the Iron Strategist overseer battle with its bespoke army
/// and boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void longwei_overseer(BattleState* battle) {
    (void) battle;
}

/// longwei_event
///
/// Resolves a Longwei narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void longwei_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
