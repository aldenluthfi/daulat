//! battle.c
//!
//! Battle state, turn loop, resolve, cascading flips, and event log.
//! BattleState is the model the engine projects to the frontend via
//! `< STATE battle.*` lines. The resolve phase applies damage to
//! the meter, triggering cascading flips.
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
/// - BattleState* battle -> battle state to modify
/// - EventKind kind -> event type to push
///
static void push_event(BattleState* battle, EventKind kind) {
    if (battle->event_count >= MAX_EVENTS)
        return;
    uint16_t index = (battle->event_head + battle->event_count) % MAX_EVENTS;
    battle->events[index].kind    = kind;
    battle->events[index].turn_no = battle->turn_no;
    battle->event_count++;
}

/// find_king
///
/// Locate the king piece for a given side.
///
/// Params:
/// - const BattleState* battle -> battle state to search
/// - Side owner -> side to find king for
///
/// Return:
/// const PieceState* -> king piece or NULL if not found
///
static const PieceState* find_king(const BattleState* battle, Side owner) {
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        if (battle->pieces[i].owner == owner &&
            battle->pieces[i].template->id == PIECE_KING) {
            return &battle->pieces[i];
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
/// - BattleState* battle -> battle state to initialize
/// - const BattleConfig* config -> battle configuration
///
void battle_init(BattleState* battle, const BattleConfig* config) {
    memset(battle, 0, sizeof(*battle));
    battle->config          = *config;
    battle->active_side     = config->player_side;
    battle->turn_no         = 1;
    battle->max_turns       = config->max_turns;
    battle->cp[SIDE_PLAYER] = config->starting_cp;
    battle->cp[SIDE_ENEMY]  = config->starting_cp;
    board_init(&battle->board, config->width, config->height);
    bus_init(&battle->bus);
    rng_init(&battle->rng, config->rng_seed);
    battle->meter_cap[SIDE_PLAYER]          = 20;
    battle->meter_cap[SIDE_ENEMY]           = 20;
    battle->meter_overflow_cap[SIDE_PLAYER] = 40;
    battle->meter_overflow_cap[SIDE_ENEMY]  = 40;
    battle->meter[SIDE_PLAYER]              = battle->meter_cap[SIDE_PLAYER];
    battle->meter[SIDE_ENEMY]               = battle->meter_cap[SIDE_ENEMY];
    for (uint8_t i = 0; i < config->modifier_count; i++) {
        const Modifier* modifier = config->modifiers[i];
        for (uint8_t j = 0; j < modifier->effect_count; j++) {
            Effect effect         = modifier->effects[j];
            effect.owner          = SIDE_PLAYER;
            effect.duration_turns = -1;
            bus_register(&battle->bus, &effect);
        }
    }
    for (uint8_t i = 0; i < config->trait_count; i++) {
        const BoardTrait* trait = config->traits[i];
        for (uint8_t j = 0; j < trait->effect_count; j++) {
            Effect effect         = trait->effects[j];
            effect.owner          = SIDE_PLAYER;
            effect.duration_turns = -1;
            bus_register(&battle->bus, &effect);
        }
    }
    if (config->run != NULL)
        meta_apply_relics(battle, config->run);
    battle->actions_left = 3;
    push_event(battle, EVT_BATTLE_ENDED);
    struct EffectCtx context = {0};
    bus_emit(&battle->bus, battle, TRIGGER_BATTLE_START, &context);
}

/// battle_destroy
///
/// Tear down a battle. Currently a no-op.
///
/// Params:
/// - BattleState* battle -> battle state to destroy
///
void battle_destroy(BattleState* battle) {
    (void)battle;
}

/*--------------------------------------------------------------------------*\
                              TURN START
\*--------------------------------------------------------------------------*/

/// battle_turn_start
///
/// Begin a turn: tick bus, emit income/draw triggers, set actions_left.
///
/// Params:
/// - BattleState* battle -> battle state to modify
///
void battle_turn_start(BattleState* battle) {
    bus_tick_turn_start(&battle->bus);
    struct EffectCtx context = {0};
    bus_emit(&battle->bus, battle, TRIGGER_TURN_START, &context);
    int income                  = 5;
    context.as.query.income_out = &income;
    bus_emit(&battle->bus, battle, TRIGGER_QUERY_TURN_INCOME, &context);
    battle->cp[battle->active_side] += income;
    int draw_count            = 1;
    context.as.card.count_out = &draw_count;
    bus_emit(&battle->bus, battle, TRIGGER_QUERY_DRAW_COUNT, &context);
    for (int i = 0; i < draw_count; i++) {
        card_draw(battle, battle->active_side);
    }
    battle->actions_left = 3;
    push_event(battle, EVT_TURN_STARTED);
}

/*--------------------------------------------------------------------------*\
                              CASCADING FLIPS
\*--------------------------------------------------------------------------*/

/// apply_damage_with_cascading_flips
///
/// Apply damage to a side, triggering flips when meter empties.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - Side defender -> side taking damage
/// - int damage -> total damage to apply
///
static void apply_damage_with_cascading_flips(
    BattleState* battle,
    Side         defender,
    int          damage
) {
    while (damage > 0) {
        int meter_value = battle->meter[defender];
        if (meter_value <= damage) {
            damage -= meter_value;
            battle->meter[defender] = 0;
            uint16_t candidates[MAX_PIECES];
            uint16_t count = 0;
            for (uint16_t i = 0; i < battle->piece_count; i++) {
                if (battle->pieces[i].owner != defender)
                    continue;
                if (battle->pieces[i].template->id == PIECE_KING)
                    continue;
                candidates[count++] = i;
            }
            if (count > 0) {
                uint16_t pick = candidates[rng_range(&battle->rng, count)];
                uint32_t id   = battle->pieces[pick].id;
                uint16_t event_index =
                    (battle->event_head + battle->event_count) % MAX_EVENTS;
                if (battle->event_count < MAX_EVENTS) {
                    battle->events[event_index].kind    = EVT_PIECE_FLIPPED;
                    battle->events[event_index].turn_no = battle->turn_no;
                    battle->events[event_index].as.flipped.piece_id = id;
                    battle->event_count++;
                }
                /* Emit TRIGGER_RESOLVE_FLIP before actual flip for immunity effects */
                struct EffectCtx flip_ctx     = {0};
                flip_ctx.as.flipped.piece     = &battle->pieces[pick];
                flip_ctx.as.flipped.old_owner = defender;
                flip_ctx.as.flipped.new_owner = side_opposite(defender);
                flip_ctx.as.flipped.cause     = FLIPPED_METER_CASCADE;
                bus_emit(&battle->bus, battle, TRIGGER_RESOLVE_FLIP, &flip_ctx);
                /* Check if piece gained immunity from TRIGGER_RESOLVE_FLIP */
                if (!(battle->pieces[pick].flags & PSF_IMMUNE_FLIP)) {
                    piece_flip(battle, id);
                } else {
                    battle->pieces[pick].flags &= ~PSF_IMMUNE_FLIP;
                }
            }
            battle->meter[defender] = battle->meter_cap[defender];
        } else {
            battle->meter[defender] -= damage;
            damage = 0;
        }
        if (battle_check_end(battle) != BATTLE_IN_PROGRESS) {
            battle->battle_ended = true;
            push_event(battle, EVT_BATTLE_ENDED);
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
/// - BattleState* battle -> battle state to resolve
///
void battle_resolve(BattleState* battle) {
    Side             active     = battle->active_side;
    Side             passive    = side_opposite(active);
    struct EffectCtx context    = {0};
    context.as.resolve.attacker = NULL;
    context.as.resolve.target   = NULL;
    bus_emit(&battle->bus, battle, TRIGGER_RESOLVE_DEFENSE, &context);
    bus_emit(&battle->bus, battle, TRIGGER_RESOLVE_ATTACK, &context);
    int total_damage = 0;
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        PieceState* piece = &battle->pieces[i];
        if (piece->owner != active)
            continue;
        MoveList move_list = {0};
        mg_generate_threat(piece, battle, &move_list);
        for (uint8_t j = 0; j < move_list.count; j++) {
            const PieceState* threat_target =
                board_at(&battle->board, move_list.squares[j]);
            if (threat_target == NULL)
                continue;
            if (threat_target->owner == passive) {
                int piece_damage = piece_value(piece);
                total_damage += piece_damage;
                uint16_t event_index =
                    (battle->event_head + battle->event_count) % MAX_EVENTS;
                if (battle->event_count < MAX_EVENTS) {
                    battle->events[event_index].kind = EVT_PIECE_DEALT_DAMAGE;
                    battle->events[event_index].turn_no = battle->turn_no;
                    battle->events[event_index].as.dealt_damage.attacker =
                        piece->id;
                    battle->events[event_index].as.dealt_damage.victim_side =
                        passive;
                    battle->events[event_index].as.dealt_damage.dmg =
                        piece_damage;
                    battle->event_count++;
                }
            }
        }
    }
    if (total_damage > 0) {
        apply_damage_with_cascading_flips(battle, passive, total_damage);
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
/// - BattleState* battle -> battle state to modify
///
void battle_turn_end(BattleState* battle) {
    while (battle->hand_count[battle->active_side] > 0) {
        battle_sell_card(battle, 0);
    }
    push_event(battle, EVT_RESOLVE_BEGAN);
    battle_resolve(battle);
    push_event(battle, EVT_RESOLVE_ENDED);
    struct EffectCtx context = {0};
    bus_emit(&battle->bus, battle, TRIGGER_TURN_END, &context);
    bus_tick_turn_end(&battle->bus);
    push_event(battle, EVT_TURN_ENDED);
    battle->active_side = side_opposite(battle->active_side);
    if (battle->active_side == battle->config.player_side) {
        battle->turn_no++;
    }
    battle->battle_ended = (battle_check_end(battle) != BATTLE_IN_PROGRESS);
}

/// battle_check_end
///
/// Check whether the battle has reached a terminal state.
///
/// Params:
/// - const BattleState* battle -> battle state to check
///
/// Return:
/// BattleResult -> BATTLE_IN_PROGRESS, PLAYER_WON, ENEMY_WON, or DRAW
///
BattleResult battle_check_end(const BattleState* battle) {
    const PieceState* player_king =
        find_king(battle, battle->config.player_side);
    const PieceState* enemy_king =
        find_king(battle, side_opposite(battle->config.player_side));
    if (player_king == NULL)
        return BATTLE_ENEMY_WON;
    if (enemy_king == NULL)
        return BATTLE_PLAYER_WON;
    if (battle->turn_no >= battle->max_turns)
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
/// - const BattleState* battle -> battle state to check
/// - uint32_t piece_id -> piece to move
/// - Position to -> destination square
///
/// Return:
/// bool -> true if the move is legal
///
bool battle_can_move(
    const BattleState* battle,
    uint32_t           piece_id,
    Position           to
) {
    const PieceState* piece = piece_by_id((BattleState*)battle, piece_id);
    if (piece == NULL)
        return false;
    if (piece->owner != battle->active_side)
        return false;
    MoveList move_list = {0};
    mg_generate(piece, battle, &move_list);
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
/// - BattleState* battle -> battle state to modify
/// - uint32_t piece_id -> piece to move
/// - Position to -> destination square
///
/// Return:
/// bool -> true if the move was executed
///
bool battle_action_move(BattleState* battle, uint32_t piece_id, Position to) {
    if (!battle_can_move(battle, piece_id, to))
        return false;
    PieceState* piece = piece_by_id(battle, piece_id);
    Position    from  = piece->pos;
    board_remove(&battle->board, from);
    board_place(&battle->board, piece, to);
    piece->moves_used++;
    piece->flags |= PSF_HAS_MOVED;
    battle->actions_left--;
    uint16_t event_index =
        (battle->event_head + battle->event_count) % MAX_EVENTS;
    if (battle->event_count < MAX_EVENTS) {
        battle->events[event_index].kind              = EVT_PIECE_MOVED;
        battle->events[event_index].turn_no           = battle->turn_no;
        battle->events[event_index].as.moved.piece_id = piece_id;
        battle->events[event_index].as.moved.from     = from;
        battle->events[event_index].as.moved.to       = to;
        battle->event_count++;
    }
    struct EffectCtx context = {0};
    context.as.piece.piece   = piece;
    bus_emit(&battle->bus, battle, TRIGGER_PIECE_MOVED, &context);
    Side enemy = side_opposite(piece->owner);
    if (board_at(&battle->board, piece->pos)->owner == enemy) {
        bus_emit(
            &battle->bus,
            battle,
            TRIGGER_PIECE_ENTERED_ENEMY_TERR,
            &context
        );
    }
    return true;
}

/// battle_can_buy
///
/// Check whether a piece can be bought at a position.
///
/// Params:
/// - const BattleState* battle -> battle state to check
/// - uint16_t template_id -> template id to buy
/// - Position at -> purchase position
///
/// Return:
/// bool -> true if the purchase is legal
///
bool battle_can_buy(
    const BattleState* battle,
    uint16_t           template_id,
    Position           at
) {
    const PieceTemplate* template = piece_template(template_id);
    if (template == NULL)
        return false;
    if (battle->cp[battle->active_side] < template->base_value)
        return false;
    if (!pos_in_bounds(at, battle->board.width, battle->board.height))
        return false;
    if (board_at(&battle->board, at) != NULL)
        return false;
    return true;
}

/// battle_action_buy
///
/// Execute a buy action.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - uint16_t template_id -> template id to buy
/// - Position at -> purchase position
///
/// Return:
/// bool -> true if the purchase was executed
///
bool battle_action_buy(BattleState* battle, uint16_t template_id, Position at) {
    if (!battle_can_buy(battle, template_id, at))
        return false;
    const PieceTemplate* template = piece_template(template_id);
    battle->cp[battle->active_side] -= template->base_value;
    uint32_t id = piece_spawn(battle, template_id, at, battle->active_side);
    if (id == 0)
        return false;
    battle->actions_left--;
    return true;
}

/// battle_can_combine
///
/// Check whether two pieces can be combined. Always returns false.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - uint32_t ingredient_a -> first piece id (unused)
/// - uint32_t ingredient_b -> second piece id (unused)
///
/// Return:
/// bool -> always false
///
bool battle_can_combine(
    const BattleState* battle,
    uint32_t           ingredient_a,
    uint32_t           ingredient_b
) {
    (void)battle;
    (void)ingredient_a;
    (void)ingredient_b;
    return false;
}

/// battle_action_combine
///
/// Execute a combine action. Currently a no-op stub.
///
/// Params:
/// - BattleState* battle -> battle state (unused)
/// - uint32_t ingredient_a -> first piece id (unused)
/// - uint32_t ingredient_b -> second piece id (unused)
///
/// Return:
/// bool -> always false
///
bool battle_action_combine(
    BattleState* battle,
    uint32_t     ingredient_a,
    uint32_t     ingredient_b
) {
    (void)battle;
    (void)ingredient_a;
    (void)ingredient_b;
    return false;
}

/// battle_can_play_card
///
/// Check whether a card can be played.
///
/// Params:
/// - const BattleState* battle -> battle state to check
/// - uint8_t index -> hand index of card
/// - const TargetSpec* target -> target specification
///
/// Return:
/// bool -> true if the card can be played
///
bool battle_can_play_card(
    const BattleState* battle,
    uint8_t            index,
    const TargetSpec*  target
) {
    if (index >= battle->hand_count[battle->active_side])
        return false;
    const CardTemplate* template =
        battle->hand[battle->active_side][index].template;
    if (template->play_cost >= 0 &&
        battle->cp[battle->active_side] < template->play_cost)
        return false;
    (void)target;
    return true;
}

/// battle_play_card
///
/// Execute a card play action.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - uint8_t index -> hand index of card to play
/// - const TargetSpec* target -> target specification
///
/// Return:
/// bool -> true if the card was played
///
bool battle_play_card(
    BattleState*      battle,
    uint8_t           index,
    const TargetSpec* target
) {
    if (!battle_can_play_card(battle, index, target))
        return false;
    const CardTemplate* template =
        battle->hand[battle->active_side][index].template;
    battle->cp[battle->active_side] -= template->play_cost;
    for (uint8_t i = 0; i < template->play_effect_count; i++) {
        Effect effect    = template->on_play[i];
        effect.owner     = battle->active_side;
        effect.source_id = (uint32_t)template->id;
        bus_register(&battle->bus, &effect);
    }
    card_remove_from_hand(battle, battle->active_side, index);
    battle->actions_left--;
    return true;
}

/// battle_can_sell_card
///
/// Check whether a card can be sold.
///
/// Params:
/// - const BattleState* battle -> battle state to check
/// - uint8_t index -> hand index of card
///
/// Return:
/// bool -> true if the card can be sold
///
bool battle_can_sell_card(const BattleState* battle, uint8_t index) {
    return index < battle->hand_count[battle->active_side];
}

/// battle_sell_card
///
/// Execute a card sell action.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - uint8_t index -> hand index of card to sell
///
/// Return:
/// bool -> true if the card was sold
///
bool battle_sell_card(BattleState* battle, uint8_t index) {
    if (!battle_can_sell_card(battle, index))
        return false;
    const CardTemplate* template =
        battle->hand[battle->active_side][index].template;
    battle->cp[battle->active_side] += template->sell_value;
    for (uint8_t i = 0; i < template->sell_effect_count; i++) {
        Effect effect    = template->on_sell[i];
        effect.owner     = battle->active_side;
        effect.source_id = (uint32_t)template->id;
        bus_register(&battle->bus, &effect);
    }
    card_remove_from_hand(battle, battle->active_side, index);
    battle->actions_left--;
    return true;
}

/// battle_end_player_turn
///
/// Signal end of player turn. Currently a no-op.
///
/// Params:
/// - BattleState* battle -> battle state (unused)
///
void battle_end_player_turn(BattleState* battle) {
    (void)battle;
}

/*--------------------------------------------------------------------------*\
                              ACCESSORS
\*--------------------------------------------------------------------------*/

/// battle_piece_at
///
/// Get the piece at a board position.
///
/// Params:
/// - const BattleState* battle -> battle state to query
/// - Position position -> board position
///
/// Return:
/// const PieceState* -> piece at position or NULL
///
const PieceState*
battle_piece_at(const BattleState* battle, Position position) {
    return board_at(&battle->board, position);
}

/// battle_piece_by_id
///
/// Get a piece by its runtime id.
///
/// Params:
/// - const BattleState* battle -> battle state to search
/// - uint32_t id -> runtime piece id
///
/// Return:
/// const PieceState* -> piece or NULL if not found
///
const PieceState* battle_piece_by_id(const BattleState* battle, uint32_t id) {
    return piece_by_id((BattleState*)battle, id);
}

/// battle_pieces
///
/// Collect all pieces belonging to a side.
///
/// Params:
/// - const BattleState* battle -> battle state to query
/// - Side side -> side to collect
/// - const PieceState** out -> output array
/// - size_t capacity -> output array capacity
///
/// Return:
/// size_t -> number of pieces written
///
size_t battle_pieces(
    const BattleState* battle,
    Side               side,
    const PieceState** out,
    size_t             capacity
) {
    size_t count = 0;
    for (uint16_t i = 0; i < battle->piece_count && count < capacity; i++) {
        if (battle->pieces[i].owner == side)
            out[count++] = &battle->pieces[i];
    }
    return count;
}

/// battle_territory
///
/// Get the territory owner at a position.
///
/// Params:
/// - const BattleState* battle -> battle state to query
/// - Position position -> board position
///
/// Return:
/// Side -> SIDE_PLAYER, SIDE_ENEMY, or SIDE_NEUTRAL
///
Side battle_territory(const BattleState* battle, Position position) {
    const PieceState* p2 = board_at(&battle->board, position);
    if (p2 != NULL)
        return p2->owner;
    return SIDE_NEUTRAL;
}

/// battle_threat_count
///
/// Count how many pieces of a side threaten a position.
///
/// Params:
/// - const BattleState* battle -> battle state to query
/// - Position position -> board position
/// - Side attacker -> attacking side
///
/// Return:
/// int -> number of threatening pieces
///
int battle_threat_count(
    const BattleState* battle,
    Position           position,
    Side               attacker
) {
    int count = 0;
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        const PieceState* piece = &battle->pieces[i];
        if (piece->owner != attacker)
            continue;
        MoveList move_list = {0};
        mg_generate_threat(piece, battle, &move_list);
        for (uint8_t j = 0; j < move_list.count; j++) {
            if (pos_equal(move_list.squares[j], position))
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
/// - const BattleState* battle -> battle state to query
/// - Side side -> side to collect
/// - const CardInstance** out -> output array
/// - size_t capacity -> output array capacity
///
/// Return:
/// size_t -> number of cards written
///
size_t battle_hand(
    const BattleState*   battle,
    Side                 side,
    const CardInstance** out,
    size_t               capacity
) {
    size_t count = battle->hand_count[side];
    if (count > capacity)
        count = capacity;
    for (size_t i = 0; i < count; i++)
        out[i] = &battle->hand[side][i];
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
/// - const BattleState* battle -> battle state to query
/// - uint32_t piece_id -> piece to query
/// - MoveList* out -> output buffer
///
/// Return:
/// size_t -> number of legal moves
///
size_t battle_legal_moves(
    const BattleState* battle,
    uint32_t           piece_id,
    MoveList*          out
) {
    const PieceState* piece = piece_by_id((BattleState*)battle, piece_id);
    if (piece == NULL) {
        out->count = 0;
        return 0;
    }
    mg_generate(piece, battle, out);
    return out->count;
}

/// battle_valid_buy_squares
///
/// Enumerate valid purchase squares. Currently returns 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
/// - uint16_t template_id -> template id (unused)
/// - Position* out -> output buffer (unused)
/// - size_t capacity -> output capacity (unused)
///
/// Return:
/// size_t -> always 0
///
size_t battle_valid_buy_squares(
    const BattleState* battle,
    Side               side,
    uint16_t           template_id,
    Position*          out,
    size_t             capacity
) {
    (void)battle;
    (void)side;
    (void)template_id;
    (void)out;
    (void)capacity;
    return 0;
}

/// battle_valid_combinations
///
/// Enumerate valid piece combinations. Currently returns 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
/// - uint32_t (*out_pairs)[2] -> output buffer (unused)
/// - size_t capacity -> output capacity (unused)
///
/// Return:
/// size_t -> always 0
///
size_t battle_valid_combinations(
    const BattleState* battle,
    Side               side,
    uint32_t (*out_pairs)[2],
    size_t capacity
) {
    (void)battle;
    (void)side;
    (void)out_pairs;
    (void)capacity;
    return 0;
}

/// battle_card_targets
///
/// Enumerate valid targets for a card. Currently returns 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
/// - uint8_t hand_index -> hand index (unused)
/// - TargetSpec* out -> output buffer (unused)
/// - size_t capacity -> output capacity (unused)
///
/// Return:
/// size_t -> always 0
///
size_t battle_card_targets(
    const BattleState* battle,
    Side               side,
    uint8_t            hand_index,
    TargetSpec*        out,
    size_t             capacity
) {
    (void)battle;
    (void)side;
    (void)hand_index;
    (void)out;
    (void)capacity;
    return 0;
}

/// battle_recipe_preview
///
/// Look up a recipe for two piece templates.
///
/// Params:
/// - uint16_t ingredient_a -> first ingredient template id
/// - uint16_t ingredient_b -> second ingredient template id
///
/// Return:
/// const Recipe* -> recipe or NULL
///
const Recipe*
battle_recipe_preview(uint16_t ingredient_a, uint16_t ingredient_b) {
    return recipe_find(ingredient_a, ingredient_b);
}

/*--------------------------------------------------------------------------*\
                              PROJECTION QUERIES
\*--------------------------------------------------------------------------*/

/// battle_query_cost
///
/// Query the base cost of a piece template.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
/// - uint16_t template_id -> template id to query
///
/// Return:
/// int -> base value of template, or 0 if not found
///
int battle_query_cost(
    const BattleState* battle,
    Side               side,
    uint16_t           template_id
) {
    (void)battle;
    (void)side;
    const PieceTemplate* template = piece_template(template_id);
    if (template == NULL)
        return 0;
    return template->base_value;
}

/// battle_query_sell_value
///
/// Query the sell value of a card.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
/// - const CardInstance* card -> card to query
///
/// Return:
/// int -> sell value of card, or 0 if card is NULL
///
int battle_query_sell_value(
    const BattleState*  battle,
    Side                side,
    const CardInstance* card
) {
    (void)battle;
    (void)side;
    if (card == NULL)
        return 0;
    return card->template->sell_value;
}

/// battle_query_draw_count
///
/// Query the number of cards drawn per turn.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
///
/// Return:
/// int -> always 1
///
int battle_query_draw_count(const BattleState* battle, Side side) {
    (void)battle;
    (void)side;
    return 1;
}

/// battle_query_turn_income
///
/// Query the command point income per turn.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side side -> side (unused)
///
/// Return:
/// int -> always 5
///
int battle_query_turn_income(const BattleState* battle, Side side) {
    (void)battle;
    (void)side;
    return 5;
}

/// battle_projected_damage
///
/// Query projected damage output. Currently returns 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side attacker -> attacking side (unused)
///
/// Return:
/// int -> always 0
///
int battle_projected_damage(const BattleState* battle, Side attacker) {
    (void)battle;
    (void)attacker;
    return 0;
}

/// battle_projected_flips
///
/// Query projected flip count. Currently returns 0.
///
/// Params:
/// - const BattleState* battle -> battle state (unused)
/// - Side attacker -> attacking side (unused)
///
/// Return:
/// int -> always 0
///
int battle_projected_flips(const BattleState* battle, Side attacker) {
    (void)battle;
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
/// - BattleState* battle -> battle state to drain
/// - Event* out -> output buffer
/// - size_t capacity -> output buffer capacity
///
/// Return:
/// size_t -> number of events drained
///
size_t battle_drain_events(BattleState* battle, Event* out, size_t capacity) {
    size_t count =
        (battle->event_count < capacity) ? battle->event_count : capacity;
    for (size_t i = 0; i < count; i++) {
        uint16_t event_index = (battle->event_head + i) % MAX_EVENTS;
        out[i]               = battle->events[event_index];
    }
    battle->event_head = (battle->event_head + count) % MAX_EVENTS;
    battle->event_count -= count;
    return count;
}

/// battle_clear_events
///
/// Clear all events from the ring buffer.
///
/// Params:
/// - BattleState* battle -> battle state to modify
///
void battle_clear_events(BattleState* battle) {
    battle->event_head  = 0;
    battle->event_count = 0;
}
