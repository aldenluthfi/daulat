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
    const struct BattleState* bs,
    const PieceState*         piece
) {
    static const Position OFFSETS[4] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    for (int i = 0; i < 4; i++) {
        Position p = {
            (int8_t)(piece->pos.x + OFFSETS[i].x),
            (int8_t)(piece->pos.y + OFFSETS[i].y),
        };
        if (!pos_in_bounds(p, bs->board.width, bs->board.height))
            continue;
        const PieceState* neighbour = board_at(&bs->board, p);
        if (neighbour != NULL && neighbour->owner == piece->owner)
            return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*\
                              INNATES
\*--------------------------------------------------------------------------*/

void eff_innate_bulwark(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs    = ctx->bs;
    PieceState*         target = ctx->as.resolve.target;
    if (bs == NULL || target == NULL || target->tmpl == NULL)
        return;
    if (target->tmpl->kingdom != KINGDOM_LONGWEI)
        return;
    if (!has_friendly_neighbour(bs, target))
        return;
    if (ctx->as.resolve.reduction_out != NULL)
        *ctx->as.resolve.reduction_out += 50;
}

void eff_innate_reclaim(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)ctx;
    (void)args;
    (void)n;
    /* Reclaim exposes battle_action_reclaim via run->reclaim_cost_
     * override; no per-trigger body needed. */
}

void eff_innate_double_time(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    PieceState* piece = ctx->as.movement.piece;
    if (piece == NULL || piece->tmpl == NULL)
        return;
    if (piece->tmpl->kingdom != KINGDOM_KEWARANI)
        return;
    if (ctx->as.query.moves_out != NULL)
        *ctx->as.query.moves_out += 1;
}

void eff_innate_royal_sub(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)ctx;
    (void)args;
    (void)n;
    /* Royal Substitution exposes battle_action_substitute; the
     * latch counter lives on RunState.royal_sub_per_battle. */
}

void eff_innate_conquerors_reward(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    PieceState* attacker = ctx->as.resolve.attacker;
    if (attacker == NULL || attacker->tmpl == NULL)
        return;
    if (attacker->tmpl->kingdom != KINGDOM_CAELAN)
        return;
    attacker->value_mod += piece_value(attacker) / 2;
}
