//!
//! Constants, limits, and identifier enums for the Regnum battle engine.
//! No project-name prefix on macros. Size limits are #define only.
//! Id enums are the single source of truth for all element ids.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef DEFS_H
#define DEFS_H

#include "core.h"

/*--------------------------------------------------------------------------*\
                              SIZE LIMITS
\*--------------------------------------------------------------------------*/

#define MAX_PIECES 128
#define MAX_PIECES_PER_SIDE 64
#define MAX_HAND 12
#define MAX_CARDSET 96
#define MAX_EFFECTS 256
#define MAX_EFFECT_ARGS 6
#define MAX_PIECE_BUFFS 8
#define MAX_PIECE_PASSIVES 4
#define MAX_CARD_EFFECTS 6
#define MAX_MOVE_PARAMS 32
#define MAX_SUB_MOVEGENS 4
#define MAX_MOVES 64
#define MAX_BOARD_DIM 20
#define MAX_RELICS_HELD 26
#define MAX_EVENTS 128
#define MAX_BATTLE_MODIFIERS 8
#define MAX_BOARD_TRAITS 4
#define MAX_HANDLERS 16
#define MAX_COMBO_KINGDOMS 5
#define MAX_COMBO_PAIRS 32

/*--------------------------------------------------------------------------*\
                              VERSION
\*--------------------------------------------------------------------------*/

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

/*--------------------------------------------------------------------------*\
                              PIECE IDS
\*--------------------------------------------------------------------------*/

typedef enum {
    PIECE_KING = 0,
    /* Longwei (Xiangqi/Janggi) */
    PIECE_BING,
    PIECE_XIANG,
    PIECE_MA,
    PIECE_PAO,
    PIECE_LIUBO_DIVINER,
    PIECE_SANG,
    PIECE_NORTHERN_CAVALRY,
    PIECE_HWACHA,
    /* Harushima (Shogi/Chu Shogi) */
    PIECE_FUHYO,
    PIECE_KYOSHA,
    PIECE_GINSHO,
    PIECE_KINSHO,
    PIECE_SHISHI,
    PIECE_HONORABLE_HORSE,
    PIECE_PROMOTED_BISHOP,
    PIECE_DAIMYO,
    PIECE_DRAGON,
    /* Kewarani (Senterej) */
    PIECE_MEDEQ,
    PIECE_MAKWANAM,
    PIECE_SABA,
    PIECE_FARAS,
    PIECE_NEGUS_GUARD,
    PIECE_MEDEQ_SQUAD,
    PIECE_SULTANS_LEVY,
    /* Zarqan (Tamerlane/Shatranj) */
    PIECE_WAZIR,
    PIECE_JAMAL,
    PIECE_TALLIYA,
    PIECE_OLD_KING,
    PIECE_CATAPHRACT,
    PIECE_ZIRAAFA,
    PIECE_SHAHZADEH,
    PIECE_WAR_ELEPHANT,
    /* Caelan (standard chess) */
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_ROOK,
    PIECE_QUEEN,
    PIECE_CHANCELLOR,
    PIECE_GRYPHON,
    PIECE_SOVEREIGN_BANNER,
    PIECE_ID_COUNT
} PieceId;

/*--------------------------------------------------------------------------*\
                              CARD IDS
\*--------------------------------------------------------------------------*/

typedef enum {
    /* Universal (12) */
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
    /* Longwei (7) */
    CARD_RIVER_WADE,
    CARD_FORTUNE_TELLER,
    CARD_SKY_LADDERS,
    CARD_MANDATE,
    CARD_WAR_CHARIOT,
    CARD_BULL_RUSH,
    CARD_WISDOM,
    /* Harushima (7) */
    CARD_RONIN,
    CARD_TAKENOKO,
    CARD_BUSHIDO,
    CARD_HONOR,
    CARD_STRATAGEM,
    CARD_GLORY,
    CARD_REVENGE,
    /* Kewarani (7) */
    CARD_SULTANS_GOLD,
    CARD_PILGRIMAGE,
    CARD_CORONATION,
    CARD_JIHAD,
    CARD_BAZAAR,
    CARD_WAHHAB,
    CARD_HAJJ,
    /* Zarqan (8) */
    CARD_COUNSEL,
    CARD_AMBITION,
    CARD_CONQUEST,
    CARD_FORTUNE,
    CARD_CRUSADE,
    CARD_DIVINE_RIGHT,
    CARD_INTRIGUE,
    CARD_TREACHERY,
    /* Caelan (8) */
    CARD_CASTLING,
    CARD_CATHEDRAL,
    CARD_ASSEMBLY,
    CARD_PROMOTION,
    CARD_GOLD_STANDARD,
    CARD_FORMATION,
    CARD_QUEENS_DECREE,
    CARD_DIVINE_INTERVENTION,
    CARD_ID_COUNT
} CardId;

/*--------------------------------------------------------------------------*\
                              RELIC IDS
\*--------------------------------------------------------------------------*/

typedef enum {
    /* Economy */
    RELIC_MERCHANTS_LEDGER,
    RELIC_TREASURY_KEY,
    RELIC_GOLDEN_LAMP,
    RELIC_TRIBUTE_CHEST,
    RELIC_WANDERING_COIN,
    /* Meter */
    RELIC_STORM_BELL,
    RELIC_OVERFLOW_ORB,
    RELIC_METER_MASON,
    RELIC_TIDE_CALLER,
    RELIC_RESERVOIR_STONE,
    /* Cards */
    RELIC_CARDSMITH_HAMMER,
    RELIC_DECK_TRAY,
    RELIC_WILD_JOKER,
    RELIC_SCRIBBLERS_QUILL,
    RELIC_CARDINAL_ROBE,
    /* Combinations */
    RELIC_ALCHEMISTS_STONE,
    RELIC_FUSION_FURNACE,
    RELIC_COMBINE_CATALYST,
    RELIC_TRANSMUTE_CRYSTAL,
    RELIC_MASTERWORK_ANVIL,
    /* Board */
    RELIC_FORTRESS_WALL,
    RELIC_BATTLE_STANDARD,
    RELIC_MIRROR_SHIELD,
    RELIC_PATHFINDER_BOOTS,
    RELIC_TERRITORY_MAP,
    RELIC_ID_COUNT
} RelicId;

/*--------------------------------------------------------------------------*\
                              OTHER IDS
\*--------------------------------------------------------------------------*/

typedef enum {
    INNATE_BULWARK,
    INNATE_RECLAIM,
    INNATE_DOUBLE_TIME,
    INNATE_ROYAL_SUBSTITUTION,
    INNATE_CONQUERORS_REWARD,
    INNATE_COUNT
} InnateId;

typedef enum { CHAIN_BRONZE, CHAIN_SILVER, CHAIN_GOLD, CHAIN_COUNT } ChainId;

typedef enum {
    MODIFIER_ECONOMY_BOOST,
    MODIFIER_ECONOMY_DRAIN,
    MODIFIER_METER_BOOST,
    MODIFIER_METER_DRAIN,
    MODIFIER_CARDS_BOOST,
    MODIFIER_CARDS_DRAIN,
    MODIFIER_BOARD_BOOST,
    MODIFIER_BOARD_DRAIN,
    MODIFIER_COUNT
} ModifierId;

typedef enum {
    TRAIT_RIVER_CROSSING,
    TRAIT_PALACE,
    TRAIT_FOG_COAST,
    TRAIT_ISLAND_CHAIN,
    TRAIT_TRADE_ROUTE,
    TRAIT_CONTESTED_MARKET,
    TRAIT_SANDSTORM,
    TRAIT_MIRAGE,
    TRAIT_CASTLE_CORNERS,
    TRAIT_SIEGE_TRENCH,
    TRAIT_COUNT
} TraitId;

typedef enum {
    EVENT_UNIVERSAL_WANDERER,
    EVENT_UNIVERSAL_MARKET,
    EVENT_UNIVERSAL_AMBUSH,
    EVENT_LONGWEI_RIVER_FESTIVAL,
    EVENT_LONGWEI_SKY_LADDER,
    EVENT_HARUSHIMA_HONOR_TRIAL,
    EVENT_HARUSHIMA_TAKENOKO_SWARM,
    EVENT_KEWARANI_PILGRIMAGE_SEASON,
    EVENT_KEWARANI_BAZAAR_RUMOR,
    EVENT_ZARQAN_SANDSTORM,
    EVENT_ZARQAN_OASIS_DISCOVERY,
    EVENT_CAELAN_TOURNAMENT,
    EVENT_CAELAN_CASTLE_FESTIVAL,
    EVENT_COUNT
} EventId;

typedef enum {
    OVERSEER_IRON_STRATEGIST,
    OVERSEER_ETERNAL_RECURSION,
    OVERSEER_CARAVAN_OF_CONQUEST,
    OVERSEER_MANY_FACED_KING,
    OVERSEER_CROWNED_HERETIC,
    OVERSEER_VORATH,
    OVERSEER_COUNT
} OverseerId;

typedef enum {
    SYNERGY_LONGWEI,
    SYNERGY_HARUSHIMA,
    SYNERGY_KEWARANI,
    SYNERGY_ZARQAN,
    SYNERGY_CAELAN,
    SYNERGY_COUNT
} SynergyId;

typedef enum {
    MASTERY_CARD_LONGWEI,
    MASTERY_CARD_HARUSHIMA,
    MASTERY_CARD_KEWARANI,
    MASTERY_CARD_ZARQAN,
    MASTERY_CARD_CAELAN,
    MASTERY_COUNT
} MasteryId;

typedef enum {
    FIGUREHEAD_POWER_LONGWEI,
    FIGUREHEAD_POWER_HARUSHIMA,
    FIGUREHEAD_POWER_KEWARANI,
    FIGUREHEAD_POWER_ZARQAN,
    FIGUREHEAD_POWER_CAELAN,
    FIGUREHEAD_POWER_COUNT
} FigureheadPowerId;

typedef enum {
    ARCHETYPE_STANDARD,
    ARCHETYPE_RECLAIMER,
    ARCHETYPE_TRICKSTER,
    ARCHETYPE_BERSERKER,
    ARCHETYPE_DEFENDER,
    ARCHETYPE_COUNT
} ArchetypeId;

#endif /* DEFS_H */