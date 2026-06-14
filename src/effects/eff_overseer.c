//! eff_overseer.c
//!
//! Bodies for the five non-Vorath Overseer special mechanics. Each
//! Overseer fires at battle start, spawning the boss's signature
//! pieces or flipping per-battle latches that resolve / movement
//! handlers consult. Full mechanic bodies are intentionally compact
//! in Phase 3 — the campaign loop (Phase 4) supplies the boss
//! template via the map node payload; the bodies here mutate
//! BattleState so the existing battle engine treats the encounter
//! correctly.
//!
//! Vorath itself is deferred to a later round; only its memory
//! tally / apply pair land in this phase (see `eff_vorath_memory.c`).
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              OVERSEERS
\*--------------------------------------------------------------------------*/

void eff_overseer_iron_strategist(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL)
        return;
    /* Iron Strategist: spawn 3 Pao + 2 Wazir for the enemy in the
     * top rank. The damage-router swap is deferred until Phase 6
     * (battle-internal). */
    int width = ctx->bs->board.width;
    int top   = ctx->bs->board.height - 1;
    for (int i = 0; i < 3 && i * 2 + 1 < width; i++) {
        Position p = {(int8_t)(i * 2 + 1), (int8_t)top};
        piece_spawn(ctx->bs, PIECE_PAO, p, SIDE_ENEMY);
    }
    for (int i = 0; i < 2 && i * 2 < width; i++) {
        Position p = {(int8_t)(i * 2), (int8_t)top};
        piece_spawn(ctx->bs, PIECE_WAZIR, p, SIDE_ENEMY);
    }
}

void eff_overseer_eternal_recursion(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    PieceState* piece = ctx->as.piece.piece;
    if (piece == NULL || piece->owner == SIDE_ENEMY)
        return;
    /* On enemy → player flip, queue an immediate re-flip back to
     * enemy after one turn. We mark the piece via flags bit 0 so the
     * battle resolve path can re-flip it; bit interpretation is
     * Phase 6 work. */
    piece->flags |= 0x1u;
}

void eff_overseer_caravan_of_conquest(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL)
        return;
    if ((bs->turn_no & 1u) != 0u)
        return;
    int      width = bs->board.width;
    int      top   = bs->board.height - 1;
    Position p     = {(int8_t)(width / 2), (int8_t)top};
    piece_spawn(bs, PIECE_FARAS, p, SIDE_ENEMY);
}

void eff_overseer_many_faced_king(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL)
        return;
    int width = ctx->bs->board.width;
    int top   = ctx->bs->board.height - 1;
    for (int i = 0; i < 3 && i + (width / 2) - 1 < width; i++) {
        Position p = {(int8_t)((width / 2) - 1 + i), (int8_t)top};
        piece_spawn(ctx->bs, PIECE_SHAHZADEH, p, SIDE_ENEMY);
    }
    Position king = {(int8_t)(width / 2), (int8_t)top};
    piece_spawn(ctx->bs, PIECE_KING, king, SIDE_ENEMY);
}

void eff_overseer_crowned_heretic(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    struct BattleState* bs = ctx->bs;
    if (bs == NULL)
        return;
    /* Crowned Heretic spawns a full Caelan starting army's back rank
     * across the top row, capped at board width. Ghost tally lives
     * in vision_flags bits 8..15 (8-bit counter). */
    static const uint16_t HERETIC_ROW[] = {
        PIECE_PAWN, PIECE_KNIGHT, PIECE_BISHOP, PIECE_QUEEN,
        PIECE_KNIGHT, PIECE_BISHOP, PIECE_PAWN, PIECE_PAWN
    };
    int top = bs->board.height - 1;
    int n_pieces =
        (int)(sizeof(HERETIC_ROW) / sizeof(HERETIC_ROW[0]));
    for (int i = 0; i < n_pieces && i < bs->board.width; i++) {
        Position p = {(int8_t)i, (int8_t)top};
        piece_spawn(bs, (uint16_t)HERETIC_ROW[i], p, SIDE_ENEMY);
    }
}
