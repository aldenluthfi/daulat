//!
//! Movement effects: movegen swap, free moves, extra steps.
//! Movement effects hook into the battle action API for movement.
//! Extra steps extend the max_steps parameter in mg_basics primitives.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              MOVEMENT MODIFIERS
\*--------------------------------------------------------------------------*/

/// Swap a piece's movegen to a new pattern for the rest of the battle.
void eff_swap_movegen(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Grant an extra move step to a piece this action.
void eff_grant_extra_step(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Allow a piece to move through friendly pieces this action.
void eff_grant_friendly_pass(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n; 
}
