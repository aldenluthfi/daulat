//! piece.h
//!
//! Piece templates and runtime state for the Regnum battle engine.
//! Template is immutable const data; State is per-instance mutable data.
//! Split Kewarani behavior is a passive Effect, not an engine branch.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef PIECE_H
#define PIECE_H

#include "effect.h"
#include "movegen.h"

/*--------------------------------------------------------------------------*\
                              PIECE TEMPLATE
\*--------------------------------------------------------------------------*/

/// Immutable description of a piece type. Lives in src/data/data_*.c.
typedef struct PieceTemplate {
    uint16_t    id;
    const char* name;
    Kingdom     kingdom;
    Tier        tier;
    int         base_value;
    MoveGen     move;
    MoveGen     threat; /* .func==NULL → reuse move           */
    Effect      passives[MAX_PIECE_PASSIVES];
    uint8_t     passive_count;
} PieceTemplate;

/*--------------------------------------------------------------------------*\
                              PIECE STATE
\*--------------------------------------------------------------------------*/

/// Runtime instance of a piece on the board.
typedef struct PieceState {
    uint32_t             id;
    const PieceTemplate* template;
    MoveGen              move_override;
    MoveGen              threat_override;
    Side                 owner;
    Position             pos;
    int                  value_mod;
    Effect               buffs[MAX_PIECE_BUFFS];
    uint8_t              buff_count;
    uint8_t              moves_used;
    uint16_t             flags;
} PieceState;

/*--------------------------------------------------------------------------*\
                              PIECE API
\*--------------------------------------------------------------------------*/

/// Spawn a piece from a template at position. Registers passives to
/// the bus and emits TRIGGER_PIECE_PLACED with PLACED_SPAWN.
uint32_t piece_spawn(
    struct BattleState* battle,
    uint16_t            template_id,
    Position            pos,
    Side                owner
);

/// Spawn a piece with an explicit placement cause (BOUGHT, COMBINE,
/// SPLIT, ...). The standard `piece_spawn` is the SPAWN-cause shortcut.
uint32_t piece_spawn_with_cause(
    struct BattleState* battle,
    uint16_t            template_id,
    Position            pos,
    Side                owner,
    PlacementCause      cause
);

/// Remove a piece from the board and evict its passives from the bus.
/// Default cause is SACRIFICE; use `piece_remove_with_cause` for the
/// other cases (Mandate card, splitter substitution).
void piece_remove(struct BattleState* battle, uint32_t piece_id);

/// Remove a piece with an explicit removal cause.
void piece_remove_with_cause(
    struct BattleState* battle,
    uint32_t            piece_id,
    RemovalCause        cause
);

/// Flip a piece to the opposite side. Default cause is METER_CASCADE;
/// use `piece_flip_with_cause` for Reclaim, forced flips, or Mercy.
void piece_flip(struct BattleState* battle, uint32_t piece_id);

/// Flip a piece with an explicit flip cause.
void piece_flip_with_cause(
    struct BattleState* battle,
    uint32_t            piece_id,
    FlipCause           cause
);

/// Find a piece by its runtime id. Returns NULL if not found.
PieceState* piece_by_id(struct BattleState* battle, uint32_t piece_id);

/// Compute the current value of a piece (base + modifiers).
int piece_value(const PieceState* piece);

#endif /* PIECE_H */
