//! ai.h
//!
//! AI archetypes, weights, and action selection for the Regnum engine.
//! Each kingdom has an AIArchetype; ai_pick_one dispatches to kingdom logic.
//! Scoring stubs (ai_score_move, ai_score_buy) are reserved for GDD §8.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef AI_H
#define AI_H

#include "types.h"

struct BattleState;
struct PieceState;
struct CardInstance;
struct MoveList;
struct Recipe;

/*--------------------------------------------------------------------------*\
                              ACTION
\*--------------------------------------------------------------------------*/

typedef enum {
    ACTION_NONE,
    ACTION_MOVE,
    ACTION_BUY,
    ACTION_COMBINE,
    ACTION_PLAY_CARD,
    ACTION_SELL_CARD,
    ACTION_END_TURN
} ActionKind;

typedef struct {
    ActionKind kind;
    union {
        struct {
            uint32_t piece_id;
            Position to;
        } move;
        struct {
            uint16_t tmpl_id;
            Position at;
        } buy;
        struct {
            uint32_t a, b;
        } combine;
        struct {
            uint8_t    hand_idx;
            TargetSpec target;
        } play_card;
        struct {
            uint8_t hand_idx;
        } sell_card;
    } as;
} Action;

/*--------------------------------------------------------------------------*\
                              AI WEIGHTS
\*--------------------------------------------------------------------------*/

typedef struct {
    int value_diff_w;
    int territory_w;
    int aggression_w;
    int sell_threshold;
    int save_threshold;
    int max_piece_cost;
    int combo_chain_bonus;
    int reclaim_priority;
    int extra[4];
} AIWeights;

/*--------------------------------------------------------------------------*\
                              AI ARCHETYPE
\*--------------------------------------------------------------------------*/

typedef Action (*AIPickFunc)(const struct BattleState* bs);

typedef struct {
    const char* name;
    Kingdom     kingdom;
    AIPickFunc  pick;
    AIWeights   primary;
    AIWeights   fallback;
    int         fallback_meter_diff;
} AIArchetype;

/*--------------------------------------------------------------------------*\
                              AI API
\*--------------------------------------------------------------------------*/

/// ai_play_turn
///
/// Run the full AI half-turn.
///
/// Params:
/// - struct BattleState* bs -> battle state to operate on
///
void ai_play_turn(struct BattleState* bs);

/// ai_pick_one
///
/// Pick a single action for the current state.
///
/// Params:
/// - struct BattleState* bs -> battle state to evaluate
///
/// Return:
/// Action -> chosen action
///
Action ai_pick_one(struct BattleState* bs);

/// ai_score_move
///
/// Score a move action.
///
/// Params:
/// - const struct BattleState* bs -> battle state to evaluate
/// - const Action* action -> action to score
/// - const AIWeights* w -> scoring weights
///
/// Return:
/// int -> action score
///
int ai_score_move(
    const struct BattleState* bs,
    const Action*             action,
    const AIWeights*          w
);

/// ai_score_buy
///
/// Score a buy action.
///
/// Params:
/// - const struct BattleState* bs -> battle state to evaluate
/// - const Action* action -> action to score
/// - const AIWeights* w -> scoring weights
///
/// Return:
/// int -> action score
///
int ai_score_buy(
    const struct BattleState* bs,
    const Action*             action,
    const AIWeights*          w
);

#endif /* AI_H */