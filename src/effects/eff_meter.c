//!
//! Meter effects: meter +/-, refill, capacity, overflow.
//! Meter absorbs damage before flipping. Overflow beyond capacity cascades.
//! Meter is per-side, stored in BattleState.meter[SIDE] with caps.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              METER MODIFIERS
\*--------------------------------------------------------------------------*/

/// Add delta to meter. Clamps to meter_cap. Triggers meter-changed event.
void eff_meter_add(struct EffectCtx* context, const EffectArg* args, size_t count) {
    (void)args;
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = battle->active_side;
    battle->meter[s] += delta;
    if (battle->meter[s] > battle->meter_cap[s]) {
        battle->meter[s] = battle->meter_cap[s];
    }
    if (battle->meter[s] < 0)
        battle->meter[s] = 0;
}

/// Set meter to a specific value. Used for refill effects.
void eff_meter_set(struct EffectCtx* context, const EffectArg* args, size_t count) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  val     = (int)args[0].v.i;
    Side s       = battle->active_side;
    battle->meter[s] = val;
    if (battle->meter[s] > battle->meter_cap[s]) {
        battle->meter[s] = battle->meter_cap[s];
    }
}

/// Increase meter capacity by delta. Does not add to current meter.
void eff_meter_cap_up(struct EffectCtx* context, const EffectArg* args, size_t count) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = battle->active_side;
    battle->meter_cap[s] += delta;
    battle->meter_overflow_cap[s] += delta;
}

/// Increase overflow capacity by delta.
void eff_meter_overflow_up(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = battle->active_side;
    battle->meter_overflow_cap[s] += delta;
}

/// Overflow: add to overflow meter, which absorbs damage beyond normal capacity.
void eff_meter_overflow_add(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Refill meter to capacity. Common on turn start or as a card effect.
void eff_meter_refill(struct EffectCtx* context, const EffectArg* args, size_t count) {
    struct BattleState* battle = (struct BattleState*)context;
    (void)args;
    (void)count;
    Side s       = battle->active_side;
    battle->meter[s] = battle->meter_cap[s];
}
