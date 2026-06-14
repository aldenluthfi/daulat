//! eff_chain.c
//!
//! Bodies for the three penalty chains (Bronze / Silver / Gold).
//! Bronze trims starting cp; Silver flags the battle to spawn a free
//! enemy piece (the actual spawn happens in battle_init since
//! handlers can't yet allocate pieces with full templates); Gold has
//! no per-battle body — the campaign-loop liberation lock is
//! managed by the map layer reading `RunState.subjugated`.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              CHAIN BODIES
\*--------------------------------------------------------------------------*/

void eff_chain_bronze(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL)
        return;
    ctx->bs->cp[SIDE_PLAYER] -= 15;
    if (ctx->bs->cp[SIDE_PLAYER] < 0)
        ctx->bs->cp[SIDE_PLAYER] = 0;
}

void eff_chain_silver(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    Kingdom k = ctx->bs->config.run->current_kingdom;
    if ((unsigned)k < KINGDOM_COUNT)
        ctx->bs->silver_chain_pending[k] += 1;
}

void eff_chain_gold(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)ctx;
    (void)args;
    (void)n;
    /* Gold chain only acts at the campaign layer; see map.c. */
}
