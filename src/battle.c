//! battle.c
//!
//! Battle state, turn loop, resolve, cascading flips, and event log.
//! BattleState is the model consumed by the SDL3 front-end.
//! Resolve phase applies damage to meter, triggering cascading flips.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"
#include <string.h>

/*--------------------------------------------------------------------------*\
                              STATIC HELPERS
\*--------------------------------------------------------------------------*/

/// push_event
///
/// Append an event to the ring buffer.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - EventKind kind -> event type to push
///
static void push_event(BattleState* bs, EventKind kind) {
    if (bs->event_count >= MAX_EVENTS)
        return;
    uint16_t idx            = (bs->event_head + bs->event_count) % MAX_EVENTS;
    bs->events[idx].kind    = kind;
    bs->events[idx].turn_no = bs->turn_no;
    bs->event_count++;
}

/// find_king
///
/// Locate the king piece for a given side.
///
/// Params:
/// - const BattleState* bs -> battle state to search
/// - Side owner -> side to find king for
///
/// Return:
/// const PieceState* -> king piece or NULL if not found
///
static const PieceState* find_king(const BattleState* bs, Side owner) {
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        if (bs->pieces[i].owner == owner &&
            bs->pieces[i].tmpl->id == PIECE_KING) {
            return &bs->pieces[i];
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

/// battle_init
///
/// Initialize a battle from configuration. Seeds RNG, inits board and bus.
///
/// Params:
/// - BattleState* bs -> battle state to initialize
/// - const BattleConfig* cfg -> battle configuration
///
void battle_init(BattleState* bs, const BattleConfig* cfg) {
    memset(bs, 0, sizeof(*bs));
    bs->config          = *cfg;
    bs->active_side     = cfg->player_side;
    bs->turn_no         = 1;
    bs->max_turns       = cfg->max_turns;
    bs->cp[SIDE_PLAYER] = cfg->starting_cp;
    bs->cp[SIDE_ENEMY]  = cfg->starting_cp;
    board_init(&bs->board, cfg->width, cfg->height);
    bus_init(&bs->bus);
    rng_init(&bs->rng, cfg->rng_seed);
    bs->meter_cap[SIDE_PLAYER]          = 20;
    bs->meter_cap[SIDE_ENEMY]           = 20;
    bs->meter_overflow_cap[SIDE_PLAYER] = 40;
    bs->meter_overflow_cap[SIDE_ENEMY]  = 40;
    bs->meter[SIDE_PLAYER]              = bs->meter_cap[SIDE_PLAYER];
    bs->meter[SIDE_ENEMY]               = bs->meter_cap[SIDE_ENEMY];
    for (uint8_t i = 0; i < cfg->modifier_count; i++) {
        const Modifier* modifier = cfg->modifiers[i];
        for (uint8_t j = 0; j < modifier->effect_count; j++) {
            Effect effect         = modifier->effects[j];
            effect.owner          = SIDE_PLAYER;
            effect.duration_turns = -1;
            bus_register(&bs->bus, &effect);
        }
    }
    for (uint8_t i = 0; i < cfg->trait_count; i++) {
        const BoardTrait* trait = cfg->traits[i];
        for (uint8_t j = 0; j < trait->effect_count; j++) {
            Effect effect         = trait->effects[j];
            effect.owner          = SIDE_PLAYER;
            effect.duration_turns = -1;
            bus_register(&bs->bus, &effect);
        }
    }
    if (cfg->run != NULL) {
        for (uint8_t i = 0; i < cfg->run->relic_count; i++) {
            const RelicTemplate* relic = cfg->run->relics[i];
            for (uint8_t j = 0; j < relic->effect_count; j++) {
                Effect effect         = relic->effects[j];
                effect.owner          = SIDE_PLAYER;
                effect.duration_turns = -1;
                bus_register(&bs->bus, &effect);
            }
        }
    }
    bs->actions_left = 3;
    push_event(bs, EVT_BATTLE_ENDED);
    struct EffectCtx ctx = {0};
    bus_emit(&bs->bus, bs, TRIGGER_BATTLE_START, &ctx);
}

/// battle_destroy
///
/// Tear down a battle. Currently a no-op.
///
/// Params:
/// - BattleState* bs -> battle state to destroy
///
void battle_destroy(BattleState* bs) {
    (void)bs;
}

/*--------------------------------------------------------------------------*\
                              TURN START
\*--------------------------------------------------------------------------*/

/// battle_turn_start
///
/// Begin a turn: tick bus, emit income/draw triggers, set actions_left.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_turn_start(BattleState* bs) {
    bus_tick_turn_start(&bs->bus);
    struct EffectCtx ctx = {0};
    bus_emit(&bs->bus, bs, TRIGGER_TURN_START, &ctx);
    int income              = 5;
    ctx.as.query.income_out = &income;
    bus_emit(&bs->bus, bs, TRIGGER_QUERY_TURN_INCOME, &ctx);
    bs->cp[bs->active_side] += income;
    int draw_count        = 1;
    ctx.as.card.count_out = &draw_count;
    bus_emit(&bs->bus, bs, TRIGGER_QUERY_DRAW_COUNT, &ctx);
    for (int i = 0; i < draw_count; i++) {
        card_draw(bs, bs->active_side);
    }
    bs->actions_left = 3;
    push_event(bs, EVT_TURN_STARTED);
}

/*--------------------------------------------------------------------------*\
                              CASCADING FLIPS
\*--------------------------------------------------------------------------*/

/// apply_damage_with_cascading_flips
///
/// Apply damage to a side, triggering flips when meter empties.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - Side defender -> side taking damage
/// - int damage -> total damage to apply
///
static void
apply_damage_with_cascading_flips(BattleState* bs, Side defender, int damage) {
    while (damage > 0) {
        int meter_value = bs->meter[defender];
        if (meter_value <= damage) {
            damage -= meter_value;
            bs->meter[defender] = 0;
            uint16_t candidates[MAX_PIECES];
            uint16_t count = 0;
            for (uint16_t i = 0; i < bs->piece_count; i++) {
                if (bs->pieces[i].owner != defender)
                    continue;
                if (bs->pieces[i].tmpl->id == PIECE_KING)
                    continue;
                candidates[count++] = i;
            }
            if (count > 0) {
                uint16_t pick = candidates[rng_range(&bs->rng, count)];
                uint32_t id   = bs->pieces[pick].id;
                uint16_t event_index =
                    (bs->event_head + bs->event_count) % MAX_EVENTS;
                if (bs->event_count < MAX_EVENTS) {
                    bs->events[event_index].kind    = EVT_PIECE_FLIPPED;
                    bs->events[event_index].turn_no = bs->turn_no;
                    bs->events[event_index].as.flipped.piece_id = id;
                    bs->event_count++;
                }
                piece_flip(bs, id);
            }
            bs->meter[defender] = bs->meter_cap[defender];
        } else {
            bs->meter[defender] -= damage;
            damage = 0;
        }
        if (battle_check_end(bs) != BATTLE_IN_PROGRESS) {
            bs->battle_ended = true;
            push_event(bs, EVT_BATTLE_ENDED);
            return;
        }
    }
}

/*--------------------------------------------------------------------------*\
                              RESOLVE
\*--------------------------------------------------------------------------*/

/// battle_resolve
///
/// Resolve phase: emit defense/attack triggers, deal damage, cascade flips.
///
/// Params:
/// - BattleState* bs -> battle state to resolve
///
void battle_resolve(BattleState* bs) {
    Side             active  = bs->active_side;
    Side             passive = side_opposite(active);
    struct EffectCtx ctx     = {0};
    ctx.as.resolve.attacker  = NULL;
    ctx.as.resolve.target    = NULL;
    bus_emit(&bs->bus, bs, TRIGGER_RESOLVE_DEFENSE, &ctx);
    bus_emit(&bs->bus, bs, TRIGGER_RESOLVE_ATTACK, &ctx);
    int total_damage = 0;
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        PieceState* piece = &bs->pieces[i];
        if (piece->owner != active)
            continue;
        MoveList move_list = {0};
        mg_generate_threat(piece, bs, &move_list);
        for (uint8_t j = 0; j < move_list.count; j++) {
            const PieceState* threat_target =
                board_at(&bs->board, move_list.squares[j]);
            if (threat_target == NULL)
                continue;
            if (threat_target->owner == passive) {
                int piece_damage = piece_value(piece);
                total_damage += piece_damage;
                uint16_t event_index =
                    (bs->event_head + bs->event_count) % MAX_EVENTS;
                if (bs->event_count < MAX_EVENTS) {
                    bs->events[event_index].kind    = EVT_PIECE_DEALT_DAMAGE;
                    bs->events[event_index].turn_no = bs->turn_no;
                    bs->events[event_index].as.dealt_damage.attacker =
                        piece->id;
                    bs->events[event_index].as.dealt_damage.victim_side =
                        passive;
                    bs->events[event_index].as.dealt_damage.dmg = piece_damage;
                    bs->event_count++;
                }
            }
        }
    }
    if (total_damage > 0) {
        apply_damage_with_cascading_flips(bs, passive, total_damage);
    }
}

/*--------------------------------------------------------------------------*\
                              TURN END
\*--------------------------------------------------------------------------*/

/// battle_turn_end
///
/// End a turn: sell hand, resolve, tick bus, switch active side.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_turn_end(BattleState* bs) {
    while (bs->hand_count[bs->active_side] > 0) {
        battle_sell_card(bs, 0);
    }
    push_event(bs, EVT_RESOLVE_BEGAN);
    battle_resolve(bs);
    push_event(bs, EVT_RESOLVE_ENDED);
    struct EffectCtx ctx = {0};
    bus_emit(&bs->bus, bs, TRIGGER_TURN_END, &ctx);
    bus_tick_turn_end(&bs->bus);
    push_event(bs, EVT_TURN_ENDED);
    bs->active_side = side_opposite(bs->active_side);
    if (bs->active_side == bs->config.player_side) {
        bs->turn_no++;
    }
    bs->battle_ended = (battle_check_end(bs) != BATTLE_IN_PROGRESS);
}

/// battle_check_end
///
/// Check whether the battle has reached a terminal state.
///
/// Params:
/// - const BattleState* bs -> battle state to check
///
/// Return:
/// BattleResult -> BATTLE_IN_PROGRESS, PLAYER_WON, ENEMY_WON, or DRAW
///
BattleResult battle_check_end(const BattleState* bs) {
    const PieceState* player_king = find_king(bs, bs->config.player_side);
    const PieceState* enemy_king =
        find_king(bs, side_opposite(bs->config.player_side));
    if (player_king == NULL)
        return BATTLE_ENEMY_WON;
    if (enemy_king == NULL)
        return BATTLE_PLAYER_WON;
    if (bs->turn_no >= bs->max_turns)
        return BATTLE_DRAW;
    return BATTLE_IN_PROGRESS;
}

/*--------------------------------------------------------------------------*\
                              ACTION API
\*--------------------------------------------------------------------------*/

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
bool battle_can_move(const BattleState* bs, uint32_t piece_id, Position to) {
    const PieceState* piece = piece_by_id((BattleState*)bs, piece_id);
    if (piece == NULL)
        return false;
    if (piece->owner != bs->active_side)
        return false;
    MoveList move_list = {0};
    mg_generate(piece, bs, &move_list);
    for (uint8_t i = 0; i < move_list.count; i++) {
        if (pos_equal(move_list.squares[i], to))
            return true;
    }
    return false;
}

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
bool battle_action_move(BattleState* bs, uint32_t piece_id, Position to) {
    if (!battle_can_move(bs, piece_id, to))
        return false;
    PieceState* piece = piece_by_id(bs, piece_id);
    Position    from  = piece->pos;
    board_remove(&bs->board, from);
    board_place(&bs->board, piece, to);
    piece->moves_used++;
    piece->flags |= PSF_HAS_MOVED;
    bs->actions_left--;
    uint16_t event_index = (bs->event_head + bs->event_count) % MAX_EVENTS;
    if (bs->event_count < MAX_EVENTS) {
        bs->events[event_index].kind              = EVT_PIECE_MOVED;
        bs->events[event_index].turn_no           = bs->turn_no;
        bs->events[event_index].as.moved.piece_id = piece_id;
        bs->events[event_index].as.moved.from     = from;
        bs->events[event_index].as.moved.to       = to;
        bs->event_count++;
    }
    struct EffectCtx ctx = {0};
    ctx.as.piece.piece   = piece;
    bus_emit(&bs->bus, bs, TRIGGER_PIECE_MOVED, &ctx);
    Side enemy = side_opposite(piece->owner);
    if (board_at(&bs->board, piece->pos)->owner == enemy) {
        bus_emit(&bs->bus, bs, TRIGGER_PIECE_ENTERED_ENEMY_TERR, &ctx);
    }
    return true;
}

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
bool battle_can_buy(const BattleState* bs, uint16_t tmpl_id, Position at) {
    const PieceTemplate* tmpl = piece_template(tmpl_id);
    if (tmpl == NULL)
        return false;
    if (bs->cp[bs->active_side] < tmpl->base_value)
        return false;
    if (!pos_in_bounds(at, bs->board.width, bs->board.height))
        return false;
    if (board_at(&bs->board, at) != NULL)
        return false;
    return true;
}

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
bool battle_action_buy(BattleState* bs, uint16_t tmpl_id, Position at) {
    if (!battle_can_buy(bs, tmpl_id, at))
        return false;
    const PieceTemplate* tmpl = piece_template(tmpl_id);
    bs->cp[bs->active_side] -= tmpl->base_value;
    uint32_t id = piece_spawn(bs, tmpl_id, at, bs->active_side);
    if (id == 0)
        return false;
    bs->actions_left--;
    return true;
}

/// battle_can_combine
///
/// Check whether two pieces can be combined. Always returns false.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - uint32_t a -> first piece id (unused)
/// - uint32_t b -> second piece id (unused)
///
/// Return:
/// bool -> always false
///
bool battle_can_combine(const BattleState* bs, uint32_t a, uint32_t b) {
    (void)bs;
    (void)a;
    (void)b;
    return false;
}

/// battle_action_combine
///
/// Execute a combine action. Currently a no-op stub.
///
/// Params:
/// - BattleState* bs -> battle state (unused)
/// - uint32_t a -> first piece id (unused)
/// - uint32_t b -> second piece id (unused)
///
/// Return:
/// bool -> always false
///
bool battle_action_combine(BattleState* bs, uint32_t a, uint32_t b) {
    (void)bs;
    (void)a;
    (void)b;
    return false;
}

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
) {
    if (idx >= bs->hand_count[bs->active_side])
        return false;
    const CardTemplate* tmpl = bs->hand[bs->active_side][idx].tmpl;
    if (tmpl->play_cost >= 0 && bs->cp[bs->active_side] < tmpl->play_cost)
        return false;
    (void)tgt;
    return true;
}

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
bool battle_play_card(BattleState* bs, uint8_t idx, const TargetSpec* tgt) {
    if (!battle_can_play_card(bs, idx, tgt))
        return false;
    const CardTemplate* tmpl = bs->hand[bs->active_side][idx].tmpl;
    bs->cp[bs->active_side] -= tmpl->play_cost;
    for (uint8_t i = 0; i < tmpl->play_effect_count; i++) {
        Effect effect    = tmpl->on_play[i];
        effect.owner     = bs->active_side;
        effect.source_id = (uint32_t)tmpl->id;
        bus_register(&bs->bus, &effect);
    }
    card_remove_from_hand(bs, bs->active_side, idx);
    bs->actions_left--;
    return true;
}

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
bool battle_can_sell_card(const BattleState* bs, uint8_t idx) {
    return idx < bs->hand_count[bs->active_side];
}

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
bool battle_sell_card(BattleState* bs, uint8_t idx) {
    if (!battle_can_sell_card(bs, idx))
        return false;
    const CardTemplate* tmpl = bs->hand[bs->active_side][idx].tmpl;
    bs->cp[bs->active_side] += tmpl->sell_value;
    for (uint8_t i = 0; i < tmpl->sell_effect_count; i++) {
        Effect effect    = tmpl->on_sell[i];
        effect.owner     = bs->active_side;
        effect.source_id = (uint32_t)tmpl->id;
        bus_register(&bs->bus, &effect);
    }
    card_remove_from_hand(bs, bs->active_side, idx);
    bs->actions_left--;
    return true;
}

/// battle_end_player_turn
///
/// Signal end of player turn. Currently a no-op.
///
/// Params:
/// - BattleState* bs -> battle state (unused)
///
void battle_end_player_turn(BattleState* bs) {
    (void)bs;
}

/*--------------------------------------------------------------------------*\
                              ACCESSORS
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
const PieceState* battle_piece_at(const BattleState* bs, Position p) {
    return board_at(&bs->board, p);
}

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
const PieceState* battle_piece_by_id(const BattleState* bs, uint32_t id) {
    return piece_by_id((BattleState*)bs, id);
}

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
) {
    size_t count = 0;
    for (uint16_t i = 0; i < bs->piece_count && count < cap; i++) {
        if (bs->pieces[i].owner == side)
            out[count++] = &bs->pieces[i];
    }
    return count;
}

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
Side battle_territory(const BattleState* bs, Position p) {
    const PieceState* p2 = board_at(&bs->board, p);
    if (p2 != NULL)
        return p2->owner;
    return SIDE_NEUTRAL;
}

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
int battle_threat_count(const BattleState* bs, Position p, Side attacker) {
    int count = 0;
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        const PieceState* piece = &bs->pieces[i];
        if (piece->owner != attacker)
            continue;
        MoveList move_list = {0};
        mg_generate_threat(piece, bs, &move_list);
        for (uint8_t j = 0; j < move_list.count; j++) {
            if (pos_equal(move_list.squares[j], p))
                count++;
        }
    }
    return count;
}

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
) {
    size_t count = bs->hand_count[side];
    if (count > cap)
        count = cap;
    for (size_t i = 0; i < count; i++)
        out[i] = &bs->hand[side][i];
    return count;
}

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
const CardTemplate* battle_card_tmpl(uint16_t id) {
    return card_template(id);
}

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
const PieceTemplate* battle_piece_tmpl(uint16_t id) {
    return piece_template(id);
}

/*--------------------------------------------------------------------------*\
                              AFFORDANCE
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
battle_legal_moves(const BattleState* bs, uint32_t piece_id, MoveList* out) {
    const PieceState* piece = piece_by_id((BattleState*)bs, piece_id);
    if (piece == NULL) {
        out->count = 0;
        return 0;
    }
    mg_generate(piece, bs, out);
    return out->count;
}

/// battle_valid_buy_squares
///
/// Enumerate valid purchase squares. Currently returns 0.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
/// - uint16_t tmpl_id -> template id (unused)
/// - Position* out -> output buffer (unused)
/// - size_t cap -> output capacity (unused)
///
/// Return:
/// size_t -> always 0
///
size_t battle_valid_buy_squares(
    const BattleState* bs,
    Side               side,
    uint16_t           tmpl_id,
    Position*          out,
    size_t             cap
) {
    (void)bs;
    (void)side;
    (void)tmpl_id;
    (void)out;
    (void)cap;
    return 0;
}

/// battle_valid_combinations
///
/// Enumerate valid piece combinations. Currently returns 0.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
/// - uint32_t (*out_pairs)[2] -> output buffer (unused)
/// - size_t cap -> output capacity (unused)
///
/// Return:
/// size_t -> always 0
///
size_t battle_valid_combinations(
    const BattleState* bs,
    Side               side,
    uint32_t (*out_pairs)[2],
    size_t cap
) {
    (void)bs;
    (void)side;
    (void)out_pairs;
    (void)cap;
    return 0;
}

/// battle_card_targets
///
/// Enumerate valid targets for a card. Currently returns 0.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
/// - uint8_t hand_idx -> hand index (unused)
/// - TargetSpec* out -> output buffer (unused)
/// - size_t cap -> output capacity (unused)
///
/// Return:
/// size_t -> always 0
///
size_t battle_card_targets(
    const BattleState* bs,
    Side               side,
    uint8_t            hand_idx,
    TargetSpec*        out,
    size_t             cap
) {
    (void)bs;
    (void)side;
    (void)hand_idx;
    (void)out;
    (void)cap;
    return 0;
}

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
const Recipe* battle_recipe_preview(uint16_t a, uint16_t b) {
    return recipe_find(a, b);
}

/*--------------------------------------------------------------------------*\
                              PROJECTION QUERIES
\*--------------------------------------------------------------------------*/

/// battle_query_cost
///
/// Query the base cost of a piece template.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
/// - uint16_t tmpl_id -> template id to query
///
/// Return:
/// int -> base value of template, or 0 if not found
///
int battle_query_cost(const BattleState* bs, Side side, uint16_t tmpl_id) {
    (void)bs;
    (void)side;
    const PieceTemplate* tmpl = piece_template(tmpl_id);
    if (tmpl == NULL)
        return 0;
    return tmpl->base_value;
}

/// battle_query_sell_value
///
/// Query the sell value of a card.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
/// - const CardInstance* card -> card to query
///
/// Return:
/// int -> sell value of card, or 0 if card is NULL
///
int battle_query_sell_value(
    const BattleState*  bs,
    Side                side,
    const CardInstance* card
) {
    (void)bs;
    (void)side;
    if (card == NULL)
        return 0;
    return card->tmpl->sell_value;
}

/// battle_query_draw_count
///
/// Query the number of cards drawn per turn.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
///
/// Return:
/// int -> always 1
///
int battle_query_draw_count(const BattleState* bs, Side side) {
    (void)bs;
    (void)side;
    return 1;
}

/// battle_query_turn_income
///
/// Query the command point income per turn.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side side -> side (unused)
///
/// Return:
/// int -> always 5
///
int battle_query_turn_income(const BattleState* bs, Side side) {
    (void)bs;
    (void)side;
    return 5;
}

/// battle_projected_damage
///
/// Query projected damage output. Currently returns 0.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side attacker -> attacking side (unused)
///
/// Return:
/// int -> always 0
///
int battle_projected_damage(const BattleState* bs, Side attacker) {
    (void)bs;
    (void)attacker;
    return 0;
}

/// battle_projected_flips
///
/// Query projected flip count. Currently returns 0.
///
/// Params:
/// - const BattleState* bs -> battle state (unused)
/// - Side attacker -> attacking side (unused)
///
/// Return:
/// int -> always 0
///
int battle_projected_flips(const BattleState* bs, Side attacker) {
    (void)bs;
    (void)attacker;
    return 0;
}

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
size_t battle_drain_events(BattleState* bs, Event* out, size_t cap) {
    size_t count = (bs->event_count < cap) ? bs->event_count : cap;
    for (size_t i = 0; i < count; i++) {
        uint16_t event_index = (bs->event_head + i) % MAX_EVENTS;
        out[i]               = bs->events[event_index];
    }
    bs->event_head = (bs->event_head + count) % MAX_EVENTS;
    bs->event_count -= count;
    return count;
}

/// battle_clear_events
///
/// Clear all events from the ring buffer.
///
/// Params:
/// - BattleState* bs -> battle state to modify
///
void battle_clear_events(BattleState* bs) {
    bs->event_head  = 0;
    bs->event_count = 0;
}
