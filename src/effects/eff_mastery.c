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

static void mastery_l1(struct EffectCtx* ctx, Kingdom k) {
    if (ctx->bs == NULL)
        return;
    if (ctx->as.meta.kingdom != k)
        return;
    meta_apply_innate(ctx->bs, k);
}

static void mastery_l2(struct EffectCtx* ctx, CardId card_id) {
    if (ctx->bs == NULL)
        return;
    struct BattleState* bs = ctx->bs;
    if (bs->cardset_count[SIDE_PLAYER] >= MAX_CARDSET)
        return;
    const CardTemplate* tmpl = card_template(card_id);
    if (tmpl == NULL)
        return;
    bs->cardset[SIDE_PLAYER][bs->cardset_count[SIDE_PLAYER]++] = tmpl;
}

static void mastery_l3(struct EffectCtx* ctx, Kingdom k) {
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    ctx->bs->config.run->mastery_l3[k] = 1;
}

/*--------------------------------------------------------------------------*\
                              LEVEL 1
\*--------------------------------------------------------------------------*/

void eff_mastery_l1_longwei(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l1(ctx, KINGDOM_LONGWEI);
}
void eff_mastery_l1_harushima(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l1(ctx, KINGDOM_HARUSHIMA);
}
void eff_mastery_l1_kewarani(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l1(ctx, KINGDOM_KEWARANI);
}
void eff_mastery_l1_zarqan(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l1(ctx, KINGDOM_ZARQAN);
}
void eff_mastery_l1_caelan(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l1(ctx, KINGDOM_CAELAN);
}

/*--------------------------------------------------------------------------*\
                              LEVEL 2
\*--------------------------------------------------------------------------*/

void eff_mastery_l2_longwei(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l2(ctx, CARD_MINGZHUS_SEAL);
}
void eff_mastery_l2_harushima(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l2(ctx, CARD_TOMOHITOS_PATIENCE);
}
void eff_mastery_l2_kewarani(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l2(ctx, CARD_SELASSIES_MARCH);
}
void eff_mastery_l2_zarqan(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l2(ctx, CARD_TIMURS_CONQUEST);
}
void eff_mastery_l2_caelan(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l2(ctx, CARD_ISABELLAS_CORONATION);
}

/*--------------------------------------------------------------------------*\
                              LEVEL 3
\*--------------------------------------------------------------------------*/

void eff_mastery_l3_longwei(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l3(ctx, KINGDOM_LONGWEI);
}
void eff_mastery_l3_harushima(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l3(ctx, KINGDOM_HARUSHIMA);
}
void eff_mastery_l3_kewarani(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l3(ctx, KINGDOM_KEWARANI);
}
void eff_mastery_l3_zarqan(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l3(ctx, KINGDOM_ZARQAN);
}
void eff_mastery_l3_caelan(
    struct EffectCtx* ctx, const EffectArg* a, size_t n
) {
    (void)a; (void)n; mastery_l3(ctx, KINGDOM_CAELAN);
}
