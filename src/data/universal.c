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

/// eff_sacrifice
///
/// Immediate play effect removing one of the playing side's pieces and
/// gaining twice its value as meter.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when a piece was sacrificed
///
static bool eff_sacrifice(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo*   piece  = battle_at(battle, card_square(context->args[1]));

    if (!piece || piece->side != side || piece->piece->id == PIECE_KING) {
        return false;
    }

    int gain = battle_value(battle, piece, nullptr) * 2;

    battle_remove(battle, piece);
    battle_meter_gain(battle, side, gain);

    return true;
}

/// eff_reforge
///
/// One-shot observer that, when a piece of the playing side flips away,
/// attaches a mark discounting the next purchase of that piece type.
///
/// Params:
/// - context -> beneficiary side in args[0], spent flag in args[3]
/// - x       -> flip candidate pointer
///
/// Return: true when the discount mark was attached
///
static bool eff_reforge(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo**  slot   = x;

    if (context->args[3] || !*slot || (*slot)->side != side) {
        return false;
    }

    Effect mark = {
        .func      = eff_noop,
        .trigger   = ON_TURN_START,
        .lasts_for = TURNS_2
    };
    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &mark);

    if (attached) {
        attached->context->args[1] = (void*) (uintptr_t) CARD_REFORGE;
        attached->context->args[2] = (void*) (uintptr_t) (*slot)->piece->id;
        attached->context->args[4] = (void*) (uintptr_t) battle->turn;
    }

    context->args[3] = (void*) 1;

    return true;
}

/// eff_mercy
///
/// Redirects the playing side's next flip onto a chosen piece, later
/// uses overriding earlier ones by attaching after them.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1],
///              spent flag in args[3]
/// - x       -> flip candidate pointer to redirect
///
/// Return: true when the flip was redirected
///
static bool eff_mercy(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    PieceInfo**  slot   = x;

    if (context->args[3] || !*slot || (*slot)->side != side) {
        return false;
    }

    PieceInfo* target = battle_at(battle, card_square(context->args[1]));

    if (!target || target->side != side || target->piece->id == PIECE_KING) {
        return false;
    }

    *slot            = target;
    context->args[3] = (void*) 1;

    return true;
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

/// eff_chain_break
///
/// Immediate play effect force-flipping one chosen enemy piece.
///
/// Params:
/// - context -> beneficiary side in args[0], target square in args[1]
/// - x       -> played card, unused
///
/// Return: true when an enemy piece was flipped
///
static bool eff_chain_break(EffectContext* context, void* x) {
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
                (int8_t) (king->square.y + dy)
            };

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
            {{.func      = eff_noop,
              .name      = "Pawn Storm",
              .trigger   = ON_TURN_START,
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
            {{.func      = eff_sacrifice,
              .name      = "Sacrifice",
              .trigger   = ON_CARD_PLAY,
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
            {{.func      = eff_mercy,
              .name      = "Mercy",
              .trigger   = ON_PIECE_FLIP_PRE,
              .lasts_for = ENTIRE_BATTLE}},
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
            {{.func      = eff_chain_break,
              .name      = "Chain Break",
              .trigger   = ON_CARD_PLAY,
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

const BattleModifier MODIFIER_REGISTRY[MODIFIER_COUNT]   = {};
const ChainPenalty   CHAIN_REGISTRY[CHAIN_PENALTY_COUNT] = {};

const Piece* const   PIECE_REGISTRY[PIECE_COUNT]         = {
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
    [CARD_PAWN_STORM]    = &UNIVERSAL_CARDS[0],
    [CARD_REVITALIZE]    = &UNIVERSAL_CARDS[1],
    [CARD_HOSTAGE]       = &UNIVERSAL_CARDS[2],
    [CARD_LAST_STAND]    = &UNIVERSAL_CARDS[3],
    [CARD_SACRIFICE]     = &UNIVERSAL_CARDS[4],
    [CARD_REFORGE]       = &UNIVERSAL_CARDS[5],
    [CARD_MERCY]         = &UNIVERSAL_CARDS[6],
    [CARD_BLOODLETTING]  = &UNIVERSAL_CARDS[7],
    [CARD_COUNTER_COUP]  = &UNIVERSAL_CARDS[8],
    [CARD_SPITE]         = &UNIVERSAL_CARDS[9],
    [CARD_CHAIN_BREAK]   = &UNIVERSAL_CARDS[10],
    [CARD_HYDRA]         = &UNIVERSAL_CARDS[11],

    [CARD_RIVER_WADE]    = &LONGWEI_CARDS[0],
    [CARD_CHARGE]        = &LONGWEI_CARDS[1],
    [CARD_FORMATION]     = &LONGWEI_CARDS[2],
    [CARD_DIVINATION]    = &LONGWEI_CARDS[3],
    [CARD_CANNON_VOLLEY] = &LONGWEI_CARDS[4],
    [CARD_PALACE_DECREE] = &LONGWEI_CARDS[5],
    [CARD_MANDATE]       = &LONGWEI_CARDS[6],

    [CARD_RONIN]         = &HARUSHIMA_CARDS[0],
    [CARD_RESURRECTION]  = &HARUSHIMA_CARDS[1],
    [CARD_GOLD_STANDARD] = &HARUSHIMA_CARDS[2],
    [CARD_PROMOTION]     = &HARUSHIMA_CARDS[3],
    [CARD_DUAL_DROP]     = &HARUSHIMA_CARDS[4],
    [CARD_FORCE_DROP]    = &HARUSHIMA_CARDS[5],
    [CARD_BUSHIDO]       = &HARUSHIMA_CARDS[6],

    [CARD_SULTANS_GOLD]  = &KEWARANI_CARDS[0],
    [CARD_MARCH]         = &KEWARANI_CARDS[1],
    [CARD_DOUBLE_TIME]   = &KEWARANI_CARDS[2],
    [CARD_SALT_ROAD]     = &KEWARANI_CARDS[3],
    [CARD_CARAVAN]       = &KEWARANI_CARDS[4],
    [CARD_DOUBLESTRIKE]  = &KEWARANI_CARDS[5],
    [CARD_HAJJ]          = &KEWARANI_CARDS[6],

    [CARD_COUNSEL]       = &ZARQAN_CARDS[0],
    [CARD_PILLAGE]       = &ZARQAN_CARDS[1],
    [CARD_ROYAL_DECOY]   = &ZARQAN_CARDS[2],
    [CARD_BAZAAR]        = &ZARQAN_CARDS[3],
    [CARD_STEPPE_RIDERS] = &ZARQAN_CARDS[4],
    [CARD_AMBITION]      = &ZARQAN_CARDS[5],
    [CARD_CITADEL]       = &ZARQAN_CARDS[6],
    [CARD_CONQUEST]      = &ZARQAN_CARDS[7],

    [CARD_CASTLING]      = &CAELAN_CARDS[0],
    [CARD_QUEENS_GAMBIT] = &CAELAN_CARDS[1],
    [CARD_VENGEANCE]     = &CAELAN_CARDS[2],
    [CARD_QUEENS_DECREE] = &CAELAN_CARDS[3],
    [CARD_CATHEDRAL]     = &CAELAN_CARDS[4],
    [CARD_CORONATION]    = &CAELAN_CARDS[5],
    [CARD_CRUSADE]       = &CAELAN_CARDS[6],
    [CARD_DIVINE_RIGHT]  = &CAELAN_CARDS[7],
};
const BoardTrait* const TRAIT_REGISTRY[BOARD_TRAIT_COUNT] = {};

const KingdomID         KINGDOM_ADJACENT[KINGDOM_COUNT]   = {
    [KINGDOM_LONGWEI]   = KINGDOM_KEWARANI,
    [KINGDOM_KEWARANI]  = KINGDOM_ZARQAN,
    [KINGDOM_ZARQAN]    = KINGDOM_HARUSHIMA,
    [KINGDOM_HARUSHIMA] = KINGDOM_CAELAN,
    [KINGDOM_CAELAN]    = KINGDOM_LONGWEI,
};

const char* const EVENT_NAME[EVENT_COUNT]     = {};
const char* const EVENT_TEXT[EVENT_COUNT]     = {};
const char* const EVENT_OPTION_A[EVENT_COUNT] = {};
const char* const EVENT_OPTION_B[EVENT_COUNT] = {};

/*----------------------------------------------------------------------------*\
                               DISPATCH TABLES
\*----------------------------------------------------------------------------*/

void (*const KINGDOM_INNATE[KINGDOM_COUNT])(BattleState*, Side, MasteryLevel) =
    {
        longwei_innate,
        kewarani_innate,
        zarqan_innate,
        harushima_innate,
        caelan_innate,
};

void (*const KINGDOM_CLIMAX[KINGDOM_COUNT])(BattleState*, Side) = {
    longwei_climax,
    kewarani_climax,
    zarqan_climax,
    harushima_climax,
    caelan_climax,
};

void (*const KINGDOM_OVERSEER[KINGDOM_COUNT])(BattleState*) = {
    longwei_overseer,
    kewarani_overseer,
    zarqan_overseer,
    harushima_overseer,
    caelan_overseer,
};

void (*const KINGDOM_EVENT[KINGDOM_COUNT])(EngineState*, EventID, EventChoice) =
    {
        longwei_event,
        kewarani_event,
        zarqan_event,
        harushima_event,
        caelan_event,
};

/*----------------------------------------------------------------------------*\
                                    VORATH
\*----------------------------------------------------------------------------*/

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
