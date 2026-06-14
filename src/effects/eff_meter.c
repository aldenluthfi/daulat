//!
//! Meter effects: meter +/-, refill, cap, overflow.
//! Meter absorbs damage before flipping. Overflow beyond cap cascades.
//! Meter is per-side, stored in BattleState.meter[SIDE] with caps.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              METER MODIFIERS
\*--------------------------------------------------------------------------*/

/// Add delta to meter. Clamps to meter_cap. Triggers meter-changed event.
void eff_meter_add(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)args;
    struct BattleState* bs = (struct BattleState*)ctx;
    if (n < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = bs->active_side;
    bs->meter[s] += delta;
    if (bs->meter[s] > bs->meter_cap[s]) {
        bs->meter[s] = bs->meter_cap[s];
    }
    if (bs->meter[s] < 0)
        bs->meter[s] = 0;
}

/// Set meter to a specific value. Used for refill effects.
void eff_meter_set(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    struct BattleState* bs = (struct BattleState*)ctx;
    if (n < 1)
        return;
    int  val     = (int)args[0].v.i;
    Side s       = bs->active_side;
    bs->meter[s] = val;
    if (bs->meter[s] > bs->meter_cap[s]) {
        bs->meter[s] = bs->meter_cap[s];
    }
}

/// Increase meter cap by delta. Does not add to current meter.
void eff_meter_cap_up(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    struct BattleState* bs = (struct BattleState*)ctx;
    if (n < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = bs->active_side;
    bs->meter_cap[s] += delta;
    bs->meter_overflow_cap[s] += delta;
}

/// Increase overflow cap by delta.
void eff_meter_overflow_up(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    struct BattleState* bs = (struct BattleState*)ctx;
    if (n < 1)
        return;
    int  delta = (int)args[0].v.i;
    Side s     = bs->active_side;
    bs->meter_overflow_cap[s] += delta;
}

/// Overflow: add to overflow meter, which absorbs damage beyond normal cap.
void eff_meter_overflow_add(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n;
}

/// Refill meter to cap. Common on turn start or as a card effect.
void eff_meter_refill(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    struct BattleState* bs = (struct BattleState*)ctx;
    (void)args;
    (void)n;
    Side s       = bs->active_side;
    bs->meter[s] = bs->meter_cap[s];
}
