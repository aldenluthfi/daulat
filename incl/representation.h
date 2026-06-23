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

/// EffectTrigger
///
/// This enum enumerates all the triggers that can activate an effect.
/// Triggers are conditions that cause effects to fire during gameplay.
///
enum EffectTrigger {
    QUERY_CARD_DRAW_COUNT,
    QUERY_CARD_PLAY_COST,
    QUERY_CARD_SELL_COST,

    QUERY_PIECE_ACTION_COST_MOVE,
    QUERY_PIECE_ACTION_COST_BUY,
    QUERY_PIECE_ACTION_COST_COMBINE,

    QUERY_PIECE_CP_COST_BUY,
    QUERY_PIECE_CP_COST_RECLAIM,

    QUERY_PIECE_CAN_FLIP,
    QUERY_PIECE_CAN_MOVE,

    QUERY_PIECE_DAMAGE_DEALT,
    QUERY_PIECE_DAMAGE_TAKEN,

    QUERY_METER_DAMAGE_TAKEN,

    QUERY_CP_INCOME,

    ON_PIECE_FLIP,
    ON_PIECE_BUY,
    ON_PIECE_MOVE,
    ON_PIECE_COMBINE,

    ON_CARD_PLAY,
    ON_CARD_SELL,

    ON_TURN_START,
    ON_TURN_END,

    ON_BATTLE_START,
    ON_BATTLE_END,
};

/// EffectDuration
///
/// This enum enumerates all possible effect durations. Effects persist for
/// their duration and are automatically removed when they expire.
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
    PIECE_SENTINEL,
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
};

/// CardTier
///
/// This enum enumerates all card tiers. Tiers determine card availability
/// based on map progression and affect sell values.
///
enum CardTier {
    TIER_DISTRICT,
    TIER_TOWN,
    TIER_PROVINCE,
    TIER_COUNTRY,
    TIER_MASTERY,
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
};

/*----------------------------------------------------------------------------*\
                               DIFFICULTY.C ENUMS
\*----------------------------------------------------------------------------*/

/// Difficulty
///
/// This enum enumerates all difficulty levels. Higher difficulties add
/// penalties and are unlocked after defeating the previous level once.
///
enum Difficulty {
    DIFFICULTY_FREE,
    DIFFICULTY_BOUND,
    DIFFICULTY_SHACKLED,
    DIFFICULTY_ENSLAVED,
};

/*----------------------------------------------------------------------------*\
                              CHALLENGE_RUN.C ENUMS
\*----------------------------------------------------------------------------*/

/// ChallengeRunID
///
/// This enum enumerates all challenge run modifiers.
///
enum ChallengeRunID {
    CHALLENGE_DAILY_CONQUEST,
    CHALLENGE_SOLO_VANGUARD,
    CHALLENGE_PACIFIST_DOCTRINE,
    CHALLENGE_BLIND_DRAFT,
    CHALLENGE_THE_TRAITORS_GAMBIT,
    CHALLENGE_CLOCKWORK,
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
///
struct Effect {
    void           (*func)(EffectContext* context, void* x);

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
#define MAX_BOARD_SIZE 20 * 20

/// Board
///
/// Represents the game board with its traits.
///
struct Board {
    BoardTrait* trait;
};

/// Square
///
/// Represents a position on the board using x/y coordinates.
///
struct Square {
    int8_t x;
    int8_t y;
};

/*----------------------------------------------------------------------------*\
                                PIECE.C STRUCTS
\*----------------------------------------------------------------------------*/

/// Piece
///
/// Represents a piece definition with its movement and attack functions.
///
struct Piece {
    Square* (*at)(BattleState* battle_state);
    Square* (*mv)(BattleState* battle_state);

    PieceID id;
    char*   name;
    char*   desc;
};

/// PieceInfo
///
/// Runtime information about a piece on the board including its position
/// and current state flags.
///
struct PieceInfo {
    Piece* piece;

    Square square;
    bool   immune;
    bool   flipped;
};

/*----------------------------------------------------------------------------*\
                                CARD.C STRUCTS
\*----------------------------------------------------------------------------*/

/// Card
///
/// A card that can be drawn and played during battle.
///
struct Card {
    EFFECT_ITEM_BASE;

    CardID   id;
    CardTier tier;
};

/*----------------------------------------------------------------------------*\
                                   KINGDOM.C
\*----------------------------------------------------------------------------*/

/// KingdomState
///
/// Tracks the state of a kingdom including its mastery level, chain penalties,
///
/// and maps for each tier.
///
struct KingdomState {
    KingdomID     id;
    MasteryLevel  mastery;
    ChainPenalty* chain;

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
};

/*----------------------------------------------------------------------------*\
                            CAMPAING_MAP.C STRUCTS
\*----------------------------------------------------------------------------*/

/// MapNode
///
/// A single node on the campaign map representing a battle, elite, archive,
///
/// offering, or event.
///
struct MapNode {
    MapNodeID       type;

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

/// EventState
///
/// Tracks the state of a narrative event including the choice made.
///
struct EventState {
    EventID     id;
    KingdomID   kingdom;

    EventChoice choice_taken;
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

    Card       hand[MAX_DRAWN_CARDS];
    LinkedList effects;
};

/// BattleState
///
/// The complete state of a battle including turn count, board, modifier,
///
/// and both player states.
///
struct BattleState {
    size_t          turn;
    Board           board;
    BattleModifier* modifier;

    PlayerState     white;
    PlayerState     black;
};

/*----------------------------------------------------------------------------*\
                                 RUN.C STRUCTS
\*----------------------------------------------------------------------------*/

/// RunState
///
/// Tracks the state of a complete run including unlocked relics, pieces,
/// synergies, kingdom progress, difficulty, and Vorath counter.
///
struct RunState {
    bool         relics[RELIC_COUNT];
    bool         pieces[PIECE_COUNT];
    bool         synergies[KINGDOM_COUNT];

    KingdomState kingdoms[KINGDOM_COUNT];

    Difficulty   difficulty;
    size_t       vorath_counter;
};

/*----------------------------------------------------------------------------*\
                                ENGINE.C STRUCTS
\*----------------------------------------------------------------------------*/

/// EngineState
///
/// The master game state containing all masteries and a pointer to the
/// current run.
///
struct EngineState {
    MasteryLevel masteries[KINGDOM_COUNT];
    RunState*    run;
};
