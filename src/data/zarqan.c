//! zarqan.c
//!
//! Zarqan kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Royal Substitution innate, climax, Many-Faced
//! King overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// wazir_gen
///
/// Generates the Wazir's single orthogonal steps.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void wazir_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, ORTHOGONAL_DIRECTIONS, threat);
}

/// wazir_mv
///
/// Generates the Wazir's movement through wazir_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* wazir_mv(BattleState* battle, PieceInfo* self) {
    wazir_gen(battle, self, false);

    return mg_end();
}

/// wazir_at
///
/// Generates the Wazir's attack coverage through wazir_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* wazir_at(BattleState* battle, PieceInfo* self) {
    wazir_gen(battle, self, true);

    return mg_end();
}

/// jamal_gen
///
/// Generates the Jamal's (1,3) leaps.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void jamal_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {
            {1, 3},  {1, -3},  {-1, 3},  {-1, -3},
            {3, 1},  {3, -1},  {-3, 1},  {-3, -1},
            {0, 0},
        },
        threat
    );
}

/// jamal_mv
///
/// Generates the Jamal's movement through jamal_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* jamal_mv(BattleState* battle, PieceInfo* self) {
    jamal_gen(battle, self, false);

    return mg_end();
}

/// jamal_at
///
/// Generates the Jamal's attack coverage through jamal_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* jamal_at(BattleState* battle, PieceInfo* self) {
    jamal_gen(battle, self, true);

    return mg_end();
}

/// talliya_mv
///
/// Generates the Talliya's movement: diagonal slides of at least two
/// squares.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* talliya_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, DIAGONAL_DIRECTIONS, 2, 127, false);

    return mg_end();
}

/// talliya_at
///
/// Generates the Talliya's attack coverage: diagonal slides of at least
/// two squares.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* talliya_at(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, DIAGONAL_DIRECTIONS, 2, 127, true);

    return mg_end();
}

/// ziraafa_gen
///
/// Generates the Ziraafa's bent path: one empty diagonal step then a
/// straight orthogonal run of at least three squares outward along either
/// component of the diagonal.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void ziraafa_gen(BattleState* battle, PieceInfo* self, bool threat) {
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
                    if (threat && step >= 3 && occupant->side != self->side) {
                        mg_push(dest);
                    }

                    break;
                }

                if (step >= 3) {
                    mg_push(dest);
                }
            }
        }
    }
}

/// ziraafa_mv
///
/// Generates the Ziraafa's movement through ziraafa_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* ziraafa_mv(BattleState* battle, PieceInfo* self) {
    ziraafa_gen(battle, self, false);

    return mg_end();
}

/// ziraafa_at
///
/// Generates the Ziraafa's attack coverage through ziraafa_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* ziraafa_at(BattleState* battle, PieceInfo* self) {
    ziraafa_gen(battle, self, true);

    return mg_end();
}

/// king_gen
///
/// Generates the eight king steps, shared by the Shahzadeh and Old King.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void king_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, ALL_DIRECTIONS, threat);
}

/// shahzadeh_mv
///
/// Generates the Shahzadeh's king movement; the swap-with-king option is
/// appended by its embedded effect.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* shahzadeh_mv(BattleState* battle, PieceInfo* self) {
    king_gen(battle, self, false);

    return mg_end();
}

/// shahzadeh_at
///
/// Generates the Shahzadeh's king coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* shahzadeh_at(BattleState* battle, PieceInfo* self) {
    king_gen(battle, self, true);

    return mg_end();
}

/// old_king_mv
///
/// Generates the Old King's king movement.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* old_king_mv(BattleState* battle, PieceInfo* self) {
    king_gen(battle, self, false);

    return mg_end();
}

/// old_king_at
///
/// Generates the Old King's king coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* old_king_at(BattleState* battle, PieceInfo* self) {
    king_gen(battle, self, true);

    return mg_end();
}

/// cataphract_gen
///
/// Generates the Cataphract's pattern: the union of Knight and Jamal.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void cataphract_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_KNIGHT, PIECE_JAMAL, PIECE_NONE},
        threat
    );
}

/// cataphract_mv
///
/// Generates the Cataphract's movement through cataphract_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* cataphract_mv(BattleState* battle, PieceInfo* self) {
    cataphract_gen(battle, self, false);

    return mg_end();
}

/// cataphract_at
///
/// Generates the Cataphract's attack coverage through cataphract_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* cataphract_at(BattleState* battle, PieceInfo* self) {
    cataphract_gen(battle, self, true);

    return mg_end();
}

/// rook_mv
///
/// Generates the Rook's movement: unbounded orthogonal slides.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* rook_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, ORTHOGONAL_DIRECTIONS, 1, 127, false);

    return mg_end();
}

/// rook_at
///
/// Generates the Rook's attack coverage: unbounded orthogonal slides.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* rook_at(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, ORTHOGONAL_DIRECTIONS, 1, 127, true);

    return mg_end();
}

/// elephant_mv
///
/// Generates the War Elephant's movement: the Ziraafa pattern copied
/// through the registry.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* elephant_mv(BattleState* battle, PieceInfo* self) {
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_ZIRAAFA, PIECE_NONE},
        false
    );

    return mg_end();
}

/// elephant_at
///
/// Generates the War Elephant's attack coverage: all adjacent squares, so
/// it strikes every adjacent enemy at once.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* elephant_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                PIECE EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_shahzadeh_swap
///
/// Appends the owner's king square to the Shahzadeh's move list, letting a
/// move onto it swap the two, which battle_move performs for a friendly
/// destination.
///
/// Params:
/// - context -> Shahzadeh's context, args[0] self
/// - x       -> move list, extended through the shared scratch
///
/// Return: true when the swap option was added
///
static bool eff_shahzadeh_swap(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self) {
        return false;
    }

    BattleState* battle = battle_current();

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t sx = 0; sx < battle->board.width; sx++) {
            PieceInfo* cell = battle_at(battle, (Square) {sx, y});

            if (cell && cell->side == self->side &&
                cell->piece->id == PIECE_KING) {
                mg_push((Square) {sx, y});
                mg_end();

                return true;
            }
        }
    }

    return false;
}

/// eff_citadel_flag
///
/// Clears a boolean gate for the citadel's target, used to forbid its
/// movement, attacks, and flipping.
///
/// Params:
/// - context -> target piece in args[1]
/// - x       -> boolean gate to clear
///
/// Return: true when the gate was cleared
///
static bool eff_citadel_flag(EffectContext* context, void* x) {
    if (battle_subject() != context->args[1]) {
        return false;
    }

    *(bool*) x = false;

    return true;
}

/*----------------------------------------------------------------------------*\
                                 CARD EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_counsel
///
/// Discards one card from next turn's hand by vetoing the first drawable
/// card once.
///
/// Params:
/// - context -> spent flag in args[3]
/// - x       -> can-draw flag to clear
///
/// Return: true when a card was discarded
///
static bool eff_counsel(EffectContext* context, void* x) {
    if (context->args[3]) {
        return false;
    }

    *(bool*) x       = false;
    context->args[3] = (void*) 1;

    return true;
}

/// eff_pillage
///
/// Immediate play effect adding five centipawns for each friendly piece on
/// the board, including the king.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true, the gain always applies
///
static bool eff_pillage(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    int          count  = 0;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t sx = 0; sx < battle->board.width; sx++) {
            PieceInfo* cell = battle_at(battle, (Square) {sx, y});

            if (cell && cell->side == side) {
                count++;
            }
        }
    }

    battle_player(battle, side)->cp += 5 * count;

    return true;
}

/// eff_royal_decoy_first
///
/// Advertises every friendly piece as the first swap target for Royal
/// Decoy.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_royal_decoy_first(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side;
    });

    return card_target_count(x) > 0;
}

/// eff_royal_decoy_second
///
/// Advertises every friendly piece other than the first pick as the second
/// swap target for Royal Decoy.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_royal_decoy_second(EffectContext* context, void* x) {
    Side   side  = (Side) (uintptr_t) context->args[0];
    Square first = card_target_at(battle_pending_picks()[0].value);

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side &&
               !(p->square.x == first.x && p->square.y == first.y);
    });

    return card_target_count(x) > 0;
}

/// eff_royal_decoy_pick
///
/// Royal Decoy resolution: swaps the positions of the two chosen friendly
/// pieces.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* pick list, first and second pieces
///
/// Return: true when the swap was performed
///
static bool eff_royal_decoy_pick(EffectContext* context, void* x) {
    CardTarget*  picks  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (picks[0].kind != TARGET_PIECE || picks[1].kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* first  = battle_at(battle, card_target_at(picks[0].value));
    PieceInfo* second = battle_at(battle, card_target_at(picks[1].value));

    if (!first || !second || first == second || first->side != side ||
        second->side != side) {
        return false;
    }

    battle_swap(battle, first, second);

    return true;
}

/// eff_bazaar_targets
///
/// Bazaar targeting: advertises every friendly non-king piece as a square
/// target.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_bazaar_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side && p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_bazaar_pick
///
/// Bazaar resolution: sells the chosen friendly piece for 150% of its
/// value.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the piece was sold
///
static bool eff_bazaar_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side || piece->piece->id == PIECE_KING) {
        return false;
    }

    int value = battle_value(battle, piece, nullptr);

    battle_remove(battle, piece);
    battle_player(battle, side)->cp += value * 150 / 100;

    return true;
}

/// eff_steppe_riders
///
/// Immediate play effect granting every friendly Knight, Jamal, and
/// Cataphract one extra move this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when at least one rider was granted a move
///
static bool eff_steppe_riders(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    bool         granted = false;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t sx = 0; sx < battle->board.width; sx++) {
            PieceInfo* cell = battle_at(battle, (Square) {sx, y});

            if (!cell || cell->side != side) {
                continue;
            }

            PieceID id = cell->piece->id;

            if (id == PIECE_KNIGHT || id == PIECE_JAMAL ||
                id == PIECE_CATAPHRACT) {
                piece_grant_free_move(cell, "Steppe Ride");
                granted = true;
            }
        }
    }

    return granted;
}

/// eff_copy_piece_targets
///
/// Advertises every friendly piece as the piece to receive a copied move,
/// the first step shared by Ambition and Conquest.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_copy_piece_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side;
    });

    return card_target_count(x) > 0;
}

/// eff_copy_type_targets
///
/// Advertises every registered piece identity as the movement to copy, the
/// second step shared by Ambition and Conquest.
///
/// Params:
/// - context -> beneficiary side in args[0], unused
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one identity was offered
///
static bool eff_copy_type_targets(EffectContext* context, void* x) {
    (void) context;

    BattleState* battle = battle_current();
    Square       tsq    = card_target_at(battle_pending_picks()[0].value);

    card_targets_piece_type(x, ^bool(const Piece* pc) {
        PieceInfo* target = battle_at(battle, tsq);

        return battle_piece_unlocked(pc->id) &&
               (!target || target->piece->id != pc->id);
    });

    return card_target_count(x) > 0;
}

/// eff_ambition_pick
///
/// Ambition resolution: the chosen piece copies the chosen identity's
/// movement pattern for this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* pick list, piece square then piece type
///
/// Return: true when the copy was attached
///
static bool eff_ambition_pick(EffectContext* context, void* x) {
    CardTarget*  picks  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (picks[0].kind != TARGET_PIECE ||
        picks[1].kind != TARGET_PIECE_TYPE) {
        return false;
    }

    PieceInfo* target = battle_at(battle, card_target_at(picks[0].value));
    PieceID    copied = (PieceID) picks[1].value;

    if (!target || target->side != side || copied >= PIECE_COUNT ||
        !PIECE_REGISTRY[copied]) {
        return false;
    }

    piece_adopt_move(battle, target, copied, TURNS_1, true);

    return true;
}

/// eff_citadel_targets
///
/// Citadel targeting: advertises every piece on the board, either side, as
/// a square target.
///
/// Params:
/// - context -> beneficiary side in args[0], unused
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_citadel_targets(EffectContext* context, void* x) {
    (void) context;

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == SIDE_WHITE || p->side == SIDE_BLACK;
    });

    return card_target_count(x) > 0;
}

/// eff_citadel_pick
///
/// Citadel resolution: makes the chosen piece immobile and immune for two
/// turns by attaching veto gates to its own side's list.
///
/// Params:
/// - context -> beneficiary side in args[0], unused
/// - x       -> CardTarget* chosen square
///
/// Return: true when the citadel was attached
///
static bool eff_citadel_pick(EffectContext* context, void* x) {
    (void) context;

    CardTarget*  chosen = x;
    BattleState* battle = battle_current();

    if (chosen->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* target = battle_at(battle, card_target_at(chosen->value));

    if (!target || !target->piece) {
        return false;
    }

    static const EffectTrigger gates[] = {
        QUERY_PIECE_CAN_MOVE,
        QUERY_PIECE_CAN_ATTACK,
        QUERY_PIECE_CAN_FLIP,
    };

    for (size_t i = 0; i < 3; i++) {
        Effect gate = {
            .func      = eff_citadel_flag,
            .name      = "Citadel",
            .trigger   = gates[i],
            .lasts_for = TURNS_2,
        };
        Effect* attached =
            effect_attach(&battle_player(battle, target->side)->effects, &gate);

        if (attached) {
            attached->context->args[1] = target;
        }
    }

    return true;
}

/// eff_conquest_pick
///
/// Conquest resolution: the chosen piece permanently adopts the chosen
/// identity's movement pattern for the rest of the battle. Shares the two
/// query steps with Ambition.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* pick list, piece square then piece type
///
/// Return: true when the adoption was attached
///
static bool eff_conquest_pick(EffectContext* context, void* x) {
    CardTarget*  picks  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (picks[0].kind != TARGET_PIECE ||
        picks[1].kind != TARGET_PIECE_TYPE) {
        return false;
    }

    PieceInfo* target = battle_at(battle, card_target_at(picks[0].value));
    PieceID    copied = (PieceID) picks[1].value;

    if (!target || target->side != side || copied >= PIECE_COUNT ||
        !PIECE_REGISTRY[copied]) {
        return false;
    }

    piece_adopt_move(battle, target, copied, ENTIRE_BATTLE, true);

    return true;
}

/// eff_timur_watch
///
/// Timur's Conquest watcher: once the side's meter drops below twenty it
/// swaps the friendly king with a Zarqan piece, the automatic Royal
/// Substitution. Fires after each cascade step settles.
///
/// Params:
/// - context -> args[0] owning side, args[1] spent flag
/// - x       -> flipped piece, unused
///
/// Return: true when the automatic swap fired
///
static bool eff_timur_watch(EffectContext* context, void* x) {
    (void) x;

    if (context->args[1]) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (battle_player(battle, side)->meter >= 20) {
        return false;
    }

    PieceInfo* king = battle_find_king(battle, side);

    if (!king) {
        return false;
    }

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (!cell || cell == &VOID_CELL || cell->side != side ||
            cell->piece->kingdom != KINGDOM_ZARQAN) {
            continue;
        }

        battle_swap(battle, king, cell);

        context->args[1] = (void*) 1;

        return true;
    }

    return false;
}

/// eff_timur
///
/// Timur's Conquest: attaches the automatic Royal Substitution watcher to
/// the playing side for the rest of the battle.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when the watcher was attached
///
static bool eff_timur(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    static const Effect watch = {
        .func      = eff_timur_watch,
        .name      = "Timur's Conquest",
        .trigger   = ON_PIECE_FLIP_POST,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &watch);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
    }

    return attached != nullptr;
}

/*----------------------------------------------------------------------------*\
                                 KINGDOM DATA
\*----------------------------------------------------------------------------*/

const Piece ZARQAN_PIECES[] = {
    {
        .at      = wazir_at,
        .mv      = wazir_mv,
        .name    = "Wazir",
        .desc    = "1 orthogonal.",
        .id      = PIECE_WAZIR,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 15,
    },
    {
        .at      = jamal_at,
        .mv      = jamal_mv,
        .name    = "Jamal",
        .desc    = "(1,3) leaper.",
        .id      = PIECE_JAMAL,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 20,
    },
    {
        .at      = talliya_at,
        .mv      = talliya_mv,
        .name    = "Talliya",
        .desc    = "Diagonal slider. Must move at least 2.",
        .id      = PIECE_TALLIYA,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_TOWN,
        .class   = MOVE_SLIDER,
        .value   = 30,
    },
    {
        .at      = ziraafa_at,
        .mv      = ziraafa_mv,
        .name    = "Ziraafa",
        .desc    = "1 diagonal step, then 3+ squares straight.",
        .id      = PIECE_ZIRAAFA,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SPECIAL,
        .value   = 35,
    },
    {
        .at      = shahzadeh_at,
        .mv      = shahzadeh_mv,
        .effects =
            {
                {
                    .func      = eff_shahzadeh_swap,
                    .name      = "Royal Swap",
                    .trigger   = QUERY_PIECE_MOVES,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Shahzadeh",
        .desc    = "Second king. Swaps positions with the real king.",
        .id      = PIECE_SHAHZADEH,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_LEAPER,
        .value   = 100,
    },
    {
        .at      = old_king_at,
        .mv      = old_king_mv,
        .name    = "Old King",
        .desc    = "King movement. Counts as a normal piece.",
        .id      = PIECE_OLD_KING,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_TOWN,
        .class   = MOVE_LEAPER,
        .value   = 30,
    },
    {
        .at      = cataphract_at,
        .mv      = cataphract_mv,
        .name    = "Cataphract",
        .desc    = "May move as Knight or Jamal each action.",
        .id      = PIECE_CATAPHRACT,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_TOWN,
        .class   = MOVE_LEAPER,
        .value   = 40,
    },
    {
        .at      = rook_at,
        .mv      = rook_mv,
        .name    = "Rook",
        .desc    = "Orthogonal slider. The only way to obtain a Rook.",
        .id      = PIECE_ROOK,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SLIDER,
        .value   = 50,
    },
    {
        .at      = elephant_at,
        .mv      = elephant_mv,
        .name    = "War Elephant",
        .desc    = "Ziraafa movement. Damages two adjacent enemies "
                   "simultaneously.",
        .id      = PIECE_WAR_ELEPHANT,
        .kingdom = KINGDOM_ZARQAN,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SPECIAL,
        .value   = 50,
    },
};

const Card ZARQAN_CARDS[] = {
    {
        .effects =
            {{.func      = eff_counsel,
              .name      = "Counsel",
              .trigger   = QUERY_CARD_CAN_DRAW,
              .lasts_for = TURNS_2}},
        .name      = "Counsel",
        .desc      = "Peek at next turn's cards. Discard 1 from the "
                     "upcoming hand.",
        .id        = CARD_COUNSEL,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_pillage,
              .name      = "Pillage",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Pillage",
        .desc      = "Gain 5 cp for each friendly piece on the board, "
                     "including the king.",
        .id        = CARD_PILLAGE,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 20,
    },
    {
        .effects =
            {{.func      = eff_royal_decoy_first,
              .name      = "Royal Decoy",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_royal_decoy_second,
              .name      = "Royal Decoy",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_royal_decoy_pick,
              .name      = "Royal Decoy",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Royal Decoy",
        .desc      = "Swap positions of any 2 of your pieces.",
        .id        = CARD_ROYAL_DECOY,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_bazaar_targets,
              .name      = "Bazaar",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_bazaar_pick,
              .name      = "Bazaar",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Bazaar",
        .desc      = "Sell one of your pieces for 150% of its value.",
        .id        = CARD_BAZAAR,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 35,
    },
    {
        .effects =
            {{.func      = eff_steppe_riders,
              .name      = "Steppe Riders",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Steppe Riders",
        .desc      = "All your Knights, Jamals, and Cataphracts move twice "
                     "this turn.",
        .id        = CARD_STEPPE_RIDERS,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_copy_piece_targets,
              .name      = "Ambition",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_copy_type_targets,
              .name      = "Ambition",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_ambition_pick,
              .name      = "Ambition",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Ambition",
        .desc      = "Target piece copies any other piece's movement "
                     "pattern this turn.",
        .id        = CARD_AMBITION,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_citadel_targets,
              .name      = "Citadel",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_citadel_pick,
              .name      = "Citadel",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Citadel",
        .desc      = "Target piece becomes immobile and immune for 2 "
                     "turns.",
        .id        = CARD_CITADEL,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 0,
        .sell_cost = 50,
    },
    {
        .effects =
            {{.func      = eff_copy_piece_targets,
              .name      = "Conquest",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_copy_type_targets,
              .name      = "Conquest",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_conquest_pick,
              .name      = "Conquest",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Conquest",
        .desc      = "Target piece permanently adopts any other piece's "
                     "movement pattern this battle.",
        .id        = CARD_CONQUEST,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 50,
        .sell_cost = 80,
    },
    {
        .effects =
            {{.func      = eff_timur,
              .name      = "Timur's Conquest",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Timur's Conquest",
        .desc      = "Royal Substitution triggers when the meter drops "
                     "below 20.",
        .id        = CARD_TIMURS_CONQUEST,
        .tier      = TIER_MASTERY,
        .kingdom   = KINGDOM_ZARQAN,
        .play_cost = 40,
        .sell_cost = 60,
    },
};

/// eff_sandstorm
///
/// Sandstorm: on even turns a slider may move at most three squares. Move
/// entries farther than Chebyshev three from the subject are dropped from
/// the list in place. Bent movers are MOVE_SPECIAL and exempt.
///
/// Params:
/// - context -> unused
/// - x       -> Square* move list to prune
///
/// Return: true when the list was pruned
///
static bool eff_sandstorm(EffectContext* context, void* x) {
    (void) context;

    BattleState* battle  = battle_current();
    PieceInfo*   subject = battle_subject();

    if (!subject || subject->piece->class != MOVE_SLIDER ||
        battle->turn % 2 != 0) {
        return false;
    }

    Square* moves   = x;
    size_t  write   = 0;
    bool    pruned  = false;

    for (size_t read = 0; !(moves[read].x == -1 && moves[read].y == -1);
         read++) {
        int dx = moves[read].x - subject->square.x;
        int dy = moves[read].y - subject->square.y;

        dx     = dx < 0 ? -dx : dx;
        dy     = dy < 0 ? -dy : dy;

        if ((dx > 3 || dy > 3)) {
            pruned = true;
            continue;
        }

        moves[write] = moves[read];
        write++;
    }

    moves[write] = SQUARE_END;

    return pruned;
}

/// eff_mirage
///
/// Mirage: a twentieth of the board's squares are voided at build.
///
/// Params:
/// - context -> unused
/// - x       -> unused built board
///
/// Return: true, the scatter always applies
///
static bool eff_mirage(EffectContext* context, void* x) {
    (void) context;
    (void) x;

    battle_scatter_voids(battle_current(), 5);

    return true;
}

const BoardTrait ZARQAN_TRAITS[] = {
    {
        .name      = "Sandstorm",
        .desc      = "On even turns, all sliders move at most 3 squares.",
        .id        = BOARD_TRAIT_SANDSTORM,
        .effects   = {{
            .func      = eff_sandstorm,
            .name      = "Sandstorm",
            .trigger   = QUERY_PIECE_MOVES,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
    {
        .name = "Mirage",
        .desc = "5% of squares cannot be entered, marked when revealed.",
        .id   = BOARD_TRAIT_MIRAGE,
        .effects = {{
            .func      = eff_mirage,
            .name      = "Mirage",
            .trigger   = ON_BOARD_BUILD,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
};

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// eff_royal_sub
///
/// Royal Substitution: any Zarqan piece may swap places with the friendly
/// king by moving onto its square. The king's square is appended to the
/// piece's move list so battle_move performs the swap.
///
/// Params:
/// - context -> args[0] owning side
/// - x       -> Square* move list to extend
///
/// Return: true when the king square was appended
///
static bool eff_royal_sub(EffectContext* context, void* x) {
    Side       side  = (Side) (uintptr_t) context->args[0];
    PieceInfo* piece = battle_subject();

    if (!piece || piece->side != side ||
        piece->piece->kingdom != KINGDOM_ZARQAN) {
        return false;
    }

    PieceInfo* king = battle_find_king(battle_current(), side);

    if (!king) {
        return false;
    }

    Square* moves = x;
    size_t  count = 0;

    while (!(moves[count].x == -1 && moves[count].y == -1)) {
        if (moves[count].x == king->square.x &&
            moves[count].y == king->square.y) {
            return false;
        }

        count++;
    }

    moves[count]     = king->square;
    moves[count + 1] = SQUARE_END;

    return true;
}

/// zarqan_innate
///
/// Attaches Royal Substitution so any Zarqan piece can swap with the
/// friendly king. The once-per-battle and free-action limits are relaxed:
/// the swap is repeatable and costs a normal move action.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void zarqan_innate(BattleState* battle, Side side, MasteryLevel level) {
    (void) level;

    Effect substitution = {
        .func      = eff_royal_sub,
        .name      = "Royal Substitution",
        .trigger   = QUERY_PIECE_MOVES,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &substitution);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
    }
}

/// zarqan_climax
///
/// Fires the Zarqan combo climax for the given side, swapping its two
/// highest-value pieces.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void zarqan_climax(BattleState* battle, Side side) {
    PieceInfo* best   = nullptr;
    PieceInfo* second = nullptr;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* cell = battle_at(battle, (Square) {x, y});

            if (!cell || cell->side != side || cell->piece->id == PIECE_KING) {
                continue;
            }

            if (!best || cell->piece->value > best->piece->value) {
                second = best;
                best   = cell;
            } else if (!second || cell->piece->value > second->piece->value) {
                second = cell;
            }
        }
    }

    if (!best || !second) {
        return;
    }

    Square bsq                                    = best->square;
    Square ssq                                    = second->square;

    battle->board.piece_board[ssq.y * 20 + ssq.x] = best;
    battle->board.piece_board[bsq.y * 20 + bsq.x] = second;

    best->square                                  = ssq;
    second->square                                = bsq;
}

/// zarqan_overseer
///
/// Sets up the Many-Faced King overseer battle with its bespoke army and
/// boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void zarqan_overseer(BattleState* battle) {
    (void) battle;
}

/// zarqan_event
///
/// Resolves a Zarqan narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void zarqan_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
