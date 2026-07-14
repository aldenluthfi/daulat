//! piece.c
//!
//! Movement generation kit and the piece behaviour effects shared across
//! the kingdom data files. Owns the three movegen generics, the shared
//! scratch buffer, the named direction sets, and the generic effects that
//! embed a slot, grant a free move, and copy another piece's movement.
//!
//! Created: 13/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                                  MOVEGEN KIT
\*----------------------------------------------------------------------------*/

const Square ORTHOGONAL_DIRECTIONS[] = {
    {0, -1},
    {-1, 0},
    {1, 0},
    {0, 1},
    {0, 0},
};

const Square DIAGONAL_DIRECTIONS[] = {
    {-1, -1},
    {1, -1},
    {-1, 1},
    {1, 1},
    {0, 0},
};

const Square ALL_DIRECTIONS[] = {
    {-1, -1},
    {0, -1},
    {1, -1},
    {-1, 0},
    {1, 0},
    {-1, 1},
    {0, 1},
    {1, 1},
    {0, 0},
};

static Square SCRATCH[MAX_BOARD_SIZE + 1];
static size_t SCRATCH_CURSOR;

/// mg_begin
///
/// Resets the shared static scratch buffer that movement generation
/// writes into. See the header for the full scratch buffer convention.
///
void mg_begin(void) {
    SCRATCH_CURSOR = 0;
}

/// mg_push
///
/// Appends one square to the scratch buffer. Squares past the buffer
/// capacity are dropped.
///
/// Params:
/// - square -> square to append
///
void mg_push(Square square) {
    if (SCRATCH_CURSOR < MAX_BOARD_SIZE) {
        SCRATCH[SCRATCH_CURSOR] = square;
        SCRATCH_CURSOR++;
    }
}

/// mg_end
///
/// Terminates the scratch buffer with SQUARE_END and returns it. The
/// sentinel is overwritten by the next push, so sequential generator
/// calls concatenate their output.
///
/// Return: the scratch buffer, SQUARE_END terminated
///
Square* mg_end(void) {
    SCRATCH[SCRATCH_CURSOR] = SQUARE_END;

    return SCRATCH;
}

/// mg_emit
///
/// Pushes the destination when it satisfies the generation semantics:
/// movement wants empty squares only, threat wants every square not
/// held by a friendly piece.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - dest   -> candidate destination
/// - threat -> true for at (coverage), false for mv (movement)
///
static void
mg_emit(BattleState* battle, PieceInfo* self, Square dest, bool threat) {
    PieceInfo* occupant = battle_at(battle, dest);

    if (!occupant || (threat && occupant->side != self->side)) {
        mg_push(dest);
    }
}

/// mg_leap
///
/// Emits one destination per offset from self's square. Offsets are
/// mirrored vertically for black. Movement emits only empty squares;
/// threat emits every covered square not holding a friendly piece.
///
/// Params:
/// - battle  -> battle providing the board
/// - self    -> piece generating from its square and side
/// - offsets -> zero vector terminated offset array, white perspective
/// - threat  -> true for at (coverage), false for mv (movement)
///
void mg_leap(
    BattleState*  battle,
    PieceInfo*    self,
    const Square* offsets,
    bool          threat
) {
    for (size_t i = 0; offsets[i].x != 0 || offsets[i].y != 0; i++) {
        int8_t dy =
            self->side == SIDE_BLACK ? (int8_t) -offsets[i].y : offsets[i].y;

        Square dest = {
            (int8_t) (self->square.x + offsets[i].x),
            (int8_t) (self->square.y + dy),
        };

        if (battle_in_bounds(battle, dest)) {
            mg_emit(battle, self, dest, threat);
        }
    }
}

/// mg_slide
///
/// Walks each direction from self's square emitting squares between min
/// and max steps. Blockers stop the walk regardless of min. Movement
/// emits empty squares; threat also emits the first non-friendly
/// occupant reached.
///
/// Params:
/// - battle     -> battle providing the board
/// - self       -> piece generating from its square and side
/// - directions -> zero vector terminated direction array
/// - min        -> minimum step distance to emit
/// - max        -> maximum step distance, 127 for unbounded
/// - threat     -> true for at (coverage), false for mv (movement)
///
void mg_slide(
    BattleState*  battle,
    PieceInfo*    self,
    const Square* directions,
    int8_t        min,
    int8_t        max,
    bool          threat
) {
    for (size_t i = 0; directions[i].x != 0 || directions[i].y != 0; i++) {
        int8_t dx = directions[i].x;
        int8_t dy = self->side == SIDE_BLACK ? (int8_t) -directions[i].y
                                             : directions[i].y;

        for (int step = 1; step <= max; step++) {
            Square dest = {
                (int8_t) (self->square.x + dx * step),
                (int8_t) (self->square.y + dy * step),
            };

            if (!battle_in_bounds(battle, dest)) {
                break;
            }

            PieceInfo* occupant = battle_at(battle, dest);

            if (occupant) {
                if (threat && step >= min && occupant->side != self->side) {
                    mg_push(dest);
                }

                break;
            }

            if (step >= min) {
                mg_push(dest);
            }
        }
    }
}

/// mg_compound
///
/// Emits the union of other pieces' patterns generated from self's own
/// square and side, matching mv or at according to threat. Parts with
/// no registry entry are skipped.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> piece generating from its square and side
/// - parts  -> PIECE_NONE terminated array of pieces to copy
/// - threat -> true for at (coverage), false for mv (movement)
///
void mg_compound(
    BattleState*   battle,
    PieceInfo*     self,
    const PieceID* parts,
    bool           threat
) {
    for (size_t i = 0; parts[i] != PIECE_NONE; i++) {
        const Piece* part = PIECE_REGISTRY[parts[i]];

        if (!part) {
            continue;
        }

        if (threat) {
            part->at(battle, self);
        } else {
            part->mv(battle, self);
        }
    }
}

/*----------------------------------------------------------------------------*\
                                 PIECE EFFECTS
\*----------------------------------------------------------------------------*/

/// piece_is_pawn
///
/// Reports whether the given piece identity is one of the kingdoms' pawn
/// grade pieces, used by the mass advance and pawn filter card bodies.
///
/// Params:
/// - id -> piece identity to classify
///
/// Return: true when the identity is a pawn grade piece
///
bool piece_is_pawn(PieceID id) {
    return id == PIECE_BING || id == PIECE_MEDEQ || id == PIECE_FUHYO ||
           id == PIECE_PAWN || id == PIECE_WAZIR;
}

/// piece_embed_effect
///
/// Claims the first free embedded effect slot on a piece, copies the
/// template into it, allocates its context, and stores the owning piece
/// in args[0].
///
/// Params:
/// - piece    -> piece to embed the effect on
/// - template -> effect template to copy into the slot
///
/// Return: the embedded effect, nullptr when no slot or memory is free
///
Effect* piece_embed_effect(PieceInfo* piece, const Effect* template) {
    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        Effect* effect = &piece->piece->effects[slot];

        if (effect->func) {
            continue;
        }

        *effect         = *template;
        effect->context = calloc(1, sizeof(EffectContext));

        if (!effect->context) {
            effect->func = nullptr;
            return nullptr;
        }

        effect->context->args[0] = piece;

        return effect;
    }

    return nullptr;
}

/// eff_free_move
///
/// Makes the embedded piece's move action free once, on the turn the
/// grant was issued. Consumes itself after one application so the free
/// move never carries to a later turn.
///
/// Params:
/// - context -> args[0] self, args[1] granted turn, args[3] spent flag
/// - x       -> int* action cost to zero out
///
/// Return: true when the free move applied, false otherwise
///
bool eff_free_move(EffectContext* context, void* x) {
    PieceInfo* self = context->args[0];

    if (battle_subject() != self || context->args[3]) {
        return false;
    }

    if (battle_current()->turn != (size_t) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x        = 0;
    context->args[3] = (void*) 1;

    return true;
}

/// piece_grant_free_move
///
/// Embeds a one-shot free move effect on a piece, tagged with the given
/// name and bound to the current turn.
///
/// Params:
/// - piece -> piece to grant the free move to
/// - name  -> effect name for the fire log line
///
void piece_grant_free_move(PieceInfo* piece, const char* name) {
    Effect free_move = {
        .func      = eff_free_move,
        .name      = (char*) name,
        .trigger   = QUERY_PIECE_ACTION_COST_MOVE,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* granted = piece_embed_effect(piece, &free_move);

    if (granted) {
        granted->context->args[1] = (void*) (uintptr_t) battle_current()->turn;
    }
}

/// eff_double_move
///
/// Makes every second move action by the embedded piece free, alternating
/// between a paid move that arms the grant and a free move that spends it.
///
/// Params:
/// - context -> args[0] self, args[1] armed flag
/// - x       -> int* action cost to zero out on a free move
///
/// Return: true when the free move applied, false otherwise
///
bool eff_double_move(EffectContext* context, void* x) {
    PieceInfo* self = context->args[0];

    if (battle_subject() != self) {
        return false;
    }

    if (context->args[1]) {
        context->args[1] = nullptr;
        *(int*) x        = 0;

        return true;
    }

    context->args[1] = (void*) 1;

    return false;
}

/// eff_copy_mv
///
/// Regenerates the target's move list from a copied piece type. The
/// replace flag resets the scratch first so the copied pattern replaces
/// the target's own moves; otherwise the copied moves are appended.
///
/// Params:
/// - context -> args[1] target, args[2] copied PieceID, args[3] replace
/// - x       -> unused move list
///
/// Return: true when the target's list was regenerated
///
bool eff_copy_mv(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* target = context->args[1];

    if (battle_subject() != target) {
        return false;
    }

    if (context->args[3]) {
        mg_begin();
    }

    mg_compound(
        battle_current(),
        target,
        (const PieceID[]){(PieceID) (uintptr_t) context->args[2], PIECE_NONE},
        false
    );
    mg_end();

    return true;
}

/// eff_copy_at
///
/// Regenerates the target's attack coverage from a copied piece type,
/// replacing or appending according to the replace flag, mirroring
/// eff_copy_mv for the threat side.
///
/// Params:
/// - context -> args[1] target, args[2] copied PieceID, args[3] replace
/// - x       -> unused coverage list
///
/// Return: true when the target's coverage was regenerated
///
bool eff_copy_at(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* target = context->args[1];

    if (battle_subject() != target) {
        return false;
    }

    if (context->args[3]) {
        mg_begin();
    }

    mg_compound(
        battle_current(),
        target,
        (const PieceID[]){(PieceID) (uintptr_t) context->args[2], PIECE_NONE},
        true
    );
    mg_end();

    return true;
}

/// piece_adopt_move
///
/// Attaches a pair of move and attack copying effects to the target's
/// player list so the target moves and threatens as the copied piece for
/// the given duration.
///
/// Params:
/// - battle  -> battle the target lives in
/// - target  -> piece that adopts the copied movement
/// - copied  -> piece identity to copy the movement of
/// - lasts   -> duration of the adopted movement
/// - replace -> true to replace the target's own movement, false to add
///
void piece_adopt_move(
    BattleState*   battle,
    PieceInfo*     target,
    PieceID        copied,
    EffectDuration lasts,
    bool           replace
) {
    const Effect templates[] = {
        {.func      = eff_copy_mv,
         .name      = "Adopt Move",
         .trigger   = QUERY_PIECE_MOVES,
         .lasts_for = lasts},
        {.func      = eff_copy_at,
         .name      = "Adopt Move",
         .trigger   = QUERY_PIECE_ATTACKS,
         .lasts_for = lasts},
    };

    for (size_t i = 0; i < 2; i++) {
        Effect* attached = effect_attach(
            &battle_player(battle, target->side)->effects,
            &templates[i]
        );

        if (attached) {
            attached->context->args[1] = target;
            attached->context->args[2] = (void*) (uintptr_t) copied;
            attached->context->args[3] = (void*) (uintptr_t) replace;
        }
    }
}
