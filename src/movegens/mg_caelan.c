//! mg_caelan.c
//!
//! Bespoke Caelan movement patterns drawn from chess and Grant
//! Acedrex. The kingdom's stock pieces (Pawn, Knight, Bishop, Rook,
//! Queen) reuse mg_basics primitives directly; only the Gryphon's
//! two-stage diagonal-then-orthogonal walk lives here.
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
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             n      -> unused
/// - MoveList          *out    -> destination list
///
void mg_ca_gryphon(
    const PieceState*  piece,
    const BattleState* bs,
    const EffectArg*   params,
    size_t             n,
    MoveList*          out
) {
    (void)params;
    (void)n;
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
    for (int d = 0; d < 4; d++) {
        Position diag = {piece->pos.x + DIAG[d][0], piece->pos.y + DIAG[d][1]};
        if (!pos_in_bounds(diag, bs->board.width, bs->board.height)) {
            continue;
        }
        if (board_at(&bs->board, diag) != NULL)
            continue;
        for (int od = 0; od < 4; od++) {
            for (int dist = 1; dist < 20; dist++) {
                Position to = {
                    diag.x + ORTHO[od][0] * dist,
                    diag.y + ORTHO[od][1] * dist
                };
                if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
                    break;
                }
                const PieceState* at = board_at(&bs->board, to);
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
