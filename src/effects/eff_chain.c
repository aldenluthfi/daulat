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
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL)
        return;
    context->battle->cp[SIDE_PLAYER] -= 15;
    if (context->battle->cp[SIDE_PLAYER] < 0)
        context->battle->cp[SIDE_PLAYER] = 0;
}

void eff_chain_silver(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    Kingdom k = context->battle->config.run->current_kingdom;
    if ((unsigned)k < KINGDOM_COUNT)
        context->battle->silver_chain_pending[k] += 1;
}

void eff_chain_gold(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
    /* Gold chain only acts at the campaign layer; see map.c. */
}
