//! mg_zarqan.c
//!
//! Bespoke Zarqan movement patterns drawn from Tamerlane Chess and
//! Shatranj. Covers the Ziraafa's diagonal-then-orthogonal walk, the
//! Shahzadeh's king swap (exposed as a synthetic destination), and
//! the War Elephant's multi-target threat pattern.
//!
//! Shared movegen helpers live in mg_basics.c; this file only adds
//! the bespoke functions and consumes the standard helper set.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                                 MG ZQ ZIRAAFA
\*--------------------------------------------------------------------------*/

/// mg_zq_ziraafa
///
/// One diagonal step followed by an orthogonal slide of at least
/// three squares in a consistent direction. The diagonal square must
/// be empty; the orthogonal slide stops on the first occupant and
/// only the final square may hold an enemy.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             n      -> unused
/// - MoveList          *out    -> destination list
///
void mg_zq_ziraafa(
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
        for (int dist = 3; dist < 20; dist++) {
            Position to = {
                diag.x + ORTHO[d][0] * dist,
                diag.y + ORTHO[d][1] * dist
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

/*--------------------------------------------------------------------------*\
                                 MG ZQ SWAP WITH KING
\*--------------------------------------------------------------------------*/

/// mg_zq_swap_with_king
///
/// Synthetic MoveGen used by Shahzadeh's free king swap. Returns the
/// friendly king's square so the action API can treat the swap as a
/// regular move target without bespoke validation paths.
///
/// Params:
/// - const PieceState  *piece  -> moving piece (the Shahzadeh)
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             n      -> unused
/// - MoveList          *out    -> destination list
///
void mg_zq_swap_with_king(
    const PieceState*  piece,
    const BattleState* bs,
    const EffectArg*   params,
    size_t             n,
    MoveList*          out
) {
    (void)params;
    (void)n;
    Side enemy = side_opposite(piece->owner);
    for (uint16_t i = 0; i < bs->piece_count; i++) {
        const PieceState* p = &bs->pieces[i];
        if (p->owner != enemy)
            continue;
        if (p->tmpl->id == PIECE_KING) {
            ml_push(out, p->pos);
            return;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 MG ZQ WAR ELEPHANT
\*--------------------------------------------------------------------------*/

/// mg_zq_war_elephant
///
/// Threat pattern enumerating every adjacent enemy square so a
/// single Ziraafa-line relocate can threaten multiple neighbours
/// simultaneously, matching the War Elephant's GDD entry.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> unused
/// - size_t             n      -> unused
/// - MoveList          *out    -> destination list
///
void mg_zq_war_elephant(
    const PieceState*  piece,
    const BattleState* bs,
    const EffectArg*   params,
    size_t             n,
    MoveList*          out
) {
    (void)params;
    (void)n;
    static const int8_t ADJ[8][2] = {
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
        Position to = {piece->pos.x + ADJ[i][0], piece->pos.y + ADJ[i][1]};
        if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
            continue;
        }
        if (is_enemy(piece, board_at(&bs->board, to))) {
            ml_push(out, to);
        }
    }
}
