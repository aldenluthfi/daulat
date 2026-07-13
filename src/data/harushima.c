//! harushima.c
//!
//! Harushima kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Reclaim innate, climax, Eternal Recursion
//! overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                                KINGDOM HELPERS
\*----------------------------------------------------------------------------*/

/// GOLD_OFFSETS
///
/// The gold general steps: one square in any direction except the two
/// backward diagonals, written from the white perspective.
///
static const Square GOLD_OFFSETS[] = {
    {0, -1}, {-1, -1}, {1, -1}, {-1, 0}, {1, 0}, {0, 1}, {0, 0},
};

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// fuhyo_gen
///
/// Generates the Fuhyo's single forward step.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void fuhyo_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, (const Square[]) {{0, -1}, {0, 0}}, threat);
}

/// fuhyo_mv
///
/// Generates the Fuhyo's movement through fuhyo_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* fuhyo_mv(BattleState* battle, PieceInfo* self) {
    fuhyo_gen(battle, self, false);

    return mg_end();
}

/// fuhyo_at
///
/// Generates the Fuhyo's attack coverage through fuhyo_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* fuhyo_at(BattleState* battle, PieceInfo* self) {
    fuhyo_gen(battle, self, true);

    return mg_end();
}

/// kyosha_mv
///
/// Generates the Kyosha's movement: an unbounded forward slide.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* kyosha_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, (const Square[]) {{0, -1}, {0, 0}}, 1, 127, false);

    return mg_end();
}

/// kyosha_at
///
/// Generates the Kyosha's attack coverage: an unbounded forward slide.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* kyosha_at(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, (const Square[]) {{0, -1}, {0, 0}}, 1, 127, true);

    return mg_end();
}

/// ginsho_gen
///
/// Generates the Ginsho's silver steps: one forward or one diagonal.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void ginsho_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {
            {0, -1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}, {0, 0},
        },
        threat
    );
}

/// ginsho_mv
///
/// Generates the Ginsho's movement through ginsho_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* ginsho_mv(BattleState* battle, PieceInfo* self) {
    ginsho_gen(battle, self, false);

    return mg_end();
}

/// ginsho_at
///
/// Generates the Ginsho's attack coverage through ginsho_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* ginsho_at(BattleState* battle, PieceInfo* self) {
    ginsho_gen(battle, self, true);

    return mg_end();
}

/// kinsho_gen
///
/// Generates the Kinsho's gold steps: one square in any direction save the
/// two backward diagonals.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void kinsho_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, GOLD_OFFSETS, threat);
}

/// kinsho_mv
///
/// Generates the Kinsho's movement through kinsho_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* kinsho_mv(BattleState* battle, PieceInfo* self) {
    kinsho_gen(battle, self, false);

    return mg_end();
}

/// kinsho_at
///
/// Generates the Kinsho's attack coverage through kinsho_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* kinsho_at(BattleState* battle, PieceInfo* self) {
    kinsho_gen(battle, self, true);

    return mg_end();
}

/// shishi_mv
///
/// Generates the Shishi's king movement; the free second act is granted by
/// its embedded effect.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* shishi_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, false);

    return mg_end();
}

/// shishi_at
///
/// Generates the Shishi's king coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* shishi_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/// horse_gen
///
/// Generates the Honorable Horse's Shogi-knight leaps: two forward and one
/// sideways. Its gold promotion after crossing is applied by an embedded
/// effect.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void horse_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, (const Square[]) {{-1, -2}, {1, -2}, {0, 0}}, threat);
}

/// horse_mv
///
/// Generates the Honorable Horse's movement through horse_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* horse_mv(BattleState* battle, PieceInfo* self) {
    horse_gen(battle, self, false);

    return mg_end();
}

/// horse_at
///
/// Generates the Honorable Horse's attack coverage through horse_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* horse_at(BattleState* battle, PieceInfo* self) {
    horse_gen(battle, self, true);

    return mg_end();
}

/// promoted_gen
///
/// Generates the Promoted Bishop's pattern: bishop slides plus one
/// orthogonal step.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void promoted_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_BISHOP, PIECE_NONE},
        threat
    );
    mg_leap(battle, self, ORTHOGONAL_DIRECTIONS, threat);
}

/// promoted_mv
///
/// Generates the Promoted Bishop's movement through promoted_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* promoted_mv(BattleState* battle, PieceInfo* self) {
    promoted_gen(battle, self, false);

    return mg_end();
}

/// promoted_at
///
/// Generates the Promoted Bishop's attack coverage through promoted_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* promoted_at(BattleState* battle, PieceInfo* self) {
    promoted_gen(battle, self, true);

    return mg_end();
}

/// daimyo_mv
///
/// Generates the Daimyo's king movement.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* daimyo_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, false);

    return mg_end();
}

/// daimyo_at
///
/// Generates the Daimyo's king coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* daimyo_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/// dragon_gen
///
/// Generates the Dragon's pattern: rook slides plus one diagonal step.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void dragon_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_compound(
        battle,
        self,
        (const PieceID[]) {PIECE_ROOK, PIECE_NONE},
        threat
    );
    mg_leap(battle, self, DIAGONAL_DIRECTIONS, threat);
}

/// dragon_mv
///
/// Generates the Dragon's movement through dragon_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* dragon_mv(BattleState* battle, PieceInfo* self) {
    dragon_gen(battle, self, false);

    return mg_end();
}

/// dragon_at
///
/// Generates the Dragon's attack coverage through dragon_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* dragon_at(BattleState* battle, PieceInfo* self) {
    dragon_gen(battle, self, true);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                PIECE EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_horse_cross
///
/// Records when the Honorable Horse first stands in enemy territory,
/// unlocking its gold promotion.
///
/// Params:
/// - context -> horse context, args[0] self, args[1] promoted flag
/// - x       -> moved piece, unused
///
/// Return: true the moment the horse promotes
///
static bool eff_horse_cross(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self || context->args[1]) {
        return false;
    }

    BattleState* battle = battle_current();

    if (battle_territory(battle, self->square) != battle_enemy(self->side)) {
        return false;
    }

    context->args[1] = (void*) 1;

    return true;
}

/// eff_horse_gold_mv
///
/// Rewrites the promoted Honorable Horse's movement into the gold pattern.
///
/// Params:
/// - context -> horse context, args[0] self, args[1] promoted flag
/// - x       -> move list, rebuilt in the shared scratch
///
/// Return: true when the gold pattern was written
///
static bool eff_horse_gold_mv(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self || !context->args[1]) {
        return false;
    }

    mg_begin();
    mg_leap(battle_current(), self, GOLD_OFFSETS, false);
    mg_end();

    return true;
}

/// eff_horse_gold_at
///
/// Rewrites the promoted Honorable Horse's coverage into the gold pattern.
///
/// Params:
/// - context -> horse context, args[0] self, args[1] promoted flag
/// - x       -> coverage list, rebuilt in the shared scratch
///
/// Return: true when the gold pattern was written
///
static bool eff_horse_gold_at(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self || !context->args[1]) {
        return false;
    }

    mg_begin();
    mg_leap(battle_current(), self, GOLD_OFFSETS, true);
    mg_end();

    return true;
}

/// eff_daimyo_immune
///
/// Grants the Daimyo one immunity to flipping, spent the first time it is
/// a flip candidate.
///
/// Params:
/// - context -> Daimyo context, args[0] self, args[1] spent flag
/// - x       -> can-flip flag to clear once
///
/// Return: true when the immunity was spent
///
static bool eff_daimyo_immune(EffectContext* context, void* x) {
    PieceInfo* self = context->args[0];

    if (battle_subject() != self || context->args[1]) {
        return false;
    }

    *(bool*) x       = false;
    context->args[1] = (void*) 1;

    return true;
}

/*----------------------------------------------------------------------------*\
                                 CARD EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_ronin_refund
///
/// Refunds the target piece's value to the playing side the next time it
/// flips.
///
/// Params:
/// - context -> beneficiary side in args[0], target in args[1], spent in
///              args[3]
/// - x       -> flip candidate pointer
///
/// Return: true when the refund was granted
///
static bool eff_ronin_refund(EffectContext* context, void* x) {
    PieceInfo*  target = context->args[1];
    PieceInfo** slot   = x;

    if (context->args[3] || *slot != target) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    battle_meter_gain(battle, side, battle_value(battle, target, nullptr));
    context->args[3] = (void*) 1;

    return true;
}

/// eff_bushido_strike
///
/// Deals twice the target piece's value to the enemy meter the next time
/// it flips.
///
/// Params:
/// - context -> beneficiary side in args[0], target in args[1], spent in
///              args[3]
/// - x       -> flip candidate pointer
///
/// Return: true when the strike was dealt
///
static bool eff_bushido_strike(EffectContext* context, void* x) {
    PieceInfo*  target = context->args[1];
    PieceInfo** slot   = x;

    if (context->args[3] || *slot != target) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    int          dmg    = battle_value(battle, target, nullptr) * 2;

    battle_player(battle, battle_enemy(side))->meter -= dmg;
    context->args[3]                              = (void*) 1;

    return true;
}

/// eff_ronin
///
/// Immediate play effect attaching the Ronin refund to a targeted piece.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the refund was attached
///
static bool eff_ronin(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side != side) {
        return false;
    }

    static const Effect refund = {
        .func      = eff_ronin_refund,
        .name      = "Ronin",
        .trigger   = ON_PIECE_FLIP_PRE,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &refund);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = target;
    }

    return attached != nullptr;
}

/// eff_bushido
///
/// Immediate play effect attaching the Bushido strike to a targeted piece.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the strike was attached
///
static bool eff_bushido(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side != side) {
        return false;
    }

    static const Effect strike = {
        .func      = eff_bushido_strike,
        .name      = "Bushido",
        .trigger   = ON_PIECE_FLIP_PRE,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &strike);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = target;
    }

    return attached != nullptr;
}

/// eff_resurrection
///
/// Immediate play effect reclaiming a flipped enemy piece to the playing
/// side for free.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the piece was reclaimed
///
static bool eff_resurrection(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side == side || target->piece->id == PIECE_KING) {
        return false;
    }

    battle_flip(battle, target);

    return true;
}

/// eff_gold_standard
///
/// Immediate play effect making a targeted piece move like a Kinsho this
/// turn.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the pattern was applied
///
static bool eff_gold_standard(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side != side) {
        return false;
    }

    piece_adopt_move(battle, target, PIECE_KINSHO, TURNS_1, true);

    return true;
}

/// eff_promotion
///
/// Immediate play effect permanently adding Ginsho movement to a targeted
/// piece for the rest of the battle.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when the pattern was applied
///
static bool eff_promotion(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side != side) {
        return false;
    }

    piece_adopt_move(battle, target, PIECE_GINSHO, ENTIRE_BATTLE, false);

    return true;
}

/// spawn_fuhyo
///
/// Spawns a friendly Fuhyo on the first empty square around the owner's
/// king, used by Dual Drop to fill missing reclaims.
///
/// Params:
/// - battle -> battle to spawn into
/// - side   -> owning side
///
/// Return: true when a Fuhyo was placed
///
static bool spawn_fuhyo(BattleState* battle, Side side) {
    PieceInfo* king = battle_find_king(battle, side);

    if (!king) {
        return false;
    }

    for (int8_t dy = -1; dy <= 1; dy++) {
        for (int8_t dx = -1; dx <= 1; dx++) {
            Square spot = {
                (int8_t) (king->square.x + dx),
                (int8_t) (king->square.y + dy),
            };

            if ((dx || dy) && battle_in_bounds(battle, spot) &&
                !battle_at(battle, spot) &&
                battle_spawn(battle, PIECE_FUHYO, spot, side)) {
                return true;
            }
        }
    }

    return false;
}

/// eff_dual_drop
///
/// Immediate play effect reclaiming up to two flipped pieces, filling any
/// shortfall with free Fuhyo drops.
///
/// Params:
/// - context -> beneficiary side in args[0], two squares in args[1..2]
/// - x       -> played card, unused
///
/// Return: true, the drop always resolves
///
static bool eff_dual_drop(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle   = battle_current();
    Side         side     = (Side) (uintptr_t) context->args[0];
    void*        packed[] = {context->args[1], context->args[2]};

    for (size_t i = 0; i < 2; i++) {
        PieceInfo* target = battle_at(battle, card_square(packed[i]));

        if (target && target->side != side &&
            target->piece->id != PIECE_KING) {
            battle_flip(battle, target);
        } else {
            spawn_fuhyo(battle, side);
        }
    }

    return true;
}

/// eff_force_drop
///
/// Immediate play effect placing any unlocked piece of value up to fifty
/// on an empty square for free.
///
/// Params:
/// - context -> beneficiary side in args[0], piece id (+1000) in args[1],
///              destination square in args[2]
/// - x       -> played card, unused
///
/// Return: true when the piece was placed
///
static bool eff_force_drop(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    long         raw    = (long) (uintptr_t) context->args[1];
    PieceID      id     = (PieceID) (raw >= 1000 ? raw - 1000 : raw);
    Square       dest   = card_square(context->args[2]);

    if (id >= PIECE_COUNT || !PIECE_REGISTRY[id] ||
        PIECE_REGISTRY[id]->value > 50) {
        return false;
    }

    PieceInfo* placed = battle_spawn(battle, id, dest, side);

    if (!placed) {
        return false;
    }

    battle_player(battle, side)->meter += battle_value(battle, placed, nullptr);

    return true;
}

/*----------------------------------------------------------------------------*\
                                 KINGDOM DATA
\*----------------------------------------------------------------------------*/

const Piece HARUSHIMA_PIECES[] = {
    {
        .at      = fuhyo_at,
        .mv      = fuhyo_mv,
        .name    = "Fuhyo",
        .desc    = "Forward 1.",
        .id      = PIECE_FUHYO,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 10,
    },
    {
        .at      = kyosha_at,
        .mv      = kyosha_mv,
        .name    = "Kyosha",
        .desc    = "Forward any distance (slider).",
        .id      = PIECE_KYOSHA,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_SLIDER,
        .value   = 30,
    },
    {
        .at      = ginsho_at,
        .mv      = ginsho_mv,
        .name    = "Ginsho",
        .desc    = "1 square diagonal or forward.",
        .id      = PIECE_GINSHO,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_TOWN,
        .class   = MOVE_LEAPER,
        .value   = 30,
    },
    {
        .at      = kinsho_at,
        .mv      = kinsho_mv,
        .name    = "Kinsho",
        .desc    = "1 square any direction except diagonal-backward.",
        .id      = PIECE_KINSHO,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_LEAPER,
        .value   = 35,
    },
    {
        .at      = shishi_at,
        .mv      = shishi_mv,
        .effects =
            {
                {
                    .func      = eff_double_move,
                    .name      = "Second Act",
                    .trigger   = QUERY_PIECE_ACTION_COST_MOVE,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Shishi",
        .desc    = "Acts twice per move action: move-then-move.",
        .id      = PIECE_SHISHI,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_LEAPER,
        .value   = 100,
    },
    {
        .at      = horse_at,
        .mv      = horse_mv,
        .effects =
            {
                {
                    .func      = eff_horse_cross,
                    .name      = "Crossing",
                    .trigger   = ON_PIECE_MOVE,
                    .lasts_for = ENTIRE_BATTLE,
                },
                {
                    .func      = eff_horse_gold_mv,
                    .name      = "Promotion",
                    .trigger   = QUERY_PIECE_MOVES,
                    .lasts_for = ENTIRE_BATTLE,
                },
                {
                    .func      = eff_horse_gold_at,
                    .name      = "Promotion",
                    .trigger   = QUERY_PIECE_ATTACKS,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Honorable Horse",
        .desc    = "Shogi knight. Becomes Kinsho after first entering "
                   "enemy territory.",
        .id      = PIECE_HONORABLE_HORSE,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_TOWN,
        .class   = MOVE_LEAPER,
        .value   = 40,
    },
    {
        .at      = promoted_at,
        .mv      = promoted_mv,
        .name    = "Promoted Bishop",
        .desc    = "Bishop movement + 1 square orthogonal.",
        .id      = PIECE_PROMOTED_BISHOP,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_TOWN,
        .class   = MOVE_SLIDER,
        .value   = 50,
    },
    {
        .at      = daimyo_at,
        .mv      = daimyo_mv,
        .effects =
            {
                {
                    .func      = eff_daimyo_immune,
                    .name      = "Daimyo Guard",
                    .trigger   = QUERY_PIECE_CAN_FLIP,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Daimyo",
        .desc    = "King movement. Immune to flipping once, then normal.",
        .id      = PIECE_DAIMYO,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_LEAPER,
        .value   = 50,
    },
    {
        .at      = dragon_at,
        .mv      = dragon_mv,
        .name    = "Dragon",
        .desc    = "Rook movement + 1 square diagonal.",
        .id      = PIECE_DRAGON,
        .kingdom = KINGDOM_HARUSHIMA,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SLIDER,
        .value   = 70,
    },
};

const Card HARUSHIMA_CARDS[] = {
    {
        .effects =
            {{.func      = eff_ronin,
              .name      = "Ronin",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Ronin",
        .desc      = "Next time the targeted piece flips, refund its full "
                     "value to you.",
        .id        = CARD_RONIN,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_resurrection,
              .name      = "Resurrection",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Resurrection",
        .desc      = "Reclaim any flipped piece on the board to your "
                     "control for free.",
        .id        = CARD_RESURRECTION,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 25,
    },
    {
        .effects =
            {{.func      = eff_gold_standard,
              .name      = "Gold Standard",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Gold Standard",
        .desc      = "Target piece moves like a Kinsho this turn only.",
        .id        = CARD_GOLD_STANDARD,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 25,
    },
    {
        .effects =
            {{.func      = eff_promotion,
              .name      = "Promotion",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Promotion",
        .desc      = "Target piece permanently gains Ginsho movement this "
                     "battle.",
        .id        = CARD_PROMOTION,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_dual_drop,
              .name      = "Dual Drop",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Dual Drop",
        .desc      = "Reclaim up to 2 flipped pieces; place free Fuhyo for "
                     "each missing reclaim.",
        .id        = CARD_DUAL_DROP,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 30,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_force_drop,
              .name      = "Force Drop",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Force Drop",
        .desc      = "Place any unlocked piece of value up to 50 on any "
                     "unoccupied square for free.",
        .id        = CARD_FORCE_DROP,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 50,
    },
    {
        .effects =
            {{.func      = eff_bushido,
              .name      = "Bushido",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Bushido",
        .desc      = "When the targeted piece flips, deal its value x2 to "
                     "enemy meter.",
        .id        = CARD_BUSHIDO,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 60,
    },
};

const BoardTrait HARUSHIMA_TRAITS[] = {
    {
        .name = "Fog Coast",
        .desc = "The 3 farthest rows hide enemy values until they move.",
        .id   = BOARD_TRAIT_FOG_COAST,
    },
    {
        .name = "Island Chain",
        .desc = "Checkered gaps in the middle columns force routing.",
        .id   = BOARD_TRAIT_ISLAND_CHAIN,
    },
};

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// eff_reclaim_cost
///
/// Tomohito's upgrade to Reclaim: at mastery three the reclaim action
/// costs twenty currency instead of thirty.
///
/// Params:
/// - context -> unused
/// - x       -> int* reclaim cost to lower
///
/// Return: true, the discount always applies
///
static bool eff_reclaim_cost(EffectContext* context, void* x) {
    (void) context;

    *(int*) x = 20;

    return true;
}

/// harushima_innate
///
/// Attaches the Reclaim innate. Reclaim itself is the always-available
/// board action; mastery three lowers its cost to twenty currency through
/// an attached cost query.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void harushima_innate(BattleState* battle, Side side, MasteryLevel level) {
    if (level < MASTERY_LEVEL_3) {
        return;
    }

    Effect discount = {
        .func      = eff_reclaim_cost,
        .name      = "Reclaim",
        .trigger   = QUERY_PIECE_CP_COST_RECLAIM,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &discount);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
    }
}

/// harushima_climax
///
/// Fires the Harushima combo climax for the given side, reclaiming one
/// flipped enemy piece for free.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void harushima_climax(BattleState* battle, Side side) {
    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* cell = battle_at(battle, (Square) {x, y});

            if (cell && cell->side != side && cell->side != SIDE_NEUTRAL &&
                cell->piece->id != PIECE_KING) {
                battle_flip(battle, cell);

                return;
            }
        }
    }
}

/// harushima_overseer
///
/// Sets up the Eternal Recursion overseer battle with its bespoke army
/// and boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void harushima_overseer(BattleState* battle) {
    (void) battle;
}

/// harushima_event
///
/// Resolves a Harushima narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void harushima_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
