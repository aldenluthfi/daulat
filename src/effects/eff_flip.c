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
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Grant immunity to flip this battle.
void eff_grant_flip_immunity(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Trigger on-flip: destroy an adjacent enemy piece.
void eff_flip_destroy_adjacent(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
) {
    (void)ctx;
    (void)args;
    (void)n; 
}
