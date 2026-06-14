//!
//! Board state, territory, and threat queries.
//! Board holds a flat array of pointers; territory is king-adjacent squares.
//! Threat map enumerates every square an attacking piece threatens.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

static int board_index(const Board* board, Position p) {
    return p.y * board->width + p.x;
}

/*--------------------------------------------------------------------------*\
                              BOARD INIT
\*--------------------------------------------------------------------------*/

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

PieceState* board_at(const Board* board, Position pos) {
    if (!pos_in_bounds(pos, board->width, board->height))
        return NULL;
    return board->squares[board_index(board, pos)];
}

/*--------------------------------------------------------------------------*\
                              TERRITORY
\*--------------------------------------------------------------------------*/

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