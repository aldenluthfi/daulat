//!
//! AI archetype dispatcher and default scorer.
//! Normal path: weights-only scoring via dot-product.
//! Custom AIPickFunc for distinctive logic (Reclaimer, Trickster).
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              AI DRIVER
\*--------------------------------------------------------------------------*/

static void ai_execute_action(BattleState* bs, const Action* a) {
    switch (a->kind) {
    case ACTION_MOVE:
        battle_action_move(bs, a->as.move.piece_id, a->as.move.to);
        break;
    case ACTION_BUY:
        battle_action_buy(bs, a->as.buy.tmpl_id, a->as.buy.at);
        break;
    case ACTION_COMBINE:
        battle_action_combine(bs, a->as.combine.a, a->as.combine.b);
        break;
    case ACTION_PLAY_CARD:
        battle_play_card(bs, a->as.play_card.hand_idx, &a->as.play_card.target);
        break;
    case ACTION_SELL_CARD:
        battle_sell_card(bs, a->as.sell_card.hand_idx);
        break;
    case ACTION_END_TURN:
    case ACTION_NONE:
        break;
    }
}

void ai_play_turn(BattleState* bs) {
    uint8_t actions = bs->actions_left;
    while (actions > 0 && !bs->battle_ended) {
        Action a = ai_pick_one(bs);
        if (a.kind == ACTION_END_TURN)
            break;
        ai_execute_action(bs, &a);
        actions--;
    }
}

Action ai_pick_one(BattleState* bs) {
    Action best = {.kind = ACTION_END_TURN};

    for (uint16_t i = 0; i < bs->piece_count; i++) {
        const PieceState* p = &bs->pieces[i];
        if (p->owner != bs->active_side)
            continue;
        MoveList ml = {0};
        mg_generate(p, bs, &ml);
        if (ml.count > 0) {
            uint64_t idx          = rng_range(&bs->rng, ml.count);
            best.kind             = ACTION_MOVE;
            best.as.move.piece_id = p->id;
            best.as.move.to       = ml.squares[idx];
            return best;
        }
    }

    if (bs->hand_count[bs->active_side] > 0) {
        uint64_t idx = rng_range(&bs->rng, bs->hand_count[bs->active_side]);
        best.kind    = ACTION_SELL_CARD;
        best.as.sell_card.hand_idx = (uint8_t)idx;
    }

    return best;
}

/*--------------------------------------------------------------------------*\
                              SCORING
\*--------------------------------------------------------------------------*/

int ai_score_move(
    const BattleState* bs, const Action* action, const AIWeights* w
) {
    (void)bs;
    (void)action;
    (void)w;
    return 0;
}

int ai_score_buy(
    const BattleState* bs, const Action* action, const AIWeights* w
) {
    (void)bs;
    (void)action;
    (void)w;
    return 0;
}