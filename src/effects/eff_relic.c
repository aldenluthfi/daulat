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
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.card.value_out != NULL)
        *context->as.card.value_out += 5;
}

void eff_minted_coin(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.query.income_out != NULL)
        *context->as.query.income_out += 5;
}

void eff_tax_stamp(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->as.card.card == NULL)
        return;
    const CardTemplate* template = context->as.card.card->template;
    if (template != NULL && template->play_cost > 0)
        context->battle->cp[SIDE_PLAYER] += 10;
}

void eff_bulk_discount(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->self == NULL)
        return;
    if (context->as.query.cost_out == NULL)
        return;
    int buys_this_turn  = context->self->scratch[0].v.i;
    int cheapest_so_far = context->self->scratch[1].v.i;
    if (buys_this_turn < 3)
        return;
    if (cheapest_so_far > 0 && *context->as.query.cost_out <= cheapest_so_far)
        *context->as.query.cost_out = 0;
}

void eff_war_chest(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    if (battle == NULL)
        return;
    battle->meter[SIDE_PLAYER] += battle->cp[SIDE_PLAYER] / 5;
}

void eff_trade_routes(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    context->battle->config.run->flags |= RUN_FOREIGN_MARKUP_OFF;
}

/*--------------------------------------------------------------------------*\
                              METER (6)
\*--------------------------------------------------------------------------*/

void eff_soul_shard(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL)
        return;
    if (context->as.flipped.new_owner != SIDE_PLAYER)
        return;
    context->battle->meter[SIDE_PLAYER] += 30;
}

void eff_veterans_bond(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    if (battle == NULL || context->as.query.meter_cap_out == NULL)
        return;
    int veterans = 0;
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        const PieceState* piece = &battle->pieces[i];
        if (piece->owner == SIDE_PLAYER && piece_value(piece) >= 50)
            veterans++;
    }
    *context->as.query.meter_cap_out += 20 * veterans;
}

void eff_dead_mans_pact(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    if (battle == NULL || context->self == NULL)
        return;
    if (context->self->scratch[0].v.i != 0)
        return;
    if (battle->meter[SIDE_PLAYER] > 0)
        return;
    battle->meter[SIDE_PLAYER]    = 20;
    context->self->scratch[0].v.i = 1;
}

void eff_iron_king(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.query.meter_cap_out != NULL)
        *context->as.query.meter_cap_out += 10;
}

void eff_bloodthirst(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    if (battle == NULL)
        return;
    if (battle->meter[SIDE_PLAYER] > battle->meter[SIDE_ENEMY])
        battle->meter[SIDE_PLAYER] += 5;
}

void eff_last_breath(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    struct PieceState*  piece  = context->as.piece.piece;
    if (battle == NULL || piece == NULL)
        return;
    if (piece->owner != SIDE_PLAYER)
        return;
    battle->meter[SIDE_ENEMY] -= piece_value(piece);
    if (battle->meter[SIDE_ENEMY] < 0)
        battle->meter[SIDE_ENEMY] = 0;
}

/*--------------------------------------------------------------------------*\
                              CARDS (5)
\*--------------------------------------------------------------------------*/

void eff_tacticians_scroll(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    int* out = context->as.card.count_out;
    if (out != NULL && *out < 4)
        *out = 4;
}

void eff_librarians_notes(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
    /* Peek-skip action exposed via the UI; turn-start re-arms the
     * one-per-turn latch. The action itself lives in battle.c. */
}

void eff_country_seal(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.card.card == NULL || context->as.card.value_out == NULL)
        return;
    const CardTemplate* template = context->as.card.card->template;
    if (template != NULL && template->tier == TIER_COUNTRY)
        *context->as.card.value_out += 20;
}

void eff_deep_hand(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
    /* Once-per-battle draw action; consumed via
     * battle_relic_deep_hand_draw and gated by scratch[0]. */
}

void eff_gilded_archive(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.card.card == NULL || context->as.card.value_out == NULL)
        return;
    const CardTemplate* template = context->as.card.card->template;
    if (template != NULL && template->tier == TIER_DISTRICT)
        *context->as.card.value_out += 10;
}

/*--------------------------------------------------------------------------*\
                              COMBINATIONS (4)
\*--------------------------------------------------------------------------*/

void eff_alchemists_kit(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.query.cost_out != NULL)
        *context->as.query.cost_out = 0;
}

void eff_masters_notes(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    context->battle->config.run->flags |= RUN_DOUBLE_ARCHIVE;
}

void eff_philosophers_stone(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->as.piece.piece == NULL)
        return;
    if (context->self == NULL || context->self->scratch[0].v.i != 0)
        return;
    context->as.piece.piece->value_mod += 20;
    context->self->scratch[0].v.i = 1;
}

void eff_inherited_power(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.piece.piece != NULL)
        context->as.piece.piece->value_mod += 5;
}

/*--------------------------------------------------------------------------*\
                              BOARD (5)
\*--------------------------------------------------------------------------*/

void eff_eagle_eye(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.query.vision_flags_out != NULL)
        *context->as.query.vision_flags_out |= VISION_ENEMY_VALUES;
    if (context->battle != NULL && context->battle->config.run != NULL)
        context->battle->config.run->flags |= RUN_VISION_ENEMY_VALUES;
}

void eff_surveyors_map(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    context->battle->config.run->flags |= RUN_PREREVEAL_MODIFIER;
}

void eff_forward_command(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->as.resolve.attacker == NULL)
        return;
    if (context->as.resolve.damage_out == NULL)
        return;
    Position position      = context->as.resolve.attacker->pos;
    Side     attacker_side = context->as.resolve.attacker->owner;
    if (battle_territory(context->battle, position) !=
        side_opposite(attacker_side))
        return;
    *context->as.resolve.damage_out += 5;
}

void eff_fortified_line(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.resolve.attacker == NULL)
        return;
    if (context->as.resolve.attacker->moves_used != 0)
        return;
    if (context->as.resolve.damage_out != NULL)
        *context->as.resolve.damage_out += 5;
}

void eff_warlords_banner(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.resolve.damage_out != NULL)
        *context->as.resolve.damage_out += 5;
}
