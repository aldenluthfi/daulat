//! eff_figurehead.c
//!
//! Bodies for the five figurehead powers (one per kingdom). These
//! activate at run start (Tomohito, Selassie, Timur) or at battle
//! start (Mingzhu, Isabella). Each one writes to RunState or to
//! BattleState; the actual piece-spawn for Mingzhu's free Pao is
//! gated on the battle's current kingdom.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              FIGUREHEADS
\*--------------------------------------------------------------------------*/

void eff_fh_mingzhu(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs  = ctx->bs;
    if (bs == NULL || bs->config.run == NULL)
        return;
    Kingdom k = bs->config.run->current_kingdom;
    bool    any_territory =
        (bs->config.run->mastery_l3[KINGDOM_LONGWEI] != 0);
    if (k == KINGDOM_LONGWEI || any_territory) {
        Position p = {0, 1};
        piece_spawn(bs, PIECE_PAO, p, SIDE_PLAYER);
    } else {
        bs->cp[SIDE_PLAYER] += 20;
    }
}

void eff_fh_tomohito(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    RunState* run = ctx->bs->config.run;
    run->reclaim_cost_override =
        (run->mastery_l3[KINGDOM_HARUSHIMA] != 0) ? 5 : 10;
}

void eff_fh_selassie(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL)
        return;
    meta_apply_innate(ctx->bs, KINGDOM_KEWARANI);
}

void eff_fh_timur(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    RunState* run = ctx->bs->config.run;
    run->royal_sub_per_battle =
        (run->mastery_l3[KINGDOM_ZARQAN] != 0) ? 3 : 2;
}

void eff_fh_isabella(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL)
        return;
    /* Insert one Caelan card at hand[0]; Phase 4 picks the exact
     * card from data_caelan once the cardset is wired. */
    if (bs->hand_count[SIDE_PLAYER] < MAX_HAND) {
        const CardTemplate* tmpl =
            card_template((CardId)CARD_CASTLING);
        if (tmpl != NULL) {
            for (uint8_t i = bs->hand_count[SIDE_PLAYER]; i > 0; i--)
                bs->hand[SIDE_PLAYER][i] = bs->hand[SIDE_PLAYER][i - 1];
            bs->hand[SIDE_PLAYER][0].tmpl  = tmpl;
            bs->hand[SIDE_PLAYER][0].flags = 0;
            bs->hand_count[SIDE_PLAYER]++;
        }
    }
}
