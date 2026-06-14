//!
//! Board state, territory, and threat queries for the Regnum engine.
//! Board holds a flat array of MAX_BOARD_DIM x MAX_BOARD_DIM pointers.
//! Territory is computed from piece positions each turn.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef BOARD_H
#define BOARD_H

#include "types.h"

struct PieceState;

/*--------------------------------------------------------------------------*\
                              BOARD
\*--------------------------------------------------------------------------*/

/// The playing surface. Flat storage indexed [y * width + x].
typedef struct {
    struct PieceState* squares[MAX_BOARD_DIM * MAX_BOARD_DIM];
    int width;
    int height;
} Board;

/*--------------------------------------------------------------------------*\
                              BOARD API
\*--------------------------------------------------------------------------*/

/// Initialize a board with given dimensions. All squares start empty.
void board_init(Board* board, int width, int height);

/// Place a piece at a square. Returns false if square is occupied.
bool board_place(Board* board, struct PieceState* piece, Position pos);

/// Remove the piece at a square and return it. Returns NULL if empty.
struct PieceState* board_remove(Board* board, Position pos);

/// Return the piece at a square, or NULL if empty / out of bounds.
struct PieceState* board_at(const Board* board, Position pos);

/// Compute which side controls each square.
void board_compute_territory(const Board* board, Side territory[]);

/// Count how many squares of each territory.
void board_territory_counts(const Board* board, int counts[3]);

/// Build a threat map: for each square, how many pieces threaten it.
void board_threat_map(const Board* board, int threats[]);

/// Return true if a path between two positions is unobstructed.
bool board_has_line_of_sight(const Board* board, Position from, Position to);

#endif /* BOARD_H */
