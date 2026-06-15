//!
//! Flip effects: on-flip handlers, Kewarani splitter spawns.
//! TRIGGER_PIECE_FLIPPED fires first; Kewarani splitters intercept.
//! Flip effects are keyed to TRIGGER_PIECE_FLIPPED in the effect bus.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              FLIP MODIFIERS
\*--------------------------------------------------------------------------*/

/// Spawn a Medeq on flip (Kewarani splitter behavior).
void eff_splitter_medeq(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Grant immunity to flip this battle.
void eff_grant_flip_immunity(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Trigger on-flip: destroy an adjacent enemy piece.
void eff_flip_destroy_adjacent(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}
