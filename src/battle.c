//!
//! Battle state, turn loop, resolve, cascading flips, and event log.
//! BattleState is the model consumed by the SDL3 front-end.
//! Resolve phase applies damage to meter, triggering cascading flips.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"
#include <string.h>

/*--------------------------------------------------------------------------*\
                              STATIC HELPERS
\*--------------------------------------------------------------------------*/

static void push_event(BattleState* bs, EventKind kind) {
    if (bs->event_count >= MAX_EVENTS)
        return;
    uint16_t idx            = (bs->event_head + bs->event_count) % MAX_EVENTS;
    bs->events[idx].kind    = kind;
    bs->events[idx].turn_no = bs->turn_no;
    bs->event_count++;
}

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
        const Modifier* m = cfg->modifiers[i];
        for (uint8_t j = 0; j < m->effect_count; j++) {
            Effect e         = m->effects[j];
            e.owner          = SIDE_PLAYER;
            e.duration_turns = -1;
            bus_register(&bs->bus, &e);
        }
    }
    for (uint8_t i = 0; i < cfg->trait_count; i++) {
        const BoardTrait* t = cfg->traits[i];
        for (uint8_t j = 0; j < t->effect_count; j++) {
            Effect e         = t->effects[j];
            e.owner          = SIDE_PLAYER;
            e.duration_turns = -1;
            bus_register(&bs->bus, &e);
        }
    }
    if (cfg->run != NULL) {
        for (uint8_t i = 0; i < cfg->run->relic_count; i++) {
            const RelicTemplate* r = cfg->run->relics[i];
            for (uint8_t j = 0; j < r->effect_count; j++) {
                Effect e         = r->effects[j];
                e.owner          = SIDE_PLAYER;
                e.duration_turns = -1;
                bus_register(&bs->bus, &e);
            }
        }
    }
    bs->actions_left = 3;
    push_event(bs, EVT_BATTLE_ENDED);
    struct EffectCtx ctx = {0};
    bus_emit(&bs->bus, bs, TRIGGER_BATTLE_START, &ctx);
}

void battle_destroy(BattleState* bs) {
    (void)bs;
}

/*--------------------------------------------------------------------------*\
                              TURN START
\*--------------------------------------------------------------------------*/

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

static void
apply_damage_with_cascading_flips(BattleState* bs, Side defender, int dmg) {
    while (dmg > 0) {
        int m = bs->meter[defender];
        if (m <= dmg) {
            dmg -= m;
            bs->meter[defender] = 0;
            uint16_t candidates[MAX_PIECES];
            uint16_t n = 0;
            for (uint16_t i = 0; i < bs->piece_count; i++) {
                if (bs->pieces[i].owner != defender)
                    continue;
                if (bs->pieces[i].tmpl->id == PIECE_KING)
                    continue;
                candidates[n++] = i;
            }
            if (n > 0) {
                uint16_t pick = candidates[rng_range(&bs->rng, n)];
                uint32_t id   = bs->pieces[pick].id;
                uint16_t idx  = (bs->event_head + bs->event_count) % MAX_EVENTS;
                if (bs->event_count < MAX_EVENTS) {
                    bs->events[idx].kind                = EVT_PIECE_FLIPPED;
                    bs->events[idx].turn_no             = bs->turn_no;
                    bs->events[idx].as.flipped.piece_id = id;
                    bs->event_count++;
                }
                piece_flip(bs, id);
            }
            bs->meter[defender] = bs->meter_cap[defender];
        } else {
            bs->meter[defender] -= dmg;
            dmg = 0;
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
        PieceState* p = &bs->pieces[i];
        if (p->owner != active)
            continue;
        MoveList threats = {0};
        mg_generate_threat(p, bs, &threats);
        for (uint8_t j = 0; j < threats.count; j++) {
            const PieceState* target = board_at(&bs->board, threats.squares[j]);
            if (target == NULL)
                continue;
            if (target->owner == passive) {
                int dmg = piece_value(p);
                total_damage += dmg;
                uint16_t idx = (bs->event_head + bs->event_count) % MAX_EVENTS;
                if (bs->event_count < MAX_EVENTS) {
                    bs->events[idx].kind    = EVT_PIECE_DEALT_DAMAGE;
                    bs->events[idx].turn_no = bs->turn_no;
                    bs->events[idx].as.dealt_damage.attacker    = p->id;
                    bs->events[idx].as.dealt_damage.victim_side = passive;
                    bs->events[idx].as.dealt_damage.dmg         = dmg;
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

bool battle_can_move(const BattleState* bs, uint32_t piece_id, Position to) {
    const PieceState* p = piece_by_id((BattleState*)bs, piece_id);
    if (p == NULL)
        return false;
    if (p->owner != bs->active_side)
        return false;
    MoveList ml = {0};
    mg_generate(p, bs, &ml);
    for (uint8_t i = 0; i < ml.count; i++) {
        if (pos_equal(ml.squares[i], to))
            return true;
    }
    return false;
}

bool battle_action_move(BattleState* bs, uint32_t piece_id, Position to) {
    if (!battle_can_move(bs, piece_id, to))
        return false;
    PieceState* p    = piece_by_id(bs, piece_id);
    Position    from = p->pos;
    board_remove(&bs->board, from);
    board_place(&bs->board, p, to);
    p->moves_used++;
    p->flags |= PSF_HAS_MOVED;
    bs->actions_left--;
    uint16_t idx = (bs->event_head + bs->event_count) % MAX_EVENTS;
    if (bs->event_count < MAX_EVENTS) {
        bs->events[idx].kind              = EVT_PIECE_MOVED;
        bs->events[idx].turn_no           = bs->turn_no;
        bs->events[idx].as.moved.piece_id = piece_id;
        bs->events[idx].as.moved.from     = from;
        bs->events[idx].as.moved.to       = to;
        bs->event_count++;
    }
    struct EffectCtx ctx = {0};
    ctx.as.piece.piece   = p;
    bus_emit(&bs->bus, bs, TRIGGER_PIECE_MOVED, &ctx);
    Side enemy = side_opposite(p->owner);
    if (board_at(&bs->board, p->pos)->owner == enemy) {
        bus_emit(&bs->bus, bs, TRIGGER_PIECE_ENTERED_ENEMY_TERR, &ctx);
    }
    return true;
}

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

bool battle_can_combine(const BattleState* bs, uint32_t a, uint32_t b) {
    (void)bs;
    (void)a;
    (void)b;
    return false;
}

bool battle_action_combine(BattleState* bs, uint32_t a, uint32_t b) {
    (void)bs;
    (void)a;
    (void)b;
    return false;
}

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

bool battle_play_card(BattleState* bs, uint8_t idx, const TargetSpec* tgt) {
    if (!battle_can_play_card(bs, idx, tgt))
        return false;
    const CardTemplate* tmpl = bs->hand[bs->active_side][idx].tmpl;
    bs->cp[bs->active_side] -= tmpl->play_cost;
    for (uint8_t i = 0; i < tmpl->play_effect_count; i++) {
        Effect e    = tmpl->on_play[i];
        e.owner     = bs->active_side;
        e.source_id = (uint32_t)tmpl->id;
        bus_register(&bs->bus, &e);
    }
    card_remove_from_hand(bs, bs->active_side, idx);
    bs->actions_left--;
    return true;
}

bool battle_can_sell_card(const BattleState* bs, uint8_t idx) {
    return idx < bs->hand_count[bs->active_side];
}

bool battle_sell_card(BattleState* bs, uint8_t idx) {
    if (!battle_can_sell_card(bs, idx))
        return false;
    const CardTemplate* tmpl = bs->hand[bs->active_side][idx].tmpl;
    bs->cp[bs->active_side] += tmpl->sell_value;
    for (uint8_t i = 0; i < tmpl->sell_effect_count; i++) {
        Effect e    = tmpl->on_sell[i];
        e.owner     = bs->active_side;
        e.source_id = (uint32_t)tmpl->id;
        bus_register(&bs->bus, &e);
    }
    card_remove_from_hand(bs, bs->active_side, idx);
    bs->actions_left--;
    return true;
}

void battle_end_player_turn(BattleState* bs) {
    (void)bs;
}

/*--------------------------------------------------------------------------*\
                              ACCESSORS
\*--------------------------------------------------------------------------*/

const PieceState* battle_piece_at(const BattleState* bs, Position p) {
    return board_at(&bs->board, p);
}

const PieceState* battle_piece_by_id(const BattleState* bs, uint32_t id) {
    return piece_by_id((BattleState*)bs, id);
}

size_t battle_pieces(
    const BattleState* bs,
    Side               side,
    const PieceState** out,
    size_t             cap
) {
    size_t n = 0;
    for (uint16_t i = 0; i < bs->piece_count && n < cap; i++) {
        if (bs->pieces[i].owner == side)
            out[n++] = &bs->pieces[i];
    }
    return n;
}

Side battle_territory(const BattleState* bs, Position p) {
    const PieceState* p2 = board_at(&bs->board, p);
    if (p2 != NULL)
        return p2->owner;
    return SIDE_NEUTRAL;
}

int battle_threat_count(const BattleState* bs, Position p, Side attacker) {
    int n = 0;
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        const PieceState* pc = &bs->pieces[i];
        if (pc->owner != attacker)
            continue;
        MoveList ml = {0};
        mg_generate_threat(pc, bs, &ml);
        for (uint8_t j = 0; j < ml.count; j++) {
            if (pos_equal(ml.squares[j], p))
                n++;
        }
    }
    return n;
}

size_t battle_hand(
    const BattleState*   bs,
    Side                 side,
    const CardInstance** out,
    size_t               cap
) {
    size_t n = bs->hand_count[side];
    if (n > cap)
        n = cap;
    for (size_t i = 0; i < n; i++)
        out[i] = &bs->hand[side][i];
    return n;
}

const CardTemplate* battle_card_tmpl(uint16_t id) {
    return card_template(id);
}

const PieceTemplate* battle_piece_tmpl(uint16_t id) {
    return piece_template(id);
}

/*--------------------------------------------------------------------------*\
                              AFFORDANCE
\*--------------------------------------------------------------------------*/

size_t
battle_legal_moves(const BattleState* bs, uint32_t piece_id, MoveList* out) {
    const PieceState* p = piece_by_id((BattleState*)bs, piece_id);
    if (p == NULL) {
        out->count = 0;
        return 0;
    }
    mg_generate(p, bs, out);
    return out->count;
}

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

const Recipe* battle_recipe_preview(uint16_t a, uint16_t b) {
    return recipe_find(a, b);
}

/*--------------------------------------------------------------------------*\
                              PROJECTION QUERIES
\*--------------------------------------------------------------------------*/

int battle_query_cost(const BattleState* bs, Side side, uint16_t tmpl_id) {
    (void)bs;
    (void)side;
    const PieceTemplate* tmpl = piece_template(tmpl_id);
    if (tmpl == NULL)
        return 0;
    return tmpl->base_value;
}

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

int battle_query_draw_count(const BattleState* bs, Side side) {
    (void)bs;
    (void)side;
    return 1;
}

int battle_query_turn_income(const BattleState* bs, Side side) {
    (void)bs;
    (void)side;
    return 5;
}

int battle_projected_damage(const BattleState* bs, Side attacker) {
    (void)bs;
    (void)attacker;
    return 0;
}

int battle_projected_flips(const BattleState* bs, Side attacker) {
    (void)bs;
    (void)attacker;
    return 0;
}

/*--------------------------------------------------------------------------*\
                              EVENT LOG
\*--------------------------------------------------------------------------*/

size_t battle_drain_events(BattleState* bs, Event* out, size_t cap) {
    size_t n = (bs->event_count < cap) ? bs->event_count : cap;
    for (size_t i = 0; i < n; i++) {
        uint16_t idx = (bs->event_head + i) % MAX_EVENTS;
        out[i]       = bs->events[idx];
    }
    bs->event_head = (bs->event_head + n) % MAX_EVENTS;
    bs->event_count -= n;
    return n;
}

void battle_clear_events(BattleState* bs) {
    bs->event_head  = 0;
    bs->event_count = 0;
}
