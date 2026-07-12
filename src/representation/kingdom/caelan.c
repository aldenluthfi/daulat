//! caelan.c
//!
//! Caelan kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Conqueror's Reward innate, climax, Crowned
//! Heretic overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// pawn_mv
///
/// Generates the Pawn's movement: forward one square. The double-step
/// from the spawn square is added by the pawn's embedded effect.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* pawn_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, (const Square[]) { { 0, -1 }, { 0, 0 } },
            false);

    return mg_end();
}

/// pawn_at
///
/// Generates the Pawn's attack coverage: the two forward diagonals.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* pawn_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self,
            (const Square[]) { { -1, -1 }, { 1, -1 }, { 0, 0 } },
            true);

    return mg_end();
}

/// knight_gen
///
/// Generates the Knight's eight leaps for either movement or coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void knight_gen(BattleState* battle, PieceInfo* self,
                       bool threat) {
    mg_leap(battle, self, (const Square[]) {
        { -1, -2 }, { 1, -2 }, { -2, -1 }, { 2, -1 },
        { -2,  1 }, { 2,  1 }, { -1,  2 }, { 1,  2 },
        {  0,  0 },
    }, threat);
}

/// knight_mv
///
/// Generates the Knight's movement through knight_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* knight_mv(BattleState* battle, PieceInfo* self) {
    knight_gen(battle, self, false);

    return mg_end();
}

/// knight_at
///
/// Generates the Knight's attack coverage through knight_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* knight_at(BattleState* battle, PieceInfo* self) {
    knight_gen(battle, self, true);

    return mg_end();
}

/// bishop_mv
///
/// Generates the Bishop's movement: unbounded diagonal slides.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* bishop_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, DIAGONAL_DIRECTIONS, 1, 127, false);

    return mg_end();
}

/// bishop_at
///
/// Generates the Bishop's attack coverage: unbounded diagonal slides.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* bishop_at(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, DIAGONAL_DIRECTIONS, 1, 127, true);

    return mg_end();
}

/// queen_mv
///
/// Generates the Queen's movement: unbounded slides in all eight
/// directions.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* queen_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, ALL_DIRECTIONS, 1, 127, false);

    return mg_end();
}

/// queen_at
///
/// Generates the Queen's attack coverage: unbounded slides in all eight
/// directions.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* queen_at(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, ALL_DIRECTIONS, 1, 127, true);

    return mg_end();
}

/// gryphon_gen
///
/// Generates the Gryphon's bent pattern: one empty diagonal step then an
/// outward orthogonal slide along either component of the diagonal. The
/// diagonal intermediate is never a destination.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void gryphon_gen(BattleState* battle, PieceInfo* self,
                        bool threat) {
    for (size_t i = 0; DIAGONAL_DIRECTIONS[i].x != 0
                       || DIAGONAL_DIRECTIONS[i].y != 0; i++) {
        int8_t dx = DIAGONAL_DIRECTIONS[i].x;
        int8_t dy = DIAGONAL_DIRECTIONS[i].y;

        Square inter = {
            (int8_t) (self->square.x + dx),
            (int8_t) (self->square.y + dy),
        };

        if (!battle_in_bounds(battle, inter)
            || battle_at(battle, inter)) {
            continue;
        }

        Square legs[] = { { dx, 0 }, { 0, dy } };

        for (size_t leg = 0; leg < 2; leg++) {
            for (int step = 1; step < MAX_BOARD_SIZE; step++) {
                Square dest = {
                    (int8_t) (inter.x + legs[leg].x * step),
                    (int8_t) (inter.y + legs[leg].y * step),
                };

                if (!battle_in_bounds(battle, dest)) {
                    break;
                }

                PieceInfo* occupant = battle_at(battle, dest);

                if (occupant) {
                    if (threat && occupant->side != self->side) {
                        mg_push(dest);
                    }

                    break;
                }

                mg_push(dest);
            }
        }
    }
}

/// gryphon_mv
///
/// Generates the Gryphon's movement through gryphon_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* gryphon_mv(BattleState* battle, PieceInfo* self) {
    gryphon_gen(battle, self, false);

    return mg_end();
}

/// gryphon_at
///
/// Generates the Gryphon's attack coverage through gryphon_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* gryphon_at(BattleState* battle, PieceInfo* self) {
    gryphon_gen(battle, self, true);

    return mg_end();
}

/// chancellor_gen
///
/// Generates the Chancellor's pattern: the union of Rook and Knight,
/// for either movement or coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void chancellor_gen(BattleState* battle, PieceInfo* self,
                           bool threat) {
    mg_compound(battle, self,
                (const PieceID[]) { PIECE_ROOK, PIECE_KNIGHT,
                                    PIECE_NONE },
                threat);
}

/// chancellor_mv
///
/// Generates the Chancellor's movement through chancellor_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* chancellor_mv(BattleState* battle, PieceInfo* self) {
    chancellor_gen(battle, self, false);

    return mg_end();
}

/// chancellor_at
///
/// Generates the Chancellor's attack coverage through chancellor_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* chancellor_at(BattleState* battle, PieceInfo* self) {
    chancellor_gen(battle, self, true);

    return mg_end();
}

/// banner_mv
///
/// Generates the Sovereign Banner's movement: Queen movement copied
/// through the registry.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* banner_mv(BattleState* battle, PieceInfo* self) {
    mg_compound(battle, self, (const PieceID[]) { PIECE_QUEEN,
                                                  PIECE_NONE }, false);

    return mg_end();
}

/// banner_at
///
/// Generates the Sovereign Banner's attack coverage: Queen coverage
/// copied through the registry.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* banner_at(BattleState* battle, PieceInfo* self) {
    mg_compound(battle, self, (const PieceID[]) { PIECE_QUEEN,
                                                  PIECE_NONE }, true);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                PIECE EFFECTS
\*----------------------------------------------------------------------------*/

/// square_pack
///
/// Packs a square into a nonzero integer so it can live in a context
/// slot with zero meaning unset.
///
/// Params:
/// - square -> square to pack
///
/// Return: packed nonzero representation
///
static uintptr_t square_pack(Square square) {
    return (uintptr_t) (square.y * 20 + square.x) + 1;
}

/// eff_pawn_double_step
///
/// Adds the Pawn's double-step to its move list while it still stands on
/// its spawn square. The context lazily records the spawn square in
/// args[1] and an active flag in args[2]; once the pawn is seen off its
/// spawn square the flag clears forever.
///
/// Params:
/// - context -> pawn's private context, args[0] holding self
/// - x       -> move list being generated, unused directly since the
///              extension goes through the shared scratch buffer
///
/// Return: true when the double step was added to the list
///
static bool eff_pawn_double_step(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self) {
        return false;
    }

    uintptr_t here = square_pack(self->square);

    if (!context->args[1]) {
        context->args[1] = (void*) here;
        context->args[2] = (void*) 1;
    }

    if (!context->args[2]) {
        return false;
    }

    if ((uintptr_t) context->args[1] != here) {
        context->args[2] = nullptr;
        return false;
    }

    BattleState* battle  = battle_current();
    int8_t       forward = self->side == SIDE_BLACK ? 1 : -1;

    Square one = {
        self->square.x,
        (int8_t) (self->square.y + forward),
    };

    Square two = {
        self->square.x,
        (int8_t) (self->square.y + 2 * forward),
    };

    if (!battle_in_bounds(battle, one) || !battle_in_bounds(battle, two)
        || battle_at(battle, one) || battle_at(battle, two)) {
        return false;
    }

    mg_push(two);
    mg_end();

    return true;
}

/// eff_banner_aura
///
/// Extends the move lists of allies adjacent to the Sovereign Banner:
/// per list entry, a stepper delta (both components within one) appends
/// the entry extended by its own delta, while a leaper delta grows its
/// larger component by one. Sliders and special movers are untouched.
///
/// Params:
/// - context -> banner's private context, args[0] holding self
/// - x       -> subject's move list being generated
///
/// Return: true when at least one extension was added to the list
///
static bool eff_banner_aura(EffectContext* context, void* x) {
    PieceInfo* self    = context->args[0];
    PieceInfo* subject = battle_subject();
    Square*    list    = x;

    if (!subject || subject == self || subject->side != self->side
        || subject->piece->class != MOVE_LEAPER) {
        return false;
    }

    int span_x = abs(subject->square.x - self->square.x);
    int span_y = abs(subject->square.y - self->square.y);

    if ((span_x > 1 || span_y > 1) || (span_x == 0 && span_y == 0)) {
        return false;
    }

    BattleState* battle = battle_current();

    size_t count  = 0;
    size_t pushed = 0;

    while (!(list[count].x == -1 && list[count].y == -1)) {
        count++;
    }

    for (size_t i = 0; i < count; i++) {
        int dx = list[i].x - subject->square.x;
        int dy = list[i].y - subject->square.y;

        Square extension;

        if (abs(dx) <= 1 && abs(dy) <= 1) {
            extension.x = (int8_t) (list[i].x + dx);
            extension.y = (int8_t) (list[i].y + dy);
        } else {
            if (abs(dx) >= abs(dy)) {
                dx += dx > 0 ? 1 : -1;
            } else {
                dy += dy > 0 ? 1 : -1;
            }

            extension.x = (int8_t) (subject->square.x + dx);
            extension.y = (int8_t) (subject->square.y + dy);
        }

        if (battle_in_bounds(battle, extension)
            && !battle_at(battle, extension)) {
            mg_push(extension);
            pushed++;
        }
    }

    mg_end();

    return pushed > 0;
}

/*----------------------------------------------------------------------------*\
                                 KINGDOM DATA
\*----------------------------------------------------------------------------*/

const Piece CAELAN_PIECES[] = {
    {
        .at      = pawn_at,
        .mv      = pawn_mv,
        .effects = {
            {
                .func      = eff_pawn_double_step,
                .name      = "Double Step",
                .trigger   = QUERY_PIECE_MOVES,
                .lasts_for = ENTIRE_BATTLE,
            },
        },
        .name    = "Pawn",
        .desc    = "Forward 1 (or 2 if it hasn't moved before). "
                   "Attacks diagonally.",
        .id      = PIECE_PAWN,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 10,
    },
    {
        .at      = knight_at,
        .mv      = knight_mv,
        .name    = "Knight",
        .desc    = "Knight L.",
        .id      = PIECE_KNIGHT,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 30,
    },
    {
        .at      = bishop_at,
        .mv      = bishop_mv,
        .name    = "Bishop",
        .desc    = "Diagonal slider.",
        .id      = PIECE_BISHOP,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_TOWN,
        .class   = MOVE_SLIDER,
        .value   = 30,
    },
    {
        .at      = queen_at,
        .mv      = queen_mv,
        .name    = "Queen",
        .desc    = "Rook and bishop combined.",
        .id      = PIECE_QUEEN,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SLIDER,
        .value   = 90,
    },
    {
        .at      = gryphon_at,
        .mv      = gryphon_mv,
        .name    = "Gryphon",
        .desc    = "1 diagonal step then any distance orthogonal.",
        .id      = PIECE_GRYPHON,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_SPECIAL,
        .value   = 100,
    },
    {
        .at      = chancellor_at,
        .mv      = chancellor_mv,
        .name    = "Chancellor",
        .desc    = "Moves as Rook or leaps as Knight each action.",
        .id      = PIECE_CHANCELLOR,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_SLIDER,
        .value   = 70,
    },
    {
        .at      = banner_at,
        .mv      = banner_mv,
        .effects = {
            {
                .func      = eff_banner_aura,
                .name      = "Banner Aura",
                .trigger   = QUERY_PIECE_MOVES,
                .lasts_for = ENTIRE_BATTLE,
            },
        },
        .name    = "Sovereign Banner",
        .desc    = "Queen movement. Adjacent allies: step-movers gain 1 "
                   "extra square, leapers extend longest leg by 1, "
                   "sliders unaffected.",
        .id      = PIECE_SOVEREIGN_BANNER,
        .kingdom = KINGDOM_CAELAN,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_SLIDER,
        .value   = 110,
    },
};

const Card       CAELAN_CARDS[]  = { {} };
const BoardTrait CAELAN_TRAITS[] = { {} };

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// caelan_innate
///
/// Attaches the Conqueror's Reward innate to the given side at the
/// given mastery level.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void caelan_innate(BattleState* battle, Side side, MasteryLevel level) {
    (void) battle;
    (void) side;
    (void) level;
}

/// caelan_climax
///
/// Fires the Caelan combo climax for the given side.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void caelan_climax(BattleState* battle, Side side) {
    (void) battle;
    (void) side;
}

/// caelan_overseer
///
/// Sets up the Crowned Heretic overseer battle with its bespoke army
/// and boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void caelan_overseer(BattleState* battle) {
    (void) battle;
}

/// caelan_event
///
/// Resolves a Caelan narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void caelan_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
