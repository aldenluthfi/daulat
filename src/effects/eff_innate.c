//! eff_innate.c
//!
//! Bodies for the five kingdom innates (Bulwark, Reclaim, Double
//! Time, Royal Substitution, Conqueror's Reward). Bulwark and
//! Double Time are pure query handlers; Reclaim and Royal
//! Substitution expose actions through the run-state flags they
//! mutate; Conqueror's Reward augments the attacker's value_mod on
//! a successful flip.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

static bool has_friendly_neighbour(
    const struct BattleState* battle,
    const PieceState*         piece
) {
    static const Position OFFSETS[4] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (int i = 0; i < 4; i++) {
        Position position = {
            (int8_t)(piece->pos.x + OFFSETS[i].x),
            (int8_t)(piece->pos.y + OFFSETS[i].y),
        };
        if (!pos_in_bounds(position, battle->board.width, battle->board.height))
            continue;
        const PieceState* neighbour = board_at(&battle->board, position);
        if (neighbour != NULL && neighbour->owner == piece->owner)
            return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*\
                              INNATES
\*--------------------------------------------------------------------------*/

void eff_innate_bulwark(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    PieceState*         target = context->as.resolve.target;
    if (battle == NULL || target == NULL || target->template == NULL)
        return;
    if (target->template->kingdom != KINGDOM_LONGWEI)
        return;
    if (!has_friendly_neighbour(battle, target))
        return;
    if (context->as.resolve.reduction_out != NULL)
        *context->as.resolve.reduction_out += 50;
}

void eff_innate_reclaim(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
    /* Reclaim exposes battle_action_reclaim via run->reclaim_cost_
     * override; no per-trigger body needed. */
}

void eff_innate_double_time(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    PieceState* piece = context->as.movement.piece;
    if (piece == NULL || piece->template == NULL)
        return;
    if (piece->template->kingdom != KINGDOM_KEWARANI)
        return;
    if (context->as.query.moves_out != NULL)
        *context->as.query.moves_out += 1;
}

void eff_innate_royal_sub(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
    /* Royal Substitution exposes battle_action_substitute. The per-
     * battle use count comes from TRIGGER_QUERY_ROYAL_SUB_COUNT; the
     * default count is 1, Timur's figurehead overrides to 2 or 3. */
}

void eff_innate_conquerors_reward(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    PieceState* attacker = context->as.resolve.attacker;
    if (attacker == NULL || attacker->template == NULL)
        return;
    if (attacker->template->kingdom != KINGDOM_CAELAN)
        return;
    attacker->value_mod += piece_value(attacker) / 2;
}
