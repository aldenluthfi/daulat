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

/// eff_swap_movegen
///
/// Swap a piece's movegen to a new pattern for the rest of the battle.
/// args[0]: MoveGenFunc id (stored in piece->move_override).
///
void eff_swap_movegen(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// eff_grant_extra_step
///
/// Grant an extra move step to a piece this action. This is used for
/// pieces like Bing (Longwei) which gain sideways step after damaging.
///
void eff_grant_extra_step(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// eff_grant_friendly_pass
///
/// Allow a piece to move through friendly pieces this action.
///
void eff_grant_friendly_pass(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}
