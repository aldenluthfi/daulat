//!
//! AI archetypes, weights, and action selection for the Regnum engine.
//! Each kingdom has an AIArchetype; ai_pick_one dispatches to kingdom logic.
//! Scoring stubs (ai_score_move, ai_score_buy) are reserved for GDD §8.
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
            uint8_t hand_idx;
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

typedef Action (*AIPickFunc)(const struct BattleState *bs);

typedef struct {
    const char *name;
    Kingdom kingdom;
    AIPickFunc pick;
    AIWeights primary;
    AIWeights fallback;
    int fallback_meter_diff;
} AIArchetype;

/*--------------------------------------------------------------------------*\
                              AI API
\*--------------------------------------------------------------------------*/

/// Run the full AI half-turn.
void ai_play_turn(struct BattleState *bs);

/// Pick a single action (for debug / step-through).
Action ai_pick_one(struct BattleState *bs);

/// Score a move action.
int ai_score_move(
    const struct BattleState *bs,
    const Action *action,
    const AIWeights *w
);

/// Score a buy action.
int ai_score_buy(
    const struct BattleState *bs,
    const Action *action,
    const AIWeights *w
);

#endif /* AI_H */