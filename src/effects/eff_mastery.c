//! eff_mastery.c
//!
//! Bodies for the 15 mastery hooks (5 kingdoms × 3 levels). Level 1
//! activates the kingdom's innate ability immediately on Town-map
//! entry; level 2 appends the figurehead card to the cardset; level
//! 3 flips the `mastery_l3` flag that figurehead / innate bodies
//! consult when computing their upgraded behaviour.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

static void mastery_l1(struct EffectCtx* context, Kingdom k) {
    if (context->battle == NULL)
        return;
    if (context->as.meta.kingdom != k)
        return;
    meta_apply_innate(context->battle, k);
}

static void mastery_l2(struct EffectCtx* context, CardId card_id) {
    if (context->battle == NULL)
        return;
    struct BattleState* battle = context->battle;
    if (battle->cardset_count[SIDE_PLAYER] >= MAX_CARDSET)
        return;
    const CardTemplate* template = card_template(card_id);
    if (template == NULL)
        return;
    battle->cardset[SIDE_PLAYER][battle->cardset_count[SIDE_PLAYER]++] =
        template;
}

static void mastery_l3(struct EffectCtx* context, Kingdom k) {
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    context->battle->config.run->mastery_l3[k] = 1;
}

/*--------------------------------------------------------------------------*\
                              LEVEL 1
\*--------------------------------------------------------------------------*/

void eff_mastery_l1_longwei(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l1(context, KINGDOM_LONGWEI);
}
void eff_mastery_l1_harushima(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l1(context, KINGDOM_HARUSHIMA);
}
void eff_mastery_l1_kewarani(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l1(context, KINGDOM_KEWARANI);
}
void eff_mastery_l1_zarqan(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l1(context, KINGDOM_ZARQAN);
}
void eff_mastery_l1_caelan(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l1(context, KINGDOM_CAELAN);
}

/*--------------------------------------------------------------------------*\
                              LEVEL 2
\*--------------------------------------------------------------------------*/

void eff_mastery_l2_longwei(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l2(context, CARD_MINGZHUS_SEAL);
}
void eff_mastery_l2_harushima(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l2(context, CARD_TOMOHITOS_PATIENCE);
}
void eff_mastery_l2_kewarani(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l2(context, CARD_SELASSIES_MARCH);
}
void eff_mastery_l2_zarqan(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l2(context, CARD_TIMURS_CONQUEST);
}
void eff_mastery_l2_caelan(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l2(context, CARD_ISABELLAS_CORONATION);
}

/*--------------------------------------------------------------------------*\
                              LEVEL 3
\*--------------------------------------------------------------------------*/

void eff_mastery_l3_longwei(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l3(context, KINGDOM_LONGWEI);
}
void eff_mastery_l3_harushima(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l3(context, KINGDOM_HARUSHIMA);
}
void eff_mastery_l3_kewarani(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l3(context, KINGDOM_KEWARANI);
}
void eff_mastery_l3_zarqan(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l3(context, KINGDOM_ZARQAN);
}
void eff_mastery_l3_caelan(
    struct EffectCtx* context,
    const EffectArg*  a,
    size_t            count
) {
    (void)a;
    (void)count;
    mastery_l3(context, KINGDOM_CAELAN);
}
