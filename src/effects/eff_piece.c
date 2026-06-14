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
void eff_spawn_piece(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Remove a piece from the board.
void eff_remove_piece(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Swap two pieces' positions.
void eff_swap_pieces(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}

/// Force-flip a piece to the opposite side.
void eff_force_flip(struct EffectCtx* ctx, const EffectArg* args, size_t n) {
    (void)ctx;
    (void)args;
    (void)n; 
}
