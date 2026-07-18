//! universal.c
//!
//! Kingdom-agnostic game data. Holds the King, the universal and mastery
//! card pools, the modifier and chain registries, the aggregating
//! registry pointer tables, the event string tables, the kingdom
//! dispatch tables, and the Vorath battle setup.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// king_mv
///
/// Generates the King's movement: one square in any direction, the eight
/// unit vectors of ALL_DIRECTIONS used as leap offsets.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* king_mv(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, false);

    return mg_end();
}

/// king_at
///
/// Generates the King's attack coverage: one square in any direction,
/// the eight unit vectors of ALL_DIRECTIONS used as leap offsets.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* king_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                 CARD EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_revitalize
///
/// Immediate play effect restoring fifty meter to the playing side.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true, the restore always applies
///
static bool eff_revitalize(EffectContext* context, void* x) {
    (void) x;

    battle_meter_gain(
        battle_current(),
        (Side) (uintptr_t) context->args[0],
        50
    );

    return true;
}

/// eff_hostage
///
/// One-shot observer granting twenty bonus meter the next time a piece
/// flips to the playing side.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> flipped piece
///
/// Return: true when the bonus was granted
///
static bool eff_hostage(EffectContext* context, void* x) {
    Side       side = (Side) (uintptr_t) context->args[0];
    PieceInfo* flip = x;

    if (context->args[3] || !flip || flip->side != side) {
        return false;
    }

    battle_meter_gain(battle_current(), side, 20);
    context->args[3] = (void*) 1;

    return true;
}

/// eff_last_stand
///
/// Forbids the playing side's pieces from flipping this turn, routing the
/// cascade into its all-immune branch that clamps the meter without a
/// flip.
///
/// Params:
/// - context -> unused
/// - x       -> can-flip flag to clear
///
/// Return: true, the veto always applies
///
static bool eff_last_stand(EffectContext* context, void* x) {
    (void) context;

    *(bool*) x = false;

    return true;
}

/// eff_sacrifice_targets
///
/// Sacrifice targeting: advertises every friendly non-king piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_sacrifice_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side == side && p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_sacrifice_pick
///
/// Sacrifice resolution: removes the chosen friendly piece and gains twice
/// its value as meter.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when a piece was sacrificed
///
static bool eff_sacrifice_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side || piece->piece->id == PIECE_KING) {
        return false;
    }

    int gain = battle_value(battle, piece, nullptr) * 2;

    battle_remove(battle, piece);
    battle_meter_gain(battle, side, gain);

    return true;
}

/// eff_reforge_cost
///
/// The purchase-cost half of Reforge's discount mark: while the mark is
/// live and unspent, the marked piece type costs thirty percent less to buy
/// on the flip turn or the turn after.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> int* purchase cost
///
/// Return: true when the discount applied
///
static bool eff_reforge_cost(EffectContext* context, void* x) {
    const Piece* buy = battle_buy_piece();

    if (!buy) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    Effect*      mark   = effect_find_mark(
        &battle_player(battle, side)->effects,
        CARD_REFORGE,
        (void*) (uintptr_t) side
    );

    if (!mark || mark->context->args[3] ||
        (PieceID) (uintptr_t) mark->context->args[2] != buy->id ||
        battle->turn > (size_t) (uintptr_t) mark->context->args[4] + 1) {
        return false;
    }

    *(int*) x = *(int*) x * 70 / 100;

    return true;
}

/// eff_reforge_buy
///
/// The consume half of Reforge's discount mark: buying the marked piece type
/// spends the discount so only the first purchase benefits.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> PieceInfo* just bought
///
/// Return: true when the discount was spent
///
static bool eff_reforge_buy(EffectContext* context, void* x) {
    PieceInfo*   piece  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (piece->side != side) {
        return false;
    }

    Effect* mark = effect_find_mark(
        &battle_player(battle, side)->effects,
        CARD_REFORGE,
        (void*) (uintptr_t) side
    );

    if (!mark || mark->context->args[3] ||
        (PieceID) (uintptr_t) mark->context->args[2] != piece->piece->id) {
        return false;
    }

    mark->context->args[3] = (void*) 1;

    return true;
}

/// eff_reforge
///
/// One-shot observer that, when a piece of the playing side flips away,
/// attaches a discount mark plus its cost and consume effects. The mark
/// records the flipped type, the flip turn, and a spent flag.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> flip candidate pointer
///
/// Return: true when the discount was attached
///
static bool eff_reforge(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo**  slot   = x;

    if (context->args[3] || !*slot || (*slot)->side != side) {
        return false;
    }

    LinkedList* list = &battle_player(battle, side)->effects;

    Effect      mark = {
        .func      = eff_noop,
        .trigger   = ON_TURN_START,
        .lasts_for = TURNS_2,
    };
    Effect cost = {
        .func      = eff_reforge_cost,
        .name      = "Reforge",
        .trigger   = QUERY_PIECE_CP_COST_BUY,
        .lasts_for = TURNS_2,
    };
    Effect consume = {
        .func      = eff_reforge_buy,
        .name      = "Reforge",
        .trigger   = ON_PIECE_BUY,
        .lasts_for = TURNS_2,
    };

    Effect* attached = effect_attach(list, &mark);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = (void*) (uintptr_t) CARD_REFORGE;
        attached->context->args[2] = (void*) (uintptr_t) (*slot)->piece->id;
        attached->context->args[4] = (void*) (uintptr_t) battle->turn;
    }

    Effect* cost_effect = effect_attach(list, &cost);

    if (cost_effect) {
        cost_effect->context->args[0] = (void*) (uintptr_t) side;
    }

    Effect* consume_effect = effect_attach(list, &consume);

    if (consume_effect) {
        consume_effect->context->args[0] = (void*) (uintptr_t) side;
    }

    context->args[3] = (void*) 1;

    return true;
}

/// eff_pawn_storm_action
///
/// The action-cost half of Pawn Storm: the first three pawn buys this turn
/// cost no action. Reads the shared counter mark.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> int* action cost
///
/// Return: true when the action cost was waived
///
static bool eff_pawn_storm_action(EffectContext* context, void* x) {
    const Piece* buy = battle_buy_piece();

    if (!buy || !piece_is_pawn(buy->id)) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    Effect*      mark   = effect_find_mark(
        &battle_player(battle, side)->effects,
        CARD_PAWN_STORM,
        (void*) (uintptr_t) side
    );

    if (!mark || (uintptr_t) mark->context->args[2] >= 3) {
        return false;
    }

    *(int*) x = 0;

    return true;
}

/// eff_pawn_storm_cost
///
/// The purchase-cost half of Pawn Storm: the third pawn this turn is free.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> int* purchase cost
///
/// Return: true when the pawn was made free
///
static bool eff_pawn_storm_cost(EffectContext* context, void* x) {
    const Piece* buy = battle_buy_piece();

    if (!buy || !piece_is_pawn(buy->id)) {
        return false;
    }

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    Effect*      mark   = effect_find_mark(
        &battle_player(battle, side)->effects,
        CARD_PAWN_STORM,
        (void*) (uintptr_t) side
    );

    if (!mark || (uintptr_t) mark->context->args[2] != 2) {
        return false;
    }

    *(int*) x = 0;

    return true;
}

/// eff_pawn_storm_buy
///
/// The counting half of Pawn Storm: each pawn actually bought bumps the
/// shared counter, up to three.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> PieceInfo* just bought
///
/// Return: true when the counter was bumped
///
static bool eff_pawn_storm_buy(EffectContext* context, void* x) {
    PieceInfo*   piece  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (piece->side != side || !piece_is_pawn(piece->piece->id)) {
        return false;
    }

    Effect* mark = effect_find_mark(
        &battle_player(battle, side)->effects,
        CARD_PAWN_STORM,
        (void*) (uintptr_t) side
    );

    if (!mark || (uintptr_t) mark->context->args[2] >= 3) {
        return false;
    }

    mark->context->args[2] = (void*) ((uintptr_t) mark->context->args[2] + 1);

    return true;
}

/// eff_pawn_storm
///
/// Pawn Storm setup: on play, attaches the shared counter mark plus the
/// action-cost, purchase-cost, and counting effects, all lasting this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> unused played card
///
/// Return: true always
///
static bool eff_pawn_storm(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    LinkedList*  list   = &battle_player(battle, side)->effects;

    Effect       mark   = {
        .func      = eff_noop,
        .trigger   = ON_TURN_START,
        .lasts_for = TURNS_1,
    };

    Effect* attached = effect_attach(list, &mark);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = (void*) (uintptr_t) CARD_PAWN_STORM;
    }

    Effect parts[] = {
        {.func      = eff_pawn_storm_action,
         .name      = "Pawn Storm",
         .trigger   = QUERY_PIECE_ACTION_COST_BUY,
         .lasts_for = TURNS_1},
        {.func      = eff_pawn_storm_cost,
         .name      = "Pawn Storm",
         .trigger   = QUERY_PIECE_CP_COST_BUY,
         .lasts_for = TURNS_1},
        {.func      = eff_pawn_storm_buy,
         .name      = "Pawn Storm",
         .trigger   = ON_PIECE_BUY,
         .lasts_for = TURNS_1},
    };

    for (size_t i = 0; i < sizeof(parts) / sizeof(parts[0]); i++) {
        Effect* part = effect_attach(list, &parts[i]);

        if (part) {
            part->context->args[0] = (void*) (uintptr_t) side;
        }
    }

    return true;
}

/// eff_mercy_redirect
///
/// The observer Mercy attaches: redirects the playing side's next flip onto
/// the chosen piece, later plays overriding earlier ones by attaching after
/// them. args[1] holds the chosen square, args[3] the spent flag.
///
/// Params:
/// - context -> beneficiary side in args[0], square in args[1], spent [3]
/// - x       -> flip candidate pointer to redirect
///
/// Return: true when the flip was redirected
///
static bool eff_mercy_redirect(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo**  slot   = x;

    if (context->args[3] || !*slot || (*slot)->side != side) {
        return false;
    }

    int        square = (int) (uintptr_t) context->args[1];
    PieceInfo* target = battle_at(battle, card_target_at(square));

    if (!target || target->side != side || target->piece->id == PIECE_KING) {
        return false;
    }

    *slot            = target;
    context->args[3] = (void*) 1;

    return true;
}

/// eff_mercy_targets
///
/// Mercy targeting: advertises every friendly non-king piece as the flip to
/// protect next.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_mercy_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side == side && p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_mercy_pick
///
/// Mercy resolution: attaches the redirect observer bound to the chosen
/// square, so the playing side's next flip lands there.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the observer attached
///
static bool eff_mercy_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    static const Effect redirect = {
        .func      = eff_mercy_redirect,
        .name      = "Mercy",
        .trigger   = ON_PIECE_FLIP_PRE,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &redirect);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = (void*) (uintptr_t) target->value;
    }

    return attached != nullptr;
}

/// eff_bloodletting
///
/// Adds half of the playing side's missing meter to each of its attacks
/// this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> damage value to raise
///
/// Return: true when extra damage was added
///
static bool eff_bloodletting(EffectContext* context, void* x) {
    BattleState* battle  = battle_current();
    Side         side    = (Side) (uintptr_t) context->args[0];
    PieceInfo*   subject = battle_subject();

    if (!battle_victim() || !subject || subject->side != side) {
        return false;
    }

    int missing =
        battle_meter_max(battle, side) - battle_player(battle, side)->meter;

    if (missing <= 0) {
        return false;
    }

    *(int*) x += missing / 2;

    return true;
}

/// eff_counter_coup
///
/// Echoes half of the meter damage the playing side takes back onto the
/// enemy meter this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> incoming meter damage
///
/// Return: true when an echo was dealt
///
static bool eff_counter_coup(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    int          echo   = *(int*) x / 2;

    if (echo <= 0) {
        return false;
    }

    battle_damage(battle, battle_enemy(side), echo);

    return true;
}

/// eff_spite
///
/// One-shot observer dealing three times a lost piece's value to the
/// enemy meter the next time the playing side loses a piece.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> flip candidate pointer
///
/// Return: true when the retaliation was dealt
///
static bool eff_spite(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo**  slot   = x;

    if (context->args[3] || !*slot || (*slot)->side != side) {
        return false;
    }

    int dmg = battle_value(battle, *slot, nullptr) * 3;

    battle_damage(battle, battle_enemy(side), dmg);
    context->args[3] = (void*) 1;

    return true;
}

/// eff_chain_break_targets
///
/// Chain Break targeting: advertises every enemy non-king piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_chain_break_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
      return p->side != side && p->side != SIDE_NEUTRAL &&
             p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_chain_break_pick
///
/// Chain Break resolution: force-flips the chosen enemy piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when an enemy piece was flipped
///
static bool eff_chain_break_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side == side || piece->piece->id == PIECE_KING) {
        return false;
    }

    battle_flip(battle, piece);

    return true;
}

/// eff_hydra
///
/// One-shot observer spawning two friendly pawns beside the king the next
/// time the playing side loses a piece.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> flip candidate pointer
///
/// Return: true when at least one pawn was spawned
///
static bool eff_hydra(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo**  slot   = x;

    if (context->args[3] || !*slot || (*slot)->side != side) {
        return false;
    }

    PieceInfo* king = battle_find_king(battle, side);

    if (!king) {
        return false;
    }

    int spawned = 0;

    for (int8_t dy = -1; dy <= 1 && spawned < 2; dy++) {
        for (int8_t dx = -1; dx <= 1 && spawned < 2; dx++) {
            Square spot = {
                (int8_t) (king->square.x + dx),
                (int8_t) (king->square.y + dy)};

            if ((dx || dy) && battle_in_bounds(battle, spot) &&
                !battle_at(battle, spot) &&
                battle_spawn(battle, PIECE_PAWN, spot, side)) {
                spawned++;
            }
        }
    }

    context->args[3] = (void*) 1;

    return spawned > 0;
}

/*----------------------------------------------------------------------------*\
                                UNIVERSAL DATA
\*----------------------------------------------------------------------------*/

const Piece UNIVERSAL_PIECES[] = {
    {
        .at      = king_at,
        .mv      = king_mv,
        .name    = "King",
        .desc    = "Moves one square in any direction. Cannot be "
                   "bought. Flipping it loses the battle.",
        .id      = PIECE_KING,
        .kingdom = KINGDOM_NONE,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 10,
    },
};

const Card UNIVERSAL_CARDS[] = {
    {
        .effects =
            {{.func      = eff_pawn_storm,
              .name      = "Pawn Storm",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Pawn Storm",
        .desc      = "Buy up to 3 pawns this turn without action cost; "
                     "the third is free.",
        .id        = CARD_PAWN_STORM,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_revitalize,
              .name      = "Revitalize",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Revitalize",
        .desc      = "Restore 50 to your meter.",
        .id        = CARD_REVITALIZE,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 80,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_hostage,
              .name      = "Hostage",
              .trigger   = ON_PIECE_FLIP,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Hostage",
        .desc      = "Next flip to your side: your meter gains 20 bonus.",
        .id        = CARD_HOSTAGE,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 20,
    },
    {
        .effects =
            {{.func      = eff_last_stand,
              .name      = "Last Stand",
              .trigger   = QUERY_PIECE_CAN_FLIP,
              .lasts_for = TURNS_1}},
        .name      = "Last Stand",
        .desc      = "This turn your meter cannot trigger a flip. Damage "
                     "still applies.",
        .id        = CARD_LAST_STAND,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_sacrifice_targets,
              .name      = "Sacrifice",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_sacrifice_pick,
              .name      = "Sacrifice",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Sacrifice",
        .desc      = "Remove one of your pieces. Gain its value x2 as "
                     "meter.",
        .id        = CARD_SACRIFICE,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 25,
    },
    {
        .effects =
            {{.func      = eff_reforge,
              .name      = "Reforge",
              .trigger   = ON_PIECE_FLIP_PRE,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Reforge",
        .desc      = "Next time a piece flips, its type is 30% cheaper "
                     "to buy next turn.",
        .id        = CARD_REFORGE,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_mercy_targets,
              .name      = "Mercy",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_mercy_pick,
              .name      = "Mercy",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Mercy",
        .desc      = "Target which piece flips next, overriding earlier "
                     "uses.",
        .id        = CARD_MERCY,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_bloodletting,
              .name      = "Bloodletting",
              .trigger   = QUERY_PIECE_DAMAGE_DEALT,
              .lasts_for = TURNS_1}},
        .name      = "Bloodletting",
        .desc      = "This turn your attacks deal extra damage equal to "
                     "50% of your missing meter.",
        .id        = CARD_BLOODLETTING,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_counter_coup,
              .name      = "Counter Coup",
              .trigger   = QUERY_METER_DAMAGE_TAKEN,
              .lasts_for = TURNS_1}},
        .name      = "Counter Coup",
        .desc      = "This turn all damage you take echoes back at 50% "
                     "to enemy meter.",
        .id        = CARD_COUNTER_COUP,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 50,
    },
    {
        .effects =
            {{.func      = eff_spite,
              .name      = "Spite",
              .trigger   = ON_PIECE_FLIP_PRE,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Spite",
        .desc      = "Next time you lose a piece, deal its value x3 to "
                     "enemy meter.",
        .id        = CARD_SPITE,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 15,
        .sell_cost = 70,
    },
    {
        .effects =
            {{.func      = eff_chain_break_targets,
              .name      = "Chain Break",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_chain_break_pick,
              .name      = "Chain Break",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Chain Break",
        .desc      = "Force-flip one enemy piece of your choice.",
        .id        = CARD_CHAIN_BREAK,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 100,
        .sell_cost = 70,
    },
    {
        .effects =
            {{.func      = eff_hydra,
              .name      = "Hydra",
              .trigger   = ON_PIECE_FLIP_PRE,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Hydra",
        .desc      = "Next time a piece flips, 2 friendly pawns spawn "
                     "beside your king.",
        .id        = CARD_HYDRA,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_NONE,
        .play_cost = 0,
        .sell_cost = 80,
    },
};

/*----------------------------------------------------------------------------*\
                                   MODIFIERS
\*----------------------------------------------------------------------------*/

/// eff_lean_times
///
/// Lean Times: each side opens the battle with twenty less currency.
///
/// Params:
/// - context -> args[0] the side this copy serves
/// - x       -> unused battle node
///
/// Return: true, the deduction always applies
///
static bool eff_lean_times(EffectContext* context, void* x) {
    (void) x;

    PlayerState* player =
        battle_player(battle_current(), (Side) (uintptr_t) context->args[0]);

    player->cp -= 20;

    if (player->cp < 0) {
        player->cp = 0;
    }

    return true;
}

/// eff_windfall
///
/// Windfall: each side opens the battle with thirty more currency.
///
/// Params:
/// - context -> args[0] the side this copy serves
/// - x       -> unused battle node
///
/// Return: true, the bonus always applies
///
static bool eff_windfall(EffectContext* context, void* x) {
    (void) x;

    battle_player(battle_current(), (Side) (uintptr_t) context->args[0])->cp +=
        30;

    return true;
}

/// eff_open_market
///
/// Open Market: every piece costs half its currency this battle.
///
/// Params:
/// - context -> unused
/// - x       -> int* buy cost to halve
///
/// Return: true, the discount always applies
///
static bool eff_open_market(EffectContext* context, void* x) {
    (void) context;

    *(int*) x /= 2;

    return true;
}

/// eff_devalued
///
/// Devalued Currency: card sell values are halved this battle.
///
/// Params:
/// - context -> unused
/// - x       -> int* sell cost to halve
///
/// Return: true, the halving always applies
///
static bool eff_devalued(EffectContext* context, void* x) {
    (void) context;

    *(int*) x /= 2;

    return true;
}

/// eff_tax_collector
///
/// Tax Collector: selling a card yields five extra currency.
///
/// Params:
/// - context -> args[0] the side this copy serves
/// - x       -> unused sold card
///
/// Return: true, the bonus always applies
///
static bool eff_tax_collector(EffectContext* context, void* x) {
    (void) x;

    battle_player(battle_current(), (Side) (uintptr_t) context->args[0])->cp +=
        5;

    return true;
}

/// eff_glass_cannon
///
/// Glass Cannon: cascade meter refills fill to only half their value.
///
/// Params:
/// - context -> unused
/// - x       -> int* refill amount to halve
///
/// Return: true, the halving always applies
///
static bool eff_glass_cannon(EffectContext* context, void* x) {
    (void) context;

    *(int*) x /= 2;

    return true;
}

/// eff_iron_will
///
/// Iron Will: a quarter of the damage a side deals also strikes its own
/// meter. The copy on the receiver's list knows the dealer is its enemy.
///
/// Params:
/// - context -> args[0] the receiving side this copy serves
/// - x       -> int* meter damage the receiver is taking
///
/// Return: true when the dealer took recoil
///
static bool eff_iron_will(EffectContext* context, void* x) {
    int recoil = *(int*) x * 25 / 100;

    if (recoil <= 0) {
        return false;
    }

    Side receiver = (Side) (uintptr_t) context->args[0];

    battle_meter_gain(battle_current(), battle_enemy(receiver), -recoil);

    return true;
}

/// eff_mirror
///
/// Mirror: a quarter of the damage a side deals heals its own meter. The
/// copy on the receiver's list knows the dealer is its enemy.
///
/// Params:
/// - context -> args[0] the receiving side this copy serves
/// - x       -> int* meter damage the receiver is taking
///
/// Return: true when the dealer was healed
///
static bool eff_mirror(EffectContext* context, void* x) {
    int heal = *(int*) x * 25 / 100;

    if (heal <= 0) {
        return false;
    }

    Side receiver = (Side) (uintptr_t) context->args[0];

    battle_meter_gain(battle_current(), battle_enemy(receiver), heal);

    return true;
}

/// eff_overflow
///
/// Overflow: a piece flipped to a side gains thirty value, swelling that
/// side's meter maximum. The copy on the gaining side's list fires.
///
/// Params:
/// - context -> unused
/// - x       -> PieceInfo* that just flipped
///
/// Return: true, the bonus always applies
///
static bool eff_overflow(EffectContext* context, void* x) {
    (void) context;

    ((PieceInfo*) x)->piece->value += 30;

    return true;
}

/// eff_rich_hand
///
/// Rich Hand: draw four cards per turn.
///
/// Params:
/// - context -> unused
/// - x       -> int* draw count to set
///
/// Return: true, the count always applies
///
static bool eff_rich_hand(EffectContext* context, void* x) {
    (void) context;

    *(int*) x = 4;

    return true;
}

/// eff_sparse_hand
///
/// Sparse Hand: draw two cards per turn.
///
/// Params:
/// - context -> unused
/// - x       -> int* draw count to set
///
/// Return: true, the count always applies
///
static bool eff_sparse_hand(EffectContext* context, void* x) {
    (void) context;

    *(int*) x = 2;

    return true;
}

/// eff_kingdom_purity
///
/// Kingdom Purity: only cards native to the region being fought in may be
/// drawn; kingdomless universal cards always pass to pad a short pool.
///
/// Params:
/// - context -> unused
/// - x       -> bool* draw eligibility to veto
///
/// Return: true when a foreign card was blocked
///
static bool eff_kingdom_purity(EffectContext* context, void* x) {
    (void) context;

    BattleState* battle = battle_current();
    Card*        card   = battle_subject_card();

    if (!battle->node || !battle->node->kingdom || !card) {
        return false;
    }

    if (card->kingdom == KINGDOM_NONE ||
        card->kingdom == battle->node->kingdom->id) {
        return false;
    }

    *(bool*) x = false;

    return true;
}

/// eff_lucky_strike
///
/// Lucky Strike: once per turn, the first drawn card becomes a random card
/// of the highest tier in the available draw pool. args[1] stamps the turn
/// already served as turn + 1, since a zeroed slot means untouched.
///
/// Params:
/// - context -> args[0] beneficiary side, args[1] last served turn + 1
/// - x       -> Card** hand whose first slot is rewritten
///
/// Return: true when the first card was promoted
///
static bool eff_lucky_strike(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    size_t       turn   = battle->turn;

    if ((size_t) (uintptr_t) context->args[1] == turn + 1) {
        return false;
    }

    Side   side = (Side) (uintptr_t) context->args[0];
    CardID pool[CARD_COUNT];
    size_t size = battle_draw_pool(battle, side, pool);

    if (size == 0) {
        return false;
    }

    UnlockTier best = CARD_REGISTRY[pool[0]]->tier;

    for (size_t i = 1; i < size; i++) {
        if (CARD_REGISTRY[pool[i]]->tier > best) {
            best = CARD_REGISTRY[pool[i]]->tier;
        }
    }

    CardID top[CARD_COUNT];
    size_t count = 0;

    for (size_t i = 0; i < size; i++) {
        if (CARD_REGISTRY[pool[i]]->tier == best) {
            top[count] = pool[i];
            count++;
        }
    }

    Card** hand      = x;

    hand[0]          = (Card*) CARD_REGISTRY[top[battle_rand() % count]];
    context->args[1] = (void*) (uintptr_t) (turn + 1);

    return true;
}

/// eff_extended_front
///
/// Extended Front: the board is built two columns wider.
///
/// Params:
/// - context -> unused
/// - x       -> Square* board dimension {width, height} in {x, y}
///
/// Return: true, the widening always applies
///
static bool eff_extended_front(EffectContext* context, void* x) {
    (void) context;

    ((Square*) x)->x += 2;

    return true;
}

/// eff_compressed
///
/// Compressed: the board is built two columns narrower.
///
/// Params:
/// - context -> unused
/// - x       -> Square* board dimension {width, height} in {x, y}
///
/// Return: true, the narrowing always applies
///
static bool eff_compressed(EffectContext* context, void* x) {
    (void) context;

    ((Square*) x)->x -= 2;

    return true;
}

/// eff_dense_terrain
///
/// Dense Terrain: a fifth of the board's squares are voided at build.
///
/// Params:
/// - context -> unused
/// - x       -> unused built board
///
/// Return: true, the scatter always applies
///
static bool eff_dense_terrain(EffectContext* context, void* x) {
    (void) context;
    (void) x;

    battle_scatter_voids(battle_current(), 20);

    return true;
}

/// eff_bloodbath
///
/// Bloodbath: an emptied meter flips two pieces instead of one. Adds one to
/// the flip count the cascade queries.
///
/// Params:
/// - context -> unused
/// - x       -> int* flip count for this meter empty
///
/// Return: true always
///
static bool eff_bloodbath(EffectContext* context, void* x) {
    (void) context;

    *(int*) x += 1;

    return true;
}

/// eff_fog_of_war
///
/// Fog of War: an enemy piece is hidden from the board unless it stands on
/// a square the human currently attacks. Builds the human attack coverage,
/// then hides every enemy piece off that coverage.
///
/// Params:
/// - context -> args[0] human side
/// - x       -> Board* whose visible flags are cleared
///
/// Return: true when at least one enemy piece was hidden
///
static bool eff_fog_of_war(EffectContext* context, void* x) {
    BattleState* battle                   = battle_current();
    Side         side                     = (Side) (uintptr_t) context->args[0];
    Board*       board                    = x;

    bool         attacked[MAX_BOARD_SIZE] = {};

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x0 = 0; x0 < battle->board.width; x0++) {
            PieceInfo* piece = battle_at(battle, (Square){x0, y});

            if (!piece || piece == &VOID_CELL || piece->side != side) {
                continue;
            }

            Square* coverage = battle_attacks(battle, piece);

            for (size_t i = 0; !(coverage[i].x == -1 && coverage[i].y == -1);
                 i++) {
                attacked[coverage[i].y * 20 + coverage[i].x] = true;
            }
        }
    }

    bool hid = false;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x0 = 0; x0 < battle->board.width; x0++) {
            size_t     index = (size_t) (y * 20 + x0);
            PieceInfo* piece = battle_at(battle, (Square){x0, y});

            if (!piece || piece == &VOID_CELL || piece->side == side ||
                piece->side == SIDE_NEUTRAL) {
                continue;
            }

            if (!attacked[index]) {
                board->visible[index] = false;
                hid                   = true;
            }
        }
    }

    return hid;
}

const BattleModifier MODIFIER_REGISTRY[MODIFIER_COUNT] = {
    [MODIFIER_LEAN_TIMES] =
        {
            .name    = "Lean Times",
            .desc    = "Both sides start with 20 less cp.",
            .id      = MODIFIER_LEAN_TIMES,
            .effects = {{
                .func      = eff_lean_times,
                .name      = "Lean Times",
                .trigger   = ON_BATTLE_START,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_WINDFALL] =
        {
            .name    = "Windfall",
            .desc    = "Both sides start with 30 more cp.",
            .id      = MODIFIER_WINDFALL,
            .effects = {{
                .func      = eff_windfall,
                .name      = "Windfall",
                .trigger   = ON_BATTLE_START,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_OPEN_MARKET] =
        {
            .name    = "Open Market",
            .desc    = "All pieces cost 50% this battle.",
            .id      = MODIFIER_OPEN_MARKET,
            .effects = {{
                .func      = eff_open_market,
                .name      = "Open Market",
                .trigger   = QUERY_PIECE_CP_COST_BUY,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_DEVALUED_CURRENCY] =
        {
            .name    = "Devalued Currency",
            .desc    = "All card sell values are halved.",
            .id      = MODIFIER_DEVALUED_CURRENCY,
            .effects = {{
                .func      = eff_devalued,
                .name      = "Devalued Currency",
                .trigger   = QUERY_CARD_SELL_COST,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_TAX_COLLECTOR] =
        {
            .name    = "Tax Collector",
            .desc    = "+5 cp on top of each card sold.",
            .id      = MODIFIER_TAX_COLLECTOR,
            .effects = {{
                .func      = eff_tax_collector,
                .name      = "Tax Collector",
                .trigger   = ON_CARD_SELL,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_GLASS_CANNON] =
        {
            .name    = "Glass Cannon",
            .desc    = "Both meters refill at only 50% of value.",
            .id      = MODIFIER_GLASS_CANNON,
            .effects = {{
                .func      = eff_glass_cannon,
                .name      = "Glass Cannon",
                .trigger   = QUERY_METER_REFILL,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_BLOODBATH] =
        {
            .name    = "Bloodbath",
            .desc    = "Each flip triggers 2 pieces instead of 1.",
            .id      = MODIFIER_BLOODBATH,
            .effects = {{
                .func      = eff_bloodbath,
                .name      = "Bloodbath",
                .trigger   = QUERY_FLIP_COUNT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_IRON_WILL] =
        {
            .name    = "Iron Will",
            .desc    = "Damage you deal hits your own meter at 25%.",
            .id      = MODIFIER_IRON_WILL,
            .effects = {{
                .func      = eff_iron_will,
                .name      = "Iron Will",
                .trigger   = QUERY_METER_DAMAGE_TAKEN,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_OVERFLOW] =
        {
            .name    = "Overflow",
            .desc    = "A flipped piece you gain gets +30 value.",
            .id      = MODIFIER_OVERFLOW,
            .effects = {{
                .func      = eff_overflow,
                .name      = "Overflow",
                .trigger   = ON_PIECE_FLIP,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_MIRROR] =
        {
            .name    = "Mirror",
            .desc    = "Damage you deal heals your own meter at 25%.",
            .id      = MODIFIER_MIRROR,
            .effects = {{
                .func      = eff_mirror,
                .name      = "Mirror",
                .trigger   = QUERY_METER_DAMAGE_TAKEN,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_RICH_HAND] =
        {
            .name    = "Rich Hand",
            .desc    = "Draw 4 cards per turn.",
            .id      = MODIFIER_RICH_HAND,
            .effects = {{
                .func      = eff_rich_hand,
                .name      = "Rich Hand",
                .trigger   = QUERY_CARD_DRAW_COUNT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_SPARSE_HAND] =
        {
            .name    = "Sparse Hand",
            .desc    = "Draw 2 cards per turn.",
            .id      = MODIFIER_SPARSE_HAND,
            .effects = {{
                .func      = eff_sparse_hand,
                .name      = "Sparse Hand",
                .trigger   = QUERY_CARD_DRAW_COUNT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_KINGDOM_PURITY] =
        {
            .name    = "Kingdom Purity",
            .desc    = "Only cards of the region being fought in are drawn.",
            .id      = MODIFIER_KINGDOM_PURITY,
            .effects = {{
                .func      = eff_kingdom_purity,
                .name      = "Kingdom Purity",
                .trigger   = QUERY_CARD_CAN_DRAW,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_LUCKY_STRIKE] =
        {
            .name = "Lucky Strike",
            .desc = "First card drawn each turn is the highest tier available.",
            .id   = MODIFIER_LUCKY_STRIKE,
            .effects = {{
                .func      = eff_lucky_strike,
                .name      = "Lucky Strike",
                .trigger   = ON_CARDS_DRAWN,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_FOG_OF_WAR] =
        {
            .name    = "Fog of War",
            .desc    = "Enemy pieces are hidden off squares you attack.",
            .id      = MODIFIER_FOG_OF_WAR,
            .effects = {{
                .func      = eff_fog_of_war,
                .name      = "Fog of War",
                .trigger   = QUERY_BOARD_STATE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_DENSE_TERRAIN] =
        {
            .name    = "Dense Terrain",
            .desc    = "20% of squares are missing, revealed at start.",
            .id      = MODIFIER_DENSE_TERRAIN,
            .effects = {{
                .func      = eff_dense_terrain,
                .name      = "Dense Terrain",
                .trigger   = ON_BOARD_BUILD,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_EXTENDED_FRONT] =
        {
            .name    = "Extended Front",
            .desc    = "Board is 2 columns wider.",
            .id      = MODIFIER_EXTENDED_FRONT,
            .effects = {{
                .func      = eff_extended_front,
                .name      = "Extended Front",
                .trigger   = QUERY_BOARD_DIMENSION,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [MODIFIER_COMPRESSED] =
        {
            .name    = "Compressed",
            .desc    = "Board is 2 columns narrower.",
            .id      = MODIFIER_COMPRESSED,
            .effects = {{
                .func      = eff_compressed,
                .name      = "Compressed",
                .trigger   = QUERY_BOARD_DIMENSION,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
};

/*----------------------------------------------------------------------------*\
                                 CHAIN PENALTIES
\*----------------------------------------------------------------------------*/

/// eff_bronze_chain
///
/// Bronze Chain: the player opens a battle in the chained region with ten
/// less currency, taken from the first turn's income so the opening purse
/// composes on the shared QUERY_CP_INCOME seam.
///
/// Params:
/// - context -> unused
/// - x       -> int* the acting side's income
///
/// Return: true when the opening income is reduced
///
static bool eff_bronze_chain(EffectContext* context, void* x) {
    (void) context;

    if (battle_current()->turn != 1) {
        return false;
    }

    *(int*) x -= 10;

    return true;
}

/// eff_silver_chain
///
/// Silver Chain: the enemy fields one extra free piece in the chained
/// region. Adds one to the reinforcement count the setup queries.
///
/// Params:
/// - context -> unused
/// - x       -> int* enemy reinforcement count
///
/// Return: true always
///
static bool eff_silver_chain(EffectContext* context, void* x) {
    (void) context;

    *(int*) x += 1;

    return true;
}

const ChainPenalty CHAIN_REGISTRY[CHAIN_PENALTY_COUNT] = {
    [CHAIN_BRONZE] =
        {
            .name    = "Bronze",
            .desc    = "Battles in this region start with -10 cp.",
            .id      = CHAIN_BRONZE,
            .effects = {{
                .func      = eff_bronze_chain,
                .name      = "Bronze Chain",
                .trigger   = QUERY_CP_INCOME,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [CHAIN_SILVER] =
        {
            .name    = "Silver",
            .desc    = "Enemy gets +1 free starting piece in this region.",
            .id      = CHAIN_SILVER,
            .effects = {{
                .func      = eff_silver_chain,
                .name      = "Silver Chain",
                .trigger   = QUERY_ENEMY_ARMY_COUNT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [CHAIN_GOLD] =
        {
            .name = "Gold",
            .desc = "Figurehead Subjugated: the track locks.",
            .id   = CHAIN_GOLD,
        },
};

/*----------------------------------------------------------------------------*\
                             DIFFICULTIES & CHALLENGES
\*----------------------------------------------------------------------------*/

/// eff_bound
///
/// Bound difficulty: Vorath's pressure opens at two extra enemy pieces,
/// added to the reinforcement count the setup queries.
///
/// Params:
/// - context -> unused
/// - x       -> int* enemy reinforcement count
///
/// Return: true always
///
static bool eff_bound(EffectContext* context, void* x) {
    (void) context;

    *(int*) x += 2;

    return true;
}

/// eff_shackled
///
/// Shackled difficulty: in a chained region the opening income drops fifteen
/// further, so the Bronze -10 becomes -25. Reads the region chain rather
/// than the Bronze effect, so it fires only where a chain applies.
///
/// Params:
/// - context -> unused
/// - x       -> int* the acting side's income
///
/// Return: true when the opening income is reduced
///
static bool eff_shackled(EffectContext* context, void* x) {
    (void) context;

    BattleState* battle = battle_current();

    if (battle->turn != 1 || !battle->node || !battle->node->kingdom ||
        !battle->node->kingdom->chain) {
        return false;
    }

    *(int*) x -= 15;

    return true;
}

/// eff_enslaved
///
/// Enslaved difficulty: the region kingdom's innate arms the enemy seat
/// from the first battle, attached at setup like the human's own innates.
///
/// Params:
/// - context -> args[0] human side
/// - x       -> unused battle node
///
/// Return: true when the region has a kingdom to draw the innate from
///
static bool eff_enslaved(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();

    if (!battle->node || !battle->node->kingdom) {
        return false;
    }

    Side human = (Side) (uintptr_t) context->args[0];

    battle_attach_power(
        battle,
        battle_enemy(human),
        INNATE_REGISTRY[battle->node->kingdom->id],
        MASTERY_NONE
    );

    return true;
}

/// eff_solo_vanguard
///
/// Solo Vanguard challenge: the human may hold only one non-king piece on
/// the board, so a buy is refused while one already stands.
///
/// Params:
/// - context -> args[0] human side
/// - x       -> bool* whether the buy is permitted
///
/// Return: true when the buy is vetoed
///
static bool eff_solo_vanguard(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side &&
            cell->piece->id != PIECE_KING) {
            *(bool*) x = false;

            return true;
        }
    }

    return false;
}

/// eff_pacifist
///
/// Pacifist Doctrine challenge: the human cannot buy a piece valued above
/// twenty currency.
///
/// Params:
/// - context -> unused
/// - x       -> bool* whether the buy is permitted
///
/// Return: true when the buy is vetoed
///
static bool eff_pacifist(EffectContext* context, void* x) {
    (void) context;

    const Piece* piece = battle_buy_piece();

    if (piece && piece->value > 20) {
        *(bool*) x = false;

        return true;
    }

    return false;
}

/// eff_blind_draft
///
/// Blind Draft challenge: the whole hand hides its card identities, leaving
/// only kingdom and tier visible until a card is played or sold.
///
/// Params:
/// - context -> unused
/// - x       -> PlayerState* the human seat
///
/// Return: true always
///
static bool eff_blind_draft(EffectContext* context, void* x) {
    (void) context;

    PlayerState* player = x;

    for (size_t card = 0; card < MAX_DRAWN_CARDS; card++) {
        player->hand_visible[card] = false;
    }

    return true;
}

/// eff_traitors_gambit
///
/// The Traitor's Gambit challenge: the enemy opens with three free pieces
/// already standing in the human's half of the board.
///
/// Params:
/// - context -> args[0] human side
/// - x       -> unused battle node
///
/// Return: true, the intrusion always spawns
///
static bool eff_traitors_gambit(EffectContext* context, void* x) {
    (void) x;

    Side human = (Side) (uintptr_t) context->args[0];

    battle_reinforce(battle_current(), battle_enemy(human), human, 3);

    return true;
}

const DifficultyMode DIFFICULTY_REGISTRY[DIFFICULTY_NONE] = {
    [DIFFICULTY_FREE] =
        {
            .name = "Free",
            .desc = "Baseline campaign with no added penalties.",
            .id   = DIFFICULTY_FREE,
        },
    [DIFFICULTY_BOUND] =
        {
            .name    = "Bound",
            .desc    = "Vorath's Pressure starts at two extra enemy pieces.",
            .id      = DIFFICULTY_BOUND,
            .effects = {{
                .func      = eff_bound,
                .name      = "Bound",
                .trigger   = QUERY_ENEMY_ARMY_COUNT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [DIFFICULTY_SHACKLED] =
        {
            .name    = "Shackled",
            .desc    = "The Bronze chain penalty becomes -25 cp.",
            .id      = DIFFICULTY_SHACKLED,
            .effects = {{
                .func      = eff_shackled,
                .name      = "Shackled",
                .trigger   = QUERY_CP_INCOME,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [DIFFICULTY_ENSLAVED] =
        {
            .name    = "Enslaved",
            .desc    = "Enemy kingdom innates are active from battle one.",
            .id      = DIFFICULTY_ENSLAVED,
            .effects = {{
                .func      = eff_enslaved,
                .name      = "Enslaved",
                .trigger   = ON_BATTLE_SETUP,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
};

const ChallengeRun CHALLENGE_REGISTRY[CHALLENGE_COUNT] = {
    [CHALLENGE_DAILY_CONQUEST] =
        {
            .name = "Daily Conquest",
            .desc = "A shared daily seed; no battle-side effect.",
            .id   = CHALLENGE_DAILY_CONQUEST,
        },
    [CHALLENGE_SOLO_VANGUARD] =
        {
            .name    = "Solo Vanguard",
            .desc    = "Only one non-king piece may stand at a time.",
            .id      = CHALLENGE_SOLO_VANGUARD,
            .effects = {{
                .func      = eff_solo_vanguard,
                .name      = "Solo Vanguard",
                .trigger   = QUERY_PIECE_CAN_BUY,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [CHALLENGE_PACIFIST_DOCTRINE] =
        {
            .name    = "Pacifist Doctrine",
            .desc    = "No piece above 20 cp may be bought.",
            .id      = CHALLENGE_PACIFIST_DOCTRINE,
            .effects = {{
                .func      = eff_pacifist,
                .name      = "Pacifist Doctrine",
                .trigger   = QUERY_PIECE_CAN_BUY,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [CHALLENGE_BLIND_DRAFT] =
        {
            .name    = "Blind Draft",
            .desc    = "Card names and effects hide until played or sold.",
            .id      = CHALLENGE_BLIND_DRAFT,
            .effects = {{
                .func      = eff_blind_draft,
                .name      = "Blind Draft",
                .trigger   = QUERY_HAND_STATE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [CHALLENGE_THE_TRAITORS_GAMBIT] =
        {
            .name    = "The Traitor's Gambit",
            .desc    = "The enemy starts with three pieces in your half.",
            .id      = CHALLENGE_THE_TRAITORS_GAMBIT,
            .effects = {{
                .func      = eff_traitors_gambit,
                .name      = "The Traitor's Gambit",
                .trigger   = ON_BATTLE_SETUP,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [CHALLENGE_CLOCKWORK] =
        {
            .name = "Clockwork",
            .desc = "A thirty-second turn timer; no battle-side effect.",
            .id   = CHALLENGE_CLOCKWORK,
        },
};

/*----------------------------------------------------------------------------*\
                                   SYNERGIES
\*----------------------------------------------------------------------------*/

/// eff_syn_longwei
///
/// Longwei synergy in Kewarani: Pao attacks deal ten extra damage.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> int* damage to raise
///
/// Return: true when a Pao struck an enemy
///
static bool eff_syn_longwei(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (!battle_victim() || !subject || subject->side != side ||
        subject->piece->id != PIECE_PAO) {
        return false;
    }

    *(int*) x += 10;

    return true;
}

/// eff_syn_kewarani
///
/// Kewarani synergy in Zarqan: Kewarani pieces cost ten currency less.
///
/// Params:
/// - context -> unused
/// - x       -> int* buy cost to lower
///
/// Return: true when a Kewarani piece was discounted
///
static bool eff_syn_kewarani(EffectContext* context, void* x) {
    (void) context;

    const Piece* piece = battle_buy_piece();

    if (!piece || piece->kingdom != KINGDOM_KEWARANI) {
        return false;
    }

    *(int*) x -= 10;

    if (*(int*) x < 0) {
        *(int*) x = 0;
    }

    return true;
}

/// eff_syn_zarqan
///
/// Zarqan synergy in Harushima: Ziraafa and Talliya gain five value.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> int* damage or effective value to raise
///
/// Return: true when the subject was a Ziraafa or Talliya
///
static bool eff_syn_zarqan(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (!subject || subject->side != side ||
        (subject->piece->id != PIECE_ZIRAAFA &&
         subject->piece->id != PIECE_TALLIYA)) {
        return false;
    }

    *(int*) x += 5;

    return true;
}

/// eff_syn_harushima
///
/// Harushima synergy in Caelan: playing a Caelan card draws one bonus
/// card.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> Card* being played
///
/// Return: true when a Caelan card triggered the draw
///
static bool eff_syn_harushima(EffectContext* context, void* x) {
    Side  side = (Side) (uintptr_t) context->args[0];
    Card* card = x;

    if (card->kingdom != KINGDOM_CAELAN) {
        return false;
    }

    battle_draw(battle_current(), side, 1);

    return true;
}

/// eff_syn_caelan
///
/// Caelan synergy in Longwei: Sultan's Gold yields ten extra currency.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> Card* being played
///
/// Return: true when Sultan's Gold was played
///
static bool eff_syn_caelan(EffectContext* context, void* x) {
    Side  side = (Side) (uintptr_t) context->args[0];
    Card* card = x;

    if (card->id != CARD_SULTANS_GOLD) {
        return false;
    }

    battle_player(battle_current(), side)->cp += 10;

    return true;
}

const KingdomPower SYNERGY_REGISTRY[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI] =
        {
            .effects = {{
                .func      = eff_syn_longwei,
                .name      = "Longwei Synergy",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
            .name = "Longwei Synergy",
            .id   = KINGDOM_LONGWEI,
        },
    [KINGDOM_KEWARANI] =
        {
            .effects = {{
                .func      = eff_syn_kewarani,
                .name      = "Kewarani Synergy",
                .trigger   = QUERY_PIECE_CP_COST_BUY,
                .lasts_for = ENTIRE_BATTLE,
            }},
            .name = "Kewarani Synergy",
            .id   = KINGDOM_KEWARANI,
        },
    [KINGDOM_ZARQAN] =
        {
            .effects = {{
                .func      = eff_syn_zarqan,
                .name      = "Zarqan Synergy",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
            .name = "Zarqan Synergy",
            .id   = KINGDOM_ZARQAN,
        },
    [KINGDOM_HARUSHIMA] =
        {
            .effects = {{
                .func      = eff_syn_harushima,
                .name      = "Harushima Synergy",
                .trigger   = ON_CARD_PLAY,
                .lasts_for = ENTIRE_BATTLE,
            }},
            .name = "Harushima Synergy",
            .id   = KINGDOM_HARUSHIMA,
        },
    [KINGDOM_CAELAN] =
        {
            .effects = {{
                .func      = eff_syn_caelan,
                .name      = "Caelan Synergy",
                .trigger   = ON_CARD_PLAY,
                .lasts_for = ENTIRE_BATTLE,
            }},
            .name = "Caelan Synergy",
            .id   = KINGDOM_CAELAN,
        },
};

const KingdomPower* const INNATE_REGISTRY[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = &LONGWEI_INNATE,
    [KINGDOM_KEWARANI]  = &KEWARANI_INNATE,
    [KINGDOM_ZARQAN]    = &ZARQAN_INNATE,
    [KINGDOM_HARUSHIMA] = &HARUSHIMA_INNATE,
    [KINGDOM_CAELAN]    = &CAELAN_INNATE,
};

const KingdomPower* const CLIMAX_REGISTRY[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = &LONGWEI_CLIMAX,
    [KINGDOM_KEWARANI]  = &KEWARANI_CLIMAX,
    [KINGDOM_ZARQAN]    = &ZARQAN_CLIMAX,
    [KINGDOM_HARUSHIMA] = &HARUSHIMA_CLIMAX,
    [KINGDOM_CAELAN]    = &CAELAN_CLIMAX,
};

const Piece* const PIECE_REGISTRY[PIECE_COUNT] = {
    [PIECE_KING]             = &UNIVERSAL_PIECES[0],

    [PIECE_BING]             = &LONGWEI_PIECES[0],
    [PIECE_XIANG]            = &LONGWEI_PIECES[1],
    [PIECE_MA]               = &LONGWEI_PIECES[2],
    [PIECE_PAO]              = &LONGWEI_PIECES[3],
    [PIECE_LIUBO_DIVINER]    = &LONGWEI_PIECES[4],
    [PIECE_SANG]             = &LONGWEI_PIECES[5],
    [PIECE_NORTHERN_CAVALRY] = &LONGWEI_PIECES[6],
    [PIECE_HWACHA]           = &LONGWEI_PIECES[7],

    [PIECE_FUHYO]            = &HARUSHIMA_PIECES[0],
    [PIECE_KYOSHA]           = &HARUSHIMA_PIECES[1],
    [PIECE_GINSHO]           = &HARUSHIMA_PIECES[2],
    [PIECE_KINSHO]           = &HARUSHIMA_PIECES[3],
    [PIECE_SHISHI]           = &HARUSHIMA_PIECES[4],
    [PIECE_HONORABLE_HORSE]  = &HARUSHIMA_PIECES[5],
    [PIECE_PROMOTED_BISHOP]  = &HARUSHIMA_PIECES[6],
    [PIECE_DAIMYO]           = &HARUSHIMA_PIECES[7],
    [PIECE_DRAGON]           = &HARUSHIMA_PIECES[8],

    [PIECE_MEDEQ]            = &KEWARANI_PIECES[0],
    [PIECE_MAKWANAM]         = &KEWARANI_PIECES[1],
    [PIECE_SABA]             = &KEWARANI_PIECES[2],
    [PIECE_FARAS]            = &KEWARANI_PIECES[3],
    [PIECE_NEGUS_GUARD]      = &KEWARANI_PIECES[4],
    [PIECE_MEDEQ_SQUAD]      = &KEWARANI_PIECES[5],
    [PIECE_SULTANS_LEVY]     = &KEWARANI_PIECES[6],

    [PIECE_WAZIR]            = &ZARQAN_PIECES[0],
    [PIECE_JAMAL]            = &ZARQAN_PIECES[1],
    [PIECE_TALLIYA]          = &ZARQAN_PIECES[2],
    [PIECE_ZIRAAFA]          = &ZARQAN_PIECES[3],
    [PIECE_SHAHZADEH]        = &ZARQAN_PIECES[4],
    [PIECE_OLD_KING]         = &ZARQAN_PIECES[5],
    [PIECE_CATAPHRACT]       = &ZARQAN_PIECES[6],
    [PIECE_ROOK]             = &ZARQAN_PIECES[7],
    [PIECE_WAR_ELEPHANT]     = &ZARQAN_PIECES[8],

    [PIECE_PAWN]             = &CAELAN_PIECES[0],
    [PIECE_KNIGHT]           = &CAELAN_PIECES[1],
    [PIECE_BISHOP]           = &CAELAN_PIECES[2],
    [PIECE_QUEEN]            = &CAELAN_PIECES[3],
    [PIECE_GRYPHON]          = &CAELAN_PIECES[4],
    [PIECE_CHANCELLOR]       = &CAELAN_PIECES[5],
    [PIECE_SOVEREIGN_BANNER] = &CAELAN_PIECES[6],
};

const Card* const CARD_REGISTRY[CARD_COUNT] = {
    [CARD_PAWN_STORM]           = &UNIVERSAL_CARDS[0],
    [CARD_REVITALIZE]           = &UNIVERSAL_CARDS[1],
    [CARD_HOSTAGE]              = &UNIVERSAL_CARDS[2],
    [CARD_LAST_STAND]           = &UNIVERSAL_CARDS[3],
    [CARD_SACRIFICE]            = &UNIVERSAL_CARDS[4],
    [CARD_REFORGE]              = &UNIVERSAL_CARDS[5],
    [CARD_MERCY]                = &UNIVERSAL_CARDS[6],
    [CARD_BLOODLETTING]         = &UNIVERSAL_CARDS[7],
    [CARD_COUNTER_COUP]         = &UNIVERSAL_CARDS[8],
    [CARD_SPITE]                = &UNIVERSAL_CARDS[9],
    [CARD_CHAIN_BREAK]          = &UNIVERSAL_CARDS[10],
    [CARD_HYDRA]                = &UNIVERSAL_CARDS[11],

    [CARD_RIVER_WADE]           = &LONGWEI_CARDS[0],
    [CARD_CHARGE]               = &LONGWEI_CARDS[1],
    [CARD_FORMATION]            = &LONGWEI_CARDS[2],
    [CARD_DIVINATION]           = &LONGWEI_CARDS[3],
    [CARD_CANNON_VOLLEY]        = &LONGWEI_CARDS[4],
    [CARD_PALACE_DECREE]        = &LONGWEI_CARDS[5],
    [CARD_MANDATE]              = &LONGWEI_CARDS[6],
    [CARD_MINGZHUS_SEAL]        = &LONGWEI_CARDS[7],

    [CARD_RONIN]                = &HARUSHIMA_CARDS[0],
    [CARD_RESURRECTION]         = &HARUSHIMA_CARDS[1],
    [CARD_GOLD_STANDARD]        = &HARUSHIMA_CARDS[2],
    [CARD_PROMOTION]            = &HARUSHIMA_CARDS[3],
    [CARD_DUAL_DROP]            = &HARUSHIMA_CARDS[4],
    [CARD_FORCE_DROP]           = &HARUSHIMA_CARDS[5],
    [CARD_BUSHIDO]              = &HARUSHIMA_CARDS[6],
    [CARD_TOMOHITOS_PATIENCE]   = &HARUSHIMA_CARDS[7],

    [CARD_SULTANS_GOLD]         = &KEWARANI_CARDS[0],
    [CARD_MARCH]                = &KEWARANI_CARDS[1],
    [CARD_DOUBLE_TIME]          = &KEWARANI_CARDS[2],
    [CARD_SALT_ROAD]            = &KEWARANI_CARDS[3],
    [CARD_CARAVAN]              = &KEWARANI_CARDS[4],
    [CARD_DOUBLESTRIKE]         = &KEWARANI_CARDS[5],
    [CARD_HAJJ]                 = &KEWARANI_CARDS[6],
    [CARD_SELASSIES_MARCH]      = &KEWARANI_CARDS[7],

    [CARD_COUNSEL]              = &ZARQAN_CARDS[0],
    [CARD_PILLAGE]              = &ZARQAN_CARDS[1],
    [CARD_ROYAL_DECOY]          = &ZARQAN_CARDS[2],
    [CARD_BAZAAR]               = &ZARQAN_CARDS[3],
    [CARD_STEPPE_RIDERS]        = &ZARQAN_CARDS[4],
    [CARD_AMBITION]             = &ZARQAN_CARDS[5],
    [CARD_CITADEL]              = &ZARQAN_CARDS[6],
    [CARD_CONQUEST]             = &ZARQAN_CARDS[7],
    [CARD_TIMURS_CONQUEST]      = &ZARQAN_CARDS[8],

    [CARD_CASTLING]             = &CAELAN_CARDS[0],
    [CARD_QUEENS_GAMBIT]        = &CAELAN_CARDS[1],
    [CARD_VENGEANCE]            = &CAELAN_CARDS[2],
    [CARD_QUEENS_DECREE]        = &CAELAN_CARDS[3],
    [CARD_CATHEDRAL]            = &CAELAN_CARDS[4],
    [CARD_CORONATION]           = &CAELAN_CARDS[5],
    [CARD_CRUSADE]              = &CAELAN_CARDS[6],
    [CARD_DIVINE_RIGHT]         = &CAELAN_CARDS[7],
    [CARD_ISABELLAS_CORONATION] = &CAELAN_CARDS[8],
};
const BoardTrait* const TRAIT_REGISTRY[BOARD_TRAIT_COUNT] = {
    [BOARD_TRAIT_RIVER_CROSSING]   = &LONGWEI_TRAITS[0],
    [BOARD_TRAIT_THE_PALACE]       = &LONGWEI_TRAITS[1],

    [BOARD_TRAIT_TRADE_ROUTE]      = &KEWARANI_TRAITS[0],
    [BOARD_TRAIT_CONTESTED_MARKET] = &KEWARANI_TRAITS[1],

    [BOARD_TRAIT_SANDSTORM]        = &ZARQAN_TRAITS[0],
    [BOARD_TRAIT_MIRAGE]           = &ZARQAN_TRAITS[1],

    [BOARD_TRAIT_FOG_COAST]        = &HARUSHIMA_TRAITS[0],
    [BOARD_TRAIT_ISLAND_CHAIN]     = &HARUSHIMA_TRAITS[1],

    [BOARD_TRAIT_CASTLE_CORNERS]   = &CAELAN_TRAITS[0],
    [BOARD_TRAIT_SIEGE_TRENCH]     = &CAELAN_TRAITS[1],
};

const KingdomID KINGDOM_ADJACENT[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = KINGDOM_KEWARANI,
    [KINGDOM_KEWARANI]  = KINGDOM_ZARQAN,
    [KINGDOM_ZARQAN]    = KINGDOM_HARUSHIMA,
    [KINGDOM_HARUSHIMA] = KINGDOM_CAELAN,
    [KINGDOM_CAELAN]    = KINGDOM_LONGWEI,
};

/*----------------------------------------------------------------------------*\
                               DISPATCH TABLES
\*----------------------------------------------------------------------------*/

void (*const KINGDOM_OVERSEER[KINGDOM_COUNT])(BattleState*) = {
    longwei_overseer,
    kewarani_overseer,
    zarqan_overseer,
    harushima_overseer,
    caelan_overseer,
};

/*----------------------------------------------------------------------------*\
                                    VORATH
\*----------------------------------------------------------------------------*/

/// eff_vorath_capacity
///
/// The Global Vorath Counter's battle capacity: every two accumulated
/// losses raise the enemy's meter maximum by twenty. Attached to the enemy
/// seat and fired through QUERY_METER_AMOUNT so the capacity feeds the
/// initial meter, refills, and overflow ceiling alike.
///
/// Params:
/// - context -> unused
/// - x       -> int* the enemy meter maximum being computed
///
/// Return: true when capacity was added
///
static bool eff_vorath_capacity(EffectContext* context, void* x) {
    (void) context;

    RunState* run    = battle_run();
    size_t    stacks = run ? run->vorath_counter / 2 : 0;

    if (stacks == 0) {
        return false;
    }

    *(int*) x += (int) (stacks * 20);

    return true;
}

/// VORATH_CAPACITY
///
/// The Global Vorath Counter's opening-capacity template, attached to the
/// enemy seat each battle before the meters compute.
///
static const Effect VORATH_CAPACITY = {
    .func      = eff_vorath_capacity,
    .name      = "Vorath Pressure",
    .trigger   = QUERY_METER_AMOUNT,
    .lasts_for = ENTIRE_BATTLE,
};

/// vorath_attach_capacity
///
/// Attaches the Vorath capacity effect to the given seat, so the counter's
/// pressure raises that side's meter maximum from the moment meters compute.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> seat that carries the pressured maximum
///
void vorath_attach_capacity(BattleState* battle, Side side) {
    effect_attach(&battle_player(battle, side)->effects, &VORATH_CAPACITY);
}

/// vorath_setup
///
/// Sets up the final Vorath battle: the twenty by twenty board, the
/// Grand King, his army, and the quadrant tally effects.
///
/// Params:
/// - battle -> battle to set up
///
void vorath_setup(BattleState* battle) {
    (void) battle;
}
