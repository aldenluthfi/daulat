//!
//! Battle state, configuration, turn API, action API, and event log.
//! BattleState is the model consumed by the SDL3 front-end.
//! All mutations go through battle_action_* or battle_play_card.
//! Events are pushed to a ring buffer for animation sync.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef BATTLE_H
#define BATTLE_H

#include "ai.h"
#include "board.h"
#include "card.h"
#include "effect.h"
#include "meta.h"
#include "movegen.h"
#include "piece.h"
#include "recipe.h"
#include "rng.h"
#include "run.h"
#include "types.h"

/*--------------------------------------------------------------------------*\
                              BATTLE CONFIG
\*--------------------------------------------------------------------------*/

typedef struct {
    int width;
    int height;
    int max_turns;
    int starting_cp;
    uint64_t rng_seed;
    Side player_side;
    const Modifier* modifiers[MAX_BATTLE_MODIFIERS];
    uint8_t modifier_count;
    const BoardTrait* traits[MAX_BOARD_TRAITS];
    uint8_t trait_count;
    RunState* run;
} BattleConfig;

/*--------------------------------------------------------------------------*\
                              EVENT LOG
\*--------------------------------------------------------------------------*/

typedef enum {
    EVT_TURN_STARTED,
    EVT_TURN_ENDED,
    EVT_RESOLVE_BEGAN,
    EVT_RESOLVE_ENDED,
    EVT_PIECE_PLACED,
    EVT_PIECE_MOVED,
    EVT_PIECE_COMBINED,
    EVT_PIECE_REMOVED,
    EVT_PIECE_FLIPPED,
    EVT_PIECE_DAMAGED,
    EVT_METER_CHANGED,
    EVT_CP_CHANGED,
    EVT_CARD_DRAWN,
    EVT_CARD_PLAYED,
    EVT_CARD_SOLD,
    EVT_EFFECT_APPLIED,
    EVT_BATTLE_ENDED,
    EVT_COUNT
} EventKind;

typedef struct {
    EventKind kind;
    uint16_t turn_no;
    union {
        struct {
            uint32_t piece_id;
            Position from, to;
        } moved;
        struct {
            uint32_t piece_id;
            Position pos;
            uint16_t tmpl_id;
            Side owner;
        } placed;
        struct {
            uint32_t piece_id;
            Side new_owner;
        } flipped;
        struct {
            uint32_t attacker, target;
            int dmg;
        } damaged;
        struct {
            Side s;
            int old_val, new_val;
        } meter;
        struct {
            Side s;
            int old_val, new_val;
        } cp;
        struct {
            Side s;
            uint16_t card_tmpl_id;
        } card;
        struct {
            uint32_t effect_source_id;
            EffectTrigger trigger;
        } effect;
    } as;
} Event;

/*--------------------------------------------------------------------------*\
                              BATTLE STATE
\*--------------------------------------------------------------------------*/

typedef struct BattleState {
    Board board;
    BattleConfig config;
    Side active_side;
    uint16_t turn_no;
    uint16_t max_turns;
    int cp[2];
    int meter[2];
    int meter_cap[2];
    int meter_overflow_cap[2];
    PieceState pieces[MAX_PIECES];
    uint16_t piece_count;
    uint32_t next_piece_id;
    CardInstance hand[2][MAX_HAND];
    uint8_t hand_count[2];
    const CardTemplate* cardset[2][MAX_CARDSET];
    uint16_t cardset_count[2];
    uint8_t actions_left;
    EffectBus bus;
    Rng rng;
    Event events[MAX_EVENTS];
    uint16_t event_head;
    uint16_t event_count;
    bool battle_ended;
    BattleResult result;
} BattleState;

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

void battle_init(BattleState* bs, const BattleConfig* cfg);
void battle_destroy(BattleState* bs);
BattleResult battle_check_end(const BattleState* bs);
void battle_turn_start(BattleState* bs);
void battle_turn_end(BattleState* bs);

/*--------------------------------------------------------------------------*\
                              STATE ACCESSORS
\*--------------------------------------------------------------------------*/

const PieceState* battle_piece_at(const BattleState* bs, Position p);
const PieceState* battle_piece_by_id(const BattleState* bs, uint32_t id);
size_t battle_pieces(
    const BattleState* bs, Side side, const PieceState** out, size_t cap
);
Side battle_territory(const BattleState* bs, Position p);
int battle_threat_count(const BattleState* bs, Position p, Side attacker);
size_t battle_hand(
    const BattleState* bs, Side side, const CardInstance** out, size_t cap
);
const CardTemplate* battle_card_tmpl(uint16_t id);
const PieceTemplate* battle_piece_tmpl(uint16_t id);

/*--------------------------------------------------------------------------*\
                              AFFORDANCE ENUMERATION
\*--------------------------------------------------------------------------*/

size_t
battle_legal_moves(const BattleState* bs, uint32_t piece_id, MoveList* out);
size_t battle_valid_buy_squares(
    const BattleState* bs, Side side, uint16_t tmpl_id, Position* out,
    size_t cap
);
size_t battle_valid_combinations(
    const BattleState* bs, Side side, uint32_t (*out_pairs)[2], size_t cap
);
size_t battle_card_targets(
    const BattleState* bs, Side side, uint8_t hand_idx, TargetSpec* out,
    size_t cap
);
const Recipe* battle_recipe_preview(uint16_t a, uint16_t b);

/*--------------------------------------------------------------------------*\
                              PROJECTION QUERIES
\*--------------------------------------------------------------------------*/

int battle_query_cost(const BattleState* bs, Side side, uint16_t tmpl_id);
int battle_query_sell_value(
    const BattleState* bs, Side side, const CardInstance* card
);
int battle_query_draw_count(const BattleState* bs, Side side);
int battle_query_turn_income(const BattleState* bs, Side side);
int battle_projected_damage(const BattleState* bs, Side attacker);
int battle_projected_flips(const BattleState* bs, Side attacker);

/*--------------------------------------------------------------------------*\
                              ACTION API
\*--------------------------------------------------------------------------*/

bool battle_can_play_card(
    const BattleState* bs, uint8_t idx, const TargetSpec* tgt
);
bool battle_play_card(BattleState* bs, uint8_t idx, const TargetSpec* tgt);

bool battle_can_sell_card(const BattleState* bs, uint8_t idx);
bool battle_sell_card(BattleState* bs, uint8_t idx);

bool battle_can_move(const BattleState* bs, uint32_t piece_id, Position to);
bool battle_action_move(BattleState* bs, uint32_t piece_id, Position to);

bool battle_can_buy(const BattleState* bs, uint16_t tmpl_id, Position at);
bool battle_action_buy(BattleState* bs, uint16_t tmpl_id, Position at);

bool battle_can_combine(const BattleState* bs, uint32_t a, uint32_t b);
bool battle_action_combine(BattleState* bs, uint32_t a, uint32_t b);

void battle_end_player_turn(BattleState* bs);

/*--------------------------------------------------------------------------*\
                              EVENT LOG
\*--------------------------------------------------------------------------*/

size_t battle_drain_events(BattleState* bs, Event* out, size_t cap);
void battle_clear_events(BattleState* bs);

#endif /* BATTLE_H */