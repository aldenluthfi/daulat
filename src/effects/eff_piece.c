//!
//! Piece effects: spawn, remove, swap, force flip.
//! Piece effects are keyed to TRIGGER_PIECE_* lifecycle events.
//! Spawned pieces are registered to the bus as passives immediately.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECE MODIFIERS
\*--------------------------------------------------------------------------*/

/// Spawn a piece template at a position.
void eff_spawn_piece(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Remove a piece from the board.
void eff_remove_piece(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Swap two pieces' positions.
void eff_swap_pieces(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Force-flip a piece to the opposite side.
void eff_force_flip(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}
