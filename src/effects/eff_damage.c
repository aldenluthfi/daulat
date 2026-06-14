//!
//! Damage effects: multipliers, reductions, immunity.
//! Damage is computed in battle_resolve from piece_value and modifiers.
//! Damage effects are queried via TRIGGER_QUERY_DAMAGE_MULTIPLIER.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              DAMAGE MODIFIERS
\*--------------------------------------------------------------------------*/

/// Multiply damage output by a factor (stored as int basis-points).
void eff_damage_mult(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Reduce incoming damage by a flat amount.
void eff_damage_reduce(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Grant damage immunity to a piece.
void eff_grant_immunity(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Deal damage directly to a piece (not through resolve).
void eff_deal_damage(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}
