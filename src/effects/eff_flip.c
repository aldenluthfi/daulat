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

/// eff_splitter_medeq
///
/// Kewarani splitter behavior: when this piece flips, spawn 2-3 friendly
/// Medeq pieces adjacent to the piece's position before it flips.
/// args[0]: spawn_count (2 or 3)
///
void eff_splitter_medeq(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    BattleState* bs = context->battle;
    PieceState* flipped = context->as.flipped.piece;
    if (count == 0 || flipped == NULL)
        return;
    int spawn_count = (int)args[0].v.i;
    if (spawn_count <= 0)
        spawn_count = 2;
    /* Spawn Medeq around the flipped piece's position */
    static const int DX[] = {1, -1, 0, 0};
    static const int DY[] = {0, 0, 1, -1};
    int spawned = 0;
    for (int i = 0; i < 4 && spawned < spawn_count; i++) {
        Position spawn = {flipped->pos.x + DX[i], flipped->pos.y + DY[i]};
        if (!pos_in_bounds(spawn, bs->board.width, bs->board.height))
            continue;
        if (board_at(&bs->board, spawn) != NULL)
            continue;
        piece_spawn(bs, PIECE_MEDEQ, spawn, flipped->owner);
        spawned++;
    }
}

/// eff_grant_flip_immunity
///
/// Grant the piece immunity to one flip this battle. Sets the IMMUNE_FLIP
/// flag which is checked during meter cascade resolution.
///
void eff_grant_flip_immunity(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->as.piece.piece != NULL)
        context->as.piece.piece->flags |= PSF_IMMUNE_FLIP;
}

/// eff_flip_destroy_adjacent
///
/// On flip: destroy one random adjacent enemy piece.
///
void eff_flip_destroy_adjacent(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}
