//! battle.h
//!
//! Battle state, configuration, turn API, action API, and event log.
//! BattleState is the model consumed by the SDL3 front-end.
//! All mutations go through battle_action_* or battle_play_card.
//! Events are pushed to a ring buffer for animation sync.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef BATTLE_H
#define BATTLE_H

#include "ai.h"
#include "board.h"
#include "card.h"
#include "defs.h"
#include "effect.h"
#include "meta.h"
#include "movegen.h"
#include "piece.h"
#include "recipe.h"
#include "rng.h"
#include "run.h"
#include "types.h"

/*--------------------------------------------------------------------------*\
                              ONCE-PER-BATTLE LATCHES
\*--------------------------------------------------------------------------*/

#define LATCH_DEAD_MANS_PACT     (1u << 0)
#define LATCH_PHILOSOPHERS_STONE (1u << 1)
#define LATCH_DEEP_HAND          (1u << 2)

/*--------------------------------------------------------------------------*\
                              VISION FLAGS
\*--------------------------------------------------------------------------*/

#define VISION_ENEMY_VALUES (1u << 0)

/*--------------------------------------------------------------------------*\
                              BATTLE CONFIG
\*--------------------------------------------------------------------------*/

typedef struct {
    int               width;
    int               height;
    int               max_turns;
    int               starting_cp;
    uint64_t          rng_seed;
    Side              player_side;
    const Modifier*   modifiers[MAX_BATTLE_MODIFIERS];
    uint8_t           modifier_count;
    const BoardTrait* traits[MAX_BOARD_TRAITS];
    uint8_t           trait_count;
    RunState*         run;
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
    EVT_PIECE_DEALT_DAMAGE,
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
    uint16_t  turn_no;
    union {
        struct {
            uint32_t piece_id;
            Position from, to;
        } moved;
        struct {
            uint32_t piece_id;
            Position pos;
            uint16_t tmpl_id;
            Side     owner;
        } placed;
        struct {
            uint32_t piece_id;
            Side     new_owner;
        } flipped;
        struct {
            uint32_t attacker;
            Side     victim_side;
            int      dmg;
        } dealt_damage;
        struct {
            Side side;
            int  old_val, new_val;
        } meter;
        struct {
            Side side;
            int  old_val, new_val;
        } cp;
        struct {
            Side     side;
            uint16_t card_tmpl_id;
        } card;
        struct {
            uint32_t      effect_source_id;
            EffectTrigger trigger;
        } effect;
    } as;
} Event;

/*--------------------------------------------------------------------------*\
                              BATTLE STATE
\*--------------------------------------------------------------------------*/

typedef struct BattleState {
    Board               board;
    BattleConfig        config;
    Side                active_side;
    uint16_t            turn_no;
    uint16_t            max_turns;
    int                 cp[2];
    int                 meter[2];
    int                 meter_cap[2];
    int                 meter_overflow_cap[2];
    PieceState          pieces[MAX_PIECES];
    uint16_t            piece_count;
    uint32_t            next_piece_id;
    CardInstance        hand[2][MAX_HAND];
    uint8_t             hand_count[2];
    const CardTemplate* cardset[2][MAX_CARDSET];
    uint16_t            cardset_count[2];
    uint8_t             actions_left;
    EffectBus           bus;
    Rng                 rng;
    Event               events[MAX_EVENTS];
    uint16_t            event_head;
    uint16_t            event_count;
    bool                battle_ended;
    BattleResult        result;

    /* Meta-layer scratch state */
    uint8_t  buys_this_turn[2];
    uint16_t cheapest_buy_cost[2];
    uint16_t once_per_battle_flags;
    uint32_t vision_flags;
    uint8_t  silver_chain_pending[KINGDOM_COUNT];
} BattleState;

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

/// battle_init
///
/// Initialize a battle from configuration.
///
/// Params:
/// - BattleState* bs -> battle state to initialize
/// - const BattleConfig* cfg -> battle configuration
///
void battle_init(BattleState* bs, const BattleConfig* cfg);

/// battle_destroy
///
/// Tear down a battle.
///
/// Params:
/// - BattleState* bs -> battle state to destroy
///
void battle_destroy(BattleState* bs);

/// battle_check_end
///
/// Check whether the battle has reached a terminal state.
///
/// Params:
/// - const BattleState* bs -> battle state to check
///
/// Return:
/// BattleResult -> terminal state or BATTLE_IN_PROGRESS
///
BattleResult battle_check_end(const BattleState* bs);

/// battle_turn_start
///
/// Begin a turn: tick bus, emit income/draw triggers, set actions_left.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_turn_start(BattleState* bs);

/// battle_turn_end
///
/// End a turn: sell hand, resolve, tick bus, switch active side.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_turn_end(BattleState* bs);

/*--------------------------------------------------------------------------*\
                              STATE ACCESSORS
\*--------------------------------------------------------------------------*/

/// battle_piece_at
///
/// Get the piece at a board position.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Position p -> board position
///
/// Return:
/// const PieceState* -> piece at position or NULL
///
const PieceState* battle_piece_at(const BattleState* bs, Position p);

/// battle_piece_by_id
///
/// Get a piece by its runtime id.
///
/// Params:
/// - const BattleState* bs -> battle state to search
/// - uint32_t id -> runtime piece id
///
/// Return:
/// const PieceState* -> piece or NULL if not found
///
const PieceState* battle_piece_by_id(const BattleState* bs, uint32_t id);

/// battle_pieces
///
/// Collect all pieces belonging to a side.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to collect
/// - const PieceState** out -> output array
/// - size_t cap -> output array capacity
///
/// Return:
/// size_t -> number of pieces written
///
size_t battle_pieces(
    const BattleState* bs,
    Side               side,
    const PieceState** out,
    size_t             cap
);

/// battle_territory
///
/// Get the territory owner at a position.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Position p -> board position
///
/// Return:
/// Side -> SIDE_PLAYER, SIDE_ENEMY, or SIDE_NEUTRAL
///
Side battle_territory(const BattleState* bs, Position p);

/// battle_threat_count
///
/// Count how many pieces of a side threaten a position.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Position p -> board position
/// - Side attacker -> attacking side
///
/// Return:
/// int -> number of threatening pieces
///
int battle_threat_count(const BattleState* bs, Position p, Side attacker);

/// battle_hand
///
/// Collect all cards in a side's hand.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to collect
/// - const CardInstance** out -> output array
/// - size_t cap -> output array capacity
///
/// Return:
/// size_t -> number of cards written
///
size_t battle_hand(
    const BattleState*   bs,
    Side                 side,
    const CardInstance** out,
    size_t               cap
);

/// battle_card_tmpl
///
/// Look up a card template by id.
///
/// Params:
/// - uint16_t id -> card template id
///
/// Return:
/// const CardTemplate* -> template or NULL
///
const CardTemplate* battle_card_tmpl(uint16_t id);

/// battle_piece_tmpl
///
/// Look up a piece template by id.
///
/// Params:
/// - uint16_t id -> piece template id
///
/// Return:
/// const PieceTemplate* -> template or NULL
///
const PieceTemplate* battle_piece_tmpl(uint16_t id);

/*--------------------------------------------------------------------------*\
                              AFFORDANCE ENUMERATION
\*--------------------------------------------------------------------------*/

/// battle_legal_moves
///
/// Enumerate all legal moves for a piece.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - uint32_t piece_id -> piece to query
/// - MoveList* out -> output buffer
///
/// Return:
/// size_t -> number of legal moves
///
size_t
battle_legal_moves(const BattleState* bs, uint32_t piece_id, MoveList* out);

/// battle_valid_buy_squares
///
/// Enumerate valid purchase squares.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
/// - uint16_t tmpl_id -> template id to buy
/// - Position* out -> output buffer
/// - size_t cap -> output capacity
///
/// Return:
/// size_t -> number of valid squares written
///
size_t battle_valid_buy_squares(
    const BattleState* bs,
    Side               side,
    uint16_t           tmpl_id,
    Position*          out,
    size_t             cap
);

/// battle_valid_combinations
///
/// Enumerate valid piece combinations.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
/// - uint32_t (*out_pairs)[2] -> output buffer
/// - size_t cap -> output capacity
///
/// Return:
/// size_t -> number of valid pairs written
///
size_t battle_valid_combinations(
    const BattleState* bs,
    Side               side,
    uint32_t (*out_pairs)[2],
    size_t cap
);

/// battle_card_targets
///
/// Enumerate valid targets for a card.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
/// - uint8_t hand_idx -> hand index of card
/// - TargetSpec* out -> output buffer
/// - size_t cap -> output capacity
///
/// Return:
/// size_t -> number of valid targets written
///
size_t battle_card_targets(
    const BattleState* bs,
    Side               side,
    uint8_t            hand_idx,
    TargetSpec*        out,
    size_t             cap
);

/// battle_recipe_preview
///
/// Look up a recipe for two piece templates.
///
/// Params:
/// - uint16_t a -> first ingredient template id
/// - uint16_t b -> second ingredient template id
///
/// Return:
/// const Recipe* -> recipe or NULL
///
const Recipe* battle_recipe_preview(uint16_t a, uint16_t b);

/*--------------------------------------------------------------------------*\
                              PROJECTION QUERIES
\*--------------------------------------------------------------------------*/

/// battle_query_cost
///
/// Query the base cost of a piece template.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
/// - uint16_t tmpl_id -> template id to query
///
/// Return:
/// int -> base value of template, or 0 if not found
///
int battle_query_cost(const BattleState* bs, Side side, uint16_t tmpl_id);

/// battle_query_sell_value
///
/// Query the sell value of a card.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
/// - const CardInstance* card -> card to query
///
/// Return:
/// int -> sell value of card, or 0 if card is NULL
///
int battle_query_sell_value(
    const BattleState*  bs,
    Side                side,
    const CardInstance* card
);

/// battle_query_draw_count
///
/// Query the number of cards drawn per turn.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
///
/// Return:
/// int -> draw count
///
int battle_query_draw_count(const BattleState* bs, Side side);

/// battle_query_turn_income
///
/// Query the command point income per turn.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side to query
///
/// Return:
/// int -> income per turn
///
int battle_query_turn_income(const BattleState* bs, Side side);

/// battle_projected_damage
///
/// Query projected damage output.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side attacker -> attacking side
///
/// Return:
/// int -> projected damage
///
int battle_projected_damage(const BattleState* bs, Side attacker);

/// battle_projected_flips
///
/// Query projected flip count.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side attacker -> attacking side
///
/// Return:
/// int -> projected flip count
///
int battle_projected_flips(const BattleState* bs, Side attacker);

/*--------------------------------------------------------------------------*\
                              ACTION API
\*--------------------------------------------------------------------------*/

/// battle_can_play_card
///
/// Check whether a card can be played.
///
/// Params:
/// - const BattleState* bs -> battle state to check
/// - uint8_t idx -> hand index of card
/// - const TargetSpec* tgt -> target specification
///
/// Return:
/// bool -> true if the card can be played
///
bool battle_can_play_card(
    const BattleState* bs,
    uint8_t            idx,
    const TargetSpec*  tgt
);

/// battle_play_card
///
/// Execute a card play action.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint8_t idx -> hand index of card to play
/// - const TargetSpec* tgt -> target specification
///
/// Return:
/// bool -> true if the card was played
///
bool battle_play_card(BattleState* bs, uint8_t idx, const TargetSpec* tgt);

/// battle_can_sell_card
///
/// Check whether a card can be sold.
///
/// Params:
/// - const BattleState* bs -> battle state to check
/// - uint8_t idx -> hand index of card
///
/// Return:
/// bool -> true if the card can be sold
///
bool battle_can_sell_card(const BattleState* bs, uint8_t idx);

/// battle_sell_card
///
/// Execute a card sell action.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint8_t idx -> hand index of card to sell
///
/// Return:
/// bool -> true if the card was sold
///
bool battle_sell_card(BattleState* bs, uint8_t idx);

/// battle_can_move
///
/// Check whether a piece can legally move to a destination.
///
/// Params:
/// - const BattleState* bs -> battle state to check
/// - uint32_t piece_id -> piece to move
/// - Position to -> destination square
///
/// Return:
/// bool -> true if the move is legal
///
bool battle_can_move(const BattleState* bs, uint32_t piece_id, Position to);

/// battle_action_move
///
/// Execute a move action.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint32_t piece_id -> piece to move
/// - Position to -> destination square
///
/// Return:
/// bool -> true if the move was executed
///
bool battle_action_move(BattleState* bs, uint32_t piece_id, Position to);

/// battle_can_buy
///
/// Check whether a piece can be bought at a position.
///
/// Params:
/// - const BattleState* bs -> battle state to check
/// - uint16_t tmpl_id -> template id to buy
/// - Position at -> purchase position
///
/// Return:
/// bool -> true if the purchase is legal
///
bool battle_can_buy(const BattleState* bs, uint16_t tmpl_id, Position at);

/// battle_action_buy
///
/// Execute a buy action.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint16_t tmpl_id -> template id to buy
/// - Position at -> purchase position
///
/// Return:
/// bool -> true if the purchase was executed
///
bool battle_action_buy(BattleState* bs, uint16_t tmpl_id, Position at);

/// battle_can_combine
///
/// Check whether two pieces can be combined.
///
/// Params:
/// - const BattleState* bs -> battle state to check
/// - uint32_t a -> first piece id
/// - uint32_t b -> second piece id
///
/// Return:
/// bool -> true if combination is legal
///
bool battle_can_combine(const BattleState* bs, uint32_t a, uint32_t b);

/// battle_action_combine
///
/// Execute a combine action.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint32_t a -> first piece id
/// - uint32_t b -> second piece id
///
/// Return:
/// bool -> true if combination was executed
///
bool battle_action_combine(BattleState* bs, uint32_t a, uint32_t b);

/// battle_end_player_turn
///
/// Signal end of player turn.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_end_player_turn(BattleState* bs);

/*--------------------------------------------------------------------------*\
                              EVENT LOG
\*--------------------------------------------------------------------------*/

/// battle_drain_events
///
/// Drain events from the ring buffer into the caller's buffer.
///
/// Params:
/// - BattleState* bs -> battle state to drain
/// - Event* out -> output buffer
/// - size_t cap -> output buffer capacity
///
/// Return:
/// size_t -> number of events drained
///
size_t battle_drain_events(BattleState* bs, Event* out, size_t cap);

/// battle_clear_events
///
/// Clear all events from the ring buffer.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_clear_events(BattleState* bs);

#endif /* BATTLE_H */