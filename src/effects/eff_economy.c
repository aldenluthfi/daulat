//!
//! Economy effects: cp +/-, cost modifiers, sell modifiers, income.
//! CP (construction points) is the primary resource for buying pieces.
//! Income is queried per-turn via TRIGGER_QUERY_TURN_INCOME.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              ECONOMY MODIFIERS
\*--------------------------------------------------------------------------*/

/// Add delta to cp. Triggers cp-changed event.
void eff_cp_add(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    struct BattleState* bs = (struct BattleState*)ctx;
    if (n < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = bs->active_side;
    bs->cp[s] += delta;
    if (bs->cp[s] < 0)
        bs->cp[s] = 0;
}

/// Set cp to a specific value.
void eff_cp_set(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    struct BattleState* bs = (struct BattleState*)ctx;
    if (n < 1)
        return;
    int  val  = (int)args[0].v.i;
    Side s    = bs->active_side;
    bs->cp[s] = val;
    if (bs->cp[s] < 0)
        bs->cp[s] = 0;
}

/// Increase piece cost by delta (positive = more expensive).
void eff_cost_mod(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n;
}

/// Increase sell value by delta.
void eff_sell_mod(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n;
}

/// Increase turn income by delta.
void eff_income_add(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n;
}
