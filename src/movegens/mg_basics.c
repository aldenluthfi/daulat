//! mg_basics.c
//!
//! Parameterised primitives and composition wrappers that cover
//! every "parameterised chess" movement pattern. Generic patterns
//! (knight, bishop, rook, queen, king, pawn) are not standalone
//! functions — they are these primitives invoked with the right
//! arguments from each kingdom's data file.
//!
//! Also hosts the movegen helpers used by every mg_*.c translation
//! unit (ml_push, is_enemy, is_friendly, can_move_to, can_capture,
//! can_capture_or_empty). They live here exactly once.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HELPERS
\*--------------------------------------------------------------------------*/

/// ml_push
///
/// Append `p` to `ml`. Silently drops the addition if the list is
/// already at capacity; the caller never has to bounds-check.
///
/// Params:
/// - MoveList *ml -> destination list
/// - Position  p  -> board square to append
///
void ml_push(MoveList *ml, Position p) {
    if (ml->count >= MAX_MOVES)
        return;
    ml->squares[ml->count++] = p;
}

/// is_friendly
///
/// Predicate: does `at` belong to the same side as `piece`. NULL
/// targets return false.
///
/// Params:
/// - const PieceState *piece -> querying piece
/// - const PieceState *at    -> piece on the target square or NULL
///
/// Return:
/// bool -> true when both pieces share an owner
///
bool is_friendly(const PieceState *piece, const PieceState *at) {
    if (at == NULL)
        return false;
    return at->owner == piece->owner;
}

/// is_enemy
///
/// Predicate: does `at` belong to the side opposite `piece`. NULL
/// and SIDE_NEUTRAL targets return false.
///
/// Params:
/// - const PieceState *piece -> querying piece
/// - const PieceState *at    -> piece on the target square or NULL
///
/// Return:
/// bool -> true when `at` is an opposing-side piece
///
bool is_enemy(const PieceState *piece, const PieceState *at) {
    if (at == NULL)
        return false;
    return at->owner != piece->owner && at->owner != SIDE_NEUTRAL;
}

/// can_move_to
///
/// Combine bounds and friendly-occupancy checks for a candidate
/// destination. Returns true when the move would not violate base
/// movement rules.
///
/// Params:
/// - const BattleState *bs    -> battle context
/// - const PieceState  *piece -> moving piece
/// - Position           to    -> destination square
///
/// Return:
/// bool -> true when the destination is reachable
///
bool can_move_to(const BattleState *bs, const PieceState *piece, Position to) {
    if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
        return false;
    }
    const PieceState *at = board_at(&bs->board, to);
    if (is_friendly(piece, at))
        return false;
    return true;
}

/// can_capture
///
/// Predicate: does `to` hold an enemy piece within the board.
///
/// Params:
/// - const BattleState *bs    -> battle context
/// - const PieceState  *piece -> querying piece
/// - Position           to    -> target square
///
/// Return:
/// bool -> true when an enemy piece sits at `to` within the board
///
bool can_capture(const BattleState *bs, const PieceState *piece, Position to) {
    if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
        return false;
    }
    const PieceState *at = board_at(&bs->board, to);
    return is_enemy(piece, at);
}

/// can_capture_or_empty
///
/// Predicate matching the "advance or attack" rule used by several
/// step-based pieces: the destination is on the board and either
/// empty or holds an enemy.
///
/// Params:
/// - const BattleState *bs    -> battle context
/// - const PieceState  *piece -> querying piece
/// - Position           to    -> target square
///
/// Return:
/// bool -> true when the square is occupiable by `piece`
///
bool can_capture_or_empty(
    const BattleState *bs,
    const PieceState *piece,
    Position to
) {
    if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
        return false;
    }
    const PieceState *at = board_at(&bs->board, to);
    return at == NULL || is_enemy(piece, at);
}

/*--------------------------------------------------------------------------*\
                                 MG STEP
\*--------------------------------------------------------------------------*/

/// mg_step
///
/// Single-step relocate by the (dx, dy) offset carried in params.
/// Validates against can_move_to before appending.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> [0]=dx, [1]=dy
/// - size_t             n      -> parameter count (must be ≥ 2)
/// - MoveList          *out    -> destination list
///
void mg_step(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    if (n < 2)
        return;
    Position to = {
        piece->pos.x + (int)params[0].v.i,
        piece->pos.y + (int)params[1].v.i
    };
    if (can_move_to(bs, piece, to))
        ml_push(out, to);
}

/*--------------------------------------------------------------------------*\
                                 MG STEP SET
\*--------------------------------------------------------------------------*/

/// mg_step_set
///
/// Set of single-step relocations defined by interleaved (dx, dy)
/// pairs. Each pair contributes one destination if it passes
/// can_move_to.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> alternating dx, dy values
/// - size_t             n      -> parameter count (must be even)
/// - MoveList          *out    -> destination list
///
void mg_step_set(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    for (size_t i = 0; i + 1 < n; i += 2) {
        Position to = {
            piece->pos.x + (int)params[i].v.i,
            piece->pos.y + (int)params[i + 1].v.i
        };
        if (can_move_to(bs, piece, to))
            ml_push(out, to);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG SLIDE
\*--------------------------------------------------------------------------*/

/// mg_slide
///
/// Linear slide along a single direction. The slide stops on the
/// first non-empty square; an enemy at that square is captured.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> [0]=dx, [1]=dy, [2]=min, [3]=max
/// - size_t             n      -> parameter count (must be ≥ 4)
/// - MoveList          *out    -> destination list
///
void mg_slide(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    if (n < 4)
        return;
    int dx = (int)params[0].v.i;
    int dy = (int)params[1].v.i;
    int min_d = (int)params[2].v.i;
    int max_d = (int)params[3].v.i;
    for (int dist = min_d; dist <= max_d; dist++) {
        Position to = {piece->pos.x + dx * dist, piece->pos.y + dy * dist};
        if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
            break;
        }
        const PieceState *at = board_at(&bs->board, to);
        if (is_friendly(piece, at))
            break;
        ml_push(out, to);
        if (at != NULL)
            break;
    }
}

/*--------------------------------------------------------------------------*\
                                 MG SLIDE DIRS
\*--------------------------------------------------------------------------*/

/// mg_slide_dirs
///
/// Slide along any subset of the eight clock-face directions chosen
/// by a bit mask. params: [0]=dir_mask, [1]=min_dist, [2]=max_dist.
/// Direction indices 0..7 are E SE S SW W NW N NE.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> [0]=mask, [1]=min, [2]=max
/// - size_t             n      -> parameter count (must be ≥ 3)
/// - MoveList          *out    -> destination list
///
void mg_slide_dirs(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    if (n < 3)
        return;
    uint32_t mask = (uint32_t)params[0].v.i;
    int min_d = (int)params[1].v.i;
    int max_d = (int)params[2].v.i;
    static const int8_t DX[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    static const int8_t DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    for (int dir = 0; dir < 8; dir++) {
        if (!(mask & (1u << dir)))
            continue;
        for (int dist = min_d; dist <= max_d; dist++) {
            Position to = {
                piece->pos.x + DX[dir] * dist,
                piece->pos.y + DY[dir] * dist
            };
            if (!pos_in_bounds(to, bs->board.width, bs->board.height)) {
                break;
            }
            const PieceState *at = board_at(&bs->board, to);
            if (is_friendly(piece, at))
                break;
            ml_push(out, to);
            if (at != NULL)
                break;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 MG LEAP SET
\*--------------------------------------------------------------------------*/

/// mg_leap_set
///
/// Capture-only leap set. Each (dx, dy) pair in params is a leap
/// offset that captures whatever sits at the destination; empty
/// destinations are skipped because this pattern is threat-only.
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> alternating dx, dy values
/// - size_t             n      -> parameter count (must be even)
/// - MoveList          *out    -> destination list
///
void mg_leap_set(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    for (size_t i = 0; i + 1 < n; i += 2) {
        Position to = {
            piece->pos.x + (int)params[i].v.i,
            piece->pos.y + (int)params[i + 1].v.i
        };
        if (can_capture(bs, piece, to))
            ml_push(out, to);
    }
}

/*--------------------------------------------------------------------------*\
                                 MG BLOCKABLE LEAP
\*--------------------------------------------------------------------------*/

/// mg_blockable_leap
///
/// Leap to a single (dx, dy) destination, but only if at least one
/// of the listed intermediate squares is empty. params layout:
/// [0]=dx, [1]=dy, then pairs of (mid_dx, mid_dy).
///
/// Params:
/// - const PieceState  *piece  -> moving piece
/// - const BattleState *bs     -> battle context
/// - const EffectArg   *params -> destination + intermediates
/// - size_t             n      -> parameter count (must be ≥ 2)
/// - MoveList          *out    -> destination list
///
void mg_blockable_leap(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    if (n < 2)
        return;
    int dx = (int)params[0].v.i;
    int dy = (int)params[1].v.i;
    Position to = {piece->pos.x + dx, piece->pos.y + dy};
    if (!can_capture(bs, piece, to))
        return;
    bool blocked = true;
    for (size_t i = 2; i + 1 < n; i += 2) {
        Position mid = {
            piece->pos.x + (int)params[i].v.i,
            piece->pos.y + (int)params[i + 1].v.i
        };
        if (board_at(&bs->board, mid) == NULL) {
            blocked = false;
            break;
        }
    }
    if (!blocked)
        ml_push(out, to);
}

/*--------------------------------------------------------------------------*\
                                 MG COMPOUND
\*--------------------------------------------------------------------------*/

/// mg_compound
///
/// Composition wrapper that unions multiple sub-movegens into one
/// MoveList. Implementation pending; this stub remains so data files
/// can declare compound patterns ahead of the engine support.
///
/// Params:
/// - const PieceState  *piece  -> unused
/// - const BattleState *bs     -> unused
/// - const EffectArg   *params -> sub-movegen references (unused)
/// - size_t             n      -> parameter count (unused)
/// - MoveList          *out    -> destination list (unused)
///
void mg_compound(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)piece;
    (void)bs;
    (void)params;
    (void)n;
    (void)out;
}

/*--------------------------------------------------------------------------*\
                                 MG CHOICE
\*--------------------------------------------------------------------------*/

/// mg_choice
///
/// Composition wrapper that enumerates sub-movegen options for the
/// caller to pick from. Implementation pending; this stub keeps the
/// signature stable for templates that already reference it.
///
/// Params:
/// - const PieceState  *piece  -> unused
/// - const BattleState *bs     -> unused
/// - const EffectArg   *params -> sub-movegen references (unused)
/// - size_t             n      -> parameter count (unused)
/// - MoveList          *out    -> destination list (unused)
///
void mg_choice(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)piece;
    (void)bs;
    (void)params;
    (void)n;
    (void)out;
}

/*--------------------------------------------------------------------------*\
                                 MG DOUBLE ACT
\*--------------------------------------------------------------------------*/

/// mg_double_act
///
/// Composition wrapper applying a sub-movegen twice within a single
/// action. Implementation pending; this stub leaves the slot ready
/// for the future implementation.
///
/// Params:
/// - const PieceState  *piece  -> unused
/// - const BattleState *bs     -> unused
/// - const EffectArg   *params -> sub-movegen reference (unused)
/// - size_t             n      -> parameter count (unused)
/// - MoveList          *out    -> destination list (unused)
///
void mg_double_act(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)piece;
    (void)bs;
    (void)params;
    (void)n;
    (void)out;
}

/*--------------------------------------------------------------------------*\
                                 MG TERRITORY RESTRICTED
\*--------------------------------------------------------------------------*/

/// mg_territory_restricted
///
/// Composition wrapper that filters a sub-movegen by a mask of
/// allowed territory ownerships. Implementation pending; the stub
/// keeps the signature stable for early data-file declarations.
///
/// Params:
/// - const PieceState  *piece  -> unused
/// - const BattleState *bs     -> unused
/// - const EffectArg   *params -> [0]=sub ref, [1]=mask (unused)
/// - size_t             n      -> parameter count (unused)
/// - MoveList          *out    -> destination list (unused)
///
void mg_territory_restricted(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)piece;
    (void)bs;
    (void)params;
    (void)n;
    (void)out;
}

/*--------------------------------------------------------------------------*\
                                 MG ATTACK ONLY SUBSET
\*--------------------------------------------------------------------------*/

/// mg_attack_only_subset
///
/// Composition wrapper splitting movement and attack into two
/// sub-movegens. Implementation pending; required for pawn-style
/// pieces whose relocate and attack patterns diverge.
///
/// Params:
/// - const PieceState  *piece  -> unused
/// - const BattleState *bs     -> unused
/// - const EffectArg   *params -> relocate / attack refs (unused)
/// - size_t             n      -> parameter count (unused)
/// - MoveList          *out    -> destination list (unused)
///
void mg_attack_only_subset(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)piece;
    (void)bs;
    (void)params;
    (void)n;
    (void)out;
}

/*--------------------------------------------------------------------------*\
                                 MG TODO
\*--------------------------------------------------------------------------*/

/// mg_todo
///
/// Placeholder MoveGenFunc used when a piece's movement pattern is
/// still being designed. Produces an empty MoveList so the piece is
/// effectively immobile until a real implementation replaces it.
///
/// Params:
/// - const PieceState  *piece  -> unused
/// - const BattleState *bs     -> unused
/// - const EffectArg   *params -> unused
/// - size_t             n      -> unused
/// - MoveList          *out    -> cleared to zero entries
///
void mg_todo(
    const PieceState *piece,
    const BattleState *bs,
    const EffectArg *params,
    size_t n,
    MoveList *out
) {
    (void)piece;
    (void)bs;
    (void)params;
    (void)n;
    out->count = 0;
}

/*--------------------------------------------------------------------------*\
                              MOVEGEN INVOCATION
\*--------------------------------------------------------------------------*/

/// mg_generate
///
/// Fill `*out` with every legal relocation for `piece`. The
/// per-instance override wins when present; otherwise the template's
/// move pattern runs.
///
/// Params:
/// - const PieceState  *piece -> querying piece
/// - const BattleState *bs    -> battle context
/// - MoveList          *out   -> destination list (reset to empty)
///
void mg_generate(
    const PieceState *piece,
    const BattleState *bs,
    MoveList *out
) {
    out->count = 0;
    const MoveGen *mg = NULL;
    if (piece->move_override.func != NULL) {
        mg = &piece->move_override;
    } else {
        mg = &piece->tmpl->move;
    }
    if (mg->func == NULL)
        return;
    mg->func(piece, bs, mg->params, mg->param_count, out);
}

/// mg_generate_threat
///
/// Fill `*out` with every square `piece` threatens. The override is
/// preferred, then the template's threat pattern, then the template's
/// move pattern — matching the GDD's "threat = move unless declared
/// otherwise" rule.
///
/// Params:
/// - const PieceState  *piece -> querying piece
/// - const BattleState *bs    -> battle context
/// - MoveList          *out   -> destination list (reset to empty)
///
void mg_generate_threat(
    const PieceState *piece,
    const BattleState *bs,
    MoveList *out
) {
    out->count = 0;
    const MoveGen *mg = NULL;
    if (piece->threat_override.func != NULL) {
        mg = &piece->threat_override;
    } else if (piece->tmpl->threat.func != NULL) {
        mg = &piece->tmpl->threat;
    } else {
        mg = &piece->tmpl->move;
    }
    if (mg->func == NULL)
        return;
    mg->func(piece, bs, mg->params, mg->param_count, out);
}
