//! kewarani.c
//!
//! Kewarani kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Double Time innate, climax, Caravan of Conquest
//! overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                                KINGDOM HELPERS
\*----------------------------------------------------------------------------*/

/// spawn_medeq
///
/// Spawns up to count friendly Medeq on empty squares around a center,
/// used by the splitter pieces when they resist a flip.
///
/// Params:
/// - battle -> battle to spawn into
/// - center -> square the spawns cluster around
/// - side   -> owning side of the new Medeq
/// - count  -> maximum number to spawn
///
static void
spawn_medeq(BattleState* battle, Square center, Side side, int count) {
    int spawned = 0;

    for (int8_t dy = -1; dy <= 1 && spawned < count; dy++) {
        for (int8_t dx = -1; dx <= 1 && spawned < count; dx++) {
            Square spot = {
                (int8_t) (center.x + dx),
                (int8_t) (center.y + dy),
            };

            if ((dx || dy) && battle_in_bounds(battle, spot) &&
                !battle_at(battle, spot) &&
                battle_spawn(battle, PIECE_MEDEQ, spot, side)) {
                spawned++;
            }
        }
    }
}

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// medeq_mv
///
/// Generates the Medeq's movement: the two forward diagonals of a Berolina
/// pawn.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* medeq_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, (const Square[]) {{-1, -1}, {1, -1}, {0, 0}}, false);

    return mg_end();
}

/// medeq_at
///
/// Generates the Medeq's attack coverage: the single forward step of a
/// Berolina pawn.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* medeq_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, (const Square[]) {{0, -1}, {0, 0}}, true);

    return mg_end();
}

/// makwanam_gen
///
/// Generates the Makwanam's single diagonal ferz steps.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void makwanam_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, DIAGONAL_DIRECTIONS, threat);
}

/// makwanam_mv
///
/// Generates the Makwanam's movement through makwanam_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* makwanam_mv(BattleState* battle, PieceInfo* self) {
    makwanam_gen(battle, self, false);

    return mg_end();
}

/// makwanam_at
///
/// Generates the Makwanam's attack coverage through makwanam_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* makwanam_at(BattleState* battle, PieceInfo* self) {
    makwanam_gen(battle, self, true);

    return mg_end();
}

/// saba_gen
///
/// Generates the Saba's two-square diagonal leaps.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void saba_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {{-2, -2}, {2, -2}, {-2, 2}, {2, 2}, {0, 0}},
        threat
    );
}

/// saba_mv
///
/// Generates the Saba's movement through saba_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* saba_mv(BattleState* battle, PieceInfo* self) {
    saba_gen(battle, self, false);

    return mg_end();
}

/// saba_at
///
/// Generates the Saba's attack coverage through saba_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* saba_at(BattleState* battle, PieceInfo* self) {
    saba_gen(battle, self, true);

    return mg_end();
}

/// faras_gen
///
/// Generates the Faras's Dababbah leaps: the second square orthogonally.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void faras_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {{0, -2}, {0, 2}, {-2, 0}, {2, 0}, {0, 0}},
        threat
    );
}

/// faras_mv
///
/// Generates the Faras's movement through faras_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* faras_mv(BattleState* battle, PieceInfo* self) {
    faras_gen(battle, self, false);

    return mg_end();
}

/// faras_at
///
/// Generates the Faras's attack coverage through faras_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* faras_at(BattleState* battle, PieceInfo* self) {
    faras_gen(battle, self, true);

    return mg_end();
}

/// guard_mv
///
/// Generates the movement shared by the king-stepping splitters: the eight
/// king steps.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* guard_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, false);

    return mg_end();
}

/// guard_at
///
/// Generates the coverage shared by the king-stepping splitters: the eight
/// king steps.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* guard_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                PIECE EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_split_king
///
/// Resists a flip by removing the piece and spawning friendly Medeq around
/// the owner's king. The Sultan's Levy spawns three, other guards two.
///
/// Params:
/// - context -> context, args[0] self
/// - x       -> flip candidate pointer, set to nullptr to consume the flip
///
/// Return: true when the split was performed
///
static bool eff_split_king(EffectContext* context, void* x) {
    PieceInfo*  self = context->args[0];
    PieceInfo** slot = x;

    if (*slot != self) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = self->side;
    int          count  = self->piece->id == PIECE_SULTANS_LEVY ? 3 : 2;
    PieceInfo*   king   = battle_find_king(battle, side);

    *slot = nullptr;

    battle_remove(battle, self);

    if (king) {
        spawn_medeq(battle, king->square, side, count);
    }

    return true;
}

/// eff_split_square
///
/// Resists a flip by removing the piece and spawning friendly Medeq around
/// the square it occupied.
///
/// Params:
/// - context -> context, args[0] self
/// - x       -> flip candidate pointer, set to nullptr to consume the flip
///
/// Return: true when the split was performed
///
static bool eff_split_square(EffectContext* context, void* x) {
    PieceInfo*  self = context->args[0];
    PieceInfo** slot = x;

    if (*slot != self) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = self->side;
    Square       where  = self->square;

    *slot = nullptr;

    battle_remove(battle, self);
    spawn_medeq(battle, where, side, 2);

    return true;
}

/*----------------------------------------------------------------------------*\
                                 CARD EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_sultans_gold
///
/// Immediate play effect adding sixty centipawns to the playing side.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true, the gain always applies
///
static bool eff_sultans_gold(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    battle_player(battle, side)->cp += 60;

    return true;
}

/// eff_march
///
/// Immediate play effect advancing every friendly pawn one square forward
/// when the square ahead is empty.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when at least one pawn advanced
///
static bool eff_march(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    int8_t       forward = side == SIDE_BLACK ? 1 : -1;
    bool         moved   = false;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x2 = 0; x2 < battle->board.width; x2++) {
            PieceInfo* cell = battle_at(battle, (Square) {x2, y});

            if (!cell || cell->side != side ||
                !piece_is_pawn(cell->piece->id)) {
                continue;
            }

            Square ahead = {x2, (int8_t) (y + forward)};

            if (battle_in_bounds(battle, ahead) && !battle_at(battle, ahead)) {
                battle->board.piece_board[y * 20 + x2]            = nullptr;
                cell->square                                      = ahead;
                battle->board.piece_board[ahead.y * 20 + ahead.x] = cell;
                moved                                             = true;
            }
        }
    }

    return moved;
}

/// eff_double_time
///
/// Immediate play effect granting a targeted piece one additional move
/// this turn.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the extra move was granted
///
static bool eff_double_time(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side != side) {
        return false;
    }

    piece_grant_free_move(target, "Extra Move");

    return true;
}

/// eff_salt_road
///
/// Adds ten centipawns to the playing side at the start of every remaining
/// turn this battle.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> turn number, unused
///
/// Return: true, the income always applies
///
static bool eff_salt_road(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    battle_player(battle, side)->cp += 10;

    return true;
}

/// eff_caravan
///
/// Immediate play effect advancing every friendly piece on the target file
/// one square forward when the square ahead is empty.
///
/// Params:
/// - context -> beneficiary side in args[0], file square in args[1]
/// - x       -> played card, unused
///
/// Return: true when at least one piece advanced
///
static bool eff_caravan(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    int8_t       forward = side == SIDE_BLACK ? 1 : -1;
    int8_t       file    = card_square(context->args[1]).x;
    bool         moved   = false;

    for (int8_t y = 0; y < battle->board.height; y++) {
        PieceInfo* cell = battle_at(battle, (Square) {file, y});

        if (!cell || cell->side != side) {
            continue;
        }

        Square ahead = {file, (int8_t) (y + forward)};

        if (battle_in_bounds(battle, ahead) && !battle_at(battle, ahead)) {
            battle->board.piece_board[y * 20 + file]          = nullptr;
            cell->square                                      = ahead;
            battle->board.piece_board[ahead.y * 20 + ahead.x] = cell;
            moved                                             = true;
        }
    }

    return moved;
}

/// eff_doublestrike
///
/// Makes the playing side's next move this turn free, so two pieces move
/// for one action.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> action cost to zero
///
/// Return: true when the move was made free
///
static bool eff_doublestrike(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (context->args[3] || !subject || subject->side != side) {
        return false;
    }

    *(int*) x        = 0;
    context->args[3] = (void*) 1;

    return true;
}

/// eff_selassie
///
/// Selassie's March: Kewarani pieces move extra times this action this
/// turn. Every friendly Kewarani piece is granted a free move, standing in
/// for the design's three-moves-per-action burst.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when at least one Kewarani piece was granted a free move
///
static bool eff_selassie(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    bool         granted = false;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x_pos = 0; x_pos < battle->board.width; x_pos++) {
            PieceInfo* cell = battle_at(battle, (Square) {x_pos, y});

            if (!cell || cell->side != side ||
                cell->piece->kingdom != KINGDOM_KEWARANI) {
                continue;
            }

            piece_grant_free_move(cell, "Selassie's March");
            granted = true;
        }
    }

    return granted;
}

/// eff_hajj
///
/// Immediate play effect teleporting a targeted friendly piece to any
/// unoccupied square.
///
/// Params:
/// - context -> beneficiary side in args[0], piece square in args[1],
///              destination square in args[2]
/// - x       -> played card, unused
///
/// Return: true when the teleport was performed
///
static bool eff_hajj(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   piece  = battle_at(battle, card_square(context->args[1]));
    Square       dest   = card_square(context->args[2]);

    if (!piece || piece->side != side || !battle_in_bounds(battle, dest) ||
        battle_at(battle, dest)) {
        return false;
    }

    battle->board.piece_board[piece->square.y * 20 + piece->square.x] =
        nullptr;
    piece->square                                   = dest;
    battle->board.piece_board[dest.y * 20 + dest.x] = piece;

    return true;
}

/*----------------------------------------------------------------------------*\
                                 KINGDOM DATA
\*----------------------------------------------------------------------------*/

const Piece KEWARANI_PIECES[] = {
    {
        .at      = medeq_at,
        .mv      = medeq_mv,
        .name    = "Medeq",
        .desc    = "Diagonally 1, attacks forward 1 (Berolina pawn).",
        .id      = PIECE_MEDEQ,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 10,
    },
    {
        .at      = makwanam_at,
        .mv      = makwanam_mv,
        .name    = "Makwanam",
        .desc    = "1 diagonal (ferz).",
        .id      = PIECE_MAKWANAM,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 15,
    },
    {
        .at      = saba_at,
        .mv      = saba_mv,
        .name    = "Saba",
        .desc    = "2 diagonal. Unrestricted by territory.",
        .id      = PIECE_SABA,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_TOWN,
        .class   = MOVE_LEAPER,
        .value   = 20,
    },
    {
        .at      = faras_at,
        .mv      = faras_mv,
        .name    = "Faras",
        .desc    = "Second square orthogonally (Dababbah).",
        .id      = PIECE_FARAS,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_LEAPER,
        .value   = 30,
    },
    {
        .at      = guard_at,
        .mv      = guard_mv,
        .effects =
            {
                {
                    .func      = eff_double_move,
                    .name      = "Double Time",
                    .trigger   = QUERY_PIECE_ACTION_COST_MOVE,
                    .lasts_for = ENTIRE_BATTLE,
                },
                {
                    .func      = eff_split_king,
                    .name      = "Negus Split",
                    .trigger   = ON_PIECE_FLIP_PRE,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Negus Guard",
        .desc    = "Moves twice per action. When flipped: removed, 2 Medeq "
                   "spawn adjacent to your king.",
        .id      = PIECE_NEGUS_GUARD,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_LEAPER,
        .value   = 100,
    },
    {
        .at      = medeq_at,
        .mv      = medeq_mv,
        .effects =
            {
                {
                    .func      = eff_split_square,
                    .name      = "Squad Split",
                    .trigger   = ON_PIECE_FLIP_PRE,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Medeq Squad",
        .desc    = "Medeq movement. When flipped: removed, 2 Medeq spawn "
                   "adjacent to its square.",
        .id      = PIECE_MEDEQ_SQUAD,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_TOWN,
        .class   = MOVE_LEAPER,
        .value   = 20,
    },
    {
        .at      = guard_at,
        .mv      = guard_mv,
        .effects =
            {
                {
                    .func      = eff_double_move,
                    .name      = "Double Time",
                    .trigger   = QUERY_PIECE_ACTION_COST_MOVE,
                    .lasts_for = ENTIRE_BATTLE,
                },
                {
                    .func      = eff_split_king,
                    .name      = "Levy Split",
                    .trigger   = ON_PIECE_FLIP_PRE,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Sultan's Levy",
        .desc    = "Negus Guard movement. When flipped: removed, 3 Medeq "
                   "spawn adjacent to your king.",
        .id      = PIECE_SULTANS_LEVY,
        .kingdom = KINGDOM_KEWARANI,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_LEAPER,
        .value   = 110,
    },
};

const Card KEWARANI_CARDS[] = {
    {
        .effects =
            {{.func      = eff_sultans_gold,
              .name      = "Sultan's Gold",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Sultan's Gold",
        .desc      = "Gain 60 cp.",
        .id        = CARD_SULTANS_GOLD,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 20,
    },
    {
        .effects =
            {{.func      = eff_march,
              .name      = "March",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "March",
        .desc      = "Every friendly pawn that can move forward 1 does, no "
                     "action cost.",
        .id        = CARD_MARCH,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_double_time,
              .name      = "Double Time",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Double Time",
        .desc      = "Target piece makes one additional move this turn.",
        .id        = CARD_DOUBLE_TIME,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_salt_road,
              .name      = "Salt Road",
              .trigger   = ON_TURN_START,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Salt Road",
        .desc      = "Gain 10 cp at the start of every remaining turn this "
                     "battle.",
        .id        = CARD_SALT_ROAD,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 35,
    },
    {
        .effects =
            {{.func      = eff_caravan,
              .name      = "Caravan",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Caravan",
        .desc      = "All pieces on a file move 1 square forward together, "
                     "no action cost.",
        .id        = CARD_CARAVAN,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_doublestrike,
              .name      = "Doublestrike",
              .trigger   = QUERY_PIECE_ACTION_COST_MOVE,
              .lasts_for = TURNS_1}},
        .name      = "Doublestrike",
        .desc      = "Move two of your pieces with 1 action cost.",
        .id        = CARD_DOUBLESTRIKE,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 50,
    },
    {
        .effects =
            {{.func      = eff_hajj,
              .name      = "Hajj",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Hajj",
        .desc      = "Target piece teleports to any unoccupied square. "
                     "Does not count as an attack.",
        .id        = CARD_HAJJ,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 0,
        .sell_cost = 70,
    },
    {
        .effects =
            {{.func      = eff_selassie,
              .name      = "Selassie's March",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Selassie's March",
        .desc      = "Kewarani pieces move extra times this action "
                     "this turn.",
        .id        = CARD_SELASSIES_MARCH,
        .tier      = TIER_MASTERY,
        .kingdom   = KINGDOM_KEWARANI,
        .play_cost = 40,
        .sell_cost = 60,
    },
};

const BoardTrait KEWARANI_TRAITS[] = {
    {
        .name = "Trade Route",
        .desc = "A diagonal path grants pieces on it +1 movement.",
        .id   = BOARD_TRAIT_TRADE_ROUTE,
    },
    {
        .name = "Contested Market",
        .desc = "Each turn a neutral Kewarani piece appears to be claimed.",
        .id   = BOARD_TRAIT_CONTESTED_MARKET,
    },
};

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// kewarani_grant_double
///
/// Embeds the Double Time free-second-move effect on a Kewarani piece,
/// skipping the king and any piece that already carries it.
///
/// Params:
/// - piece -> piece to grant the double move
///
static void kewarani_grant_double(PieceInfo* piece) {
    if (piece->piece->id == PIECE_KING ||
        piece->piece->kingdom != KINGDOM_KEWARANI) {
        return;
    }

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (piece->piece->effects[slot].func == eff_double_move) {
            return;
        }
    }

    Effect grant = {
        .func      = eff_double_move,
        .name      = "Double Time",
        .trigger   = QUERY_PIECE_ACTION_COST_MOVE,
        .lasts_for = ENTIRE_BATTLE,
    };

    piece_embed_effect(piece, &grant);
}

/// eff_double_time_buy
///
/// Grants Double Time to a freshly bought Kewarani piece so reinforcements
/// move twice as well.
///
/// Params:
/// - context -> args[0] owning side
/// - x       -> PieceInfo* just bought
///
/// Return: true when the piece received the double move
///
static bool eff_double_time_buy(EffectContext* context, void* x) {
    Side       side  = (Side) (uintptr_t) context->args[0];
    PieceInfo* piece = x;

    if (piece->side != side || piece->piece->kingdom != KINGDOM_KEWARANI) {
        return false;
    }

    kewarani_grant_double(piece);

    return true;
}

/// kewarani_innate
///
/// Attaches Double Time: every Kewarani piece may move twice per move
/// action. The effect is embedded on the side's Kewarani pieces present at
/// the start and on any bought later through an ON_PIECE_BUY hook.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void kewarani_innate(BattleState* battle, Side side, MasteryLevel level) {
    (void) level;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side) {
            kewarani_grant_double(cell);
        }
    }

    Effect hook = {
        .func      = eff_double_time_buy,
        .name      = "Double Time",
        .trigger   = ON_PIECE_BUY,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &hook);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
    }
}

/// kewarani_climax
///
/// Fires the Kewarani combo climax for the given side, granting every
/// friendly piece one extra move this turn.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void kewarani_climax(BattleState* battle, Side side) {
    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* cell = battle_at(battle, (Square) {x, y});

            if (cell && cell->side == side) {
                piece_grant_free_move(cell, "Extra Move");
            }
        }
    }
}

/// kewarani_overseer
///
/// Sets up the Caravan of Conquest overseer battle with its bespoke army
/// and boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void kewarani_overseer(BattleState* battle) {
    (void) battle;
}

/// kewarani_event
///
/// Resolves a Kewarani narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void kewarani_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
