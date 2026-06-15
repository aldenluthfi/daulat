//! eff_vorath_memory.c
//!
//! Vorath Memory tally and apply handlers. The tally fires every
//! time the player places a piece during a normal battle and bumps
//! `Profile.vorath_memory[template_id]`. The apply handler is consulted
//! at the Vorath fight to spawn two counter-pieces. The apply body
//! is staged so it activates when the Vorath encounter is implemented.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              TALLY
\*--------------------------------------------------------------------------*/

void eff_vorath_memory_tally(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    PieceState* piece = context->as.piece.piece;
    if (piece == NULL || piece->template == NULL)
        return;
    if (piece->owner != SIDE_PLAYER)
        return;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    Profile* profile = context->battle->config.run->profile;
    if (profile == NULL)
        return;
    uint16_t id = piece->template->id;
    if (id < PIECE_ID_COUNT) {
        if (profile->vorath_memory[id] < UINT16_MAX)
            profile->vorath_memory[id]++;
    }
}

/*--------------------------------------------------------------------------*\
                              APPLY
\*--------------------------------------------------------------------------*/

void eff_vorath_memory_apply(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)args;
    (void)count;
    if (context->battle == NULL || context->battle->config.run == NULL)
        return;
    Profile* profile = context->battle->config.run->profile;
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
     * the enemy side. Counter tuning lives in VORATH_COUNTER_TABLE. */
    int top = context->battle->board.height - 1;
    Position p0 = {0, (int8_t)top};
    Position p1 = {(int8_t)(context->battle->board.width - 1), (int8_t)top};
    piece_spawn(context->battle, top_id, p0, SIDE_ENEMY);
    piece_spawn(context->battle, top_id, p1, SIDE_ENEMY);
}
