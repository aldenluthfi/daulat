//! representation.h
//!
//! This file contains all the game entity definitions including enums for
//! pieces, cards, relics, modifiers, and structs for battle state, run state,
//! and the game engine. It is the central type definitions file for the game.
//!
//! Created: 12/06/2026
//! Author : Alden Luthfi

/*----------------------------------------------------------------------------*\
                                 EFFECT.C ENUMS
\*----------------------------------------------------------------------------*/

/// EffectTrigger
///
/// This enum enumerates all the triggers that can activate an effect.
/// Triggers are conditions that cause effects to fire during gameplay.
/// The void* x passed to every effect function is the value the trigger
/// is computing: it starts at the base value and mutates in place as it
/// passes through every matching effect. The concrete type behind x per
/// trigger:
///
/// QUERY_CARD_DRAW_COUNT           x: int*        base 3
/// QUERY_CARD_PLAY_COST            x: int*        card play_cost
/// QUERY_CARD_SELL_COST            x: int*        card sell_cost
/// QUERY_CARD_CAN_DRAW             x: bool*       base true
/// QUERY_CARD_CAN_PLAY             x: bool*       base true; effects veto
///                                                by setting false
/// QUERY_CARD_TARGETS              x: CardTarget* a targeting card fills
///                                                its legal targets here,
///                                                TARGET_NONE terminated
/// QUERY_PIECE_ACTION_COST_MOVE    x: int*        base 1
/// QUERY_PIECE_ACTION_COST_BUY     x: int*        base 1
/// QUERY_PIECE_ACTION_COST_COMBINE x: int*        base 1
/// QUERY_PIECE_CP_COST_BUY         x: int*        effective value with
///                                                home/foreign pre-applied
/// QUERY_PIECE_CP_COST_RECLAIM     x: int*        base 30
/// QUERY_PIECE_CAN_FLIP            x: bool*       true; subject candidate
/// QUERY_PIECE_CAN_MOVE            x: bool*       base true
/// QUERY_PIECE_CAN_ATTACK          x: bool*       base true
/// QUERY_PIECE_CAN_BUY             x: bool*       base true; the human buy
///                                                is refused when set false;
///                                                piece via battle_buy_piece()
/// QUERY_PIECE_MOVES               x: Square*     list from mv, edited in
///                                                place, SQUARE_END ended
/// QUERY_PIECE_ATTACKS             x: Square*     list from at, same rule
/// QUERY_PIECE_DAMAGE_DEALT        x: int*        subject piece value;
///                                                victim nullptr means
///                                                effective-value query
/// QUERY_PIECE_DAMAGE_TAKEN        x: int*        post-offense damage;
///                                                subject is the victim
/// QUERY_PIECE_VALUE               x: int*        a spawning piece's value;
///                                                run value bonuses add to it,
///                                                fired for the piece's side
/// QUERY_PIECE_HAS_MOVED           x: int*        subject's move count is
///                                                added; base 0
/// QUERY_PIECE_HAS_FLIPPED         x: int*        subject's flip count is
///                                                added; base 0
/// QUERY_METER_DAMAGE_TAKEN        x: int*        total resolve damage;
///                                                side is the receiver
/// QUERY_METER_REFILL              x: int*        battle_meter_max result
/// QUERY_METER_AMOUNT              x: int*        a side's meter maximum, base
///                                                the piece-value sum; capacity
///                                                effects add to it before the
///                                                maximum is used
/// QUERY_FLIP_COUNT                x: int*        pieces flipped per meter
///                                                empty; base 1, fired for
///                                                the emptied side
/// QUERY_CP_INCOME                 x: int*        the acting side's per-turn
///                                                income, base 10; opening
///                                                penalties reduce the first
///                                                turn's amount
/// QUERY_ENEMY_ARMY_COUNT          x: int*        free enemy reinforcements;
///                                                fired for the human seat,
///                                                penalties add to the count
/// QUERY_SQUARE_OWNER              x: Side*       holding side; base is the
///                                                nearest-piece Chebyshev
///                                                read (SIDE_NEUTRAL when
///                                                contested); square via
///                                                battle_owner_square()
/// QUERY_BOARD_DIMENSION           x: Square*     board {width, height} in
///                                                {x, y}, edited in place
/// QUERY_BOARD_STATE               x: Board*      board.visible is the human's
///                                                per-cell piece read; fog
///                                                effects hide enemy pieces
/// QUERY_HAND_STATE                x: PlayerState* the human seat; blinder
///                                                effects clear hand_visible[]
///                                                to mask a card's identity
/// ON_BOARD_BUILD                  x: Board*      the built board; effects
///                                                scatter voids onto it
/// ON_PIECE_FLIP_PRE               x: PieceInfo** pre-toggle; mutate *x
///                                                to redirect the flip or
///                                                set nullptr to consume
/// ON_PIECE_FLIP                   x: PieceInfo*  right after the side
///                                                toggle; damagers live
/// ON_PIECE_FLIP_POST              x: PieceInfo*  after the cascade step
///                                                settles (refill and
///                                                consume spawns done)
/// ON_PIECE_BUY                    x: PieceInfo*  piece just bought
/// ON_PIECE_MOVE                   x: PieceInfo*  moved piece; origin via
///                                                battle_move_from()
/// ON_PIECE_COMBINE                x: PieceInfo*  combination result
/// ON_CARD_PLAY                    x: Card*       card just played
/// ON_CARD_SELL                    x: Card*       card just sold
/// ON_CARDS_DRAWN                  x: Card**       the hand just filled;
///                                                effects rewrite drawn
///                                                cards in place
/// ON_CARD_TARGET_SELECTED         x: CardTarget* the chosen target; the
///                                                card resolves against it
/// ON_COMBO_DOUBLE                 x: KingdomID*  the kingdom whose second
///                                                same-kingdom card just
///                                                played; the acting side's
///                                                combo refund and any
///                                                double-combo effects fire
/// ON_COMBO_CLIMAX                 x: KingdomID*  the kingdom whose third
///                                                same-kingdom card just
///                                                played; the acting side's
///                                                climax for that kingdom
///                                                fires and resolves
/// ON_TURN_START                   x: uintptr_t   current turn number
/// ON_TURN_END                     x: uintptr_t   current turn number
/// ON_BATTLE_SETUP                 x: MapNode*    battle location node, fired
///                                                for the human seat before
///                                                the meters compute so setup
///                                                actions fold into the maxima
/// ON_BATTLE_START                 x: MapNode*    battle location node
/// ON_BATTLE_END                   x: uintptr_t   winning Side
/// ON_CASCADE_END                  x: int*        the lowest meter the damage
///                                                and cascade transaction drove
///                                                the receiver to; fired once
///                                                per transaction for the
///                                                receiver after the cascade
/// ON_EVENT_CHOOSE                 x: EngineState* the run; the chosen event
///                                                option's run-immediate
///                                                effects act on run state
/// QUERY_EVENT_TARGETS             x: CardTarget* a targeting event option
///                                                fills one selection step's
///                                                candidates, one slot per
///                                                step, mirroring
///                                                QUERY_CARD_TARGETS
/// ON_EVENT_TARGET_SELECTED        x: CardTarget* the TARGET_NONE terminated
///                                                picks; the option resolves
///                                                against the chosen run
///                                                targets
/// QUERY_ARCHIVE_REVEAL_COUNT      x: int*        base 1; archive reveal count
///                                                folded over held relics
///
enum EffectTrigger {
    QUERY_CARD_DRAW_COUNT,
    QUERY_CARD_PLAY_COST,
    QUERY_CARD_SELL_COST,
    QUERY_CARD_CAN_DRAW,
    QUERY_CARD_CAN_PLAY,
    QUERY_CARD_TARGETS,

    QUERY_PIECE_ACTION_COST_MOVE,
    QUERY_PIECE_ACTION_COST_BUY,
    QUERY_PIECE_ACTION_COST_COMBINE,

    QUERY_PIECE_CP_COST_BUY,
    QUERY_PIECE_CP_COST_RECLAIM,

    QUERY_PIECE_CAN_FLIP,
    QUERY_PIECE_CAN_MOVE,
    QUERY_PIECE_CAN_ATTACK,
    QUERY_PIECE_CAN_BUY,

    QUERY_PIECE_MOVES,
    QUERY_PIECE_ATTACKS,

    QUERY_PIECE_DAMAGE_DEALT,
    QUERY_PIECE_DAMAGE_TAKEN,
    QUERY_PIECE_VALUE,

    QUERY_PIECE_HAS_MOVED,
    QUERY_PIECE_HAS_FLIPPED,

    QUERY_METER_DAMAGE_TAKEN,
    QUERY_METER_REFILL,
    QUERY_METER_AMOUNT,
    QUERY_FLIP_COUNT,

    QUERY_CP_INCOME,
    QUERY_ENEMY_ARMY_COUNT,
    QUERY_SQUARE_OWNER,

    QUERY_BOARD_DIMENSION,
    QUERY_BOARD_STATE,
    QUERY_HAND_STATE,
    ON_BOARD_BUILD,

    ON_PIECE_FLIP_PRE,
    ON_PIECE_FLIP,
    ON_PIECE_FLIP_POST,
    ON_PIECE_BUY,
    ON_PIECE_MOVE,
    ON_PIECE_COMBINE,

    ON_CARD_PLAY,
    ON_CARD_SELL,
    ON_CARDS_DRAWN,
    ON_CARD_TARGET_SELECTED,
    ON_COMBO_DOUBLE,
    ON_COMBO_CLIMAX,

    ON_TURN_START,
    ON_TURN_END,

    ON_BATTLE_SETUP,
    ON_BATTLE_START,
    ON_BATTLE_END,
    ON_CASCADE_END,

    ON_EVENT_CHOOSE,
    QUERY_EVENT_TARGETS,
    ON_EVENT_TARGET_SELECTED,

    QUERY_ARCHIVE_REVEAL_COUNT,
};

/// EffectDuration
///
/// This enum enumerates all possible effect durations. A TURNS_n effect is
/// removed at the start of its list side's n-th turn after attachment, so it
/// stays live across the intervening opponent halves. An effect attached
/// during a side's own turn therefore lasts n full rounds; one attached onto
/// the opponent's list covers that opponent's next n turns. ONE_BATTLE lasts
/// a whole battle like ENTIRE_BATTLE, but a run-persistent event effect
/// carrying it is consumed after that one battle (not re-attached again).
///
enum EffectDuration {
    TURNS_1,
    TURNS_2,
    TURNS_3,
    TURNS_4,
    TURNS_5,
    TURNS_6,
    TURNS_7,
    TURNS_8,
    TURNS_9,
    TURNS_10,
    ENTIRE_BATTLE,
    ONE_BATTLE,
    ENTIRE_RUN,
};

/// EffectItemType
///
/// This enum enumerates all the types of items that can have effects attached.
///
enum EffectItemType {
    CARD,
    RELIC,
    BATTLE_MODIFIER,
    BOARD_TRAIT,
    CHAIN_PENALTY
};

/*----------------------------------------------------------------------------*\
                                  PIECE.C ENUMS
\*----------------------------------------------------------------------------*/

/// PieceID
///
/// This enum enumerates all pieces in the game across all five kingdoms.
/// Pieces include base types, combined pieces, and special capstone pieces.
///
enum PieceID {
    PIECE_KING,

    PIECE_BING,
    PIECE_XIANG,
    PIECE_MA,
    PIECE_PAO,
    PIECE_LIUBO_DIVINER,
    PIECE_SANG,
    PIECE_NORTHERN_CAVALRY,
    PIECE_HWACHA,

    PIECE_FUHYO,
    PIECE_KYOSHA,
    PIECE_GINSHO,
    PIECE_KINSHO,
    PIECE_SHISHI,
    PIECE_HONORABLE_HORSE,
    PIECE_PROMOTED_BISHOP,
    PIECE_DAIMYO,
    PIECE_DRAGON,

    PIECE_MEDEQ,
    PIECE_MAKWANAM,
    PIECE_SABA,
    PIECE_FARAS,
    PIECE_NEGUS_GUARD,
    PIECE_MEDEQ_SQUAD,
    PIECE_SULTANS_LEVY,

    PIECE_WAZIR,
    PIECE_JAMAL,
    PIECE_TALLIYA,
    PIECE_ZIRAAFA,
    PIECE_SHAHZADEH,
    PIECE_OLD_KING,
    PIECE_CATAPHRACT,
    PIECE_ROOK,
    PIECE_WAR_ELEPHANT,

    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_QUEEN,
    PIECE_GRYPHON,
    PIECE_CHANCELLOR,
    PIECE_SOVEREIGN_BANNER,

    PIECE_COUNT,
    PIECE_NONE,
};

/// Side
///
/// This enum enumerates the sides a piece or player can belong to during
/// battle. Neutral pieces belong to neither player and never resolve.
///
enum Side {
    SIDE_WHITE,
    SIDE_BLACK,
    SIDE_NEUTRAL,
};

/// MoveClass
///
/// This enum classifies pieces by their movement pattern. Effects that
/// alter movement, such as auras and board traits, filter their subjects
/// by class.
///
enum MoveClass {
    MOVE_LEAPER,
    MOVE_SLIDER,
    MOVE_SPECIAL,
};

/*----------------------------------------------------------------------------*\
                                 CARD.C ENUMS
\*----------------------------------------------------------------------------*/

/// CardID
///
/// This enum enumerates all cards in the game across all kingdoms and tiers.
///
enum CardID {
    CARD_PAWN_STORM,
    CARD_REVITALIZE,
    CARD_HOSTAGE,
    CARD_LAST_STAND,
    CARD_SACRIFICE,
    CARD_REFORGE,
    CARD_MERCY,
    CARD_BLOODLETTING,
    CARD_COUNTER_COUP,
    CARD_SPITE,
    CARD_CHAIN_BREAK,
    CARD_HYDRA,

    CARD_RIVER_WADE,
    CARD_CHARGE,
    CARD_FORMATION,
    CARD_DIVINATION,
    CARD_CANNON_VOLLEY,
    CARD_PALACE_DECREE,
    CARD_MANDATE,

    CARD_RONIN,
    CARD_RESURRECTION,
    CARD_GOLD_STANDARD,
    CARD_PROMOTION,
    CARD_DUAL_DROP,
    CARD_FORCE_DROP,
    CARD_BUSHIDO,

    CARD_SULTANS_GOLD,
    CARD_MARCH,
    CARD_DOUBLE_TIME,
    CARD_SALT_ROAD,
    CARD_CARAVAN,
    CARD_DOUBLESTRIKE,
    CARD_HAJJ,

    CARD_COUNSEL,
    CARD_PILLAGE,
    CARD_ROYAL_DECOY,
    CARD_BAZAAR,
    CARD_STEPPE_RIDERS,
    CARD_AMBITION,
    CARD_CITADEL,
    CARD_CONQUEST,

    CARD_CASTLING,
    CARD_QUEENS_GAMBIT,
    CARD_VENGEANCE,
    CARD_QUEENS_DECREE,
    CARD_CATHEDRAL,
    CARD_CORONATION,
    CARD_CRUSADE,
    CARD_DIVINE_RIGHT,

    CARD_MINGZHUS_SEAL,
    CARD_TOMOHITOS_PATIENCE,
    CARD_SELASSIES_MARCH,
    CARD_TIMURS_CONQUEST,
    CARD_ISABELLAS_CORONATION,

    CARD_COUNT
};

/// UnlockTier
///
/// This enum enumerates the progression tiers that gate when pieces and
/// cards become available, following the map ladder from district to
/// country plus the mastery reward pool.
///
enum UnlockTier {
    TIER_DISTRICT,
    TIER_TOWN,
    TIER_PROVINCE,
    TIER_COUNTRY,
    TIER_MASTERY,
};

/// TargetKind
///
/// Kinds of target a card or event can advertise through QUERY_CARD_TARGETS
/// or QUERY_EVENT_TARGETS. TARGET_PIECE and TARGET_SQUARE both carry a board
/// square in value (y * 20 + x); the effect derives whatever it needs from
/// it (a file is the square's column). TARGET_CARD carries a hand slot in
/// battle or a CardID in a run event; TARGET_PIECE_TYPE a PieceID;
/// TARGET_RELIC a RelicID; TARGET_NODE a map node index; TARGET_FIGUREHEAD a
/// KingdomID (a new run domain the four card kinds do not cover).
/// TARGET_NONE terminates a target list. New kinds extend the enum without
/// touching the protocol, which only relays a chosen index.
///
enum TargetKind {
    TARGET_PIECE,
    TARGET_SQUARE,
    TARGET_CARD,
    TARGET_PIECE_TYPE,
    TARGET_RELIC,
    TARGET_NODE,
    TARGET_FIGUREHEAD,
    TARGET_NONE,
};

/*----------------------------------------------------------------------------*\
                                 MASTERY.C ENUMS
\*----------------------------------------------------------------------------*/

/// MasteryLevel
///
/// This enum enumerates all mastery levels for figureheads. Mastery advances
/// when completing a run without chains and defeating Vorath.
///
enum MasteryLevel {
    MASTERY_NONE,

    MASTERY_LEVEL_1,
    MASTERY_LEVEL_2,
    MASTERY_LEVEL_3,
};

/*----------------------------------------------------------------------------*\
                                 KINGDOM.C ENUMS
\*----------------------------------------------------------------------------*/

/// KingdomID
///
/// This enum enumerates all five kingdoms in the game. Each kingdom has
/// unique pieces, cards, and an overseer battle.
///
enum KingdomID {
    KINGDOM_LONGWEI,
    KINGDOM_KEWARANI,
    KINGDOM_ZARQAN,
    KINGDOM_HARUSHIMA,
    KINGDOM_CAELAN,

    KINGDOM_COUNT,
    KINGDOM_NONE,
};

/*----------------------------------------------------------------------------*\
                                  RELIC.C ENUMS
\*----------------------------------------------------------------------------*/

/// RelicID
///
/// This enum enumerates all relics that are in the game, the relics are
/// separated into categories based on what aspect they affect, in order:
///
/// 1. Economy
/// 2. Meter
/// 3. Cards
/// 4. Combinations
/// 5. Board
///
enum RelicID {
    RELIC_MERCHANTS_LEDGER,
    RELIC_MINTED_COIN,
    RELIC_TAX_STAMP,
    RELIC_BULK_DISCOUNT,
    RELIC_WAR_CHEST,
    RELIC_TRADE_ROUTES,

    RELIC_SOUL_SHARD,
    RELIC_VETERANS_BOND,
    RELIC_DEAD_MANS_PACT,
    RELIC_IRON_KING,
    RELIC_BLOODTHIRST,
    RELIC_LAST_BREATH,

    RELIC_TACTICIANS_SCROLL,
    RELIC_LIBRARIANS_NOTES,
    RELIC_COUNTRY_SEAL,
    RELIC_DEEP_HAND,
    RELIC_GILDED_ARCHIVE,

    RELIC_ALCHEMISTS_KIT,
    RELIC_MASTERS_NOTES,
    RELIC_PHILOSOPHERS_STONE,
    RELIC_INHERITED_POWER,

    RELIC_EAGLE_EYE,
    RELIC_SURVEYORS_MAP,
    RELIC_FORWARD_COMMAND,
    RELIC_FORTIFIED_LINE,
    RELIC_WARLORDS_BANNER,

    RELIC_COUNT,
};

/*----------------------------------------------------------------------------*\
                                MODIFIER.C ENUMS
\*----------------------------------------------------------------------------*/

/// BattleModifierID
///
/// This enum enumerates all battle modifiers that can affect a battle.
/// Modifiers are drawn per battle and revealed before entry.
///
enum BattleModifierID {
    MODIFIER_LEAN_TIMES,
    MODIFIER_WINDFALL,
    MODIFIER_OPEN_MARKET,
    MODIFIER_DEVALUED_CURRENCY,
    MODIFIER_TAX_COLLECTOR,

    MODIFIER_GLASS_CANNON,
    MODIFIER_BLOODBATH,
    MODIFIER_IRON_WILL,
    MODIFIER_OVERFLOW,
    MODIFIER_MIRROR,

    MODIFIER_RICH_HAND,
    MODIFIER_SPARSE_HAND,
    MODIFIER_KINGDOM_PURITY,
    MODIFIER_LUCKY_STRIKE,

    MODIFIER_FOG_OF_WAR,
    MODIFIER_DENSE_TERRAIN,
    MODIFIER_EXTENDED_FRONT,
    MODIFIER_COMPRESSED,

    MODIFIER_COUNT,
};

/*----------------------------------------------------------------------------*\
                               BOARD_TRAIT.C ENUMS
\*----------------------------------------------------------------------------*/

/// BoardTraitID
///
/// This enum enumerates all board traits that can appear in battles.
///
enum BoardTraitID {
    BOARD_TRAIT_RIVER_CROSSING,
    BOARD_TRAIT_THE_PALACE,

    BOARD_TRAIT_TRADE_ROUTE,
    BOARD_TRAIT_CONTESTED_MARKET,

    BOARD_TRAIT_SANDSTORM,
    BOARD_TRAIT_MIRAGE,

    BOARD_TRAIT_FOG_COAST,
    BOARD_TRAIT_ISLAND_CHAIN,

    BOARD_TRAIT_CASTLE_CORNERS,
    BOARD_TRAIT_SIEGE_TRENCH,

    BOARD_TRAIT_COUNT,
};

/*----------------------------------------------------------------------------*\
                             CAMPAIGN_MAP.C ENUMS
\*----------------------------------------------------------------------------*/

/// MapNodeID
///
/// This enum enumerates all node types that can appear on the campaign map.
///
enum MapNodeID {
    MAP_NODE_BATTLE,
    MAP_NODE_ELITE,
    MAP_NODE_ARCHIVE,
    MAP_NODE_OFFERING,
    MAP_NODE_EVENT,
    MAP_NODE_OVERSEER,
    MAP_NODE_LIBERATION,
};

/// MapTypeID
///
/// This enum enumerates all map types in the campaign.
///
enum MapTypeID {
    MAP_TOWN,
    MAP_PROVINCE,
    MAP_COUNTRY,
};

/*----------------------------------------------------------------------------*\
                                   AI.C ENUMS
\*----------------------------------------------------------------------------*/

/// AIArchetypeID
///
/// This enum enumerates all AI archetypes for enemy kingdoms.
///
enum AIArchetypeID {
    AI_SIEGE_ENGINEER,
    AI_THE_TIDE,
    AI_TRICKSTER,
    AI_RECLAIMER,
    AI_THE_HAMMER,

    AI_ARCHETYPE_COUNT,
};

/*----------------------------------------------------------------------------*\
                               DIFFICULTY.C ENUMS
\*----------------------------------------------------------------------------*/

/// Difficulty
///
/// This enum enumerates all difficulty levels. Higher difficulties add
/// penalties and are unlocked after defeating the previous level once.
/// DIFFICULTY_NONE marks a profile that has beaten nothing yet.
///
enum Difficulty {
    DIFFICULTY_FREE,
    DIFFICULTY_BOUND,
    DIFFICULTY_SHACKLED,
    DIFFICULTY_ENSLAVED,

    DIFFICULTY_NONE,
};

/*----------------------------------------------------------------------------*\
                              CHALLENGE_RUN.C ENUMS
\*----------------------------------------------------------------------------*/

/// ChallengeRunID
///
/// This enum enumerates all challenge run modifiers. CHALLENGE_NONE
/// marks runs started without a challenge.
///
enum ChallengeRunID {
    CHALLENGE_DAILY_CONQUEST,
    CHALLENGE_SOLO_VANGUARD,
    CHALLENGE_PACIFIST_DOCTRINE,
    CHALLENGE_BLIND_DRAFT,
    CHALLENGE_THE_TRAITORS_GAMBIT,
    CHALLENGE_CLOCKWORK,

    CHALLENGE_COUNT,
    CHALLENGE_NONE,
};

/*----------------------------------------------------------------------------*\
                             CHAIN_PENALTY.C ENUMS
\*----------------------------------------------------------------------------*/

/// ChainPenaltyID
///
/// This enum enumerates all chain penalty levels.
///
enum ChainPenaltyID {
    CHAIN_NONE,
    CHAIN_BRONZE,
    CHAIN_SILVER,
    CHAIN_GOLD,

    CHAIN_PENALTY_COUNT,
};

/*----------------------------------------------------------------------------*\
                              OVERSEER_TYPE.C ENUMS
\*----------------------------------------------------------------------------*/

/// OverseerTypeID
///
/// This enum enumerates all overseer types.
///
enum OverseerTypeID {
    OVERSEER_IRON_STRATEGIST,
    OVERSEER_CARAVAN_OF_CONQUEST,
    OVERSEER_MANY_FACED_KING,
    OVERSEER_ETERNAL_RECURSION,
    OVERSEER_CROWNED_HERETIC,

    OVERSEER_COUNT,
};

/*----------------------------------------------------------------------------*\
                                 EVENT.C ENUMS
\*----------------------------------------------------------------------------*/

/// EventID
///
/// This enum enumerates all narrative events that can appear on the campaign
/// map. Events are grouped by kingdom: Longwei (5), Kewarani (5), Zarqan (5),
/// Harushima (5), Caelan (6), Universal (4).
///
enum EventID {
    EVENT_SCHOLARS_OFFER,
    EVENT_DRAGON_COURT_TRIBUTE,
    EVENT_DEFECTOR,
    EVENT_JANGGI_ELDER,
    EVENT_CANNON_SALUTE,

    EVENT_MANSAS_COURT,
    EVENT_SALT_ROAD_MERCHANT,
    EVENT_STOLEN_GUARD,
    EVENT_CAMEL_CARAVAN,
    EVENT_FEAST_OF_YOD_ABEGA,

    EVENT_WARLORDS_CHALLENGE,
    EVENT_BAZAAR_OF_SAMARKAND,
    EVENT_MIRAGE,
    EVENT_SPY_REPORT,
    EVENT_DESERT_CROSSING,

    EVENT_RONIN,
    EVENT_SPY_NETWORK,
    EVENT_BURNING_PORT,
    EVENT_FORGE_MASTER,
    EVENT_VETERAN_LANCE,

    EVENT_TOURNAMENT,
    EVENT_CHURCH_BLESSING,
    EVENT_SIEGE_ENGINEER,
    EVENT_PRETENDER,
    EVENT_ROYAL_DECREE,
    EVENT_QUEENS_FAVOR,

    EVENT_WANDERING_PIECE,
    EVENT_VORATHS_DECREE,
    EVENT_DESERTER,
    EVENT_ARCHIVE,

    EVENT_COUNT,
};

/// EventChoice
///
/// This enum enumerates the possible choices in narrative events.
///
enum EventChoice {
    NO_CHOICE,
    CHOICE_A,
    CHOICE_B,
};

/*----------------------------------------------------------------------------*\
                                EFFECT.C STRUCTS
\*----------------------------------------------------------------------------*/

/// MAX_EFFECT_ARGS
///
/// Maximum number of arguments that can be passed to an effect context.
///
#define MAX_EFFECT_ARGS  16

/// MAX_EFFECT_COUNT
///
/// Maximum number of effects that can be attached to a single effect item.
///
#define MAX_EFFECT_COUNT 8

/// EFFECT_ITEM_BASE
///
/// Base struct macro for items that can have effects attached. Shared by
/// Card, Relic, BattleModifier, BoardTrait, and ChainPenalty.
///
#define EFFECT_ITEM_BASE                                                       \
    Effect effects[MAX_EFFECT_COUNT];                                          \
    char*  name;                                                               \
    char*  desc

/// EffectContext
///
/// Holds arguments passed to an effect when it triggers. Effects receive
/// context-specific data through this structure.
///
struct EffectContext {
    void* args[MAX_EFFECT_ARGS];
};

/// Effect
///
/// Represents a single effect that can be attached to various game items.
/// func returns whether the effect actually applied: self-filtered
/// invocations return false. The name identifies the effect in the
/// protocol's effect-fire log lines, emitted only for applied fires;
/// templates set it and attach copies it.
///
struct Effect {
    bool           (*func)(EffectContext* context, void* x);
    char*          name;

    EffectTrigger  trigger;
    EffectDuration lasts_for;
    EffectContext* context;
};

/*----------------------------------------------------------------------------*\
                                BOARD.C STRUCTS
\*----------------------------------------------------------------------------*/

/// MAX_BOARD_SIZE
///
/// Maximum board size in squares (20x20 Vorath board).
///
#define MAX_BOARD_SIZE (20 * 20)

/// Board
///
/// Represents the game board with its cells and trait. Cells are indexed
/// y * 20 + x regardless of the active width. A cell holds the live piece
/// occupying it, nullptr when empty, or &VOID_CELL when the square does
/// not exist on this board. visible is the human's per-cell piece read,
/// reset all-true and refilled by QUERY_BOARD_STATE on each board emit; fog
/// effects clear a cell to hide the enemy piece on it (it renders as empty).
///
struct Board {
    PieceInfo*  piece_board[MAX_BOARD_SIZE];
    BoardTrait* trait;

    int8_t      width;
    int8_t      height;

    bool        visible[MAX_BOARD_SIZE];
};

/// Square
///
/// Represents a position on the board using x/y coordinates.
///
struct Square {
    int8_t x;
    int8_t y;
};

/// SQUARE_END
///
/// Terminator value for Square lists produced by movement generation.
/// Offset and direction arrays fed to mg_leap and mg_slide terminate
/// with the zero vector instead, since up-left { -1, -1 } is itself a
/// legitimate offset while no piece ever has a zero offset.
///
#define SQUARE_END ((Square) {-1, -1})

/*----------------------------------------------------------------------------*\
                                PIECE.C STRUCTS
\*----------------------------------------------------------------------------*/

/// Piece
///
/// Represents a piece definition with its movement and attack functions.
/// Registry entries are const templates; battle_spawn heap-copies the
/// template so per-instance fields such as value may mutate during battle.
///
struct Piece {
    Square* (*at)(BattleState* battle_state, PieceInfo* self);
    Square* (*mv)(BattleState* battle_state, PieceInfo* self);

    EFFECT_ITEM_BASE;

    PieceID    id;
    KingdomID  kingdom;
    UnlockTier tier;
    MoveClass class;

    int value;
};

/// PieceInfo
///
/// Runtime information about a live piece on the board including its
/// owned heap copy of the piece template, position, and side. Allocated
/// on the heap per live piece so its pointer is a stable identity;
/// flipping a piece toggles side in place.
///
struct PieceInfo {
    Piece* piece;
    Square square;
    Side   side;
};

/*----------------------------------------------------------------------------*\
                                CARD.C STRUCTS
\*----------------------------------------------------------------------------*/

/// Card
///
/// A card that can be drawn and played during battle. A play_cost of
/// zero means the card is free to play.
///
struct Card {
    EFFECT_ITEM_BASE;

    CardID     id;
    UnlockTier tier;
    KingdomID  kingdom;

    int        play_cost;
    int        sell_cost;
};

/// CardTarget
///
/// One legal target a card advertises through QUERY_CARD_TARGETS: a kind
/// and an int value read per kind (TARGET_PIECE and TARGET_SQUARE =
/// y * 20 + x, TARGET_CARD = hand slot, TARGET_PIECE_TYPE = PieceID).
///
struct CardTarget {
    TargetKind kind;
    int        value;
};

/*----------------------------------------------------------------------------*\
                                   KINGDOM.C
\*----------------------------------------------------------------------------*/

/// KingdomState
///
/// Tracks the state of a kingdom including its mastery level, chain penalties,
///
/// and maps for each tier. ever_chained records whether this kingdom was
/// chained at any point during the current run, since mastery requires a
/// chainless run even after the chain pointer clears on a win.
///
struct KingdomState {
    KingdomID     id;
    MasteryLevel  mastery;
    ChainPenalty* chain;
    bool          ever_chained;

    MapState*     town_map;
    MapState*     province_map;
    MapState*     country_map;
};

/*----------------------------------------------------------------------------*\
                                RELIC.C STRUCTS
\*----------------------------------------------------------------------------*/

/// Relic
///
/// A run-long passive bonus acquired from battles and events.
///
struct Relic {
    EFFECT_ITEM_BASE;

    RelicID id;
};

/*----------------------------------------------------------------------------*\
                              MODIFIER.C STRUCTS
\*----------------------------------------------------------------------------*/

/// BattleModifier
///
/// A battle-wide modifier affecting economy, meter, cards, or board.
///
struct BattleModifier {
    EFFECT_ITEM_BASE;

    BattleModifierID id;
};

/*----------------------------------------------------------------------------*\
                            BOARD_TRAIT.C STRUCTS
\*----------------------------------------------------------------------------*/

/// BoardTrait
///
/// A board modifier affecting piece movement or combat in specific ways.
///
struct BoardTrait {
    EFFECT_ITEM_BASE;

    BoardTraitID id;
    Board*       board;
};

/*----------------------------------------------------------------------------*\
                            CAMPAING_MAP.C STRUCTS
\*----------------------------------------------------------------------------*/

#define RELIC_CHOICE(A, B) ((A << 8) | B)

/// MapNode
///
/// A single node on the campaign map representing a battle, elite,
/// archive, offering, or event. The name and content are authored in the
/// static layout tables and copied in at generation; content is
/// interpreted by the type: an EventID for events, a result PieceID for
/// archives, an OverseerTypeID for overseers, or a packed relic pair
/// (a << 8 | b) for elites; unused otherwise.
///
struct MapNode {
    MapNodeID       type;
    const char*     name;
    int             content;

    MapState*       map;
    KingdomState*   kingdom;
    BoardTrait*     trait;
    BattleModifier* modifier;

    bool            revealed;
    bool            cleared;
};

/// MapState
///
/// Represents an entire map for a kingdom and tier combination.
///
struct MapState {
    MapTypeID     type;

    KingdomState* kingdom;
    DirectedGraph nodes;
};

/// EventOption
///
/// One choice of a narrative event: its option text and the effects it
/// carries. An ON_EVENT_CHOOSE effect acts on run state the instant the
/// option is taken (x = EngineState*); any other trigger is a run-persistent
/// effect re-attached to the human seat each battle from the recorded
/// choice.
///
struct EventOption {
    const char* text;
    Effect      effects[MAX_EFFECT_COUNT];
};

/// Event
///
/// A narrative event as a data item: name, flavour text, and its two
/// options, replacing the empty EVENT_NAME/TEXT/OPTION parallel arrays and
/// the KINGDOM_EVENT stub dispatch.
///
struct Event {
    const char* name;
    const char* desc;

    EventID     id;
    EventOption options[2];
};

/*----------------------------------------------------------------------------*\
                             CHAIN_PENALTY.C STRUCTS
\*----------------------------------------------------------------------------*/

/// ChainPenalty
///
/// A progression penalty representing accumulated battle losses.
///
struct ChainPenalty {
    EFFECT_ITEM_BASE;

    ChainPenaltyID id;
};

/// DifficultyMode
///
/// A run-long difficulty whose penalties compose through the battle
/// triggers, attached to the human seat at battle start.
///
struct DifficultyMode {
    EFFECT_ITEM_BASE;

    Difficulty id;
};

/// ChallengeRun
///
/// A run-long challenge whose constraints compose through the battle
/// triggers, attached to the human seat at battle start.
///
struct ChallengeRun {
    EFFECT_ITEM_BASE;

    ChallengeRunID id;
};

/// KingdomPower
///
/// A kingdom's synergy, innate, or combo climax as an effect item indexed
/// by kingdom and attached generically at battle start. A synergy or innate
/// attaches to a seat (innate carries its mastery in args[1]); a climax
/// attaches to both seats and self-filters on the played kingdom carried in
/// ON_COMBO_CLIMAX.
///
struct KingdomPower {
    EFFECT_ITEM_BASE;

    KingdomID id;
};

/*----------------------------------------------------------------------------*\
                                BATTLE.C STRUCTS
\*----------------------------------------------------------------------------*/

/// MAX_DRAWN_CARDS
///
/// Maximum number of cards that can be drawn in a single hand.
///
#define MAX_DRAWN_CARDS 10

/// PlayerState
///
/// Tracks the state of a player during battle including currency, meter,
///
/// actions, hand, and active effects.
///
struct PlayerState {
    int        cp;
    int        meter;
    int        actions;

    Card*      hand[MAX_DRAWN_CARDS];
    bool       hand_visible[MAX_DRAWN_CARDS];
    LinkedList effects;
};

/// BattleState
///
/// The complete state of a battle including turn count, board, modifier,
///
/// and both player states. node points at the campaign map node the
/// battle takes place on, giving access to kingdom pricing, board trait,
/// and node type.
///
struct BattleState {
    Board           board;

    PlayerState     white;
    PlayerState     black;

    BattleModifier* modifier;
    MapNode*        node;
    size_t          turn;
};

/*----------------------------------------------------------------------------*\
                                 RUN.C STRUCTS
\*----------------------------------------------------------------------------*/

/// RunState
///
/// Tracks the state of a complete run including unlocked relics, pieces,
/// synergies, kingdom progress, difficulty, and Vorath counter. Each
/// liberation_at entry locks that kingdom's liberation battle until
/// battles_fought reaches it after a failed attempt.
///
struct RunState {
    bool           relics[RELIC_COUNT];
    bool           pieces[PIECE_COUNT];
    bool           cards[CARD_COUNT];
    bool           synergies[KINGDOM_COUNT];

    KingdomState   kingdoms[KINGDOM_COUNT];
    EventChoice    events[EVENT_COUNT];
    int            event_picks[EVENT_COUNT];

    Difficulty     difficulty;
    ChallengeRunID challenge;

    size_t         seed;
    size_t         vorath_counter;
    size_t         battles_fought;
    size_t         liberation_at[KINGDOM_COUNT];
};

/*----------------------------------------------------------------------------*\
                                ENGINE.C STRUCTS
\*----------------------------------------------------------------------------*/

/// EngineState
///
/// The master game state containing all masteries, the highest cleared
/// difficulty, the active screen, and pointers to the current run and
/// battle. battle is nullptr outside of battle.
///
struct EngineState {
    MasteryLevel masteries[KINGDOM_COUNT];
    Difficulty   cleared;

    Screen*      screen;
    RunState*    run;
    BattleState* battle;
};

/*----------------------------------------------------------------------------*\
                                    EFFECT.C
\*----------------------------------------------------------------------------*/

Effect* effect_attach(LinkedList* list, const Effect* effect);
void    effect_fire(
    BattleState*  battle,
    Side          side,
    EffectTrigger trigger,
    void*         x
);
void        effect_tick(LinkedList* list);
void        effect_clear(LinkedList* list);
const char* effect_trigger_name(EffectTrigger trigger);
bool        eff_noop(EffectContext* context, void* x);
Effect*     effect_find_mark(LinkedList* list, uintptr_t tag, void* subject);

/*----------------------------------------------------------------------------*\
                                    BATTLE.C
\*----------------------------------------------------------------------------*/

/// VOID_CELL
///
/// Sentinel piece info marking squares that do not exist on the current
/// board. Board cells point at this constant; live pieces never do.
///
extern const PieceInfo VOID_CELL;

void                   battle_begin(EngineState* engine, MapNode* node);
void                   battle_free(BattleState* battle);
void                   battle_concede(BattleState* battle);

bool                   battle_move(BattleState* battle, Square from, Square to);
bool                   battle_buy(BattleState* battle, PieceID id, Square at);
bool                   battle_combine(BattleState* battle, Square a, Square b);
bool                   battle_play(BattleState* battle, size_t hand);
bool                   battle_card_can_play(BattleState* battle, size_t hand);
bool                   battle_card_target(BattleState* battle, size_t index);
const CardTarget*      battle_pending_picks(void);
bool                   battle_piece_unlocked(PieceID id);
int     battle_piece_moves(BattleState* battle, PieceInfo* piece);
int     battle_piece_flips(BattleState* battle, PieceInfo* piece);
size_t  battle_piece_move_turn(PieceInfo* piece);
size_t  battle_piece_flip_turn(PieceInfo* piece);
bool    battle_sell(BattleState* battle, size_t hand);
bool    battle_reclaim(BattleState* battle, Square at);
void    battle_end_turn(BattleState* battle);

Square* battle_moves(BattleState* battle, PieceInfo* piece);
Square* battle_attacks(BattleState* battle, PieceInfo* piece);
int     battle_value(BattleState* battle, PieceInfo* piece, PieceInfo* victim);
int     battle_meter_max(BattleState* battle, Side side);
Side    battle_territory(BattleState* battle, Square square);
PieceInfo* battle_at(BattleState* battle, Square square);
bool       battle_in_bounds(BattleState* battle, Square square);
PieceInfo* battle_spawn(BattleState* battle, PieceID id, Square at, Side side);
bool       battle_is_recipe_result(PieceID id);
void       battle_flip(BattleState* battle, PieceInfo* piece);
void       battle_swap(BattleState* battle, PieceInfo* a, PieceInfo* b);
void       battle_remove(BattleState* battle, PieceInfo* piece);
void       battle_scatter_voids(BattleState* battle, int percent);
void       battle_board_view(BattleState* battle);
void       battle_hand_view(BattleState* battle, Side side);
void battle_reinforce(BattleState* battle, Side owner, Side half, size_t count);
void battle_attach_power(
    BattleState*        battle,
    Side                side,
    const KingdomPower* power,
    MasteryLevel        level
);

/// Battle state accessors
///
/// Shared read and mutate helpers over battle state: the player state for
/// a side, the opposing side, the side's king, and a meter gain clamped
/// to the two hundred percent ceiling.
///
PlayerState* battle_player(BattleState* battle, Side side);
Side         battle_enemy(Side side);
PieceInfo*   battle_find_king(BattleState* battle, Side side);
void         battle_meter_gain(BattleState* battle, Side side, int amount);

/// battle_rand
///
/// Draws the next value from the battle's deterministic RNG stream, the
/// shared source for every randomised effect so a battle stays reproducible
/// from its seed.
///
/// Return: a pseudo-random unsigned int
///
unsigned int battle_rand(void);

/// battle_draw_pool
///
/// Builds the side's eligible draw pool into out (sized CARD_COUNT): every
/// unlocked, registered card that passes QUERY_CARD_CAN_DRAW. Shared by
/// battle_draw and by effects that need the same available set, such as
/// Lucky Strike.
///
/// Params:
/// - battle -> battle whose run gates the pool
/// - side   -> side the draw eligibility fires for
/// - out    -> CARD_COUNT-sized buffer receiving the eligible ids
///
/// Return: the number of eligible cards written to out
///
size_t battle_draw_pool(BattleState* battle, Side side, CardID* out);

/// Direct damage and single-piece strike
///
/// battle_damage reduces a side's meter by an amount and runs the flip
/// cascade at once, so card and relic meter damage flips pieces the
/// moment it lands rather than deferring to the next resolve.
/// battle_lunge force-moves a piece to a square, resolves that piece's
/// coverage into the enemy meter, cascades, and returns the damage dealt;
/// it powers multi-strike card choreography such as Crusade.
///
void battle_damage(BattleState* battle, Side side, int amount);
int  battle_lunge(BattleState* battle, PieceInfo* piece, Square to);

/// Subject registers
///
/// Hidden per-fire state set only by the battle.c emission points so
/// effects can self-filter without payload slots. battle_victim is
/// nullptr outside damage queries, battle_damagers is a null-terminated
/// list live only during resolve, and battle_move_from is valid only
/// during ON_PIECE_MOVE. battle_cascade_origin holds the receiver's meter
/// before the current damage and cascade transaction, valid during
/// ON_CASCADE_END. battle_run exposes the active run so data-file effects
/// can read run state without the private engine pointer.
///
BattleState* battle_current(void);
PieceInfo*   battle_subject(void);
PieceInfo*   battle_victim(void);
Card*        battle_subject_card(void);
const Piece* battle_buy_piece(void);
Square       battle_move_from(void);
Square       battle_owner_square(void);
int          battle_cascade_origin(void);
RunState*    battle_run(void);
PieceInfo**  battle_damagers(void);
void         battle_draw(BattleState* battle, Side side, size_t count);

/*----------------------------------------------------------------------------*\
                                     PIECE.C
\*----------------------------------------------------------------------------*/

/// Generic direction sets
///
/// Shared zero-vector terminated unit direction arrays. They serve both
/// as slide directions and as single-step leap offsets. Piece-specific
/// patterns are never named globals: they live inline as compound
/// literals at the piece's generator function.
///
extern const Square ORTHOGONAL_DIRECTIONS[];
extern const Square DIAGONAL_DIRECTIONS[];
extern const Square ALL_DIRECTIONS[];

/// Movegen kit
///
/// mg_begin resets the shared static scratch buffer, mg_push appends one
/// square, and mg_end terminates the list with SQUARE_END and returns the
/// buffer. The sentinel is overwritten by the next push, so sequential
/// generator calls concatenate. The buffer stays valid only until the
/// next battle_moves or battle_attacks call and must never be filled
/// reentrantly: bespoke functions that need another piece's list copy it
/// out to a local buffer first, then mg_begin and mg_push the result.
/// Offset and direction arrays are written from the white perspective
/// (forward = -y) and terminate with the zero vector { 0, 0 };
/// mg_compound parts are PIECE_NONE terminated and generate from
/// self's square and side.
/// threat selects at (coverage) semantics over mv (movement) semantics.
///
void    mg_begin(void);
void    mg_push(Square square);
Square* mg_end(void);
void    mg_leap(
    BattleState*  battle,
    PieceInfo*    self,
    const Square* offsets,
    bool          threat
);
void mg_slide(
    BattleState*  battle,
    PieceInfo*    self,
    const Square* directions,
    int8_t        min,
    int8_t        max,
    bool          threat
);
void mg_compound(
    BattleState*   battle,
    PieceInfo*     self,
    const PieceID* parts,
    bool           threat
);

/// Piece behaviour effects
///
/// Generic effects shared by the kingdom pieces: pawn classification, a
/// slot embedder, a one-shot free move grant, the alternating double
/// move, and the movement copying pair with its attach helper.
///
bool    piece_is_pawn(PieceID id);
Effect* piece_embed_effect(PieceInfo* piece, const Effect* template);
void    piece_grant_free_move(PieceInfo* piece, const char* name);
bool    eff_free_move(EffectContext* context, void* x);
bool    eff_double_move(EffectContext* context, void* x);
bool    eff_copy_mv(EffectContext* context, void* x);
bool    eff_copy_at(EffectContext* context, void* x);
void    piece_adopt_move(
    BattleState*   battle,
    PieceInfo*     target,
    PieceID        copied,
    EffectDuration lasts,
    bool           replace
);

/*----------------------------------------------------------------------------*\
                                     CARD.C
\*----------------------------------------------------------------------------*/

/// Card play target codec
///
/// card_pack encodes a board square into the packed target pointer
/// convention, offset by one so a zero square is not an absent target;
/// card_square decodes one back into a board square.
///
uintptr_t card_pack(Square square);
Square    card_square(void* packed);

/// Card targeting list helpers
///
/// A QUERY_CARD_TARGETS effect advertises legal targets by pushing them
/// onto a TARGET_NONE terminated CardTarget list; card_target_at decodes a
/// TARGET_PIECE or TARGET_SQUARE value an ON_CARD_TARGET_SELECTED effect
/// receives. There is exactly one enumerator per target category; the card
/// passes an inline block deciding which pieces, squares, or identities it
/// accepts, so every filter lives at the card with no named helper.
///
size_t card_target_count(const CardTarget* list);
void   card_target_push(CardTarget* list, TargetKind kind, int value);
Square card_target_at(int value);
void   card_targets_piece(
    CardTarget*  list,
    BattleState* battle,
    bool (^match)(const PieceInfo* piece)
);
void card_targets_square(
    CardTarget*  list,
    BattleState* battle,
    bool (^match)(Square square)
);
void card_targets_piece_type(
    CardTarget* list,
    bool (^match)(const Piece* piece)
);

/*----------------------------------------------------------------------------*\
                                      AI.C
\*----------------------------------------------------------------------------*/

void ai_take_turn(BattleState* battle);
void ai_plan(BattleState* battle);

/*----------------------------------------------------------------------------*\
                                     RUN.C
\*----------------------------------------------------------------------------*/

size_t rng_mix(size_t seed, size_t salt);
void   run_new(
    EngineState*   engine,
    size_t         seed,
    Difficulty     difficulty,
    ChallengeRunID challenge
);
void              run_free(RunState* run);
void              run_enter_map(EngineState* engine, KingdomID kingdom);
bool              run_select_node(EngineState* engine, size_t index);
void              run_fire(EngineState* engine, EffectTrigger trigger, void* x);
EngineState*      run_engine(void);
const CardTarget* run_pending_picks(void);
void              run_battle_result(EngineState* engine, bool won);
void              run_event_choose(EngineState* engine, EventChoice choice);
bool              run_event_target(EngineState* engine, size_t index);
void              run_targets_battle_nodes(CardTarget* list);
void              run_targets_figureheads(CardTarget* list);
void              run_event_pick(int value);
void              run_skip_node(size_t index);
void              run_unchain(KingdomID kingdom);
void              run_offering(EngineState* engine, CardID card);
void              run_relic_pick(EngineState* engine, RelicID relic);
size_t            run_pressure(RunState* run, KingdomID kingdom);
bool              run_innate_ready(RunState* run, KingdomID kingdom);
void              run_reduce_vorath(RunState* run, size_t amount);
void              run_node_reveal(EngineState* engine, size_t count);
void              run_remove_chain(RunState* run);
void run_begin_elite(EngineState* engine, int reward, RelicID a, RelicID b);
void run_emit_kingdoms(EngineState* engine);
void run_emit_map(EngineState* engine);
bool run_enter_vorath(EngineState* engine);

/*----------------------------------------------------------------------------*\
                                   ENGINE.C
\*----------------------------------------------------------------------------*/

void engine_init(EngineState* engine);
void engine_free(EngineState* engine);
bool engine_save(EngineState* engine, const char* path);
bool engine_load(EngineState* engine, const char* path);
void engine_finalize_run(EngineState* engine, bool vorath_won);

/*----------------------------------------------------------------------------*\
                                    RELIC.C
\*----------------------------------------------------------------------------*/

/// RELIC_REGISTRY
///
/// Global array of all relic definitions indexed by RelicID.
///
extern const Relic RELIC_REGISTRY[RELIC_COUNT];

/*----------------------------------------------------------------------------*\
                              KINGDOM/UNIVERSAL.C
\*----------------------------------------------------------------------------*/

/// Universal data
///
/// UNIVERSAL_PIECES holds pieces belonging to no kingdom (the King) and
/// UNIVERSAL_CARDS the universal and mastery card pools. The registries
/// are pointer tables indexed by id aggregating every kingdom's arrays.
/// KINGDOM_ADJACENT maps each kingdom to its synergy neighbor and the
/// EVENT_* tables hold narrative strings indexed by EventID.
///
extern const Piece               UNIVERSAL_PIECES[];
extern const Card                UNIVERSAL_CARDS[];

extern const BattleModifier      MODIFIER_REGISTRY[MODIFIER_COUNT];
extern const ChainPenalty        CHAIN_REGISTRY[CHAIN_PENALTY_COUNT];
extern const DifficultyMode      DIFFICULTY_REGISTRY[DIFFICULTY_NONE];
extern const ChallengeRun        CHALLENGE_REGISTRY[CHALLENGE_COUNT];

extern const Piece* const        PIECE_REGISTRY[PIECE_COUNT];
extern const Card* const         CARD_REGISTRY[CARD_COUNT];
extern const BoardTrait* const   TRAIT_REGISTRY[BOARD_TRAIT_COUNT];

extern const KingdomID           KINGDOM_ADJACENT[KINGDOM_COUNT];
extern const KingdomPower        SYNERGY_REGISTRY[KINGDOM_COUNT];
extern const KingdomPower* const INNATE_REGISTRY[KINGDOM_COUNT];
extern const KingdomPower* const CLIMAX_REGISTRY[KINGDOM_COUNT];

extern const Event* const        EVENT_REGISTRY[EVENT_COUNT];

void                             vorath_setup(BattleState* battle);
void vorath_attach_capacity(BattleState* battle, Side side);

/// Kingdom dispatch tables
///
/// Function pointer table indexed by KingdomID aggregating each kingdom's
/// overseer setup.
///
extern void (*const KINGDOM_OVERSEER[KINGDOM_COUNT])(BattleState*);

/*----------------------------------------------------------------------------*\
                               KINGDOM/LONGWEI.C
\*----------------------------------------------------------------------------*/

extern const Piece        LONGWEI_PIECES[];
extern const Card         LONGWEI_CARDS[];
extern const BoardTrait   LONGWEI_TRAITS[];
extern const KingdomPower LONGWEI_INNATE;
extern const KingdomPower LONGWEI_CLIMAX;

void                      longwei_overseer(BattleState* battle);

/*----------------------------------------------------------------------------*\
                              KINGDOM/KEWARANI.C
\*----------------------------------------------------------------------------*/

extern const Piece        KEWARANI_PIECES[];
extern const Card         KEWARANI_CARDS[];
extern const BoardTrait   KEWARANI_TRAITS[];
extern const KingdomPower KEWARANI_INNATE;
extern const KingdomPower KEWARANI_CLIMAX;

void                      kewarani_overseer(BattleState* battle);

/*----------------------------------------------------------------------------*\
                               KINGDOM/ZARQAN.C
\*----------------------------------------------------------------------------*/

extern const Piece        ZARQAN_PIECES[];
extern const Card         ZARQAN_CARDS[];
extern const BoardTrait   ZARQAN_TRAITS[];
extern const KingdomPower ZARQAN_INNATE;
extern const KingdomPower ZARQAN_CLIMAX;

void                      zarqan_overseer(BattleState* battle);

/*----------------------------------------------------------------------------*\
                             KINGDOM/HARUSHIMA.C
\*----------------------------------------------------------------------------*/

extern const Piece        HARUSHIMA_PIECES[];
extern const Card         HARUSHIMA_CARDS[];
extern const BoardTrait   HARUSHIMA_TRAITS[];
extern const KingdomPower HARUSHIMA_INNATE;
extern const KingdomPower HARUSHIMA_CLIMAX;

void                      harushima_overseer(BattleState* battle);

/*----------------------------------------------------------------------------*\
                               KINGDOM/CAELAN.C
\*----------------------------------------------------------------------------*/

extern const Piece        CAELAN_PIECES[];
extern const Card         CAELAN_CARDS[];
extern const BoardTrait   CAELAN_TRAITS[];
extern const KingdomPower CAELAN_INNATE;
extern const KingdomPower CAELAN_CLIMAX;

void                      caelan_overseer(BattleState* battle);
