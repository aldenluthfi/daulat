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
/// - BattleState* battle -> battle state to modify
/// - uint16_t template_id -> template id to spawn
/// - Position pos -> board position
/// - Side owner -> owning side
///
/// Return:
/// uint32_t -> runtime id of spawned piece, or 0 on failure
///
uint32_t piece_spawn_with_cause(
    BattleState*   battle,
    uint16_t       template_id,
    Position       pos,
    Side           owner,
    PlacementCause cause
) {
    if (battle->piece_count >= MAX_PIECES) {
        log_warn("piece_spawn: MAX_PIECES (%d) reached\n", MAX_PIECES);
        return 0;
    }
    const PieceTemplate* template = piece_template(template_id);
    if (template == NULL) {
        log_warn("piece_spawn: unknown template id %u\n", template_id);
        return 0;
    }
    PieceState* piece           = &battle->pieces[battle->piece_count++];
    piece->id                   = battle->next_piece_id++;
    piece->template             = template;
    piece->owner                = owner;
    piece->pos                  = pos;
    piece->value_mod            = 0;
    piece->buff_count           = 0;
    piece->moves_used           = 0;
    piece->flags                = 0;
    piece->move_override.func   = NULL;
    piece->threat_override.func = NULL;
    board_place(&battle->board, piece, pos);
    for (uint8_t i = 0; i < template->passive_count; i++) {
        Effect effect    = template->passives[i];
        effect.source_id = piece->id;
        effect.owner     = owner;
        bus_register(&battle->bus, &effect);
    }
    struct EffectCtx context = {0};
    context.as.placed.piece  = piece;
    context.as.placed.pos    = &pos;
    context.as.placed.cause  = cause;
    bus_emit(&battle->bus, battle, TRIGGER_PIECE_PLACED, &context);
    return piece->id;
}

uint32_t piece_spawn(
    BattleState* battle,
    uint16_t     template_id,
    Position     pos,
    Side         owner
) {
    return piece_spawn_with_cause(
        battle,
        template_id,
        pos,
        owner,
        PLACED_SPAWN
    );
}

/*--------------------------------------------------------------------------*\
                              REMOVE
\*--------------------------------------------------------------------------*/

/// piece_remove
///
/// Remove a piece from the board and evict its passives from the bus.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - uint32_t piece_id -> runtime id of piece to remove
///
void piece_remove_with_cause(
    BattleState* battle,
    uint32_t     piece_id,
    RemovalCause cause
) {
    PieceState* piece = piece_by_id(battle, piece_id);
    if (piece == NULL)
        return;
    board_remove(&battle->board, piece->pos);
    bus_evict_by_source(&battle->bus, piece_id);
    struct EffectCtx context = {0};
    context.as.removed.piece = piece;
    context.as.removed.cause = cause;
    bus_emit(&battle->bus, battle, TRIGGER_PIECE_REMOVED, &context);
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        if (battle->pieces[i].id == piece_id) {
            battle->pieces[i] = battle->pieces[--battle->piece_count];
            return;
        }
    }
}

void piece_remove(BattleState* battle, uint32_t piece_id) {
    piece_remove_with_cause(battle, piece_id, REMOVED_SACRIFICE);
}

/*--------------------------------------------------------------------------*\
                              FLIP
\*--------------------------------------------------------------------------*/

/// piece_flip
///
/// Flip a piece to the opposite side. Emits TRIGGER_PIECE_FLIPPED first.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - uint32_t piece_id -> runtime id of piece to flip
///
void piece_flip_with_cause(
    BattleState* battle,
    uint32_t     piece_id,
    FlipCause    cause
) {
    PieceState* piece = piece_by_id(battle, piece_id);
    if (piece == NULL)
        return;
    Side             old_owner   = piece->owner;
    Side             new_owner   = side_opposite(piece->owner);
    struct EffectCtx context     = {0};
    context.as.flipped.piece     = piece;
    context.as.flipped.old_owner = old_owner;
    context.as.flipped.new_owner = new_owner;
    context.as.flipped.cause     = cause;
    bus_emit(&battle->bus, battle, TRIGGER_PIECE_FLIPPED, &context);
    piece->owner = new_owner;
    board_remove(&battle->board, piece->pos);
    board_place(&battle->board, piece, piece->pos);
    bus_evict_by_source(&battle->bus, piece_id);
    for (uint8_t i = 0; i < piece->template->passive_count; i++) {
        Effect effect    = piece->template->passives[i];
        effect.source_id = piece->id;
        effect.owner     = piece->owner;
        bus_register(&battle->bus, &effect);
    }
}

void piece_flip(BattleState* battle, uint32_t piece_id) {
    piece_flip_with_cause(battle, piece_id, FLIPPED_METER_CASCADE);
}

/*--------------------------------------------------------------------------*\
                              QUERY
\*--------------------------------------------------------------------------*/

/// piece_by_id
///
/// Find a piece by its runtime id.
///
/// Params:
/// - BattleState* battle -> battle state to search
/// - uint32_t piece_id -> runtime id to find
///
/// Return:
/// PieceState* -> piece pointer or NULL if not found
///
PieceState* piece_by_id(BattleState* battle, uint32_t piece_id) {
    for (uint16_t i = 0; i < battle->piece_count; i++) {
        if (battle->pieces[i].id == piece_id)
            return &battle->pieces[i];
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
    return piece->template->base_value + piece->value_mod;
}
