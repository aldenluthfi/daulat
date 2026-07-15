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
    {0, -1},
    {-1, -1},
    {1, -1},
    {-1, 0},
    {1, 0},
    {0, 1},
    {0, 0},
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
    mg_leap(battle, self, (const Square[]){{0, -1}, {0, 0}}, threat);
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
    mg_slide(battle, self, (const Square[]){{0, -1}, {0, 0}}, 1, 127, false);

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
    mg_slide(battle, self, (const Square[]){{0, -1}, {0, 0}}, 1, 127, true);

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
        (const Square[]){
            {0, -1},
            {-1, -1},
            {1, -1},
            {-1, 1},
            {1, 1},
            {0, 0},
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
    mg_leap(battle, self, (const Square[]){{-1, -2}, {1, -2}, {0, 0}}, threat);
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
        (const PieceID[]){PIECE_BISHOP, PIECE_NONE},
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
        (const PieceID[]){PIECE_ROOK, PIECE_NONE},
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

    battle_damage(battle, battle_enemy(side), dmg);
    context->args[3] = (void*) 1;

    return true;
}

/// eff_ronin_targets
///
/// Ronin targeting: advertises every friendly non-king piece, the pieces
/// that can flip.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_ronin_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side == side && p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_ronin_pick
///
/// Ronin resolution: attaches the Ronin refund to the chosen friendly
/// piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the refund was attached
///
static bool eff_ronin_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side) {
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
        attached->context->args[1] = piece;
    }

    return attached != nullptr;
}

/// eff_bushido_targets
///
/// Bushido targeting: advertises every friendly piece as a square target
/// for the strike attachment.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_bushido_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side == side && p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_bushido_pick
///
/// Bushido resolution: attaches the Bushido strike to the chosen friendly
/// piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the strike was attached
///
static bool eff_bushido_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side) {
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
        attached->context->args[1] = piece;
    }

    return attached != nullptr;
}

/// eff_resurrection_targets
///
/// Resurrection targeting: advertises every flipped enemy piece, the ones
/// the GDD lets you reclaim.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_resurrection_targets(EffectContext* context, void* x) {
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    card_targets_piece(x, battle, ^bool(const PieceInfo* p) {
      return p->side != side && p->side != SIDE_NEUTRAL &&
             battle_piece_flips(battle, (PieceInfo*) p) > 0;
    });

    return card_target_count(x) > 0;
}

/// eff_resurrection_pick
///
/// Resurrection resolution: reclaims the chosen flipped enemy piece to the
/// playing side for free.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the piece was reclaimed
///
static bool eff_resurrection_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side == side || piece->side == SIDE_NEUTRAL ||
        battle_piece_flips(battle, piece) == 0) {
        return false;
    }

    battle_flip(battle, piece);

    return true;
}

/// eff_gold_standard_targets
///
/// Gold Standard targeting: advertises every friendly piece as a square
/// target.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_gold_standard_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side == side;
    });

    return card_target_count(x) > 0;
}

/// eff_gold_standard_pick
///
/// Gold Standard resolution: makes the chosen friendly piece move like a
/// Kinsho this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the pattern was applied
///
static bool eff_gold_standard_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side) {
        return false;
    }

    piece_adopt_move(battle, piece, PIECE_KINSHO, TURNS_1, true);

    return true;
}

/// eff_promotion_targets
///
/// Promotion targeting: advertises every friendly piece as a square
/// target.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_promotion_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side == side;
    });

    return card_target_count(x) > 0;
}

/// eff_promotion_pick
///
/// Promotion resolution: permanently adds Ginsho movement to the chosen
/// piece for the rest of the battle.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the pattern was applied
///
static bool eff_promotion_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side) {
        return false;
    }

    piece_adopt_move(battle, piece, PIECE_GINSHO, ENTIRE_BATTLE, false);

    return true;
}

/// eff_dual_drop_first
///
/// Dual Drop first pick, dynamic: offers each flipped enemy piece to
/// reclaim, or, when none are on the board, empty squares to drop a free
/// Fuhyo onto, so the card always has a legal target.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true, the board always offers a piece or an empty square
///
static bool eff_dual_drop_first(EffectContext* context, void* x) {
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    card_targets_piece(x, battle, ^bool(const PieceInfo* p) {
      return p->side != side && p->side != SIDE_NEUTRAL &&
             battle_piece_flips(battle, (PieceInfo*) p) > 0;
    });

    if (card_target_count(x) == 0) {
        card_targets_square(x, battle, ^bool(Square sq) {
          return !battle_at(battle, sq);
        });
    }

    return card_target_count(x) > 0;
}

/// eff_dual_drop_second
///
/// Dual Drop second pick, dynamic: offers each remaining flipped enemy
/// piece other than the first reclaim, or empty squares for the Fuhyo when
/// none remain.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true, the second slot is always fillable
///
static bool eff_dual_drop_second(EffectContext* context, void* x) {
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();
    CardTarget   first  = battle_pending_picks()[0];
    Square       fsq    = card_target_at(first.value);

    card_targets_piece(x, battle, ^bool(const PieceInfo* p) {
      if (p->side == side || p->side == SIDE_NEUTRAL ||
          battle_piece_flips(battle, (PieceInfo*) p) == 0) {
          return false;
      }

      return first.kind != TARGET_PIECE ||
             !(p->square.x == fsq.x && p->square.y == fsq.y);
    });

    if (card_target_count(x) == 0) {
        card_targets_square(x, battle, ^bool(Square sq) {
          return !battle_at(battle, sq);
        });
    }

    return card_target_count(x) > 0;
}

/// eff_dual_drop_pick
///
/// Dual Drop resolution: for each of the two picks, flips a chosen flipped
/// enemy piece back to the playing side, or drops a free Fuhyo on a chosen
/// empty square.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* pick list, up to two pieces or squares
///
/// Return: true, the drop always resolves
///
static bool eff_dual_drop_pick(EffectContext* context, void* x) {
    CardTarget*  picks  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    for (size_t i = 0; i < 2; i++) {
        if (picks[i].kind == TARGET_PIECE) {
            PieceInfo* target =
                battle_at(battle, card_target_at(picks[i].value));

            if (target && target->side != side &&
                target->side != SIDE_NEUTRAL &&
                battle_piece_flips(battle, target) > 0) {
                battle_flip(battle, target);
            }
        } else if (picks[i].kind == TARGET_SQUARE) {
            battle_spawn(
                battle,
                PIECE_FUHYO,
                card_target_at(picks[i].value),
                side
            );
        }
    }

    return true;
}

/// eff_force_drop_type_targets
///
/// Advertises every unlocked identity of value up to fifty as the piece to
/// place, the first step of Force Drop.
///
/// Params:
/// - context -> beneficiary side in args[0], unused
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one identity was offered
///
static bool eff_force_drop_type_targets(EffectContext* context, void* x) {
    (void) context;

    card_targets_piece_type(x, ^bool(const Piece* pc) {
      return battle_piece_unlocked(pc->id) && pc->value <= 50;
    });

    return card_target_count(x) > 0;
}

/// eff_force_drop_dest_targets
///
/// Advertises every unoccupied square as the drop destination, the second
/// step of Force Drop.
///
/// Params:
/// - context -> beneficiary side in args[0], unused
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one square was offered
///
static bool eff_force_drop_dest_targets(EffectContext* context, void* x) {
    (void) context;

    BattleState* battle = battle_current();

    card_targets_square(x, battle, ^bool(Square square) {
      return !battle_at(battle, square);
    });

    return card_target_count(x) > 0;
}

/// eff_force_drop_pick
///
/// Force Drop resolution: places the chosen piece on the chosen empty
/// square for free, per the GDD nothing more (no meter gain).
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* pick list, piece type then destination
///
/// Return: true when the piece was placed
///
static bool eff_force_drop_pick(EffectContext* context, void* x) {
    CardTarget*  picks  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (picks[0].kind != TARGET_PIECE_TYPE || picks[1].kind != TARGET_SQUARE) {
        return false;
    }

    PieceID id   = (PieceID) picks[0].value;
    Square  dest = card_target_at(picks[1].value);

    if (id >= PIECE_COUNT || !PIECE_REGISTRY[id] ||
        PIECE_REGISTRY[id]->value > 50) {
        return false;
    }

    return battle_spawn(battle, id, dest, side) != nullptr;
}

/// eff_tomohito
///
/// Tomohito's Patience: reclaim up to three enemy pieces this turn at
/// fifteen currency each, flipping them to the playing side while it can
/// afford the toll.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when at least one piece was reclaimed
///
static bool eff_tomohito(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle    = battle_current();
    Side         side      = (Side) (uintptr_t) context->args[0];
    PlayerState* player    = battle_player(battle, side);
    int          reclaimed = 0;

    for (size_t index = 0; index < MAX_BOARD_SIZE && reclaimed < 3; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (!cell || cell == &VOID_CELL || cell->side == side ||
            cell->side == SIDE_NEUTRAL || cell->piece->id == PIECE_KING) {
            continue;
        }

        if (player->cp < 15) {
            break;
        }

        player->cp -= 15;

        battle_flip(battle, cell);

        reclaimed++;
    }

    return reclaimed > 0;
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
        .at = shishi_at,
        .mv = shishi_mv,
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
        .at = horse_at,
        .mv = horse_mv,
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
        .at = daimyo_at,
        .mv = daimyo_mv,
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
            {{.func      = eff_ronin_targets,
              .name      = "Ronin",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_ronin_pick,
              .name      = "Ronin",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
            {{.func      = eff_resurrection_targets,
              .name      = "Resurrection",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_resurrection_pick,
              .name      = "Resurrection",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
            {{.func      = eff_gold_standard_targets,
              .name      = "Gold Standard",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_gold_standard_pick,
              .name      = "Gold Standard",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
            {{.func      = eff_promotion_targets,
              .name      = "Promotion",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_promotion_pick,
              .name      = "Promotion",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
            {{.func      = eff_dual_drop_first,
              .name      = "Dual Drop",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_dual_drop_second,
              .name      = "Dual Drop",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_dual_drop_pick,
              .name      = "Dual Drop",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
            {{.func      = eff_force_drop_type_targets,
              .name      = "Force Drop",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_force_drop_dest_targets,
              .name      = "Force Drop",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_force_drop_pick,
              .name      = "Force Drop",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
            {{.func      = eff_bushido_targets,
              .name      = "Bushido",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_bushido_pick,
              .name      = "Bushido",
              .trigger   = ON_CARD_TARGET_SELECTED,
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
    {
        .effects =
            {{.func      = eff_tomohito,
              .name      = "Tomohito's Patience",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Tomohito's Patience",
        .desc      = "Reclaim up to 3 flipped pieces this turn at 15 cp "
                     "each.",
        .id        = CARD_TOMOHITOS_PATIENCE,
        .tier      = TIER_MASTERY,
        .kingdom   = KINGDOM_HARUSHIMA,
        .play_cost = 0,
        .sell_cost = 60,
    },
};

/// eff_island_chain
///
/// Island Chain: a tenth of the board's squares are voided at build.
///
/// Params:
/// - context -> unused
/// - x       -> unused built board
///
/// Return: true, the scatter always applies
///
static bool eff_island_chain(EffectContext* context, void* x) {
    (void) context;
    (void) x;

    battle_scatter_voids(battle_current(), 10);

    return true;
}

/// eff_fog_coast
///
/// Fog Coast: an enemy piece on one of the three rows farthest from the
/// human is hidden from the board until it has moved. The human's home row
/// is the bottom when white and the top when black, so the far rows sit on
/// the enemy's edge.
///
/// Params:
/// - context -> args[0] human side
/// - x       -> Board* whose visible flags are cleared
///
/// Return: true when at least one enemy piece was hidden
///
static bool eff_fog_coast(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    Board*       board  = x;

    bool         hid    = false;

    for (int row = 0; row < 3; row++) {
        int8_t y = side == SIDE_WHITE
                       ? (int8_t) row
                       : (int8_t) (battle->board.height - 1 - row);

        for (int8_t x0 = 0; x0 < battle->board.width; x0++) {
            size_t     index = (size_t) (y * 20 + x0);
            PieceInfo* piece = battle_at(battle, (Square){x0, y});

            if (!piece || piece == &VOID_CELL || piece->side == side ||
                piece->side == SIDE_NEUTRAL) {
                continue;
            }

            if (battle_piece_moves(battle, piece) == 0) {
                board->visible[index] = false;
                hid                   = true;
            }
        }
    }

    return hid;
}

const BoardTrait HARUSHIMA_TRAITS[] = {
    {
        .name    = "Fog Coast",
        .desc    = "The 3 farthest rows hide enemy pieces until they move.",
        .id      = BOARD_TRAIT_FOG_COAST,
        .effects = {{
            .func      = eff_fog_coast,
            .name      = "Fog Coast",
            .trigger   = QUERY_BOARD_STATE,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
    {
        .name    = "Island Chain",
        .desc    = "Checkered gaps in the middle columns force routing.",
        .id      = BOARD_TRAIT_ISLAND_CHAIN,
        .effects = {{
            .func      = eff_island_chain,
            .name      = "Island Chain",
            .trigger   = ON_BOARD_BUILD,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
};

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// eff_reclaim_cost
///
/// Tomohito's upgrade to Reclaim: at mastery three the reclaim action
/// costs twenty currency instead of thirty. Below mastery three the innate
/// is present but the discount self-filters away.
///
/// Params:
/// - context -> args[1] mastery level of the innate
/// - x       -> int* reclaim cost to lower
///
/// Return: true when the mastery-three discount applied
///
static bool eff_reclaim_cost(EffectContext* context, void* x) {
    if ((MasteryLevel) (uintptr_t) context->args[1] < MASTERY_LEVEL_3) {
        return false;
    }

    *(int*) x = 20;

    return true;
}

/// eff_harushima_combo
///
/// Harushima combo climax: on the kingdom's third same-kingdom card play,
/// reclaims one enemy piece for the acting side for free.
///
/// Params:
/// - context -> args[0] acting side
/// - x       -> KingdomID* of the climaxing kingdom
///
/// Return: true when an enemy piece was reclaimed
///
static bool eff_harushima_combo(EffectContext* context, void* x) {
    if (*(KingdomID*) x != KINGDOM_HARUSHIMA) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x2 = 0; x2 < battle->board.width; x2++) {
            PieceInfo* cell = battle_at(battle, (Square){x2, y});

            if (cell && cell->side != side && cell->side != SIDE_NEUTRAL &&
                cell->piece->id != PIECE_KING) {
                battle_flip(battle, cell);

                return true;
            }
        }
    }

    return false;
}

/// HARUSHIMA_INNATE
///
/// Reclaim: the always-available board action, discounted to twenty
/// currency at mastery three through an attached cost query.
///
const KingdomPower HARUSHIMA_INNATE = {
    .effects = {{
        .func      = eff_reclaim_cost,
        .name      = "Reclaim",
        .trigger   = QUERY_PIECE_CP_COST_RECLAIM,
        .lasts_for = ENTIRE_BATTLE,
    }},
    .name = "Reclaim",
    .id   = KINGDOM_HARUSHIMA,
};

/// HARUSHIMA_CLIMAX
///
/// Reclaims one enemy piece for free on the combo climax.
///
const KingdomPower HARUSHIMA_CLIMAX = {
    .effects = {{
        .func      = eff_harushima_combo,
        .name      = "Harushima Climax",
        .trigger   = ON_COMBO_CLIMAX,
        .lasts_for = ENTIRE_BATTLE,
    }},
    .name = "Harushima Climax",
    .id   = KINGDOM_HARUSHIMA,
};

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

