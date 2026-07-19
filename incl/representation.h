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

  QUERY_NEXT_HAND,
  ON_ITEM_ACTIVATE,

  ON_MAP_ENTER,

  QUERY_RECIPE_ALLOWED,
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
  ENTIRE_RUN,
};

/// EffectItemType
///
/// This enum enumerates all the types of items that can have effects attached.
///
enum EffectItemType {
  CARD,
  PIECE,
  RELIC,
  BATTLE_MODIFIER,
  BOARD_TRAIT,
  CHAIN_PENALTY,
  DIFFICULTY,
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

enum UnlockTier {
  TIER_DISTRICT,
  TIER_TOWN,
  TIER_PROVINCE,
  TIER_COUNTRY,
  TIER_MASTERY,
};

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
enum DifficultyID {
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
#define MAX_EFFECT_ARGS 16

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
  Effect effects[MAX_EFFECT_COUNT];                                            \
  char *name;                                                                  \
  char *desc

/// EffectContext
///
/// Holds arguments passed to an effect when it triggers. Effects receive
/// context-specific data through this structure.
///
struct EffectContext {
  void *args[MAX_EFFECT_ARGS];
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
  bool (*func)(EffectContext *context, void *x);
  char *name;

  EffectTrigger trigger;
  EffectDuration lasts_for;
  EffectContext *context;
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
  PieceInfo *piece_board[MAX_BOARD_SIZE];
  BoardTrait *trait;

  int8_t width;
  int8_t height;

  bool visible[MAX_BOARD_SIZE];
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
#define SQUARE_END ((Square){-1, -1})

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
  Square *(*at)(BattleState *battle_state, PieceInfo *self);
  Square *(*mv)(BattleState *battle_state, PieceInfo *self);

  EFFECT_ITEM_BASE;

  PieceID id;
  KingdomID kingdom;
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
  Piece *piece;
  Square square;
  Side side;
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

  CardID id;
  UnlockTier tier;
  KingdomID kingdom;

  int play_cost;
  int sell_cost;
};

/// CardTarget
///
/// One legal target a card advertises through QUERY_CARD_TARGETS: a kind
/// and an int value read per kind (TARGET_PIECE and TARGET_SQUARE =
/// y * 20 + x, TARGET_CARD = hand slot, TARGET_PIECE_TYPE = PieceID).
///
struct CardTarget {
  TargetKind kind;
  int value;
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
  EFFECT_ITEM_BASE;

  KingdomID id;
  MasteryLevel mastery;
  ChainPenalty *chain;

  MapState *town_map;
  MapState *province_map;
  MapState *country_map;
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
  Board *board;
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
/// interpreted by the type: an Event for events, a result Piece for
/// archives, an OverseerTypeID for overseers, or a relic pair for elites;
/// unused otherwise.
struct MapNode {
  MapNodeID type;
  void *content;

  const char *name;
  int content;

  MapState *map;
  KingdomState *kingdom;
  BoardTrait *trait;
  BattleModifier *modifier;

  bool revealed;
  bool cleared;
};

/// MapState
///
/// Represents an entire map for a kingdom and tier combination.
///
struct MapState {
  MapTypeID type;

  KingdomState *kingdom;
  DirectedGraph nodes;
};

struct EventOption {
  EFFECT_ITEM_BASE;

  EventID id;
};

struct Event {
  const char *name;
  const char *desc;

  EventChoice choice;
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
struct Difficulty {
  EFFECT_ITEM_BASE;

  DifficultyID id;
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
  int cp;
  int meter;
  int actions;

  Card *hand[MAX_DRAWN_CARDS];
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
  Board board;

  PlayerState white;
  PlayerState black;

  BattleModifier *modifier;
  MapNode *node;
  size_t turn;
};

/*----------------------------------------------------------------------------*\
                                 RUN.C STRUCTS
\*----------------------------------------------------------------------------*/

/// RunState
///
/// Tracks the state of a complete run including unlocked relics, pieces,
/// synergies, kingdom progress, difficulty, and Vorath counter. Each
/// liberation_at entry locks that kingdom's liberation battle until
/// battles_fought reaches it after a failed attempt. recipe_forbidden marks
/// combination recipes banned by the Global Vorath Counter for the run.
///
struct RunState {
  bool relics[RELIC_COUNT];
  bool pieces[PIECE_COUNT];
  bool cards[CARD_COUNT];

  KingdomState kingdoms[KINGDOM_COUNT];
  EventChoice events[EVENT_COUNT];

  Difficulty difficulty;

  size_t seed;
  size_t vorath_counter;
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
  Difficulty cleared;

  Screen *screen;
  RunState *run;
  BattleState *battle;
};

/*----------------------------------------------------------------------------*\
                                    EFFECT.C
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
                                    BATTLE.C
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
                                    PIECE.C
\*----------------------------------------------------------------------------*/

extern const Square ORTHOGONAL_DIRECTIONS[];
extern const Square DIAGONAL_DIRECTIONS[];
extern const Square ALL_DIRECTIONS[];

void mg_leap(BattleState *battle, PieceInfo *self);
void mg_slide(BattleState *battle, PieceInfo *self);
void mg_compound(BattleState *battle, PieceInfo *a, PieceInfo *b);

/*----------------------------------------------------------------------------*\
                                    CARD.C
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
                                     AI.C
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
                                     RUN.C
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*\
                                    ENGINE.C
\*----------------------------------------------------------------------------*/

void engine_init(EngineState *engine);
void engine_free(EngineState *engine);
bool engine_save(EngineState *engine, const char *path);
bool engine_load(EngineState *engine, const char *path);

/*----------------------------------------------------------------------------*\
                              KINGDOM/UNIVERSAL.C
\*----------------------------------------------------------------------------*/

extern const Piece UNIVERSAL_PIECES[];
extern const Card UNIVERSAL_CARDS[];

extern const BoardTrait *const TRAIT_REGISTRY[BOARD_TRAIT_COUNT];
extern const BattleModifier *const MODIFIER_REGISTRY[MODIFIER_COUNT];
extern const ChainPenalty *const CHAIN_REGISTRY[CHAIN_PENALTY_COUNT];
extern const DifficultyMode *const DIFFICULTY_REGISTRY[DIFFICULTY_NONE];
extern const ChallengeRun *const CHALLENGE_REGISTRY[CHALLENGE_COUNT];

extern const Piece *const PIECE_REGISTRY[PIECE_COUNT];
extern const Card *const CARD_REGISTRY[CARD_COUNT];
extern const Relic *const RELIC_REGISTRY[RELIC_COUNT];

extern const KingdomID KINGDOM_ADJACENT[KINGDOM_COUNT];
extern const KingdomPower *const SYNERGY_REGISTRY[KINGDOM_COUNT];
extern const KingdomPower *const INNATE_REGISTRY[KINGDOM_COUNT];
extern const KingdomPower *const CLIMAX_REGISTRY[KINGDOM_COUNT];

extern const Event *const EVENT_REGISTRY[EVENT_COUNT];

/*----------------------------------------------------------------------------*\
                                  LONGWEI.C
\*----------------------------------------------------------------------------*/

extern const Piece LONGWEI_PIECES[];
extern const Card LONGWEI_CARDS[];
extern const BoardTrait LONGWEI_TRAITS[];
extern const KingdomPower LONGWEI_INNATE;
extern const KingdomPower LONGWEI_CLIMAX;

void longwei_overseer(BattleState *battle);

/*----------------------------------------------------------------------------*\
                                  KEWARANI.C
\*----------------------------------------------------------------------------*/

extern const Piece KEWARANI_PIECES[];
extern const Card KEWARANI_CARDS[];
extern const BoardTrait KEWARANI_TRAITS[];
extern const KingdomPower KEWARANI_INNATE;
extern const KingdomPower KEWARANI_CLIMAX;

void kewarani_overseer(BattleState *battle);

/*----------------------------------------------------------------------------*\
                                   ZARQAN.C
\*----------------------------------------------------------------------------*/

extern const Piece ZARQAN_PIECES[];
extern const Card ZARQAN_CARDS[];
extern const BoardTrait ZARQAN_TRAITS[];
extern const KingdomPower ZARQAN_INNATE;
extern const KingdomPower ZARQAN_CLIMAX;

void zarqan_overseer(BattleState *battle);

/*----------------------------------------------------------------------------*\
                                  HARUSHIMA.C
\*----------------------------------------------------------------------------*/

extern const Piece HARUSHIMA_PIECES[];
extern const Card HARUSHIMA_CARDS[];
extern const BoardTrait HARUSHIMA_TRAITS[];
extern const KingdomPower HARUSHIMA_INNATE;
extern const KingdomPower HARUSHIMA_CLIMAX;

void harushima_overseer(BattleState *battle);

/*----------------------------------------------------------------------------*\
                                   CAELAN.C
\*----------------------------------------------------------------------------*/

extern const Piece CAELAN_PIECES[];
extern const Card CAELAN_CARDS[];
extern const BoardTrait CAELAN_TRAITS[];
extern const KingdomPower CAELAN_INNATE;
extern const KingdomPower CAELAN_CLIMAX;

void caelan_overseer(BattleState *battle);
