//! ai.c
//!
//! AI archetype dispatcher and default scorer.
//! Normal path: weights-only scoring via dot-product.
//! Custom AIPickFunc for distinctive logic (Reclaimer, Trickster).
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              AI DRIVER
\*--------------------------------------------------------------------------*/

/// ai_execute_action
///
/// Execute a single action on the battle state.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - const Action* a -> action to execute
///
static void ai_execute_action(BattleState* battle, const Action* a) {
    switch (a->kind) {
    case ACTION_MOVE:
        battle_action_move(battle, a->as.move.piece_id, a->as.move.to);
        break;
    case ACTION_BUY:
        battle_action_buy(battle, a->as.buy.template_id, a->as.buy.at);
        break;
    case ACTION_COMBINE:
        battle_action_combine(battle, a->as.combine.a, a->as.combine.b);
        break;
    case ACTION_PLAY_CARD:
        battle_play_card(
            battle,
            a->as.play_card.hand_index,
            &a->as.play_card.target
        );
        break;
    case ACTION_SELL_CARD:
        battle_sell_card(battle, a->as.sell_card.hand_index);
        break;
    case ACTION_END_TURN:
    case ACTION_NONE:
        break;
    }
}

/// ai_play_turn
///
/// Run the full AI half-turn.
///
/// Params:
/// - BattleState* battle -> battle state to operate on
///
void ai_play_turn(BattleState* battle) {
    uint8_t actions = battle->actions_left;
    while (actions > 0 && !battle->battle_ended) {
        Action action = ai_pick_one(battle);
        if (action.kind == ACTION_END_TURN)
            break;
        ai_execute_action(battle, &action);
        actions--;
    }
}

/// ai_pick_one
///
/// Pick a single action for the current state.
///
/// Params:
/// - BattleState* battle -> battle state to evaluate
///
/// Return:
/// Action -> chosen action
///
Action ai_pick_one(BattleState* battle) {
    Action best = {.kind = ACTION_END_TURN};

    for (uint16_t i = 0; i < battle->piece_count; i++) {
        const PieceState* piece = &battle->pieces[i];
        if (piece->owner != battle->active_side)
            continue;
        MoveList move_list = {0};
        mg_generate(piece, battle, &move_list);
        if (move_list.count > 0) {
            uint64_t index        = rng_range(&battle->rng, move_list.count);
            best.kind             = ACTION_MOVE;
            best.as.move.piece_id = piece->id;
            best.as.move.to       = move_list.squares[index];
            return best;
        }
    }

    if (battle->hand_count[battle->active_side] > 0) {
        uint64_t index =
            rng_range(&battle->rng, battle->hand_count[battle->active_side]);
        best.kind                    = ACTION_SELL_CARD;
        best.as.sell_card.hand_index = (uint8_t)index;
    }

    return best;
}

/*--------------------------------------------------------------------------*\
                              SCORING
\*--------------------------------------------------------------------------*/

/// ai_score_move
///
/// Score a move action. Currently a stub returning 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - const Action* action -> action to score (unused)
/// - const AIWeights* w -> weights (unused)
///
/// Return:
/// int -> score (always 0)
///
int ai_score_move(
    const BattleState* battle,
    const Action*      action,
    const AIWeights*   w
) {
    (void)battle;
    (void)action;
    (void)w;
    return 0;
}

/// ai_score_buy
///
/// Score a buy action. Currently a stub returning 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - const Action* action -> action to score (unused)
/// - const AIWeights* w -> weights (unused)
///
/// Return:
/// int -> score (always 0)
///
int ai_score_buy(
    const BattleState* battle,
    const Action*      action,
    const AIWeights*   w
) {
    (void)battle;
    (void)action;
    (void)w;
    return 0;
}