//! mg_kewarani.c
//!
//! Bespoke Kewarani movement patterns drawn from Senterej tradition.
//! Covers the Berolina pawn (diagonal relocate plus forward attack)
//! and the Negus Guard's double-act king-set step.
//!
//! Movegen helpers (ml_push, is_enemy) live in mg_basics.c; this
//! translation unit only consumes them and adds the patterns specific
//! to Kewarani pieces.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                                 MG KW BEROLINA
\*--------------------------------------------------------------------------*/

/// mg_kw_berolina
///
/// Berolina-pawn movement: diagonal one-step relocate (empty squares
/// only) plus a one-step forward attack that captures enemies only.
/// params[0] is the forward dy so the same pattern serves both
/// directions (Kewarani plays from the top of the board, dy=-1).
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> [0]=forward dy
/// - size_t             n      -> parameter count
/// - MoveList          *out    -> destination list
///
void mg_kw_berolina(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    int fwd_dy = -1;
    if (n >= 1)
        fwd_dy = (int)params[0].v.i;
    static const int8_t DIAG[4][2] = {
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1},
    };
    for (int i = 0; i < 4; i++) {
        Position to = {piece->pos.x + DIAG[i][0], piece->pos.y + DIAG[i][1]};
        if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
            continue;
        }
        if (board_at(&bs->board, to) == NULL)
            ml_push(out, to);
    }
    Position atk = {piece->pos.x, piece->pos.y + fwd_dy};
    if (pos_in_bounds(atk, bs->board.width, bs->board.height) &&
        is_enemy(piece, board_at(&bs->board, atk))) {
        ml_push(out, atk);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG KW NEGUS GUARD
\*--------------------------------------------------------------------------*/

/// mg_kw_negus_guard
///
/// Double-act king-set step. The piece may take two consecutive
/// king-direction steps in one action; the intermediate square must
/// be empty so the chain resolves cleanly.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             n      -> unused
/// - MoveList          *out    -> destination list
///
void mg_kw_negus_guard(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)params;
    (void)n;
    static const int8_t KING[8][2] = {
        {1, 0},
        {1, 1},
        {0, 1},
        {-1, 1},
        {-1, 0},
        {-1, -1},
        {0, -1},
        {1, -1},
    };
    for (int i = 0; i < 8; i++) {
        Position mid = {piece->pos.x + KING[i][0], piece->pos.y + KING[i][1]};
        if (!pos_in_bounds(mid, bs->board.width, bs->board.height)) {
            continue;
        }
        if (board_at(&bs->board, mid) != NULL)
            continue;
        for (int j = 0; j < 8; j++) {
            Position to = {mid.x + KING[j][0], mid.y + KING[j][1]};
            if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
                continue;
            }
            const PieceState *at = board_at(&bs->board, to);
            if (at == NULL || is_enemy(piece, at))
                ml_push(out, to);
        }
    }
}
