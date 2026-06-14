//! board.c
//!
//! Board state, territory, and threat queries.
//! Board holds a flat array of pointers; territory is king-adjacent squares.
//! Threat map enumerates every square an attacking piece threatens.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

/// board_index
///
/// Convert a Position to a flat array index.
///
/// Params:
/// - const Board* board -> board with dimensions
/// - Position p -> board coordinates
///
/// Return:
/// int -> index into board->squares
///
static int board_index(const Board* board, Position p) {
    return p.y * board->width + p.x;
}

/*--------------------------------------------------------------------------*\
                              BOARD INIT
\*--------------------------------------------------------------------------*/

/// board_init
///
/// Initialize a board with given dimensions. All squares start empty.
///
/// Params:
/// - Board* board -> board to initialize
/// - int width -> board width
/// - int height -> board height
///
void board_init(Board* board, int width, int height) {
    board->width  = width;
    board->height = height;
    for (int i = 0; i < MAX_BOARD_DIM * MAX_BOARD_DIM; i++) {
        board->squares[i] = NULL;
    }
}

/*--------------------------------------------------------------------------*\
                              PLACE / REMOVE
\*--------------------------------------------------------------------------*/

/// board_place
///
/// Place a piece at a square. Returns false if square is occupied.
///
/// Params:
/// - Board* board -> board to modify
/// - PieceState* piece -> piece to place
/// - Position pos -> destination square
///
/// Return:
/// bool -> true if placement succeeded
///
bool board_place(Board* board, PieceState* piece, Position pos) {
    if (!pos_in_bounds(pos, board->width, board->height))
        return false;
    int idx = board_index(board, pos);
    if (board->squares[idx] != NULL)
        return false;
    board->squares[idx] = piece;
    piece->pos          = pos;
    return true;
}

/// board_remove
///
/// Remove the piece at a square and return it.
///
/// Params:
/// - Board* board -> board to modify
/// - Position pos -> square to clear
///
/// Return:
/// PieceState* -> removed piece or NULL if square was empty
///
PieceState* board_remove(Board* board, Position pos) {
    if (!pos_in_bounds(pos, board->width, board->height))
        return NULL;
    int         idx     = board_index(board, pos);
    PieceState* p       = board->squares[idx];
    board->squares[idx] = NULL;
    return p;
}

/*--------------------------------------------------------------------------*\
                              AT
\*--------------------------------------------------------------------------*/

/// board_at
///
/// Return the piece at a square, or NULL if empty or out of bounds.
///
/// Params:
/// - const Board* board -> board to query
/// - Position pos -> square to query
///
/// Return:
/// PieceState* -> piece at square or NULL
///
PieceState* board_at(const Board* board, Position pos) {
    if (!pos_in_bounds(pos, board->width, board->height))
        return NULL;
    return board->squares[board_index(board, pos)];
}

/*--------------------------------------------------------------------------*\
                              TERRITORY
\*--------------------------------------------------------------------------*/

/// board_compute_territory
///
/// Compute which side controls each square.
///
/// Params:
/// - const Board* board -> board to analyze
/// - Side territory[] -> output array (size: width * height)
///
void board_compute_territory(const Board* board, Side territory[]) {
    for (int i = 0; i < board->width * board->height; i++) {
        territory[i] = SIDE_NEUTRAL;
    }
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            Position    pos = {x, y};
            PieceState* p   = board_at(board, pos);
            if (p != NULL) {
                int idx        = board_index(board, pos);
                territory[idx] = p->owner;
            }
        }
    }
}

/// board_territory_counts
///
/// Count how many squares belong to each territory.
///
/// Params:
/// - const Board* board -> board to analyze
/// - int counts[3] -> output: [SIDE_PLAYER, SIDE_ENEMY, SIDE_NEUTRAL]
///
void board_territory_counts(const Board* board, int counts[3]) {
    counts[SIDE_PLAYER]  = 0;
    counts[SIDE_ENEMY]   = 0;
    counts[SIDE_NEUTRAL] = 0;
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            Position    pos = {x, y};
            PieceState* p   = board_at(board, pos);
            if (p == NULL) {
                counts[SIDE_NEUTRAL]++;
            } else {
                counts[p->owner]++;
            }
        }
    }
}

/*--------------------------------------------------------------------------*\
                              THREAT MAP
\*--------------------------------------------------------------------------*/

/// board_threat_map
///
/// Build a threat map: for each square, how many pieces threaten it.
///
/// Params:
/// - const Board* board -> board to analyze
/// - int threats[] -> output array (size: width * height)
///
void board_threat_map(const Board* board, int threats[]) {
    for (int i = 0; i < board->width * board->height; i++) {
        threats[i] = 0;
    }
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            Position    pos = {x, y};
            PieceState* p   = board_at(board, pos);
            if (p == NULL)
                continue;
            MoveList ml = {0};
            mg_generate_threat(p, NULL, &ml);
            for (uint8_t j = 0; j < ml.count; j++) {
                if (pos_in_bounds(ml.squares[j], board->width, board->height)) {
                    int idx = board_index(board, ml.squares[j]);
                    threats[idx]++;
                }
            }
        }
    }
}

/*--------------------------------------------------------------------------*\
                              LINE OF SIGHT
\*--------------------------------------------------------------------------*/

/// board_has_line_of_sight
///
/// Return true if a path between two positions is unobstructed.
///
/// Params:
/// - const Board* board -> board to query
/// - Position from -> starting square
/// - Position to -> destination square
///
/// Return:
/// bool -> true if the path is clear
///
bool board_has_line_of_sight(const Board* board, Position from, Position to) {
    int dx  = to.x - from.x;
    int dy  = to.y - from.y;
    int adx = (dx > 0) ? dx : -dx;
    int ady = (dy > 0) ? dy : -dy;
    if (adx != 0 && ady != 0 && adx != ady)
        return false;
    int      step_x = (adx == 0) ? 0 : ((dx > 0) ? 1 : -1);
    int      step_y = (ady == 0) ? 0 : ((dy > 0) ? 1 : -1);
    int      steps  = (adx > ady) ? adx : ady;
    Position cur    = from;
    for (int s = 1; s < steps; s++) {
        cur.x += step_x;
        cur.y += step_y;
        if (board_at(board, cur) != NULL)
            return false;
    }
    return true;
}