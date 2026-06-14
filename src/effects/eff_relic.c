//! eff_relic.c
//!
//! Concrete bodies for the 26 relic effects. Every handler matches
//! the trigger keyed in `data_relics.c` and mutates the battle /
//! run state in line with `res/GDD.md` § 8 (Relics) and the spec
//! tables in the plan file. Bodies are deliberately small: each one
//! reads at most a few EffectCtx fields and writes one or two pieces
//! of state.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              ECONOMY (6)
\*--------------------------------------------------------------------------*/

void eff_merchants_ledger(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.card.value_out != NULL)
        *ctx->as.card.value_out += 5;
}

void eff_minted_coin(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.query.income_out != NULL)
        *ctx->as.query.income_out += 5;
}

void eff_tax_stamp(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->as.card.card == NULL)
        return;
    const CardTemplate* tmpl = ctx->as.card.card->tmpl;
    if (tmpl != NULL && tmpl->play_cost > 0)
        ctx->bs->cp[SIDE_PLAYER] += 10;
}

void eff_bulk_discount(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL || ctx->as.query.cost_out == NULL)
        return;
    if (bs->buys_this_turn[SIDE_PLAYER] < 3)
        return;
    uint16_t cheapest = bs->cheapest_buy_cost[SIDE_PLAYER];
    if ((uint16_t)*ctx->as.query.cost_out <= cheapest)
        *ctx->as.query.cost_out = 0;
}

void eff_war_chest(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL)
        return;
    bs->meter[SIDE_PLAYER] += bs->cp[SIDE_PLAYER] / 5;
}

void eff_trade_routes(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    ctx->bs->config.run->flags |= RUN_FOREIGN_MARKUP_OFF;
}

/*--------------------------------------------------------------------------*\
                              METER (6)
\*--------------------------------------------------------------------------*/

void eff_soul_shard(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL)
        return;
    ctx->bs->meter[SIDE_PLAYER] += 30;
}

void eff_veterans_bond(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL || ctx->as.query.meter_cap_out == NULL)
        return;
    int veterans = 0;
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        const PieceState* piece = &bs->pieces[i];
        if (piece->owner == SIDE_PLAYER && piece_value(piece) >= 50)
            veterans++;
    }
    *ctx->as.query.meter_cap_out += 20 * veterans;
}

void eff_dead_mans_pact(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL)
        return;
    if (bs->once_per_battle_flags & LATCH_DEAD_MANS_PACT)
        return;
    if (bs->meter[SIDE_PLAYER] > 0)
        return;
    bs->meter[SIDE_PLAYER]      = 20;
    bs->once_per_battle_flags  |= LATCH_DEAD_MANS_PACT;
}

void eff_iron_king(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.query.meter_cap_out != NULL)
        *ctx->as.query.meter_cap_out += 10;
}

void eff_bloodthirst(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL)
        return;
    if (bs->meter[SIDE_PLAYER] > bs->meter[SIDE_ENEMY])
        bs->meter[SIDE_PLAYER] += 5;
}

void eff_last_breath(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs    = ctx->bs;
    struct PieceState*  piece = ctx->as.piece.piece;
    if (bs == NULL || piece == NULL)
        return;
    if (piece->owner != SIDE_PLAYER)
        return;
    bs->meter[SIDE_ENEMY] -= piece_value(piece);
    if (bs->meter[SIDE_ENEMY] < 0)
        bs->meter[SIDE_ENEMY] = 0;
}

/*--------------------------------------------------------------------------*\
                              CARDS (5)
\*--------------------------------------------------------------------------*/

void eff_tacticians_scroll(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    int* out = ctx->as.card.count_out;
    if (out != NULL && *out < 4)
        *out = 4;
}

void eff_librarians_notes(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)ctx;
    (void)args;
    (void)n;
    /* Peek-skip action exposed via the UI; turn-start re-arms the
     * one-per-turn latch. The action itself lives in battle.c. */
}

void eff_country_seal(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.card.card == NULL || ctx->as.card.value_out == NULL)
        return;
    const CardTemplate* tmpl = ctx->as.card.card->tmpl;
    if (tmpl != NULL && tmpl->tier == TIER_COUNTRY)
        *ctx->as.card.value_out += 20;
}

void eff_deep_hand(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)ctx;
    (void)args;
    (void)n;
    /* Once-per-battle draw action; consumed via
     * battle_relic_deep_hand_draw, gated by LATCH_DEEP_HAND. */
}

void eff_gilded_archive(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.card.card == NULL || ctx->as.card.value_out == NULL)
        return;
    const CardTemplate* tmpl = ctx->as.card.card->tmpl;
    if (tmpl != NULL && tmpl->tier == TIER_DISTRICT)
        *ctx->as.card.value_out += 10;
}

/*--------------------------------------------------------------------------*\
                              COMBINATIONS (4)
\*--------------------------------------------------------------------------*/

void eff_alchemists_kit(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.query.cost_out != NULL)
        *ctx->as.query.cost_out = 0;
}

void eff_masters_notes(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    ctx->bs->config.run->flags |= RUN_DOUBLE_ARCHIVE;
}

void eff_philosophers_stone(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL || ctx->as.piece.piece == NULL)
        return;
    if (bs->once_per_battle_flags & LATCH_PHILOSOPHERS_STONE)
        return;
    ctx->as.piece.piece->value_mod   += 20;
    bs->once_per_battle_flags        |= LATCH_PHILOSOPHERS_STONE;
}

void eff_inherited_power(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.piece.piece != NULL)
        ctx->as.piece.piece->value_mod += 5;
}

/*--------------------------------------------------------------------------*\
                              BOARD (5)
\*--------------------------------------------------------------------------*/

void eff_eagle_eye(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL)
        return;
    ctx->bs->vision_flags |= VISION_ENEMY_VALUES;
    if (ctx->bs->config.run != NULL)
        ctx->bs->config.run->flags |= RUN_VISION_ENEMY_VALUES;
}

void eff_surveyors_map(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    ctx->bs->config.run->flags |= RUN_PREREVEAL_MODIFIER;
}

void eff_forward_command(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.resolve.damage_out != NULL)
        *ctx->as.resolve.damage_out += 5;
}

void eff_fortified_line(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.resolve.damage_out != NULL)
        *ctx->as.resolve.damage_out += 5;
}

void eff_warlords_banner(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->as.resolve.damage_out != NULL)
        *ctx->as.resolve.damage_out += 5;
}
