//! mg_longwei.c
//!
//! Bespoke Longwei movement patterns that the mg_basics primitives
//! cannot express directly: blocking elbows, screen-aware captures,
//! and two-stage walks. Each function fills the destination MoveList
//! and consults the standard movegen helpers from movegen.h.
//!
//! All functions share the standard MoveGenFunc signature so the
//! engine never special-cases a piece by name — only by template
//! pointer.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                                 MG LW MA
\*--------------------------------------------------------------------------*/

/// mg_lw_ma
///
/// Knight-L move blocked if any piece occupies the intermediate
/// "elbow" square. Captures land on enemy pieces only; the empty-
/// destination path goes through mg_step/mg_step_set elsewhere.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_lw_ma(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DELTAS[8][2] = {
        {1, 2},
        {-1, 2},
        {1, -2},
        {-1, -2},
        {2, 1},
        {-2, 1},
        {2, -1},
        {-2, -1},
    };
    static const int8_t ELBOW[8][2] = {
        {1, 1},
        {-1, 1},
        {1, -1},
        {-1, -1},
        {1, 1},
        {-1, 1},
        {1, -1},
        {-1, -1},
    };
    for (int i = 0; i < 8; i++) {
        Position to = {
            piece->pos.x + DELTAS[i][0],
            piece->pos.y + DELTAS[i][1]
        };
        Position elbow = {
            piece->pos.x + ELBOW[i][0],
            piece->pos.y + ELBOW[i][1]
        };
        if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
            continue;
        }
        if (board_at(&battle->board, elbow) != NULL)
            continue;
        const PieceState* at = board_at(&battle->board, to);
        if (is_enemy(piece, at))
            ml_push(out, to);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG LW XIANG
\*--------------------------------------------------------------------------*/

/// mg_lw_xiang
///
/// (2,2) blockable diagonal leap. The intermediate diagonal square
/// must be empty; the destination is reachable if empty or holds an
/// enemy.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_lw_xiang(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIRS[4][2] = {
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1},
    };
    for (int dir = 0; dir < 4; dir++) {
        Position mid = {piece->pos.x + DIRS[dir][0], piece->pos.y + DIRS[dir][1]};
        Position to  = {
            piece->pos.x + DIRS[dir][0] * 2,
            piece->pos.y + DIRS[dir][1] * 2
        };
        if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
            continue;
        }
        if (board_at(&battle->board, mid) != NULL)
            continue;
        const PieceState* at = board_at(&battle->board, to);
        if (at == NULL || is_enemy(piece, at))
            ml_push(out, to);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG LW PAO
\*--------------------------------------------------------------------------*/

/// mg_lw_pao
///
/// Rook-line movement with the Xiangqi cannon-capture rule: a Pao
/// may capture only when exactly one screen piece sits between it
/// and the target along the chosen line.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_lw_pao(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIRS[4][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    for (int dir = 0; dir < 4; dir++) {
        int  screens     = 0;
        bool found_first = false;
        for (int step = 1; step < 20; step++) {
            Position to = {
                piece->pos.x + DIRS[dir][0] * step,
                piece->pos.y + DIRS[dir][1] * step
            };
            if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
                break;
            }
            const PieceState* at = board_at(&battle->board, to);
            if (at == NULL) {
                if (!found_first)
                    ml_push(out, to);
                continue;
            }
            if (!found_first) {
                found_first = true;
                continue;
            }
            screens++;
            if (screens == 1 && is_enemy(piece, at)) {
                ml_push(out, to);
            }
            break;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 MG LW HWACHA
\*--------------------------------------------------------------------------*/

/// mg_lw_hwacha
///
/// Cannon-rule movement applied to all 8 directions. A Hwacha can move
/// to any empty square, but can only capture when exactly one piece sits
/// between it and the target. This is the Pao mechanic extended diagonally.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_lw_hwacha(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t DIRS[8][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1},
    };
    for (int dir = 0; dir < 8; dir++) {
        bool found_screen = false;
        for (int step = 1; step < 20; step++) {
            Position to = {
                piece->pos.x + DIRS[dir][0] * step,
                piece->pos.y + DIRS[dir][1] * step
            };
            if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
                break;
            }
            const PieceState* at = board_at(&battle->board, to);
            if (at == NULL) {
                ml_push(out, to);
                continue;
            }
            if (!found_screen) {
                found_screen = true;
                continue;
            }
            if (is_enemy(piece, at)) {
                ml_push(out, to);
            }
            break;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 MG LW SANG
\*--------------------------------------------------------------------------*/

/// mg_lw_sang
///
/// One orthogonal step followed by two diagonal steps in a consistent
/// direction. Both intermediate squares must be empty; the final
/// square may be empty or hold an enemy.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_lw_sang(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    static const int8_t ORTHO[4][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
    };
    static const int8_t DIAG[4][2] = {
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1},
    };
    for (int o = 0; o < 4; o++) {
        Position mid1 = {
            piece->pos.x + ORTHO[o][0],
            piece->pos.y + ORTHO[o][1]
        };
        if (!pos_in_bounds(mid1, battle->board.width, battle->board.height)) {
            continue;
        }
        if (board_at(&battle->board, mid1) != NULL)
            continue;
        for (int dir = 0; dir < 4; dir++) {
            Position mid2 = {mid1.x + DIAG[dir][0], mid1.y + DIAG[dir][1]};
            Position to   = {mid1.x + DIAG[dir][0] * 2, mid1.y + DIAG[dir][1] * 2};
            if (!pos_in_bounds(to, battle->board.width, battle->board.height)) {
                continue;
            }
            if (board_at(&battle->board, mid2) != NULL)
                continue;
            const PieceState* at = board_at(&battle->board, to);
            if (at == NULL || is_enemy(piece, at))
                ml_push(out, to);
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 MG LW LIUBO
\*--------------------------------------------------------------------------*/

/// mg_lw_liubo
///
/// Teleport to any square currently threatened by an enemy piece.
/// Implemented by reusing mg_generate_threat over the entire board —
/// the Diviner essentially copies the enemy's threat map.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *battle     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             count      -> unused
/// - MoveList          *out    -> destination list
///
void mg_lw_liubo(
    const PieceState*  piece,
    const BattleState* battle,
    const EffectArg*   params,
    size_t count,
    MoveList*          out
) {
    (void)params;
    (void)count;
    MoveList threats = {0};
    mg_generate_threat(piece, battle, &threats);
    for (uint8_t i = 0; i < threats.count; i++) {
        ml_push(out, threats.squares[i]);
    }
}
