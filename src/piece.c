//! piece.c
//!
//! Piece spawn, remove, flip, and query operations.
//! Passives are copied to the bus at spawn time and evicted at remove time.
//! Kewarani splitter behavior is a passive Effect, not an engine branch.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              SPAWN
\*--------------------------------------------------------------------------*/

/// piece_spawn
///
/// Spawn a piece from a template at position. Registers passives to bus.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint16_t tmpl_id -> template id to spawn
/// - Position pos -> board position
/// - Side owner -> owning side
///
/// Return:
/// uint32_t -> runtime id of spawned piece, or 0 on failure
///
uint32_t
piece_spawn(BattleState* bs, uint16_t tmpl_id, Position pos, Side owner) {
    if (bs->piece_count >= MAX_PIECES) {
        log_warn("piece_spawn: MAX_PIECES (%d) reached\n", MAX_PIECES);
        return 0;
    }
    const PieceTemplate* tmpl = piece_template(tmpl_id);
    if (tmpl == NULL) {
        log_warn("piece_spawn: unknown template id %u\n", tmpl_id);
        return 0;
    }
    PieceState* p           = &bs->pieces[bs->piece_count++];
    p->id                   = bs->next_piece_id++;
    p->tmpl                 = tmpl;
    p->owner                = owner;
    p->pos                  = pos;
    p->value_mod            = 0;
    p->buff_count           = 0;
    p->moves_used           = 0;
    p->flags                = 0;
    p->streak_attack        = 0;
    p->move_override.func   = NULL;
    p->threat_override.func = NULL;
    board_place(&bs->board, p, pos);
    for (uint8_t i = 0; i < tmpl->passive_count; i++) {
        Effect e    = tmpl->passives[i];
        e.source_id = p->id;
        e.owner     = owner;
        bus_register(&bs->bus, &e);
    }
    struct EffectCtx ctx = {0};
    ctx.as.piece.piece   = p;
    ctx.as.piece.pos     = &pos;
    bus_emit(&bs->bus, bs, TRIGGER_PIECE_PLACED, &ctx);
    return p->id;
}

/*--------------------------------------------------------------------------*\
                              REMOVE
\*--------------------------------------------------------------------------*/

/// piece_remove
///
/// Remove a piece from the board and evict its passives from the bus.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint32_t piece_id -> runtime id of piece to remove
///
void piece_remove(BattleState* bs, uint32_t piece_id) {
    PieceState* p = piece_by_id(bs, piece_id);
    if (p == NULL)
        return;
    board_remove(&bs->board, p->pos);
    bus_evict_by_source(&bs->bus, piece_id);
    struct EffectCtx ctx = {0};
    ctx.as.piece.piece   = p;
    bus_emit(&bs->bus, bs, TRIGGER_PIECE_REMOVED, &ctx);
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        if (bs->pieces[i].id == piece_id) {
            bs->pieces[i] = bs->pieces[--bs->piece_count];
            return;
        }
    }
}

/*--------------------------------------------------------------------------*\
                              FLIP
\*--------------------------------------------------------------------------*/

/// piece_flip
///
/// Flip a piece to the opposite side. Emits TRIGGER_PIECE_FLIPPED first.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint32_t piece_id -> runtime id of piece to flip
///
void piece_flip(BattleState* bs, uint32_t piece_id) {
    PieceState* p = piece_by_id(bs, piece_id);
    if (p == NULL)
        return;
    struct EffectCtx ctx = {0};
    ctx.as.piece.piece   = p;
    bus_emit(&bs->bus, bs, TRIGGER_PIECE_FLIPPED, &ctx);
    p->owner = side_opposite(p->owner);
    board_remove(&bs->board, p->pos);
    board_place(&bs->board, p, p->pos);
    bus_evict_by_source(&bs->bus, piece_id);
    for (uint8_t i = 0; i < p->tmpl->passive_count; i++) {
        Effect e    = p->tmpl->passives[i];
        e.source_id = p->id;
        e.owner     = p->owner;
        bus_register(&bs->bus, &e);
    }
}

/*--------------------------------------------------------------------------*\
                              QUERY
\*--------------------------------------------------------------------------*/

/// piece_by_id
///
/// Find a piece by its runtime id.
///
/// Params:
/// - BattleState* bs -> battle state to search
/// - uint32_t piece_id -> runtime id to find
///
/// Return:
/// PieceState* -> piece pointer or NULL if not found
///
PieceState* piece_by_id(BattleState* bs, uint32_t piece_id) {
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        if (bs->pieces[i].id == piece_id)
            return &bs->pieces[i];
    }
    return NULL;
}

/// piece_value
///
/// Compute the current value of a piece (base + modifiers).
///
/// Params:
/// - const PieceState* piece -> piece to evaluate
///
/// Return:
/// int -> current value
///
int piece_value(const PieceState* piece) {
    return piece->tmpl->base_value + piece->value_mod;
}
