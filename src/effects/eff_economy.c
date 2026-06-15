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
void eff_cp_add(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side side  = battle->active_side;
    battle->cp[side] += delta;
    if (battle->cp[side] < 0)
        battle->cp[side] = 0;
}

/// Set cp to a specific value.
void eff_cp_set(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  value       = (int)args[0].v.i;
    Side side        = battle->active_side;
    battle->cp[side] = value;
    if (battle->cp[side] < 0)
        battle->cp[side] = 0;
}

/// Increase piece cost by delta (positive = more expensive).
void eff_cost_mod(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Increase sell value by delta.
void eff_sell_mod(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Increase turn income by delta.
void eff_income_add(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}
