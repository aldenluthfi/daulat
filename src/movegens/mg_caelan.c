//! mg_caelan.c
//!
//! Bespoke Caelan movement patterns drawn from chess and Grant
//! Acedrex. The kingdom's stock pieces (Pawn, Knight, Bishop, Rook,
//! Queen) reuse mg_basics primitives directly. This file implements
//! the Gryphon's two-stage diagonal-then-orthogonal movement.
//!
//! Shared movegen helpers live in mg_basics.c; this file simply
//! consumes them and adds the Gryphon-specific pattern.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                                 MG CA GRYPHON
\*--------------------------------------------------------------------------*/

/// mg_ca_gryphon
///
/// One diagonal step followed by an orthogonal slide of any
/// distance. The diagonal square must be empty; the orthogonal
/// slide stops on the first occupant and only captures on enemies.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_ca_gryphon(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIAG[4][2] = {
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1},
    };
    static const int8_t ORTHO[4][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    for (int dir = 0; dir < 4; dir++) {
        Position diag = {piece->pos.x + DIAG[dir][0], piece->pos.y + DIAG[dir][1]};
        if (!pos_in_bounds(diag, battle->board.width, battle->board.height)) {
            continue;
        }
        if (board_at(&battle->board, diag) != NULL)
            continue;
        for (int od = 0; od < 4; od++) {
            for (int dist = 1; dist < 20; dist++) {
                Position to = {
                    diag.x + ORTHO[od][0] * dist,
                    diag.y + ORTHO[od][1] * dist
                };
                if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
                    break;
                }
                const PieceState* at = board_at(&battle->board, to);
                if (at == NULL) {
                    ml_push(out, to);
                    continue;
                }
                if (is_enemy(piece, at))
                    ml_push(out, to);
                break;
            }
        }
    }
}
