//! mg_harushima.c
//!
//! Bespoke Harushima movement patterns drawn from Shogi and Chu
//! Shogi traditions: the gold and silver general step-sets, the
//! asymmetric Honorable Horse leap, and the Lion's twin-action
//! attack pattern.
//!
//! Shared movegen helpers (ml_push, is_enemy, can_capture_or_empty)
//! live in mg_basics.c so this file simply uses them — never
//! redeclares them.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                                 MG HS KINSHO
\*--------------------------------------------------------------------------*/

/// mg_hs_kinsho
///
/// Six-direction step set: forward, both forward-diagonals, left,
/// right, and backward. Diagonal-back is intentionally absent. The
/// piece may move into empty squares or capture enemies.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_hs_kinsho(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIRS[6][2] = {
        {0, 1},
        {1, 1},
        {-1, 1},
        {1, 0},
        {-1, 0},
        {0, -1},
    };
    for (int i = 0; i < 6; i++) {
        Position to = {piece->pos.x + DIRS[i][0], piece->pos.y + DIRS[i][1]};
        if (can_capture_or_empty(battle, piece, to))
            ml_push(out, to);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG HS GINSHO
\*--------------------------------------------------------------------------*/

/// mg_hs_ginsho
///
/// Five-direction step set: forward, both forward-diagonals, left
/// and right. Cannot retreat. Movement allows empty squares or
/// enemy captures.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_hs_ginsho(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIRS[5][2] = {
        {0, 1},
        {1, 1},
        {-1, 1},
        {1, 0},
        {-1, 0},
    };
    for (int i = 0; i < 5; i++) {
        Position to = {piece->pos.x + DIRS[i][0], piece->pos.y + DIRS[i][1]};
        if (can_capture_or_empty(battle, piece, to))
            ml_push(out, to);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG HS HONORABLE HORSE
\*--------------------------------------------------------------------------*/

/// mg_hs_honorable_horse
///
/// Asymmetric two-forward-one-side leap. params[0] selects which
/// side the leap favours (+1 right, -1 left); the front square must
/// be empty for the long leap, but the short diagonal target may be
/// taken even when blocked above.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> [0]=lateral direction (+1/-1)
/// - size_t             count      -> parameter count
/// - MoveList          *out    -> destination list
///
void mg_hs_honorable_horse(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    int dir = 1;
    if (count >= 1)
        dir = (int)params[0].v.i;
    Position mid = {piece->pos.x, piece->pos.y + 2};
    if (pos_in_bounds(mid, battle->board.width, battle->board.height) &&
        board_at(&battle->board, mid) == NULL) {
        Position to = {piece->pos.x + dir, piece->pos.y + 2};
        if (can_capture_or_empty(battle, piece, to))
            ml_push(out, to);
    }
    Position to2 = {piece->pos.x + dir, piece->pos.y + 1};
    if (can_capture_or_empty(battle, piece, to2))
        ml_push(out, to2);
}

/*--------------------------------------------------------------------------*\
                                 MG HS SHISHI
\*--------------------------------------------------------------------------*/

/// mg_hs_shishi
///
/// Lion move: two king-direction steps in sequence (move then move),
/// or a single step followed by an attack that does not commit to
/// the step (capture from origin). The first step must land on an
/// empty square so the chain can resolve cleanly.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_hs_shishi(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIRS[8][2] = {
        {1, 0},
        {1, 1},
        {0, 1},
        {-1, 1},
        {-1, 0},
        {-1, -1},
        {0, -1},
        {1, -1},
    };
    for (int d1 = 0; d1 < 8; d1++) {
        Position mid = {piece->pos.x + DIRS[d1][0], piece->pos.y + DIRS[d1][1]};
        if (!pos_in_bounds(mid, battle->board.width, battle->board.height)) {
            continue;
        }
        if (board_at(&battle->board, mid) != NULL)
            continue;
        for (int d2 = 0; d2 < 8; d2++) {
            Position to = {mid.x + DIRS[d2][0], mid.y + DIRS[d2][1]};
            if (can_capture_or_empty(battle, piece, to))
                ml_push(out, to);
        }
    }
    for (int d1 = 0; d1 < 8; d1++) {
        Position mid = {piece->pos.x + DIRS[d1][0], piece->pos.y + DIRS[d1][1]};
        if (!pos_in_bounds(mid, battle->board.width, battle->board.height)) {
            continue;
        }
        if (board_at(&battle->board, mid) != NULL)
            continue;
        for (int d2 = 0; d2 < 8; d2++) {
            Position to = {mid.x + DIRS[d2][0], mid.y + DIRS[d2][1]};
            if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
                continue;
            }
            if (is_enemy(piece, board_at(&battle->board, to))) {
                ml_push(out, to);
            }
        }
    }
}
