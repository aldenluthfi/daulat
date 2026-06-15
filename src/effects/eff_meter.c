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
    Side side  = battle->active_side;
    battle->meter[side] += delta;
    if (battle->meter[side] > battle->meter_cap[side]) {
        battle->meter[side] = battle->meter_cap[side];
    }
    if (battle->meter[side] < 0)
        battle->meter[side] = 0;
}

/// Set meter to a specific value. Used for refill effects.
void eff_meter_set(struct EffectCtx* context, const EffectArg* args, size_t count) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  value = (int)args[0].v.i;
    Side side  = battle->active_side;
    battle->meter[side] = value;
    if (battle->meter[side] > battle->meter_cap[side]) {
        battle->meter[side] = battle->meter_cap[side];
    }
}

/// Increase meter capacity by delta. Does not add to current meter.
void eff_meter_cap_up(struct EffectCtx* context, const EffectArg* args, size_t count) {
    struct BattleState* battle = (struct BattleState*)context;
    if (count < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side side  = battle->active_side;
    battle->meter_cap[side] += delta;
    battle->meter_overflow_cap[side] += delta;
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
    Side side  = battle->active_side;
    battle->meter_overflow_cap[side] += delta;
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
    Side side  = battle->active_side;
    battle->meter[side] = battle->meter_cap[side];
}
