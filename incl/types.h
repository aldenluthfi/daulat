//! types.h
//!
//! Core type aliases and lightweight structs for the Regnum battle engine.
//! CardInstance, flags, TargetSpec, and inline helpers.
//! NOTE: Position is defined in core.h, NOT here.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef TYPES_H
#define TYPES_H

#include "defs.h"

/*--------------------------------------------------------------------------*\
                              FORWARD DECLARATIONS
\*--------------------------------------------------------------------------*/

struct CardTemplate;
struct PieceState;

/*--------------------------------------------------------------------------*\
                              CARD INSTANCE
\*--------------------------------------------------------------------------*/

/// A card instance held in a hand — template pointer plus runtime state.
typedef struct {
    const struct CardTemplate* template;
    uint8_t flags;
} CardInstance;

/// Flags that can be set on a CardInstance.
typedef enum {
    CIF_COMBO_SECOND = 1 << 0,
    CIF_COMBO_THIRD  = 1 << 1,
    CIF_TARGETED     = 1 << 2,
} CardInstanceFlags;

/*--------------------------------------------------------------------------*\
                              PIECE STATE FLAGS
\*--------------------------------------------------------------------------*/

/// Flags that can be set on a PieceState.
typedef enum {
    PSF_HAS_MOVED     = 1 << 0,
    PSF_CAN_COMBINE   = 1 << 1,
    PSF_IMMUNE_FLIP   = 1 << 2,
    PSF_IMMUNE_DAMAGE = 1 << 3,
    PSF_STUNNED       = 1 << 4,
    PSF_SPLITTER      = 1 << 5,
} PieceStateFlags;

/*--------------------------------------------------------------------------*\
                              TARGET SPEC
\*--------------------------------------------------------------------------*/

/// A target specification for card-play actions.
typedef struct {
    Position pos;
    uint32_t piece_id;
    bool     is_piece;
    bool     is_pos;
} TargetSpec;

/*--------------------------------------------------------------------------*\
                              INLINE HELPERS
\*--------------------------------------------------------------------------*/

/// Return the opposite side. SIDE_NEUTRAL returns SIDE_NEUTRAL.
static inline Side side_opposite(Side s) {
    if (s == SIDE_PLAYER)
        return SIDE_ENEMY;
    if (s == SIDE_ENEMY)
        return SIDE_PLAYER;
    return SIDE_NEUTRAL;
}

/// Return true if two positions are the same square.
static inline bool pos_equal(Position a, Position b) {
    return a.x == b.x && a.y == b.y;
}

/// Return true if a position is within the board bounds.
static inline bool pos_in_bounds(Position position, int width, int height) {
    return position.x >= 0 && position.x < width && position.y >= 0 &&
           position.y < height;
}

#endif /* TYPES_H */