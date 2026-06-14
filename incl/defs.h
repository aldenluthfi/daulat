//! defs.h
//!
//! Constants, limits, and identifier enums for the Regnum battle engine.
//! No project-name prefix on macros. Size limits are #define only.
//! Id enums are the single source of truth for all element ids.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef DEFS_H
#define DEFS_H

#include "core.h"

/*--------------------------------------------------------------------------*\
                              SIZE LIMITS
\*--------------------------------------------------------------------------*/

#define MAX_PIECES           128
#define MAX_PIECES_PER_SIDE  64
#define MAX_HAND             12
#define MAX_CARDSET          96
#define MAX_EFFECTS          256
#define MAX_EFFECT_ARGS      6
#define MAX_PIECE_BUFFS      8
#define MAX_PIECE_PASSIVES   4
#define MAX_CARD_EFFECTS     6
#define MAX_MOVE_PARAMS      32
#define MAX_SUB_MOVEGENS     4
#define MAX_MOVES            64
#define MAX_BOARD_DIM        20
#define MAX_RELICS_HELD      26
#define MAX_EVENTS           128
#define MAX_BATTLE_MODIFIERS 8
#define MAX_BOARD_TRAITS     4
#define MAX_HANDLERS         16
#define MAX_COMBO_KINGDOMS   5
#define MAX_COMBO_PAIRS      32

/*--------------------------------------------------------------------------*\
                              APPLICATION
\*--------------------------------------------------------------------------*/

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define MAX_SCREENS   8
#define TARGET_FPS    60

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
    /* Longwei (Xiangqi/Janggi): 5 base + 3 combos */
    PIECE_BING,
    PIECE_XIANG,
    PIECE_MA,
    PIECE_PAO,
    PIECE_LIUBO_DIVINER,
    PIECE_SANG,
    PIECE_NORTHERN_CAVALRY,
    PIECE_HWACHA,
    /* Harushima (Shogi/Chu Shogi): 5 base + 4 combos */
    PIECE_FUHYO,
    PIECE_KYOSHA,
    PIECE_GINSHO,
    PIECE_KINSHO,
    PIECE_SHISHI,
    PIECE_HONORABLE_HORSE,
    PIECE_PROMOTED_BISHOP,
    PIECE_DAIMYO,
    PIECE_DRAGON,
    /* Kewarani (Senterej): 5 base + 2 combos */
    PIECE_MEDEQ,
    PIECE_MAKWANAM,
    PIECE_SABA,
    PIECE_FARAS,
    PIECE_NEGUS_GUARD,
    PIECE_MEDEQ_SQUAD,
    PIECE_SULTANS_LEVY,
    /* Zarqan (Tamerlane/Shatranj): 5 base + 4 combos */
    PIECE_WAZIR,
    PIECE_JAMAL,
    PIECE_TALLIYA,
    PIECE_ZIRAAFA,
    PIECE_SHAHZADEH,
    PIECE_OLD_KING,
    PIECE_CATAPHRACT,
    PIECE_ROOK,
    PIECE_WAR_ELEPHANT,
    /* Caelan (standard chess): 5 base + 2 combos */
    PIECE_PAWN,
    PIECE_KNIGHT,
    PIECE_BISHOP,
    PIECE_QUEEN,
    PIECE_GRYPHON,
    PIECE_CHANCELLOR,
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
    CARD_CHARGE,
    CARD_FORMATION,
    CARD_DIVINATION,
    CARD_CANNON_VOLLEY,
    CARD_PALACE_DECREE,
    CARD_MANDATE,
    /* Harushima (7) */
    CARD_RONIN,
    CARD_RESURRECTION,
    CARD_GOLD_STANDARD,
    CARD_PROMOTION,
    CARD_DUAL_DROP,
    CARD_FORCE_DROP,
    CARD_BUSHIDO,
    /* Kewarani (7) */
    CARD_SULTANS_GOLD,
    CARD_MARCH,
    CARD_DOUBLE_TIME,
    CARD_SALT_ROAD,
    CARD_CARAVAN,
    CARD_DOUBLESTRIKE,
    CARD_HAJJ,
    /* Zarqan (8) */
    CARD_COUNSEL,
    CARD_PILLAGE,
    CARD_ROYAL_DECOY,
    CARD_BAZAAR,
    CARD_STEPPE_RIDERS,
    CARD_AMBITION,
    CARD_CITADEL,
    CARD_CONQUEST,
    /* Caelan (8) */
    CARD_CASTLING,
    CARD_QUEENS_GAMBIT,
    CARD_VENGEANCE,
    CARD_QUEENS_DECREE,
    CARD_CATHEDRAL,
    CARD_CORONATION,
    CARD_CRUSADE,
    CARD_DIVINE_RIGHT,
    /* Mastery-2 figurehead cards (5) */
    CARD_MINGZHUS_SEAL,
    CARD_TOMOHITOS_PATIENCE,
    CARD_SELASSIES_MARCH,
    CARD_TIMURS_CONQUEST,
    CARD_ISABELLAS_CORONATION,
    CARD_ID_COUNT
} CardId;

/*--------------------------------------------------------------------------*\
                              RELIC IDS
\*--------------------------------------------------------------------------*/

typedef enum {
    /* Economy (6) */
    RELIC_MERCHANTS_LEDGER,
    RELIC_MINTED_COIN,
    RELIC_TAX_STAMP,
    RELIC_BULK_DISCOUNT,
    RELIC_WAR_CHEST,
    RELIC_TRADE_ROUTES,
    /* Meter (6) */
    RELIC_SOUL_SHARD,
    RELIC_VETERANS_BOND,
    RELIC_DEAD_MANS_PACT,
    RELIC_IRON_KING,
    RELIC_BLOODTHIRST,
    RELIC_LAST_BREATH,
    /* Cards (5) */
    RELIC_TACTICIANS_SCROLL,
    RELIC_LIBRARIANS_NOTES,
    RELIC_COUNTRY_SEAL,
    RELIC_DEEP_HAND,
    RELIC_GILDED_ARCHIVE,
    /* Combinations (4) */
    RELIC_ALCHEMISTS_KIT,
    RELIC_MASTERS_NOTES,
    RELIC_PHILOSOPHERS_STONE,
    RELIC_INHERITED_POWER,
    /* Board (5) */
    RELIC_EAGLE_EYE,
    RELIC_SURVEYORS_MAP,
    RELIC_FORWARD_COMMAND,
    RELIC_FORTIFIED_LINE,
    RELIC_WARLORDS_BANNER,
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
    /* Economy */
    MODIFIER_LEAN_TIMES,
    MODIFIER_WINDFALL,
    MODIFIER_OPEN_MARKET,
    /* Meter */
    MODIFIER_GLASS_CANNON,
    MODIFIER_BLOODBATH,
    MODIFIER_IRON_WILL,
    /* Cards */
    MODIFIER_RICH_HAND,
    MODIFIER_SPARSE_HAND,
    MODIFIER_KINGDOM_PURITY,
    /* Board */
    MODIFIER_FOG_OF_WAR,
    MODIFIER_DENSE_TERRAIN,
    MODIFIER_EXTENDED_FRONT,
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
    FIGUREHEAD_POWER_LONGWEI,
    FIGUREHEAD_POWER_HARUSHIMA,
    FIGUREHEAD_POWER_KEWARANI,
    FIGUREHEAD_POWER_ZARQAN,
    FIGUREHEAD_POWER_CAELAN,
    FIGUREHEAD_POWER_COUNT
} FigureheadPowerId;

typedef enum {
    ARCHETYPE_SIEGE_ENGINEER,
    ARCHETYPE_RECLAIMER,
    ARCHETYPE_THE_TIDE,
    ARCHETYPE_TRICKSTER,
    ARCHETYPE_THE_HAMMER,
    ARCHETYPE_COUNT
} ArchetypeId;

#endif /* DEFS_H */
