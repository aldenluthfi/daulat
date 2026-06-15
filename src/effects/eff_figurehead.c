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
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle  = context->battle;
    if (battle == NULL || battle->config.run == NULL)
        return;
    Kingdom k = battle->config.run->current_kingdom;
    bool    any_territory =
        (battle->config.run->mastery_l3[KINGDOM_LONGWEI] != 0);
    if (k == KINGDOM_LONGWEI || any_territory) {
        Position position = {0, 1};
        piece_spawn(battle, PIECE_PAO, position, SIDE_PLAYER);
    } else {
        battle->cp[SIDE_PLAYER] += 20;
    }
}

void eff_fh_tomohito(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    int* out = context->as.query.reclaim_cost_out;
    if (out == NULL)
        return;
    const RunState* run = context->battle->config.run;
    *out = (run->mastery_l3[KINGDOM_HARUSHIMA] != 0) ? 5 : 10;
}

void eff_fh_selassie(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL)
        return;
    meta_apply_innate(context->battle, KINGDOM_KEWARANI);
}

void eff_fh_timur(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    int* out = context->as.query.royal_sub_count_out;
    if (out == NULL)
        return;
    const RunState* run = context->battle->config.run;
    *out = (run->mastery_l3[KINGDOM_ZARQAN] != 0) ? 3 : 2;
}

void eff_fh_isabella(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    struct BattleState* battle = context->battle;
    if (battle == NULL)
        return;
    /* Insert one Caelan card at hand[0]; the campaign deck pick
     * card from data_caelan once the cardset is wired. */
    if (battle->hand_count[SIDE_PLAYER] < MAX_HAND) {
        const CardTemplate* template =
            card_template((CardId)CARD_CASTLING);
        if (template != NULL) {
            for (uint8_t i = battle->hand_count[SIDE_PLAYER]; i > 0; i--)
                battle->hand[SIDE_PLAYER][i] = battle->hand[SIDE_PLAYER][i - 1];
            battle->hand[SIDE_PLAYER][0].template  = template;
            battle->hand[SIDE_PLAYER][0].flags = 0;
            battle->hand_count[SIDE_PLAYER]++;
        }
    }
}
