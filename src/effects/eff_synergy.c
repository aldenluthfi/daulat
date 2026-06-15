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

static bool in_battle_kingdom(const struct BattleState* battle, Kingdom k) {
    if (battle == NULL || battle->config.run == NULL)
        return false;
    return battle->config.run->current_kingdom == k;
}

void eff_syn_longwei(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (!in_battle_kingdom(context->battle, KINGDOM_HARUSHIMA))
        return;
    PieceState* attacker = context->as.resolve.attacker;
    if (attacker == NULL || attacker->template == NULL)
        return;
    if (attacker->template->id != PIECE_PAO)
        return;
    if (context->as.resolve.damage_out != NULL)
        *context->as.resolve.damage_out += 10;
}

void eff_syn_harushima(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (!in_battle_kingdom(context->battle, KINGDOM_CAELAN))
        return;
    if (context->as.card.card == NULL ||
        context->as.card.card->template == NULL)
        return;
    if (context->as.card.card->template->kingdom != KINGDOM_CAELAN)
        return;
    card_draw(context->battle, SIDE_PLAYER);
}

void eff_syn_kewarani(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (!in_battle_kingdom(context->battle, KINGDOM_ZARQAN))
        return;
    if (context->as.query.cost_out == NULL)
        return;
    uint16_t template_id          = context->as.query.template_id;
    const PieceTemplate* template = piece_template(template_id);
    if (template == NULL || template->kingdom != KINGDOM_KEWARANI)
        return;
    *context->as.query.cost_out -= 10;
    if (*context->as.query.cost_out < 0)
        *context->as.query.cost_out = 0;
}

void eff_syn_zarqan(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    if (!in_battle_kingdom(battle, KINGDOM_LONGWEI))
        return;
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        PieceState* piece = &battle->pieces[i];
        if (piece->owner != SIDE_PLAYER || piece->template == NULL)
            continue;
        if (piece->template->id == PIECE_ZIRAAFA ||
            piece->template->id == PIECE_TALLIYA)
            piece->value_mod += 5;
    }
}

void eff_syn_caelan(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (!in_battle_kingdom(context->battle, KINGDOM_KEWARANI))
        return;
    if (context->as.card.card == NULL ||
        context->as.card.card->template == NULL)
        return;
    if (context->as.card.card->template->id != CARD_SULTANS_GOLD)
        return;
    context->battle->cp[SIDE_PLAYER] += 10;
}
