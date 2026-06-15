//! movegen.h
//!
//! Movement-generation API for the Regnum battle engine. Every
//! movement is a function pointer plus typed parameter list, kept
//! data-driven so the combination system can mix patterns at
//! runtime without ever touching engine source.
//!
//! Primitive patterns live in mg_basics.c; bespoke kingdom-specific
//! patterns sit in their own mg_<kingdom>.c units. All helpers and
//! prototypes are declared once here so duplicated static copies
//! never reappear inside per-kingdom translation units.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"

struct PieceState;
struct BattleState;

/*--------------------------------------------------------------------------*\
                              MOVE LIST
\*--------------------------------------------------------------------------*/

/// MoveList
///
/// Fixed-capacity container of destination squares produced by a
/// MoveGenFunc. The `count` field is the number of valid entries in
/// `squares`; everything past it is undefined.
///
typedef struct {
    Position squares[MAX_MOVES];
    uint8_t  count;
} MoveList;

/*--------------------------------------------------------------------------*\
                              MOVEGEN
\*--------------------------------------------------------------------------*/

/// MoveGenFunc
///
/// Function pointer signature for every movement generator. Each
/// implementation reads `piece` and `battle`, consults `params[0..count-1]`
/// for tuning, and writes results into `*out`.
///
typedef void (*MoveGenFunc)(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// MoveGen
///
/// Bound movement pattern: a function pointer plus the typed
/// parameter list it was constructed with. PieceTemplate carries one
/// MoveGen for relocation and one for threat enumeration; either may
/// be overridden per-instance via PieceState.move_override.
///
typedef struct MoveGen {
    MoveGenFunc func;
    EffectArg   params[MAX_MOVE_PARAMS];
    uint8_t     param_count;
} MoveGen;

/*--------------------------------------------------------------------------*\
                              MOVEGEN HELPERS
\*--------------------------------------------------------------------------*/

/// ml_push
///
/// Append a destination to a MoveList, silently dropping the
/// addition if the list is already at MAX_MOVES capacity.
///
/// Params:
/// - MoveList *ml -> destination list
/// - Position position   -> board square to append
///
void ml_push(MoveList* ml, Position position);

/// is_enemy
///
/// Predicate testing whether `at` belongs to the side opposite
/// `piece`. Neutral and absent (NULL) pieces are never considered
/// enemies, so the caller can pass a board_at() result directly.
///
/// Params:
/// - const PieceState *piece -> querying piece
/// - const PieceState *at    -> piece on the target square or NULL
///
/// Return:
/// bool -> true when `at` is a non-null opposing-side piece
///
bool is_enemy(const struct PieceState* piece, const struct PieceState* at);

/// is_friendly
///
/// Predicate testing whether `at` belongs to the same side as
/// `piece`. NULL targets return false.
///
/// Params:
/// - const PieceState *piece -> querying piece
/// - const PieceState *at    -> piece on the target square or NULL
///
/// Return:
/// bool -> true when `at` shares its owner with `piece`
///
bool is_friendly(const struct PieceState* piece, const struct PieceState* at);

/// can_move_to
///
/// Convenience predicate combining bounds and friendly-occupancy
/// checks. A destination square is reachable if it sits inside the
/// board and is either empty or holds an enemy piece.
///
/// Params:
/// - const BattleState *battle   -> battle context (used for board dims)
/// - const PieceState  *piece -> moving piece
/// - Position           to   -> destination square
///
/// Return:
/// bool -> true when the move would not violate base movement rules
///
bool can_move_to(
    const struct BattleState* battle,
    const struct PieceState*  piece,
    Position                  to
);

/// can_capture
///
/// Convenience predicate testing whether `to` holds an enemy piece
/// that `piece` could capture if its movement permits the square.
///
/// Params:
/// - const BattleState *battle    -> battle context
/// - const PieceState  *piece -> querying piece
/// - Position           to    -> target square
///
/// Return:
/// bool -> true when an enemy piece sits at `to` within the board
///
bool can_capture(
    const struct BattleState* battle,
    const struct PieceState*  piece,
    Position                  to
);

/// can_capture_or_empty
///
/// Predicate matching the harushima-style "advance or attack" rule:
/// the target is inside the board and either empty or holds an
/// enemy piece.
///
/// Params:
/// - const BattleState *battle    -> battle context
/// - const PieceState  *piece -> querying piece
/// - Position           to    -> target square
///
/// Return:
/// bool -> true when the square is occupiable by `piece`
///
bool can_capture_or_empty(
    const struct BattleState* battle,
    const struct PieceState*  piece,
    Position                  to
);

/*--------------------------------------------------------------------------*\
                         PRIMITIVE PROTOTYPES (mg_basics.c)
\*--------------------------------------------------------------------------*/

/// mg_step
///
/// Single-step relocate by a fixed offset. params[0] is delta_x,
/// params[1] is delta_y. The destination must satisfy can_move_to.
///
void mg_step(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_step_set
///
/// Set of single-step relocations defined by interleaved (delta_x, delta_y)
/// pairs in params. Used for king/wazir/ferz-style movement and any
/// arbitrary one-square jump set.
///
void mg_step_set(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_slide
///
/// Linear slide along a single direction. params: delta_x, delta_y, min_dist,
/// max_dist. Movement stops on the first non-empty square; an enemy
/// at that square is captured.
///
void mg_slide(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_slide_dirs
///
/// Slide along any of eight directions selected by a bit-mask.
/// params: dir_mask, min_dist, max_dist. Direction bit indices match
/// clock-face order E SE S SW W NW N NE.
///
void mg_slide_dirs(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_leap_set
///
/// Capture-only leap set. Each (delta_x, delta_y) pair in params is a leap
/// offset that captures whatever sits at the destination; empty
/// destinations are skipped.
///
void mg_leap_set(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_blockable_leap
///
/// Leap that succeeds only if at least one of the listed intermediate
/// squares is empty. params: delta_x, delta_y, then (mid_delta_x, mid_delta_y) pairs.
///
void mg_blockable_leap(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_compound
///
/// Composition wrapper that unions multiple sub-movegens into one
/// MoveList. Implementation pending; currently a no-op stub.
///
void mg_compound(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_choice
///
/// Composition wrapper that enumerates sub-movegen options for the
/// caller to pick from. Implementation pending; currently a no-op.
///
void mg_choice(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_double_act
///
/// Composition wrapper applying a sub-movegen twice in one action.
/// Implementation pending; currently a no-op stub.
///
void mg_double_act(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_territory_restricted
///
/// Composition wrapper filtering a sub-movegen by allowed-territory
/// mask. Implementation pending; currently a no-op stub.
///
void mg_territory_restricted(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_attack_only_subset
///
/// Composition wrapper splitting relocate and attack movements,
/// allowing distinct sub-patterns. Implementation pending; currently
/// a no-op stub.
///
void mg_attack_only_subset(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_todo
///
/// Placeholder MoveGenFunc used when a piece's movement pattern is
/// still to be designed. Always produces an empty MoveList; the
/// piece is therefore unable to move until a real implementation
/// replaces it.
///
void mg_todo(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/*--------------------------------------------------------------------------*\
                    LONGWEI (src/movegens/mg_longwei.c)
\*--------------------------------------------------------------------------*/

/// mg_lw_ma
///
/// Knight-L move blocked when any piece occupies the intermediate
/// "elbow" square. Used by the Longwei Ma piece.
///
void mg_lw_ma(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_lw_xiang
///
/// (2,2) blockable diagonal leap. Implements the Longwei Xiang's
/// movement rule including the no-attack-in-enemy-territory clause.
///
void mg_lw_xiang(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_lw_pao
///
/// Rook-line move with the Xiangqi cannon capture rule: a Pao may
/// capture only if exactly one screen piece sits between it and the
/// target along the chosen line.
///
void mg_lw_pao(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_lw_hwacha
///
/// Pao-style screen capture applied to both rook and bishop lines.
/// Drives the Longwei Hwacha combo piece.
///
void mg_lw_hwacha(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_lw_sang
///
/// One orthogonal step followed by two diagonal steps. Both
/// intermediate squares must be empty. Implements the Janggi
/// elephant move used by the Longwei Sang combo piece.
///
void mg_lw_sang(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_lw_liubo
///
/// Teleport to any square currently threatened by an enemy piece.
/// Drives the Longwei Liubo Diviner capstone.
///
void mg_lw_liubo(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/*--------------------------------------------------------------------------*\
                    HARUSHIMA (src/movegens/mg_harushima.c)
\*--------------------------------------------------------------------------*/

/// mg_hs_kinsho
///
/// Six-direction step set forbidding diagonal-backwards motion.
/// Used by the Harushima Kinsho piece and as the upgrade target for
/// the Honorable Horse after it enters enemy territory.
///
void mg_hs_kinsho(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_hs_ginsho
///
/// Five-direction step set (forward, diagonals, side-steps). Used by
/// the Harushima Ginsho piece.
///
void mg_hs_ginsho(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_hs_honorable_horse
///
/// Asymmetric two-forward-one-side leap. params[0] toggles between
/// left and right side. A passive effect rewrites move_override to
/// mg_hs_kinsho once the piece first enters enemy territory.
///
void mg_hs_honorable_horse(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_hs_shishi
///
/// Lion move: either two king-direction steps in sequence, or a
/// single step followed by an attack without committing to the move.
/// Used by the Harushima Shishi capstone.
///
void mg_hs_shishi(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/*--------------------------------------------------------------------------*\
                    KEWARANI (src/movegens/mg_kewarani.c)
\*--------------------------------------------------------------------------*/

/// mg_kw_berolina
///
/// Berolina-pawn rule: diagonal one-step relocate, forward attack.
/// params[0] is the forward delta_y (-1 for Kewarani who plays from the
/// top of the board).
///
void mg_kw_berolina(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_kw_negus_guard
///
/// Double-act king-set step: two king-style steps performed in one
/// action with an intermediate empty-square requirement. Used by the
/// Negus Guard and the Sultan's Levy splitter.
///
void mg_kw_negus_guard(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/*--------------------------------------------------------------------------*\
                    ZARQAN (src/movegens/mg_zarqan.c)
\*--------------------------------------------------------------------------*/

/// mg_zq_ziraafa
///
/// One diagonal step followed by an orthogonal slide of at least
/// three squares. Both legs must be unblocked; only the final
/// destination may hold an enemy.
///
void mg_zq_ziraafa(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_zq_swap_with_king
///
/// Move generator that exposes the friendly king's square so the
/// action API can treat the Shahzadeh's free king swap uniformly.
///
void mg_zq_swap_with_king(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/// mg_zq_war_elephant
///
/// Threat-only pattern: enumerates every adjacent enemy square. Used
/// by Ziraafa-line pieces so a single relocate threatens multiple
/// neighbours simultaneously.
///
void mg_zq_war_elephant(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/*--------------------------------------------------------------------------*\
                    CAELAN (src/movegens/mg_caelan.c)
\*--------------------------------------------------------------------------*/

/// mg_ca_gryphon
///
/// One diagonal step followed by an orthogonal slide of any
/// distance. Both legs must be unblocked; the orthogonal slide stops
/// on the first occupant.
///
void mg_ca_gryphon(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    const EffectArg*          params,
    size_t count,
    MoveList*                 out
);

/*--------------------------------------------------------------------------*\
                         MOVEGEN INVOCATION
\*--------------------------------------------------------------------------*/

/// mg_generate
///
/// Fill `*out` with every legal relocation destination for `piece`.
/// If the piece has a move_override installed, that pattern wins;
/// otherwise the template's move pattern is used.
///
void mg_generate(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    MoveList*                 out
);

/// mg_generate_threat
///
/// Fill `*out` with every square `piece` currently threatens. Falls
/// back to the move pattern when no dedicated threat pattern exists,
/// matching the GDD's resolve rule for most pieces.
///
void mg_generate_threat(
    const struct PieceState*  piece,
    const struct BattleState* battle,
    MoveList*                 out
);

#endif /* MOVEGEN_H */
