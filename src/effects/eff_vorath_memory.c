//! eff_vorath_memory.c
//!
//! Vorath Memory tally and apply handlers. The tally fires every
//! time the player places a piece during a normal battle and bumps
//! `Profile.vorath_memory[tmpl_id]`. The apply handler is consulted
//! at the Vorath fight to spawn two counter-pieces — that fight is
//! not yet implemented, but the apply body is staged so it works
//! once the Vorath encounter lands.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              TALLY
\*--------------------------------------------------------------------------*/

void eff_vorath_memory_tally(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    PieceState* piece = ctx->as.piece.piece;
    if (piece == NULL || piece->tmpl == NULL)
        return;
    if (piece->owner != SIDE_PLAYER)
        return;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    Profile* profile = ctx->bs->config.run->profile;
    if (profile == NULL)
        return;
    uint16_t id = piece->tmpl->id;
    if (id < PIECE_ID_COUNT) {
        if (profile->vorath_memory[id] < UINT16_MAX)
            profile->vorath_memory[id]++;
    }
}

/*--------------------------------------------------------------------------*\
                              APPLY
\*--------------------------------------------------------------------------*/

void eff_vorath_memory_apply(
    struct EffectCtx* ctx,
    const EffectArg*  args,
    size_t            n
) {
    (void)args;
    (void)n;
    if (ctx->bs == NULL || ctx->bs->config.run == NULL)
        return;
    Profile* profile = ctx->bs->config.run->profile;
    if (profile == NULL)
        return;
    uint16_t top_id    = 0;
    uint16_t top_count = 0;
    for (uint16_t i = 0; i < PIECE_ID_COUNT; i++) {
        if (profile->vorath_memory[i] > top_count) {
            top_count = profile->vorath_memory[i];
            top_id    = i;
        }
    }
    if (top_count == 0)
        return;
    /* Counter-piece mapping is content-tuning; for now we spawn two
     * copies of whatever the most-played piece type was, mirrored to
     * the enemy side. Future round tunes via VORATH_COUNTER_TABLE. */
    int top = ctx->bs->board.height - 1;
    Position p0 = {0, (int8_t)top};
    Position p1 = {(int8_t)(ctx->bs->board.width - 1), (int8_t)top};
    piece_spawn(ctx->bs, top_id, p0, SIDE_ENEMY);
    piece_spawn(ctx->bs, top_id, p1, SIDE_ENEMY);
}
