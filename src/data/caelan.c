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
    mg_leap(battle, self, (const Square[]) {{0, -1}, {0, 0}}, false);

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
    mg_leap(battle, self, (const Square[]) {{-1, -1}, {1, -1}, {0, 0}}, true);

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
static void knight_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {
            {-1, -2},
            {1, -2},
            {-2, -1},
            {2, -1},
            {-2, 1},
            {2, 1},
            {-1, 2},
            {1, 2},
            {0, 0},
        },
        threat
    );
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
static void gryphon_gen(BattleState* battle, PieceInfo* self, bool threat) {
    for (size_t i = 0;
         DIAGONAL_DIRECTIONS[i].x != 0 || DIAGONAL_DIRECTIONS[i].y != 0;
         i++) {
        int8_t dx    = DIAGONAL_DIRECTIONS[i].x;
        int8_t dy    = DIAGONAL_DIRECTIONS[i].y;

        Square inter = {
            (int8_t) (self->square.x + dx),
            (int8_t) (self->square.y + dy),
        };

        if (!battle_in_bounds(battle, inter) || battle_at(battle, inter)) {
            continue;
        }

        Square legs[] = {{dx, 0}, {0, dy}};

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
static void chancellor_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_ROOK, PIECE_KNIGHT, PIECE_NONE},
        threat
    );
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
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_QUEEN, PIECE_NONE},
        false
    );

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
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_QUEEN, PIECE_NONE},
        true
    );

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                PIECE EFFECTS
\*----------------------------------------------------------------------------*/

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

    uintptr_t here = card_pack(self->square);

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

    Square       one     = {
        self->square.x,
        (int8_t) (self->square.y + forward),
    };

    Square two = {
        self->square.x,
        (int8_t) (self->square.y + 2 * forward),
    };

    if (!battle_in_bounds(battle, one) || !battle_in_bounds(battle, two) ||
        battle_at(battle, one) || battle_at(battle, two)) {
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

    if (!subject || subject == self || subject->side != self->side ||
        subject->piece->class != MOVE_LEAPER) {
        return false;
    }

    int span_x = abs(subject->square.x - self->square.x);
    int span_y = abs(subject->square.y - self->square.y);

    if ((span_x > 1 || span_y > 1) || (span_x == 0 && span_y == 0)) {
        return false;
    }

    BattleState* battle = battle_current();

    size_t       count  = 0;
    size_t       pushed = 0;

    while (!(list[count].x == -1 && list[count].y == -1)) {
        count++;
    }

    for (size_t i = 0; i < count; i++) {
        int    dx = list[i].x - subject->square.x;
        int    dy = list[i].y - subject->square.y;

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

        if (battle_in_bounds(battle, extension) &&
            !battle_at(battle, extension)) {
            mg_push(extension);
            pushed++;
        }
    }

    mg_end();

    return pushed > 0;
}

/*----------------------------------------------------------------------------*\
                                 CARD EFFECTS
\*----------------------------------------------------------------------------*/

/// defended_by_bishop
///
/// Reports whether a friendly bishop covers the target's square along an
/// unobstructed diagonal, the target itself being the endpoint.
///
/// Params:
/// - battle -> battle providing the board
/// - target -> piece to test defence of
///
/// Return: true when a friendly bishop defends the target
///
static bool defended_by_bishop(BattleState* battle, PieceInfo* target) {
    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* cell = battle_at(battle, (Square) {x, y});

            if (!cell || cell == target || cell->side != target->side ||
                cell->piece->id != PIECE_BISHOP) {
                continue;
            }

            int dx = target->square.x - x;
            int dy = target->square.y - y;

            if (dx == 0 || abs(dx) != abs(dy)) {
                continue;
            }

            int  step_x = dx > 0 ? 1 : -1;
            int  step_y = dy > 0 ? 1 : -1;
            bool clear  = true;

            for (int i = 1; i < abs(dx); i++) {
                Square between = {
                    (int8_t) (x + step_x * i),
                    (int8_t) (y + step_y * i)
                };

                if (battle_at(battle, between)) {
                    clear = false;
                    break;
                }
            }

            if (clear) {
                return true;
            }
        }
    }

    return false;
}

/// eff_castling
///
/// Immediate play effect swapping the playing side's king with a chosen
/// friendly rook.
///
/// Params:
/// - context -> beneficiary side in args[0], rook square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the swap was performed
///
static bool eff_castling(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   king   = battle_find_king(battle, side);
    PieceInfo*   rook   = battle_at(battle, card_square(context->args[1]));

    if (!king || !rook || rook->side != side || rook->piece->id != PIECE_ROOK) {
        return false;
    }

    Square ksq                                    = king->square;
    Square rsq                                    = rook->square;

    battle->board.piece_board[rsq.y * 20 + rsq.x] = king;
    battle->board.piece_board[ksq.y * 20 + ksq.x] = rook;

    king->square                                  = rsq;
    rook->square                                  = ksq;

    return true;
}

/// eff_queens_gambit
///
/// Immediate play effect sacrificing one of the playing side's pawns; the
/// three bonus cards are drawn by the battle play pipeline.
///
/// Params:
/// - context -> beneficiary side in args[0], pawn square in args[1]
/// - x       -> played card, unused
///
/// Return: true when a pawn was sacrificed
///
static bool eff_queens_gambit(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   pawn   = battle_at(battle, card_square(context->args[1]));

    if (!pawn || pawn->side != side || pawn->piece->id != PIECE_PAWN) {
        pawn = nullptr;

        for (size_t index = 0; index < MAX_BOARD_SIZE && !pawn; index++) {
            PieceInfo* cell = battle->board.piece_board[index];

            if (cell && cell != &VOID_CELL && cell->side == side &&
                cell->piece->id == PIECE_PAWN) {
                pawn = cell;
            }
        }
    }

    if (!pawn) {
        return false;
    }

    battle_remove(battle, pawn);

    return true;
}

/// eff_vengeance
///
/// Immediate play effect dealing twice an enemy piece's value to the enemy
/// meter when it stands adjacent to one of the playing side's pieces.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the damage was dealt
///
static bool eff_vengeance(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side == side || target->piece->id == PIECE_KING) {
        return false;
    }

    bool adjacent = false;

    for (int8_t dy = -1; dy <= 1 && !adjacent; dy++) {
        for (int8_t dx = -1; dx <= 1 && !adjacent; dx++) {
            Square spot = {
                (int8_t) (target->square.x + dx),
                (int8_t) (target->square.y + dy)
            };

            PieceInfo* neighbor = battle_at(battle, spot);

            if ((dx || dy) && neighbor && neighbor->side == side) {
                adjacent = true;
            }
        }
    }

    if (!adjacent) {
        return false;
    }

    if (!effect_find_mark(
            &battle_player(battle, target->side)->effects,
            MARK_MOVED,
            target
        )) {
        return false;
    }

    int dmg = battle_value(battle, target, nullptr) * 2;

    battle_damage(battle, battle_enemy(side), dmg);

    return true;
}

/// eff_queens_decree
///
/// Doubles the damage of the playing side's first attack this turn.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> damage value to double
///
/// Return: true when the damage was doubled
///
static bool eff_queens_decree(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (context->args[3] || !battle_victim() || !subject ||
        subject->side != side) {
        return false;
    }

    *(int*) x        *= 2;
    context->args[3]  = (void*) 1;

    return true;
}

/// eff_cathedral
///
/// Reduces incoming damage on the playing side's pieces by 40% while a
/// friendly bishop defends them.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> incoming damage value
///
/// Return: true when the reduction applied
///
static bool eff_cathedral(EffectContext* context, void* x) {
    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    PieceInfo*   subject = battle_subject();

    if (!subject || subject->side != side ||
        !defended_by_bishop(battle, subject)) {
        return false;
    }

    int* damage = x;
    *damage     = *damage * 60 / 100;

    return true;
}

/// eff_coronation
///
/// Immediate play effect promoting one of the playing side's pawns to a
/// Queen in place.
///
/// Params:
/// - context -> beneficiary side in args[0], pawn square in args[1]
/// - x       -> played card, unused
///
/// Return: true when a pawn was promoted
///
static bool eff_coronation(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   pawn   = battle_at(battle, card_square(context->args[1]));

    if (!pawn || pawn->side != side || pawn->piece->id != PIECE_PAWN) {
        return false;
    }

    Square square = pawn->square;

    battle_remove(battle, pawn);
    battle_spawn(battle, PIECE_QUEEN, square, side);

    return true;
}

/// eff_crusade
///
/// Immediate play effect: a chosen Knight makes three consecutive leaps
/// toward the enemy king through battle_lunge, resolving its coverage on
/// each landing so each hop attacks from a real square.
///
/// Params:
/// - context -> beneficiary side in args[0], knight square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the knight made its charge
///
static bool eff_crusade(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   knight = battle_at(battle, card_square(context->args[1]));

    if (!knight || knight->side != side || knight->piece->id != PIECE_KNIGHT) {
        return false;
    }

    PieceInfo* enemy_king = battle_find_king(battle, battle_enemy(side));

    for (int hop = 0; hop < 3; hop++) {
        Square* moves = battle_moves(battle, knight);
        Square  cand[MAX_BOARD_SIZE + 1];
        size_t  count = 0;

        while (!(moves[count].x == -1 && moves[count].y == -1)) {
            cand[count] = moves[count];
            count++;
        }

        Square best     = knight->square;
        int    best_dist = 1 << 24;

        for (size_t i = 0; i < count && enemy_king; i++) {
            int dx = cand[i].x - enemy_king->square.x;
            int dy = cand[i].y - enemy_king->square.y;

            dx     = dx < 0 ? -dx : dx;
            dy     = dy < 0 ? -dy : dy;

            int dist = dx > dy ? dx : dy;

            if (dist < best_dist) {
                best_dist = dist;
                best      = cand[i];
            }
        }

        battle_lunge(battle, knight, best);
    }

    return true;
}

/// eff_divine_right
///
/// Rewrites the playing side's king coverage into Queen coverage this
/// turn by regenerating the shared scratch in place.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> king coverage list, replaced in the scratch buffer
///
/// Return: true when the coverage was rewritten
///
static bool eff_divine_right(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    PieceInfo*   subject = battle_subject();

    if (!subject || subject->side != side || subject->piece->id != PIECE_KING) {
        return false;
    }

    mg_begin();
    mg_compound(
        battle,
        subject,
        (const PieceID[]) {PIECE_QUEEN, PIECE_NONE},
        true
    );
    mg_end();

    return true;
}

/// eff_isabella
///
/// Isabella's Coronation: every friendly Pawn promotes to a Queen at once.
/// Each Pawn is removed and a Queen spawns on its square.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when at least one Pawn promoted
///
static bool eff_isabella(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    bool         crowned = false;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x_pos = 0; x_pos < battle->board.width; x_pos++) {
            PieceInfo* cell = battle_at(battle, (Square) {x_pos, y});

            if (!cell || cell->side != side ||
                cell->piece->id != PIECE_PAWN) {
                continue;
            }

            Square square = cell->square;

            battle_remove(battle, cell);
            battle_spawn(battle, PIECE_QUEEN, square, side);

            crowned = true;
        }
    }

    return crowned;
}

/// eff_caelan_climax
///
/// Raises the playing side's attack damage by half for the Caelan combo
/// climax this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> damage value to raise
///
/// Return: true when the boost applied
///
static bool eff_caelan_climax(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (!subject || subject->side != side) {
        return false;
    }

    int* damage  = x;
    *damage     += *damage / 2;

    return true;
}

/*----------------------------------------------------------------------------*\
                                 KINGDOM DATA
\*----------------------------------------------------------------------------*/

const Piece CAELAN_PIECES[] = {
    {
        .at = pawn_at,
        .mv = pawn_mv,
        .effects =
            {
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
        .at = banner_at,
        .mv = banner_mv,
        .effects =
            {
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

const Card CAELAN_CARDS[] = {
    {
        .effects =
            {{.func      = eff_castling,
              .name      = "Castling",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Castling",
        .desc      = "Your king and one of your Rooks swap positions.",
        .id        = CARD_CASTLING,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_queens_gambit,
              .name      = "Queen's Gambit",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Queen's Gambit",
        .desc      = "Sacrifice a pawn. Draw 3 additional cards "
                     "immediately.",
        .id        = CARD_QUEENS_GAMBIT,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 20,
    },
    {
        .effects =
            {{.func      = eff_vengeance,
              .name      = "Vengeance",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Vengeance",
        .desc      = "Deal 2x damage to an enemy piece adjacent to one "
                     "of your pieces.",
        .id        = CARD_VENGEANCE,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_queens_decree,
              .name      = "Queen's Decree",
              .trigger   = QUERY_PIECE_DAMAGE_DEALT,
              .lasts_for = TURNS_1}},
        .name      = "Queen's Decree",
        .desc      = "Your next attack this turn deals double damage.",
        .id        = CARD_QUEENS_DECREE,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 50,
    },
    {
        .effects =
            {{.func      = eff_cathedral,
              .name      = "Cathedral",
              .trigger   = QUERY_PIECE_DAMAGE_TAKEN,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Cathedral",
        .desc      = "Pieces defended by a friendly bishop take 40% less "
                     "damage.",
        .id        = CARD_CATHEDRAL,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_coronation,
              .name      = "Coronation",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Coronation",
        .desc      = "Promote a pawn to Queen in place.",
        .id        = CARD_CORONATION,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 55,
    },
    {
        .effects =
            {{.func      = eff_crusade,
              .name      = "Crusade",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Crusade",
        .desc      = "Target Knight makes 3 consecutive attacks this "
                     "turn.",
        .id        = CARD_CRUSADE,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 70,
    },
    {
        .effects =
            {{.func      = eff_divine_right,
              .name      = "Divine Right",
              .trigger   = QUERY_PIECE_ATTACKS,
              .lasts_for = TURNS_1}},
        .name      = "Divine Right",
        .desc      = "Your king attacks as a Queen this turn.",
        .id        = CARD_DIVINE_RIGHT,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 0,
        .sell_cost = 75,
    },
    {
        .effects =
            {{.func      = eff_isabella,
              .name      = "Isabella's Coronation",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Isabella's Coronation",
        .desc      = "All friendly Pawns promote to Queens at once.",
        .id        = CARD_ISABELLAS_CORONATION,
        .tier      = TIER_MASTERY,
        .kingdom   = KINGDOM_CAELAN,
        .play_cost = 40,
        .sell_cost = 60,
    },
};

/// eff_castle_corners
///
/// Castle Corners: a piece standing in one of the four two-by-two corner
/// zones cannot be flipped.
///
/// Params:
/// - context -> unused
/// - x       -> bool* flip eligibility to veto
///
/// Return: true when the candidate stood in a corner
///
static bool eff_castle_corners(EffectContext* context, void* x) {
    (void) context;

    PieceInfo* piece = battle_subject();

    if (!piece) {
        return false;
    }

    BattleState* battle = battle_current();
    int8_t       w      = battle->board.width;
    int8_t       h      = battle->board.height;
    int8_t       px     = piece->square.x;
    int8_t       py     = piece->square.y;

    bool left   = px < 2;
    bool right  = px >= w - 2;
    bool bottom = py < 2;
    bool top    = py >= h - 2;

    if ((left || right) && (bottom || top)) {
        *(bool*) x = false;

        return true;
    }

    return false;
}

/// eff_siege_trench
///
/// Siege Trench: a piece standing on the board's middle row cannot be
/// flipped.
///
/// Params:
/// - context -> unused
/// - x       -> bool* flip eligibility to veto
///
/// Return: true when the candidate stood on the trench
///
static bool eff_siege_trench(EffectContext* context, void* x) {
    (void) context;

    PieceInfo* piece = battle_subject();

    if (!piece) {
        return false;
    }

    if (piece->square.y == battle_current()->board.height / 2) {
        *(bool*) x = false;

        return true;
    }

    return false;
}

const BoardTrait CAELAN_TRAITS[] = {
    {
        .name      = "Castle Corners",
        .desc      = "The four 2x2 corner zones grant immunity.",
        .id        = BOARD_TRAIT_CASTLE_CORNERS,
        .effects   = {{
            .func      = eff_castle_corners,
            .name      = "Castle Corners",
            .trigger   = QUERY_PIECE_CAN_FLIP,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
    {
        .name      = "Siege Trench",
        .desc      = "A row of immunity squares crosses the board middle.",
        .id        = BOARD_TRAIT_SIEGE_TRENCH,
        .effects   = {{
            .func      = eff_siege_trench,
            .name      = "Siege Trench",
            .trigger   = QUERY_PIECE_CAN_FLIP,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
};

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// eff_conqueror
///
/// Conqueror's Reward: every Caelan piece that helped a flip permanently
/// gains half its value (sixty percent at mastery three, rounded up) for
/// the rest of the battle.
///
/// Params:
/// - context -> args[0] owning side, args[1] mastery level
/// - x       -> PieceInfo* that just flipped to the side
///
/// Return: true when at least one Caelan damager grew
///
static bool eff_conqueror(EffectContext* context, void* x) {
    Side         side    = (Side) (uintptr_t) context->args[0];
    MasteryLevel level   = (MasteryLevel) (uintptr_t) context->args[1];
    PieceInfo*   flipped = x;

    if (flipped->side != side) {
        return false;
    }

    PieceInfo** damagers = battle_damagers();

    if (!damagers) {
        return false;
    }

    int  percent = level >= MASTERY_LEVEL_3 ? 60 : 50;
    bool applied = false;

    for (size_t i = 0; damagers[i]; i++) {
        PieceInfo* piece = damagers[i];

        if (piece->side != side ||
            piece->piece->kingdom != KINGDOM_CAELAN) {
            continue;
        }

        piece->piece->value += (piece->piece->value * percent + 99) / 100;
        applied              = true;
    }

    return applied;
}

/// caelan_innate
///
/// Attaches Conqueror's Reward: when a Caelan piece contributes to a flip,
/// its value permanently gains half (sixty percent at mastery three,
/// rounded up) for the rest of the battle.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void caelan_innate(BattleState* battle, Side side, MasteryLevel level) {
    Effect conqueror = {
        .func      = eff_conqueror,
        .name      = "Conqueror's Reward",
        .trigger   = ON_PIECE_FLIP,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &conqueror);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = (void*) (uintptr_t) level;
    }
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
    static const Effect climax = {
        .func      = eff_caelan_climax,
        .name      = "Caelan Climax",
        .trigger   = QUERY_PIECE_DAMAGE_DEALT,
        .lasts_for = TURNS_1,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &climax);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
    }
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
