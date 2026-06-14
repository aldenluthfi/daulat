//! eff_synergy.c
//!
//! Bodies for the five kingdom synergies activated when an Overseer
//! has been cleared. Each one fires when the player enters a battle
//! in the lore-adjacent kingdom (Longwei→Harushima, Harushima→Caelan,
//! Kewarani→Zarqan, Zarqan→Longwei, Caelan→Kewarani).
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              SYNERGIES
\*--------------------------------------------------------------------------*/

static bool in_battle_kingdom(const struct BattleState* bs, Kingdom k) {
    if (bs == NULL || bs->config.run == NULL)
        return false;
    return bs->config.run->current_kingdom == k;
}

void eff_syn_longwei(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (!in_battle_kingdom(ctx->bs, KINGDOM_HARUSHIMA))
        return;
    PieceState* attacker = ctx->as.resolve.attacker;
    if (attacker == NULL || attacker->tmpl == NULL)
        return;
    if (attacker->tmpl->id != PIECE_PAO)
        return;
    if (ctx->as.resolve.damage_out != NULL)
        *ctx->as.resolve.damage_out += 10;
}

void eff_syn_harushima(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (!in_battle_kingdom(ctx->bs, KINGDOM_CAELAN))
        return;
    if (ctx->as.card.card == NULL || ctx->as.card.card->tmpl == NULL)
        return;
    if (ctx->as.card.card->tmpl->kingdom != KINGDOM_CAELAN)
        return;
    card_draw(ctx->bs, SIDE_PLAYER);
}

void eff_syn_kewarani(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (!in_battle_kingdom(ctx->bs, KINGDOM_ZARQAN))
        return;
    if (ctx->as.query.cost_out == NULL)
        return;
    uint16_t tmpl_id = ctx->as.query.tmpl_id;
    const PieceTemplate* tmpl = piece_template(tmpl_id);
    if (tmpl == NULL || tmpl->kingdom != KINGDOM_KEWARANI)
        return;
    *ctx->as.query.cost_out -= 10;
    if (*ctx->as.query.cost_out < 0)
        *ctx->as.query.cost_out = 0;
}

void eff_syn_zarqan(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (!in_battle_kingdom(bs, KINGDOM_LONGWEI))
        return;
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        PieceState* piece = &bs->pieces[i];
        if (piece->owner != SIDE_PLAYER || piece->tmpl == NULL)
            continue;
        if (piece->tmpl->id == PIECE_ZIRAAFA
            || piece->tmpl->id == PIECE_TALLIYA)
            piece->value_mod += 5;
    }
}

void eff_syn_caelan(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (!in_battle_kingdom(ctx->bs, KINGDOM_KEWARANI))
        return;
    if (ctx->as.card.card == NULL || ctx->as.card.card->tmpl == NULL)
        return;
    if (ctx->as.card.card->tmpl->id != CARD_SULTANS_GOLD)
        return;
    ctx->bs->cp[SIDE_PLAYER] += 10;
}
