# Regnum Project Structure Report

**Generated**: 2026-06-15
**Source**: Full codebase analysis
**Purpose**: Comprehensive documentation of project structure, structs, functions, enums, and implementation status

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Directory Structure](#2-directory-structure)
3. [Header Files (incl/)](#3-header-files-incl)
4. [Source Files (src/)](#4-source-files-src)
5. [Backend API (Protocol Communication)](#5-backend-api-protocol-communication)
6. [Implementation Status vs GDD](#6-implementation-status-vs-gdd)

---

## 1. Project Overview

Regnum is a turn-based strategy game engine with five kingdoms based on historical chess variants:

- **Longwei Empire** (Xiangqi/Janggi)
- **Harushima Shogunate** (Shogi/Chu Shogi)
- **Kewarani Negusate** (Senterej)
- **Zarqan Sultanate** (Tamerlane/Shatranj)
- **Caelan Kingdom** (standard Chess)

The project uses a headless engine communicating with a SDL frontend via stdio protocol.

---

## 2. Directory Structure

```
regnum/
├── incl/                    # Header files (31 files)
│   ├── prelude.h           # Umbrella include
│   ├── core.h              # Base types (Position, EffectArg, etc.)
│   ├── defs.h              # Constants, limits, ID enums
│   ├── types.h             # Core type aliases, lightweight structs
│   ├── board.h             # Board state
│   ├── piece.h             # Piece templates and runtime state
│   ├── card.h              # Card templates and hand management
│   ├── effect.h            # Effect-bus public interface
│   ├── effects.h           # Effect handler forward declarations
│   ├── movegen.h           # Movement generation API
│   ├── battle.h            # Battle state, config, turn/action API
│   ├── meta.h              # Relics, innates, chains, modifiers, traits, events
│   ├── data.h              # Extern declarations for static data arrays
│   ├── recipe.h            # Combination recipes
│   ├── registry.h         # Accessors for template arrays
│   ├── ai.h                # AI archetypes, weights, action selection
│   ├── engine.h            # Headless engine state
│   ├── app.h               # SDL frontend lifecycle
│   ├── run.h               # Campaign run state
│   ├── screen.h            # Screen v-table and dispatch enum
│   ├── screens.h           # Concrete screen v-table externs
│   ├── ui.h                # Immediate-mode UI helpers
│   ├── input.h             # Input edge-detector
│   ├── log.h               # Minimal logger
│   ├── rng.h               # Deterministic PRNG
│   ├── platform.h          # Platform abstraction
│   ├── profile.h           # Persistent player profile
│   ├── save.h              # Binary chunked save-file codec
│   └── protocol.h          # Stdio grammar between engine and frontend
│
├── src/
│   ├── ai.c               # AI decision-making
│   ├── battle.c            # Battle state management
│   ├── board.c             # Board operations
│   ├── card.c              # Card management
│   ├── effect.c            # Effect bus implementation
│   ├── map.c               # Map state management
│   ├── meta.c              # Meta layer (relics, innates, etc.)
│   ├── piece.c             # Piece operations
│   ├── recipe.c            # Recipe operations
│   ├── registry.c          # Registry accessors
│   ├── rng.c               # Xorshift64 PRNG
│   │
│   ├── data/              # Static game content (18 files)
│   │   ├── data_archetypes.c
│   │   ├── data_caelan.c
│   │   ├── data_chains.c
│   │   ├── data_events.c
│   │   ├── data_figureheads.c
│   │   ├── data_harushima.c
│   │   ├── data_innates.c
│   │   ├── data_kewarani.c
│   │   ├── data_longwei.c
│   │   ├── data_masteries.c
│   │   ├── data_modifiers.c
│   │   ├── data_overseers.c
│   │   ├── data_recipes.c
│   │   ├── data_relics.c
│   │   ├── data_synergies.c
│   │   ├── data_traits.c
│   │   ├── data_universal.c
│   │   └── data_zarqan.c
│   │
│   ├── effects/           # Effect implementations (18 files)
│   │   ├── eff_card.c
│   │   ├── eff_chain.c
│   │   ├── eff_damage.c
│   │   ├── eff_economy.c
│   │   ├── eff_figurehead.c
│   │   ├── eff_flip.c
│   │   ├── eff_innate.c
│   │   ├── eff_mastery.c
│   │   ├── eff_meter.c
│   │   ├── eff_movement.c
│   │   ├── eff_overseer.c
│   │   ├── eff_piece.c
│   │   ├── eff_registry.c
│   │   ├── eff_relic.c
│   │   ├── eff_run.c
│   │   ├── eff_synergy.c
│   │   └── eff_vorath_memory.c
│   │
│   ├── engine/            # Headless engine (17 files)
│   │   ├── engine.c
│   │   ├── main.c
│   │   ├── platform_posix.c
│   │   ├── profile.c
│   │   ├── protocol.c
│   │   ├── run.c
│   │   ├── save.c
│   │   ├── screen.c
│   │   ├── screen_battle.c
│   │   ├── screen_codex.c
│   │   ├── screen_event.c
│   │   ├── screen_map.c
│   │   ├── screen_mastery.c
│   │   ├── screen_results.c
│   │   ├── screen_settings.c
│   │   └── screen_title.c
│   │
│   ├── movegens/          # Movement generators (6 files)
│   │   ├── mg_basics.c
│   │   ├── mg_caelan.c
│   │   ├── mg_harushima.c
│   │   ├── mg_kewarani.c
│   │   ├── mg_longwei.c
│   │   └── mg_zarqan.c
│   │
│   └── sdl/               # SDL frontend (5 files)
│       ├── app.c
│       ├── input.c
│       ├── main.c
│       ├── platform_sdl.c
│       └── ui.c
│
├── res/
│   ├── GDD.md             # Game Design Document
│   └── report_elements.md # Element catalog
│
├── tests/                 # Test files
├── assets/               # Game assets
├── bin/                   # Compiled binaries
└── build/                 # Build artifacts
```

---

## 3. Header Files (incl/)

### 3.1 prelude.h

**Purpose**: Umbrella include file that brings in all other headers.

---

### 3.2 core.h

**Base Types and Enums:**

**Tier Enum** - Campaign/map tier levels:

```c
typedef enum Tier {
    TIER_DISTRICT,
    TIER_TOWN,
    TIER_PROVINCE,
    TIER_COUNTRY,
    TIER_CAPSTONE,
    TIER_KING
} Tier;
```

**Kingdom Enum** - The five playable kingdoms plus NONE:

```c
typedef enum Kingdom {
    KINGDOM_LONGWEI,
    KINGDOM_HARUSHIMA,
    KINGDOM_KEWARANI,
    KINGDOM_ZARQAN,
    KINGDOM_CAELAN,
    KINGDOM_NONE
} Kingdom;
```

**Side Enum** - Which player controls a piece or resource:

```c
typedef enum Side {
    SIDE_PLAYER,
    SIDE_ENEMY,
    SIDE_NEUTRAL
} Side;
```

**BattleResult Enum** - Terminal state of a battle:

```c
typedef enum BattleResult {
    BATTLE_IN_PROGRESS,
    BATTLE_PLAYER_WON,
    BATTLE_ENEMY_WON,
    BATTLE_DRAW
} BattleResult;
```

**EffectArgType Enum** - Type tag for EffectArg union:

```c
typedef enum EffectArgType {
    EARG_INT,
    EARG_FLOAT,
    EARG_PIECE_REF,
    EARG_CARD_REF,
    EARG_TMPL_REF,
    EARG_KINGDOM,
    EARG_TIER,
    EARG_POS,
    EARG_SIDE,
    EARG_MOVEGEN_REF
} EffectArgType;
```

**Position Struct** - Board coordinate:

```c
typedef struct Position {
    int8_t x, y;
} Position;
```

**EffectArg Struct** - Typed argument for effects/movegens:

```c
typedef struct EffectArg {
    EffectArgType type;
    union {
        int i;
        float f;
        uint32_t piece_id;
        uint32_t card_id;
        uint16_t template_id;
        Kingdom kingdom;
        Tier tier;
        Position pos;
        Side side;
        uint16_t movegen_id;
    } as;
} EffectArg;
```

---

### 3.3 defs.h

**Constants and Limits:**

```c
#define MAX_PIECES 128
#define MAX_PIECES_PER_SIDE 64
#define MAX_HAND 12
#define MAX_CARDSET 96
#define MAX_EFFECTS 256
#define MAX_EFFECT_ARGS 6
#define MAX_EFFECT_SCRATCH 2
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
#define MAX_MAP_NODES 24
#define MAX_NODE_EDGES 4
#define MAX_SCREENS 8
#define TARGET_FPS 60
#define SAVE_BUFFER_BYTES 8192
#define MAX_RECIPES 64
#define KINGDOM_COUNT 5
#define TIER_PER_KINGDOM 3
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
```

**PieceId Enum** - 47 piece type identifiers:

| Value          | Kingdom   | Piece Name             |
| -------------- | --------- | ---------------------- |
| 0              | Universal | PIECE_KING             |
| 1              | Longwei   | PIECE_BING             |
| 2              | Longwei   | PIECE_XIANG            |
| 3              | Longwei   | PIECE_MA               |
| 4              | Longwei   | PIECE_PAO              |
| 5              | Longwei   | PIECE_LIUBO_DIVINER    |
| 6              | Longwei   | PIECE_SANG             |
| 7              | Longwei   | PIECE_NORTHERN_CAVALRY |
| 8              | Longwei   | PIECE_HWACHA           |
| 9              | Harushima | PIECE_FUHYO            |
| 10             | Harushima | PIECE_KYOSHA           |
| 11             | Harushima | PIECE_GINSHO           |
| 12             | Harushima | PIECE_KINSHO           |
| 13             | Harushima | PIECE_SHISHI           |
| 14             | Harushima | PIECE_HONORABLE_HORSE  |
| 15             | Harushima | PIECE_PROMOTED_BISHOP  |
| 16             | Harushima | PIECE_DAIMYO           |
| 17             | Harushima | PIECE_DRAGON           |
| 18             | Kewarani  | PIECE_MEDEQ            |
| 19             | Kewarani  | PIECE_MAKWANAM         |
| 20             | Kewarani  | PIECE_SABA             |
| 21             | Kewarani  | PIECE_FARAS            |
| 22             | Kewarani  | PIECE_NEGUS_GUARD      |
| 23             | Kewarani  | PIECE_MEDEQ_SQUAD      |
| 24             | Kewarani  | PIECE_SULTANS_LEVY     |
| 25             | Zarqan    | PIECE_WAZIR            |
| 26             | Zarqan    | PIECE_JAMAL            |
| 27             | Zarqan    | PIECE_TALLIYA          |
| 28             | Zarqan    | PIECE_ZIRAAFA          |
| 29             | Zarqan    | PIECE_SHAHZADEH        |
| 30             | Zarqan    | PIECE_OLD_KING         |
| 31             | Zarqan    | PIECE_CATAPHRACT       |
| 32             | Zarqan    | PIECE_ROOK             |
| 33             | Zarqan    | PIECE_WAR_ELEPHANT     |
| 34             | Caelan    | PIECE_PAWN             |
| 35             | Caelan    | PIECE_KNIGHT           |
| 36             | Caelan    | PIECE_BISHOP           |
| 37             | Caelan    | PIECE_QUEEN            |
| 38             | Caelan    | PIECE_GRYPHON          |
| 39             | Caelan    | PIECE_CHANCELLOR       |
| 40             | Caelan    | PIECE_SOVEREIGN_BANNER |
| 41             | Caelan    | PIECE_KING             |
| PIECE_ID_COUNT | -         | Total count            |

**CardId Enum** - 57 card type identifiers:

| Value         | Kingdom   | Card Name                 |
| ------------- | --------- | ------------------------- |
| 0             | Universal | CARD_PAWN_STORM           |
| 1             | Universal | CARD_REVITALIZE           |
| 2             | Universal | CARD_HOSTAGE              |
| 3             | Universal | CARD_LAST_STAND           |
| 4             | Universal | CARD_SACRIFICE            |
| 5             | Universal | CARD_REFORGE              |
| 6             | Universal | CARD_MERCY                |
| 7             | Universal | CARD_BLOODLETTING         |
| 8             | Universal | CARD_COUNTER_COUP         |
| 9             | Universal | CARD_SPITE                |
| 10            | Universal | CARD_CHAIN_BREAK          |
| 11            | Universal | CARD_HYDRA                |
| 12            | Longwei   | CARD_RIVER_WADE           |
| 13            | Longwei   | CARD_CHARGE               |
| 14            | Longwei   | CARD_FORMATION            |
| 15            | Longwei   | CARD_DIVINATION           |
| 16            | Longwei   | CARD_CANNON_VOLLEY        |
| 17            | Longwei   | CARD_PALACE_DECREE        |
| 18            | Longwei   | CARD_MANDATE              |
| 19            | Harushima | CARD_RONIN                |
| 20            | Harushima | CARD_RESURRECTION         |
| 21            | Harushima | CARD_GOLD_STANDARD        |
| 22            | Harushima | CARD_PROMOTION            |
| 23            | Harushima | CARD_DUAL_DROP            |
| 24            | Harushima | CARD_FORCE_DROP           |
| 25            | Harushima | CARD_BUSHIDO              |
| 26            | Kewarani  | CARD_SULTANS_GOLD         |
| 27            | Kewarani  | CARD_MARCH                |
| 28            | Kewarani  | CARD_DOUBLE_TIME          |
| 29            | Kewarani  | CARD_SALT_ROAD            |
| 30            | Kewarani  | CARD_CARAVAN              |
| 31            | Kewarani  | CARD_DOUBLESTRIKE         |
| 32            | Kewarani  | CARD_HAJJ                 |
| 33            | Zarqan    | CARD_COUNSEL              |
| 34            | Zarqan    | CARD_PILLAGE              |
| 35            | Zarqan    | CARD_ROYAL_DECOY          |
| 36            | Zarqan    | CARD_BAZAAR               |
| 37            | Zarqan    | CARD_STEPPE_RIDERS        |
| 38            | Zarqan    | CARD_AMBITION             |
| 39            | Zarqan    | CARD_CITADEL              |
| 40            | Zarqan    | CARD_CONQUEST             |
| 41            | Caelan    | CARD_CASTLING             |
| 42            | Caelan    | CARD_QUEENS_GAMBIT        |
| 43            | Caelan    | CARD_VENGEANCE            |
| 44            | Caelan    | CARD_QUEENS_DECREE        |
| 45            | Caelan    | CARD_CATHEDRAL            |
| 46            | Caelan    | CARD_CORONATION           |
| 47            | Caelan    | CARD_CRUSADE              |
| 48            | Caelan    | CARD_DIVINE_RIGHT         |
| 49            | Mastery   | CARD_MINGZHUS_SEAL        |
| 50            | Mastery   | CARD_TOMOHITOS_PATIENCE   |
| 51            | Mastery   | CARD_SELASSIES_MARCH      |
| 52            | Mastery   | CARD_TIMURS_CONQUEST      |
| 53            | Mastery   | CARD_ISABELLAS_CORONATION |
| CARD_ID_COUNT | -         | Total count               |

**RelicId Enum** - 26 relic identifiers:

| Value          | Category     | Relic Name               |
| -------------- | ------------ | ------------------------ |
| 0              | Economy      | RELIC_MERCHANTS_LEDGER   |
| 1              | Economy      | RELIC_MINTED_COIN        |
| 2              | Economy      | RELIC_TAX_STAMP          |
| 3              | Economy      | RELIC_BULK_DISCOUNT      |
| 4              | Economy      | RELIC_WAR_CHEST          |
| 5              | Economy      | RELIC_TRADE_ROUTES       |
| 6              | Meter        | RELIC_SOUL_SHARD         |
| 7              | Meter        | RELIC_VETERANS_BOND      |
| 8              | Meter        | RELIC_DEAD_MANS_PACT     |
| 9              | Meter        | RELIC_IRON_KING          |
| 10             | Meter        | RELIC_BLOODTHIRST        |
| 11             | Meter        | RELIC_LAST_BREATH        |
| 12             | Cards        | RELIC_TACTICIANS_SCROLL  |
| 13             | Cards        | RELIC_LIBRARIANS_NOTES   |
| 14             | Cards        | RELIC_COUNTRY_SEAL       |
| 15             | Cards        | RELIC_DEEP_HAND          |
| 16             | Cards        | RELIC_GILDED_ARCHIVE     |
| 17             | Combinations | RELIC_ALCHEMISTS_KIT     |
| 18             | Combinations | RELIC_MASTERS_NOTES      |
| 19             | Combinations | RELIC_PHILOSOPHERS_STONE |
| 20             | Combinations | RELIC_INHERITED_POWER    |
| 21             | Board        | RELIC_EAGLE_EYE          |
| 22             | Board        | RELIC_SURVEYORS_MAP      |
| 23             | Board        | RELIC_FORWARD_COMMAND    |
| 24             | Board        | RELIC_FORTIFIED_LINE     |
| 25             | Board        | RELIC_WARLORDS_BANNER    |
| RELIC_ID_COUNT | -            | Total count              |

**InnateId Enum** - 5 innate ability identifiers:

```c
typedef enum InnateId {
    INNATE_BULWARK,
    INNATE_RECLAIM,
    INNATE_DOUBLE_TIME,
    INNATE_ROYAL_SUBSTITUTION,
    INNATE_CONQUERORS_REWARD
} InnateId;
```

**ChainId Enum** - 3 chain penalty levels:

```c
typedef enum ChainId {
    CHAIN_BRONZE,
    CHAIN_SILVER,
    CHAIN_GOLD,
    CHAIN_COUNT
} ChainId;
```

**ModifierId Enum** - 12 battle modifier identifiers:

| Value          | Category | Modifier Name           |
| -------------- | -------- | ----------------------- |
| 0              | Economy  | MODIFIER_LEAN_TIMES     |
| 1              | Economy  | MODIFIER_WINDFALL       |
| 2              | Economy  | MODIFIER_OPEN_MARKET    |
| 3              | Meter    | MODIFIER_GLASS_CANNON   |
| 4              | Meter    | MODIFIER_BLOODBATH      |
| 5              | Meter    | MODIFIER_IRON_WILL      |
| 6              | Cards    | MODIFIER_RICH_HAND      |
| 7              | Cards    | MODIFIER_SPARSE_HAND    |
| 8              | Cards    | MODIFIER_KINGDOM_PURITY |
| 9              | Board    | MODIFIER_FOG_OF_WAR     |
| 10             | Board    | MODIFIER_DENSE_TERRAIN  |
| 11             | Board    | MODIFIER_EXTENDED_FRONT |
| MODIFIER_COUNT | -        | Total count             |

**TraitId Enum** - 11 board trait identifiers:

```c
typedef enum TraitId {
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
```

**EventId Enum** - 14 event identifiers:

| Value       | Kingdom   | Event Name                       |
| ----------- | --------- | -------------------------------- |
| 0           | Universal | EVENT_UNIVERSAL_WANDERER         |
| 1           | Universal | EVENT_UNIVERSAL_MARKET           |
| 2           | Universal | EVENT_UNIVERSAL_AMBUSH           |
| 3           | Longwei   | EVENT_LONGWEI_RIVER_FESTIVAL     |
| 4           | Longwei   | EVENT_LONGWEI_SKY_LADDER         |
| 5           | Harushima | EVENT_HARUSHIMA_HONOR_TRIAL      |
| 6           | Harushima | EVENT_HARUSHIMA_TAKENOKO_SWARM   |
| 7           | Kewarani  | EVENT_KEWARANI_PILGRIMAGE_SEASON |
| 8           | Kewarani  | EVENT_KEWARANI_BAZAAR_RUMOR      |
| 9           | Zarqan    | EVENT_ZARQAN_SANDSTORM           |
| 10          | Zarqan    | EVENT_ZARQAN_OASIS_DISCOVERY     |
| 11          | Caelan    | EVENT_CAELAN_TOURNAMENT          |
| 12          | Caelan    | EVENT_CAELAN_CASTLE_FESTIVAL     |
| EVENT_COUNT | -         | Total count                      |

**OverseerId Enum** - 6 overseer/boss identifiers:

```c
typedef enum OverseerId {
    OVERSEER_IRON_STRATEGIST,
    OVERSEER_ETERNAL_RECURSION,
    OVERSEER_CARAVAN_OF_CONQUEST,
    OVERSEER_MANY_FACED_KING,
    OVERSEER_CROWNED_HERETIC,
    OVERSEER_VORATH,
    OVERSEER_COUNT
} OverseerId;
```

**SynergyId Enum** - 5 synergy bonuses:

```c
typedef enum SynergyId {
    SYNERGY_LONGWEI,
    SYNERGY_HARUSHIMA,
    SYNERGY_KEWARANI,
    SYNERGY_ZARQAN,
    SYNERGY_CAELAN,
    SYNERGY_COUNT
} SynergyId;
```

**FigureheadPowerId Enum** - 5 figurehead powers:

```c
typedef enum FigureheadPowerId {
    FIGUREHEAD_POWER_LONGWEI,
    FIGUREHEAD_POWER_HARUSHIMA,
    FIGUREHEAD_POWER_KEWARANI,
    FIGUREHEAD_POWER_ZARQAN,
    FIGUREHEAD_POWER_CAELAN,
    FIGUREHEAD_POWER_COUNT
} FigureheadPowerId;
```

**ArchetypeId Enum** - 5 archetype identifiers:

```c
typedef enum ArchetypeId {
    ARCHETYPE_SIEGE_ENGINEER,
    ARCHETYPE_RECLAIMER,
    ARCHETYPE_THE_TIDE,
    ARCHETYPE_TRICKSTER,
    ARCHETYPE_THE_HAMMER,
    ARCHETYPE_COUNT
} ArchetypeId;
```

---

### 3.4 types.h

**Lightweight Structs:**

**CardInstance Struct** - Runtime card in hand:

```c
typedef struct CardInstance {
    const CardTemplate* template;
    uint8_t flags;
} CardInstance;
```

**CardInstanceFlags Enum** - Card flags:

```c
typedef enum CardInstanceFlags {
    CIF_COMBO_SECOND = 1 << 0,
    CIF_COMBO_THIRD = 1 << 1,
    CIF_TARGETED = 1 << 2
} CardInstanceFlags;
```

**TargetSpec Struct** - Card target specification:

```c
typedef struct TargetSpec {
    Position pos;
    uint32_t piece_id;
    bool is_piece;
    bool is_pos;
} TargetSpec;
```

**PieceStateFlags Enum** - Piece flags:

```c
typedef enum PieceStateFlags {
    PSF_HAS_MOVED = 1 << 0,
    PSF_CAN_COMBINE = 1 << 1,
    PSF_IMMUNE_FLIP = 1 << 2,
    PSF_IMMUNE_DAMAGE = 1 << 3,
    PSF_STUNNED = 1 << 4,
    PSF_SPLITTER = 1 << 5
} PieceStateFlags;
```

---

### 3.5 board.h

**Board Struct** - Playing surface:

```c
typedef struct Board {
    struct PieceState* squares[MAX_BOARD_DIM * MAX_BOARD_DIM];
    int width;
    int height;
} Board;
```

---

### 3.6 piece.h

**PieceTemplate Struct** - Immutable piece description:

```c
typedef struct PieceTemplate {
    uint16_t id;
    const char* name;
    Kingdom kingdom;
    Tier tier;
    int base_value;
    MoveGen move;
    MoveGen threat;  // NULL → reuse move
    Effect passives[MAX_PIECE_PASSIVES];
    uint8_t passive_count;
} PieceTemplate;
```

**PieceState Struct** - Runtime piece instance:

```c
typedef struct PieceState {
    uint32_t id;
    const PieceTemplate* template;
    MoveGen move_override;
    MoveGen threat_override;
    Side owner;
    Position pos;
    int value_mod;
    Effect buffs[MAX_PIECE_BUFFS];
    uint8_t buff_count;
    uint8_t moves_used;
    uint16_t flags;
} PieceState;
```

---

### 3.7 card.h

**CardTemplate Struct** - Immutable card description:

```c
typedef struct CardTemplate {
    uint16_t id;
    const char* name;
    Kingdom kingdom;
    Tier tier;
    int play_cost;
    int sell_value;
    Effect on_play[MAX_CARD_EFFECTS];
    uint8_t play_effect_count;
    Effect on_sell[MAX_CARD_EFFECTS];
    uint8_t sell_effect_count;
    TargetFunc target_query;
} CardTemplate;
```

**TargetFunc Typedef** - Target enumeration function pointer:

```c
typedef size_t (*TargetFunc)(
    const BattleState*,
    Side,
    const CardInstance*,
    TargetSpec*,
    size_t
);
```

---

### 3.8 movegen.h

**MoveList Struct** - Fixed-capacity move destination container:

```c
typedef struct MoveList {
    Position squares[MAX_MOVES];
    uint8_t count;
} MoveList;
```

**MoveGen Struct** - Bound movement pattern:

```c
typedef struct MoveGen {
    MoveGenFunc func;
    EffectArg params[MAX_MOVE_PARAMS];
    uint8_t param_count;
} MoveGen;
```

**MoveGenFunc Typedef** - Movement generation function pointer:

```c
typedef void (*MoveGenFunc)(
    const PieceState*,
    const BattleState*,
    const EffectArg*,
    size_t,
    MoveList*
);
```

**Helper Functions:**

```c
void ml_push(MoveList* list, Position pos);
bool is_friendly(const PieceState* piece, Side owner);
bool is_enemy(const PieceState* piece, Side owner);
bool can_move_to(const BattleState* battle, Position pos);
bool can_capture(const BattleState* battle, Position pos);
bool can_capture_or_empty(const BattleState* battle, Position pos);
void mg_generate(const PieceState* piece, const BattleState* battle, MoveList* out);
void mg_generate_threat(const PieceState* piece, const BattleState* battle, MoveList* out);
```

---

### 3.9 effect.h

**EffectTrigger Enum** - 35 trigger types for effect dispatch:

| Value | Trigger Name                     | Purpose                       |
| ----- | -------------------------------- | ----------------------------- |
| 0     | TRIGGER_BATTLE_START             | Battle initialization         |
| 1     | TRIGGER_BATTLE_END               | Battle completion             |
| 2     | TRIGGER_TURN_START               | Turn beginning                |
| 3     | TRIGGER_TURN_END                 | Turn completion               |
| 4     | TRIGGER_RESOLVE_DEFENSE          | Defensive buffs apply         |
| 5     | TRIGGER_RESOLVE_ATTACK           | Offensive buffs apply         |
| 6     | TRIGGER_RESOLVE_DAMAGE           | Damage dealt                  |
| 7     | TRIGGER_RESOLVE_FLIP             | Piece flips                   |
| 8     | TRIGGER_CARD_DRAWN               | Card drawn                    |
| 9     | TRIGGER_CARD_PLAYED              | Card played                   |
| 10    | TRIGGER_CARD_SOLD                | Card sold                     |
| 11    | TRIGGER_COMBO_CHAIN_2            | 2-card combo                  |
| 12    | TRIGGER_COMBO_CHAIN_3            | 3-card combo                  |
| 13    | TRIGGER_PIECE_PLACED             | Piece placed                  |
| 14    | TRIGGER_PIECE_MOVED              | Piece moved                   |
| 15    | TRIGGER_PIECE_COMBINED           | Piece combined                |
| 16    | TRIGGER_PIECE_DEALT_DAMAGE       | Piece dealt damage            |
| 17    | TRIGGER_PIECE_ENTERED_ENEMY_TERR | Piece entered enemy territory |
| 18    | TRIGGER_PIECE_FLIPPED            | Piece flipped                 |
| 19    | TRIGGER_PIECE_REMOVED            | Piece removed                 |
| 20    | TRIGGER_QUERY_PIECE_COST         | Query piece cost              |
| 21    | TRIGGER_QUERY_SELL_VALUE         | Query sell value              |
| 22    | TRIGGER_QUERY_DRAW_COUNT         | Query draw count              |
| 23    | TRIGGER_QUERY_TURN_INCOME        | Query turn income             |
| 24    | TRIGGER_QUERY_MOVE_COUNT         | Query move count              |
| 25    | TRIGGER_QUERY_DAMAGE_MULT        | Query damage multiplier       |
| 26    | TRIGGER_QUERY_METER_CAP          | Query meter capacity          |
| 27    | TRIGGER_QUERY_COMBINE_COST       | Query combine cost            |
| 28    | TRIGGER_QUERY_COMBO_VALUE_BONUS  | Query combo value bonus       |
| 29    | TRIGGER_QUERY_ADJ_KING_BONUS     | Query adjacent king bonus     |
| 30    | TRIGGER_QUERY_RECLAIM_COST       | Query reclaim cost            |
| 31    | TRIGGER_QUERY_ROYAL_SUB_COUNT    | Query royal sub count         |
| 32    | TRIGGER_QUERY_VISION_FLAGS       | Query vision flags            |
| 33    | TRIGGER_MAP_ENTERED              | Map entered                   |
| 34    | TRIGGER_RUN_START                | Run started                   |
| 35    | TRIGGER_COUNT                    | Total count                   |

**PlacementCause Enum** - Why a piece appeared on the board:

```c
typedef enum PlacementCause {
    PLACED_SPAWN,
    PLACED_BOUGHT,
    PLACED_COMBINE_RESULT,
    PLACED_SPLIT
} PlacementCause;
```

**FlipCause Enum** - Why a piece changed owner:

```c
typedef enum FlipCause {
    FLIPPED_METER_CASCADE,
    FLIPPED_RECLAIM,
    FLIPPED_FORCED,
    FLIPPED_MERCY
} FlipCause;
```

**RemovalCause Enum** - Why a piece left the board:

```c
typedef enum RemovalCause {
    REMOVED_SACRIFICE,
    REMOVED_MANDATE,
    REMOVED_SPLITTER_SUBSTITUTION
} RemovalCause;
```

**EffectFunc Typedef** - Effect function pointer:

```c
typedef void (*EffectFunc)(EffectCtx*, const EffectArg*, size_t);
```

**Effect Struct** - Behavior entry registered with the bus:

```c
typedef struct Effect {
    EffectTrigger trigger;
    EffectFuncId func_id;
    EffectFunc apply;
    EffectArg args[MAX_EFFECT_ARGS];
    uint8_t arg_count;
    EffectArg scratch[MAX_EFFECT_SCRATCH];
    int16_t duration_turns;
    Side owner;
    uint32_t source_id;
} Effect;
```

**EffectSlot Struct** - Bus-internal effect entry:

```c
typedef struct EffectSlot {
    Effect effect;
    int16_t remaining_turns;
    bool active;
} EffectSlot;
```

**EffectBus Struct** - Effect dispatch container:

```c
typedef struct EffectBus {
    EffectSlot slots[MAX_EFFECTS];
    uint16_t count;
} EffectBus;
```

**EffectCtx Struct** - Per-trigger evidence union:

```c
typedef struct EffectCtx {
    EffectTrigger trigger;
    BattleState* battle;
    PieceState* self;
    union {
        struct {
            PieceState* attacker;
            PieceState* target;
            int* damage_out;
            int* reduction_out;
        } resolve;
        struct {
            PieceState* piece;
            Position at;
            PlacementCause cause;
        } placed;
        struct {
            PieceState* piece;
            Side new_owner;
            FlipCause cause;
        } flipped;
        struct {
            PieceState* piece;
            RemovalCause cause;
        } removed;
        struct {
            PieceState* piece;
        } piece;
        struct {
            const CardInstance* card;
            int* value_out;
            size_t* count_out;
        } card;
        struct {
            int* income_out;
            int* cost_out;
            int* meter_cap_out;
            int* moves_out;
            uint32_t* vision_flags_out;
        } query;
        struct {
            const PieceState* piece;
            int* moves_out;
        } movement;
        struct {
            Kingdom kingdom;
        } meta;
        struct {
            int delta;
        } generic;
    } as;
} EffectCtx;
```

**EffectFuncId Enum** - 96 effect function identifiers:

| Value | Function Name                            | Category    |
| ----- | ---------------------------------------- | ----------- |
| 0     | EFFECT_FUNC_MERCHANTS_LEDGER             | Relic       |
| 1     | EFFECT_FUNC_MINTED_COIN                  | Relic       |
| 2     | EFFECT_FUNC_TAX_STAMP                    | Relic       |
| 3     | EFFECT_FUNC_BULK_DISCOUNT                | Relic       |
| 4     | EFFECT_FUNC_WAR_CHEST                    | Relic       |
| 5     | EFFECT_FUNC_TRADE_ROUTES                 | Relic       |
| 6     | EFFECT_FUNC_SOUL_SHARD                   | Relic       |
| 7     | EFFECT_FUNC_VETERANS_BOND                | Relic       |
| 8     | EFFECT_FUNC_DEAD_MANS_PACT               | Relic       |
| 9     | EFFECT_FUNC_IRON_KING                    | Relic       |
| 10    | EFFECT_FUNC_BLOODTHIRST                  | Relic       |
| 11    | EFFECT_FUNC_LAST_BREATH                  | Relic       |
| 12    | EFFECT_FUNC_TACTICIANS_SCROLL            | Relic       |
| 13    | EFFECT_FUNC_LIBRARIANS_NOTES             | Relic       |
| 14    | EFFECT_FUNC_COUNTRY_SEAL                 | Relic       |
| 15    | EFFECT_FUNC_DEEP_HAND                    | Relic       |
| 16    | EFFECT_FUNC_GILDED_ARCHIVE               | Relic       |
| 17    | EFFECT_FUNC_ALCHEMISTS_KIT               | Relic       |
| 18    | EFFECT_FUNC_MASTERS_NOTES                | Relic       |
| 19    | EFFECT_FUNC_PHILOSOPHERS_STONE           | Relic       |
| 20    | EFFECT_FUNC_INHERITED_POWER              | Relic       |
| 21    | EFFECT_FUNC_EAGLE_EYE                    | Relic       |
| 22    | EFFECT_FUNC_SURVEYORS_MAP                | Relic       |
| 23    | EFFECT_FUNC_FORWARD_COMMAND              | Relic       |
| 24    | EFFECT_FUNC_FORTIFIED_LINE               | Relic       |
| 25    | EFFECT_FUNC_WARLORDS_BANNER              | Relic       |
| 26    | EFFECT_FUNC_CHAIN_BRONZE                 | Chain       |
| 27    | EFFECT_FUNC_CHAIN_SILVER                 | Chain       |
| 28    | EFFECT_FUNC_CHAIN_GOLD                   | Chain       |
| 29    | EFFECT_FUNC_INNATE_BULWARK               | Innate      |
| 30    | EFFECT_FUNC_INNATE_RECLAIM               | Innate      |
| 31    | EFFECT_FUNC_INNATE_DOUBLE_TIME           | Innate      |
| 32    | EFFECT_FUNC_INNATE_ROYAL_SUBSTITUTION    | Innate      |
| 33    | EFFECT_FUNC_INNATE_CONQUERORS_REWARD     | Innate      |
| 34    | EFFECT_FUNC_FH_MINGZHU                   | Figurehead  |
| 35    | EFFECT_FUNC_FH_TOMOHITO                  | Figurehead  |
| 36    | EFFECT_FUNC_FH_SELASSIE                  | Figurehead  |
| 37    | EFFECT_FUNC_FH_TIMUR                     | Figurehead  |
| 38    | EFFECT_FUNC_FH_ISABELLA                  | Figurehead  |
| 39    | EFFECT_FUNC_SYN_LONGWEI                  | Synergy     |
| 40    | EFFECT_FUNC_SYN_HARUSHIMA                | Synergy     |
| 41    | EFFECT_FUNC_SYN_KEWARANI                 | Synergy     |
| 42    | EFFECT_FUNC_SYN_ZARQAN                   | Synergy     |
| 43    | EFFECT_FUNC_SYN_CAELAN                   | Synergy     |
| 44    | EFFECT_FUNC_MASTERY_LONGWEI_L1           | Mastery     |
| 45    | EFFECT_FUNC_MASTERY_LONGWEI_L2           | Mastery     |
| 46    | EFFECT_FUNC_MASTERY_LONGWEI_L3           | Mastery     |
| 47    | EFFECT_FUNC_MASTERY_HARUSHIMA_L1         | Mastery     |
| 48    | EFFECT_FUNC_MASTERY_HARUSHIMA_L2         | Mastery     |
| 49    | EFFECT_FUNC_MASTERY_HARUSHIMA_L3         | Mastery     |
| 50    | EFFECT_FUNC_MASTERY_KEWARANI_L1          | Mastery     |
| 51    | EFFECT_FUNC_MASTERY_KEWARANI_L2          | Mastery     |
| 52    | EFFECT_FUNC_MASTERY_KEWARANI_L3          | Mastery     |
| 53    | EFFECT_FUNC_MASTERY_ZARQAN_L1            | Mastery     |
| 54    | EFFECT_FUNC_MASTERY_ZARQAN_L2            | Mastery     |
| 55    | EFFECT_FUNC_MASTERY_ZARQAN_L3            | Mastery     |
| 56    | EFFECT_FUNC_MASTERY_CAELAN_L1            | Mastery     |
| 57    | EFFECT_FUNC_MASTERY_CAELAN_L2            | Mastery     |
| 58    | EFFECT_FUNC_MASTERY_CAELAN_L3            | Mastery     |
| 59    | EFFECT_FUNC_OVERSEER_IRON_STRATEGIST     | Overseer    |
| 60    | EFFECT_FUNC_OVERSEER_ETERNAL_RECURSION   | Overseer    |
| 61    | EFFECT_FUNC_OVERSEER_CARAVAN_OF_CONQUEST | Overseer    |
| 62    | EFFECT_FUNC_OVERSEER_MANY_FACED_KING     | Overseer    |
| 63    | EFFECT_FUNC_OVERSEER_CROWNED_HERETIC     | Overseer    |
| 64    | EFFECT_FUNC_VORATH_MEMORY_TALLY          | Vorath      |
| 65    | EFFECT_FUNC_VORATH_MEMORY_APPLY          | Vorath      |
| 66    | EFFECT_FUNC_TODO                         | Placeholder |
| 67    | EFFECT_FUNC_COUNT                        | Total count |

---

### 3.10 battle.h

**BattleConfig Struct** - Battle initialization parameters:

```c
typedef struct BattleConfig {
    int width, height;
    int max_turns;
    int starting_cp;
    uint64_t rng_seed;
    Side player_side;
    const Modifier* modifiers[MAX_BATTLE_MODIFIERS];
    uint8_t modifier_count;
    const BoardTrait* traits[MAX_BOARD_TRAITS];
    uint8_t trait_count;
    RunState* run;
} BattleConfig;
```

**EventKind Enum** - 19 event log entry types:

| Value | Event Name             | Purpose            |
| ----- | ---------------------- | ------------------ |
| 0     | EVT_TURN_STARTED       | Turn began         |
| 1     | EVT_TURN_ENDED         | Turn ended         |
| 2     | EVT_RESOLVE_BEGAN      | Resolution started |
| 3     | EVT_RESOLVE_ENDED      | Resolution ended   |
| 4     | EVT_PIECE_PLACED       | Piece placed       |
| 5     | EVT_PIECE_MOVED        | Piece moved        |
| 6     | EVT_PIECE_COMBINED     | Piece combined     |
| 7     | EVT_PIECE_REMOVED      | Piece removed      |
| 8     | EVT_PIECE_FLIPPED      | Piece flipped      |
| 9     | EVT_PIECE_DEALT_DAMAGE | Piece dealt damage |
| 10    | EVT_METER_CHANGED      | Meter changed      |
| 11    | EVT_CP_CHANGED         | CP changed         |
| 12    | EVT_CARD_DRAWN         | Card drawn         |
| 13    | EVT_CARD_PLAYED        | Card played        |
| 14    | EVT_CARD_SOLD          | Card sold          |
| 15    | EVT_EFFECT_APPLIED     | Effect applied     |
| 16    | EVT_BATTLE_ENDED       | Battle ended       |
| 17    | EVT_COUNT              | Total count        |

**Event Struct** - Event log entry:

```c
typedef struct Event {
    EventKind kind;
    uint16_t turn_no;
    union {
        struct {
            uint32_t piece_id;
            Position from, to;
        } moved;
        struct {
            uint32_t piece_id;
            Position at;
        } placed;
        struct {
            uint32_t piece_id;
            Side new_owner;
        } flipped;
        struct {
            uint32_t piece_id;
            int damage;
        } dealt_damage;
        struct {
            Side side;
            int old_meter, new_meter;
        } meter;
        struct {
            Side side;
            int old_cp, new_cp;
        } cp;
        struct {
            Side side;
            const CardTemplate* card;
        } card;
        struct {
            const char* effect_name;
        } effect;
    } as;
} Event;
```

**BattleState Struct** - Complete battle model:

```c
typedef struct BattleState {
    Board board;
    BattleConfig config;
    Side active_side;
    uint16_t turn_no, max_turns;
    int cp[2], meter[2], meter_cap[2], meter_overflow_cap[2];
    PieceState pieces[MAX_PIECES];
    uint16_t piece_count;
    uint32_t next_piece_id;
    CardInstance hand[2][MAX_HAND];
    uint8_t hand_count[2];
    const CardTemplate* cardset[2][MAX_CARDSET];
    uint16_t cardset_count[2];
    uint8_t actions_left;
    EffectBus bus;
    Rng rng;
    Event events[MAX_EVENTS];
    uint16_t event_head, event_count;
    bool battle_ended;
    BattleResult result;
} BattleState;
```

---

### 3.11 meta.h

**RelicTemplate Struct** - Persistent relic item:

```c
typedef struct RelicTemplate {
    uint16_t id;
    const char* name;
    const char* description;
    Effect effects[8];
    uint8_t effect_count;
} RelicTemplate;
```

**InnateTemplate Struct** - Kingdom passive ability:

```c
typedef struct InnateTemplate {
    Kingdom kingdom;
    const char* name;
    const char* description;
    Effect effects[8];
    uint8_t effect_count;
} InnateTemplate;
```

**Chain Struct** - Penalty chain structure:

```c
typedef struct Chain {
    uint8_t level;  // 0=Bronze, 1=Silver, 2=Gold
    const char* name;
    const char* description;
    Effect penalties[8];
    uint8_t penalty_count;
} Chain;
```

**Modifier Struct** - Battle modifier:

```c
typedef struct Modifier {
    uint16_t id;
    const char* name;
    const char* description;
    ModifierId type;
    Effect effects[4];
    uint8_t effect_count;
} Modifier;
```

**BoardTrait Struct** - Board condition:

```c
typedef struct BoardTrait {
    uint16_t id;
    const char* name;
    const char* description;
    TraitId type;
    Effect effects[4];
    uint8_t effect_count;
} BoardTrait;
```

**FigureheadPower Struct** - Kingdom power after clearing:

```c
typedef struct FigureheadPower {
    Kingdom kingdom;
    const char* name;
    const char* description;
    Effect effects[4];
    uint8_t effect_count;
} FigureheadPower;
```

**Synergy Struct** - Bonus after clearing overseer:

```c
typedef struct Synergy {
    Kingdom cleared;
    const char* name;
    const char* description;
    Effect bonus;
} Synergy;
```

**Overseer Struct** - Boss piece:

```c
typedef struct Overseer {
    uint16_t id;
    const char* name;
    const char* description;
    Effect effects[8];
    uint8_t effect_count;
} Overseer;
```

**MasteryHook Struct** - Kingdom mastery hook:

```c
typedef struct MasteryHook {
    Kingdom kingdom;
    int level;  // 1 or 3
    const char* name;
    Effect effect;
} MasteryHook;
```

**EventTemplate Struct** - Map event:

```c
typedef struct EventTemplate {
    uint16_t id;
    const char* name;
    const char* description;
    EventId type;
} EventTemplate;
```

**EventOption Struct** - Event choice:

```c
typedef struct EventOption {
    const char* option_text;
    Effect effect;
} EventOption;
```

---

### 3.12 map.h

**NodeType Enum** - 7 overworld node classifications:

| Value | Node Type             | Purpose            |
| ----- | --------------------- | ------------------ |
| 0     | NODE_BATTLE           | Standard combat    |
| 1     | NODE_ELITE            | Harder combat      |
| 2     | NODE_ARCHIVE          | Recipe reveal      |
| 3     | NODE_OFFERING         | Card removal       |
| 4     | NODE_EVENT            | Narrative event    |
| 5     | NODE_OVERSEER         | Boss fight         |
| 6     | NODE_LIBERATION_TRIAL | Liberation attempt |
| 7     | NODE_TYPE_COUNT       | Total count        |

**MapNode Struct** - Overworld node:

```c
typedef struct MapNode {
    uint16_t id;
    uint8_t type;  // NodeType
    uint8_t kingdom;
    uint16_t payload_id;
    uint16_t modifier_id;
    uint16_t trait_id;
    bool visited;
    bool revealed;
    uint8_t edge_count;
    uint16_t edges[MAX_NODE_EDGES];
} MapNode;
```

**MapState Struct** - Current overworld map:

```c
typedef struct MapState {
    Kingdom kingdom;
    Tier map_tier;
    uint64_t seed;
    MapNode nodes[MAX_MAP_NODES];
    uint8_t node_count;
    uint16_t entry_node_id;
    uint16_t current_node_id;
} MapState;
```

---

### 3.13 recipe.h

**Recipe Struct** - Piece combination:

```c
typedef struct Recipe {
    uint16_t ingredient_a;
    uint16_t ingredient_b;
    uint16_t result;
    uint8_t steps;  // 1 = direct, 2 = multi-step
} Recipe;
```

---

### 3.14 ai.h

**ActionKind Enum** - 7 AI action types:

| Value | Action Kind      | Purpose        |
| ----- | ---------------- | -------------- |
| 0     | ACTION_NONE      | No action      |
| 1     | ACTION_MOVE      | Move a piece   |
| 2     | ACTION_BUY       | Buy a piece    |
| 3     | ACTION_COMBINE   | Combine pieces |
| 4     | ACTION_PLAY_CARD | Play a card    |
| 5     | ACTION_SELL_CARD | Sell a card    |
| 6     | ACTION_END_TURN  | End turn       |

**Action Struct** - AI action:

```c
typedef struct Action {
    ActionKind kind;
    union {
        struct {
            uint32_t piece_id;
            Position to;
        } move;
        struct {
            uint16_t template_id;
            Position at;
        } buy;
        struct {
            uint32_t a, b;
        } combine;
        struct {
            uint8_t hand_index;
            TargetSpec target;
        } play_card;
        struct {
            uint8_t hand_index;
        } sell_card;
    } as;
} Action;
```

**AIWeights Struct** - AI scoring weights:

```c
typedef struct AIWeights {
    int value_diff_w;
    int territory_w;
    int aggression_w;
    int sell_threshold;
    int save_threshold;
    int max_piece_cost;
    int combo_chain_bonus;
    int reclaim_priority;
    int extra[4];
} AIWeights;
```

**AIArchetype Struct** - AI personality:

```c
typedef struct AIArchetype {
    const char* name;
    Kingdom kingdom;
    AIPickFunc pick;  // NULL for default scoring
    AIWeights primary;
    AIWeights fallback;
    int fallback_meter_diff;
} AIArchetype;
```

**AIPickFunc Typedef** - AI pick function pointer:

```c
typedef Action (*AIPickFunc)(const BattleState*);
```

---

### 3.15 engine.h

**TitleState Struct** - Title screen scratch:

```c
typedef struct TitleState {
    bool resume_available;
} TitleState;
```

**MapStateUi Struct** - Map screen scratch:

```c
typedef struct MapStateUi {
    uint8_t node_cursor;
} MapStateUi;
```

**CodexState Struct** - Codex popup scratch:

```c
typedef struct CodexState {
    char kind[16];
    bool open;
} CodexState;
```

**ResultsState Struct** - Results screen scratch:

```c
typedef struct ResultsState {
    bool outcome;
} ResultsState;
```

**EngineState Struct** - Top-level engine state:

```c
typedef struct EngineState {
    FILE* in, *out;
    ScreenId current, next;
    bool transition_pending, quitting;
    struct Profile* profile;
    struct RunState* run;
    struct BattleState* battle;
    TitleState title;
    MapStateUi map;
    CodexState codex;
    ResultsState results;
} EngineState;
```

---

### 3.16 app.h

**FrontendModel Struct** - Frontend view mirror:

```c
typedef struct FrontendModel {
    char screen[32], detail[256], popup[32], popup_detail[256];
    bool engine_quit;
} FrontendModel;
```

**App Struct** - SDL frontend state:

```c
typedef struct App {
    SDL_Window* window;
    SDL_Renderer* renderer;
    Input input;
    uint64_t last_tick_ns;
    pid_t engine_pid;
    FILE* engine_in, engine_out;
    char read_buffer[1024];
    size_t read_length;
    FrontendModel model;
} App;
```

---

### 3.17 run.h

**RunEnd Enum** - Run outcome types:

```c
typedef enum RunEnd {
    RUN_END_VORATH_WIN,
    RUN_END_LOSS
} RunEnd;
```

**RunState Struct** - Campaign run state:

```c
typedef struct RunState {
    uint64_t run_seed;
    Kingdom current_kingdom;
    Tier current_map_tier;
    MapState current_map;
    RelicId relic_ids[MAX_RELICS_HELD];
    uint8_t relic_count;
    uint8_t chain_levels[KINGDOM_COUNT];
    bool subjugated[KINGDOM_COUNT];
    uint8_t liberation_respawn_counter;
    uint16_t vorath_counter;
    uint8_t vorath_pressure;
    bool cleared_maps[KINGDOM_COUNT][TIER_PER_KINGDOM];
    bool cleared_kingdoms[KINGDOM_COUNT];
    bool mastery_disqualified[KINGDOM_COUNT];
    uint8_t mastery_l3[KINGDOM_COUNT];
    uint64_t revealed_recipes;
    uint64_t forbidden_recipes;
    uint32_t flags;
    uint8_t chain_silver_pending[KINGDOM_COUNT];
    struct Profile* profile;
} RunState;
```

---

### 3.18 screen.h

**ScreenId Enum** - 9 screen identifiers:

| Value | Screen ID       | Purpose             |
| ----- | --------------- | ------------------- |
| 0     | SCREEN_TITLE    | Title/start screen  |
| 1     | SCREEN_MAP      | Overworld map       |
| 2     | SCREEN_BATTLE   | Battle screen       |
| 3     | SCREEN_EVENT    | Narrative events    |
| 4     | SCREEN_RESULTS  | End-of-run summary  |
| 5     | SCREEN_CODEX    | Codex browser       |
| 6     | SCREEN_MASTERY  | Mastery progression |
| 7     | SCREEN_SETTINGS | Settings            |
| 8     | SCREEN_COUNT    | Total count         |

**Screen Struct** - Screen v-table:

```c
typedef struct Screen {
    void (*enter)(struct EngineState*);
    void (*leave)(struct EngineState*);
    void (*handle)(struct EngineState*, const struct ProtocolVerb*);
    void (*emit)(struct EngineState*);
} Screen;
```

---

### 3.19 profile.h

**Profile Struct** - Persistent player data:

```c
typedef struct Profile {
    uint32_t version;
    uint8_t mastery_levels[KINGDOM_COUNT];
    uint64_t codex_bits[2];
    uint8_t prestige_tier;
    uint32_t vorath_defeat_count;
    uint16_t vorath_memory[PIECE_ID_COUNT];
    uint32_t total_wins;
    uint32_t total_losses;
} Profile;
```

---

### 3.20 save.h

**SaveChunkId Enum** - 3 save chunk type tags:

```c
typedef enum SaveChunkId {
    CHUNK_PROFILE = 0x46524F50u,  // 'PROF'
    CHUNK_RUN_META = 0x4D4E5552u, // 'RUNM'
    CHUNK_MAP_STATE = 0x5350414Du // 'MAPS'
} SaveChunkId;
```

**SaveWriter Struct** - Fixed-capacity write buffer:

```c
typedef struct SaveWriter {
    uint8_t buf[SAVE_BUFFER_BYTES];
    uint32_t pos, chunk_origin;
    bool in_chunk, overflow;
} SaveWriter;
```

**SaveReader Struct** - Read-only save file view:

```c
typedef struct SaveReader {
    uint8_t buf[SAVE_BUFFER_BYTES];
    uint32_t total, pos;
    uint16_t version;
    uint32_t chunk_count;
} SaveReader;
```

---

### 3.21 protocol.h

**ProtocolSeverity Enum** - Log message severity:

```c
typedef enum ProtocolSeverity {
    PROTOCOL_LOG_INFO,
    PROTOCOL_LOG_WARN,
    PROTOCOL_LOG_ERR
} ProtocolSeverity;
```

**ProtocolVerb Struct** - Parsed frontend command:

```c
typedef struct ProtocolVerb {
    char verb[PROTOCOL_VERB_BYTES];
    const char* tail;
} ProtocolVerb;
```

---

### 3.22 Other Headers Summary

| Header       | Purpose                                                        |
| ------------ | -------------------------------------------------------------- |
| `input.h`    | Frame-grained keyboard state with down/pressed/released arrays |
| `log.h`      | Minimal logger                                                 |
| `rng.h`      | Xorshift64 PRNG state                                          |
| `platform.h` | Platform abstraction for file I/O                              |
| `screens.h`  | Concrete screen v-table externs                                |
| `ui.h`       | Immediate-mode UI helpers                                      |
| `data.h`     | Extern declarations for static data arrays                     |
| `registry.h` | Accessors for template arrays                                  |

---

## 4. Source Files (src/)

### 4.1 Core Files

#### src/ai.c

**AI Decision-Making Functions:**

| Function            | Signature                                                                            | Purpose                                                     |
| ------------------- | ------------------------------------------------------------------------------------ | ----------------------------------------------------------- |
| `ai_play_turn`      | `void ai_play_turn(BattleState* battle)`                                             | Executes the full AI half-turn, loops while actions remain  |
| `ai_pick_one`       | `Action ai_pick_one(const BattleState* battle)`                                      | Picks a single action (currently: random move or sell card) |
| `ai_score_move`     | `int ai_score_move(const BattleState* battle, const PieceState* piece, Position to)` | Stub returning 0 (for future implementation)                |
| `ai_score_buy`      | `int ai_score_buy(const BattleState* battle, uint16_t template_id)`                  | Stub returning 0 (for future implementation)                |
| `ai_execute_action` | `void ai_execute_action(BattleState* battle, Action action)`                         | Executes chosen action types                                |

#### src/battle.c

**Battle State Management Functions:**

| Function                | Signature                                                                                                                        | Purpose                               |
| ----------------------- | -------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------- |
| `battle_init`           | `BattleState* battle_init(const BattleConfig* config)`                                                                           | Initialize battle from config         |
| `battle_destroy`        | `void battle_destroy(BattleState* battle)`                                                                                       | Free battle resources                 |
| `battle_action_move`    | `bool battle_action_move(BattleState* battle, uint32_t piece_id, Position to)`                                                   | Execute move action                   |
| `battle_action_buy`     | `bool battle_action_buy(BattleState* battle, uint16_t template_id, Position at)`                                                 | Execute buy action                    |
| `battle_action_combine` | `bool battle_action_combine(BattleState* battle, uint32_t a, uint32_t b)`                                                        | Execute combine action                |
| `battle_play_card`      | `bool battle_play_card(BattleState* battle, uint8_t hand_index, TargetSpec target)`                                              | Play a card from hand                 |
| `battle_sell_card`      | `bool battle_sell_card(BattleState* battle, uint8_t hand_index)`                                                                 | Sell a card from hand                 |
| `battle_draw_cards`     | `void battle_draw_cards(BattleState* battle, Side side, uint8_t count)`                                                          | Draw cards from cardset               |
| `battle_resolve`        | `void battle_resolve(BattleState* battle)`                                                                                       | Process end-of-turn damage resolution |
| `battle_check_end`      | `BattleResult battle_check_end(BattleState* battle)`                                                                             | Check win/loss conditions             |
| `battle_get_piece`      | `PieceState* battle_get_piece(BattleState* battle, uint32_t id)`                                                                 | Get piece by ID                       |
| `battle_get_piece_at`   | `PieceState* battle_get_piece_at(BattleState* battle, Position pos)`                                                             | Get piece at position                 |
| `battle_add_piece`      | `PieceState* battle_add_piece(BattleState* battle, const PieceTemplate* tmpl, Position pos, Side owner)`                         | Add piece to board                    |
| `battle_remove_piece`   | `void battle_remove_piece(BattleState* battle, uint32_t id)`                                                                     | Remove piece from board               |
| `battle_flip_piece`     | `void battle_flip_piece(BattleState* battle, uint32_t id, Side new_owner)`                                                       | Flip piece ownership                  |
| `battle_spawn_piece`    | `PieceState* battle_spawn_piece(BattleState* battle, const PieceTemplate* tmpl, Position pos, Side owner, PlacementCause cause)` | Spawn piece at position               |

#### src/board.c

**Board Operations Functions:**

| Function             | Signature                                                                                        | Purpose                           |
| -------------------- | ------------------------------------------------------------------------------------------------ | --------------------------------- |
| `board_init`         | `void board_init(Board* board, int width, int height)`                                           | Initialize board with dimensions  |
| `board_get_piece`    | `PieceState* board_get_piece(const Board* board, Position pos)`                                  | Get piece at position             |
| `board_set_piece`    | `void board_set_piece(Board* board, Position pos, PieceState* piece)`                            | Set piece at position             |
| `board_clear_square` | `void board_clear_square(Board* board, Position pos)`                                            | Clear position                    |
| `board_is_valid`     | `bool board_is_valid(const Board* board, Position pos)`                                          | Check if position is on board     |
| `board_territory`    | `Side board_territory(const Board* board, Position pos, const PieceState* pieces, size_t count)` | Calculate territory control       |
| `board_threatens`    | `bool board_threatens(const Board* board, const PieceState* piece, Position pos)`                | Check if piece threatens position |

#### src/card.c

**Card Management Functions:**

| Function              | Signature                                                                                                                     | Purpose                            |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------- | ---------------------------------- |
| `card_target_default` | `size_t card_target_default(const BattleState* battle, Side side, const CardInstance* card, TargetSpec* targets, size_t max)` | Default target enumeration         |
| `card_draw`           | `void card_draw(BattleState* battle, Side side, uint8_t count)`                                                               | Draw cards from cardset to hand    |
| `card_auto_sell`      | `void card_auto_sell(BattleState* battle, Side side)`                                                                         | Auto-sell unsold cards at turn end |
| `card_combo_check`    | `void card_combo_check(BattleState* battle, Side side)`                                                                       | Check for combo chains             |

#### src/effect.c

**Effect Bus Implementation Functions:**

| Function            | Signature                                                          | Purpose                              |
| ------------------- | ------------------------------------------------------------------ | ------------------------------------ |
| `effect_bus_init`   | `void effect_bus_init(EffectBus* bus)`                             | Initialize effect bus                |
| `effect_bus_add`    | `void effect_bus_add(EffectBus* bus, const Effect* effect)`        | Add effect to bus                    |
| `effect_bus_remove` | `void effect_bus_remove(EffectBus* bus, uint32_t source_id)`       | Remove effect from bus               |
| `effect_bus_emit`   | `void effect_bus_emit(EffectBus* bus, EffectCtx* ctx)`             | Emit trigger to all matching effects |
| `effect_bus_clear`  | `void effect_bus_clear(EffectBus* bus)`                            | Clear all effects                    |
| `effect_find_slot`  | `EffectSlot* effect_find_slot(EffectBus* bus, uint32_t source_id)` | Find slot by source_id               |
| `effect_tick`       | `void effect_tick(EffectBus* bus)`                                 | Decrement duration counters          |

#### src/map.c

**Map State Management Functions:**

| Function          | Signature                                                                        | Purpose                  |
| ----------------- | -------------------------------------------------------------------------------- | ------------------------ |
| `map_generate`    | `void map_generate(MapState* map, Kingdom kingdom, Tier tier, uint64_t seed)`    | Generate map from seed   |
| `map_get_node`    | `MapNode* map_get_node(MapState* map, uint16_t id)`                              | Get node by ID           |
| `map_select_node` | `bool map_select_node(MapState* map, uint16_t node_id)`                          | Select and move to node  |
| `map_adjacent`    | `uint8_t map_adjacent(const MapState* map, uint16_t node_id, uint16_t* out_ids)` | Get adjacent nodes       |
| `map_advance`     | `bool map_advance(RunState* run)`                                                | Advance to next map tier |

#### src/meta.c

**Meta Layer Functions:**

| Function                | Signature                                                             | Purpose                       |
| ----------------------- | --------------------------------------------------------------------- | ----------------------------- |
| `meta_apply_relic`      | `void meta_apply_relic(EffectBus* bus, const RelicTemplate* relic)`   | Apply relic effects to battle |
| `meta_apply_innate`     | `void meta_apply_innate(EffectBus* bus, Kingdom kingdom)`             | Apply innate ability          |
| `meta_apply_chain`      | `void meta_apply_chain(EffectBus* bus, uint8_t level)`                | Apply chain penalty           |
| `meta_apply_synergy`    | `void meta_apply_synergy(EffectBus* bus, Kingdom cleared)`            | Apply synergy bonus           |
| `meta_apply_figurehead` | `void meta_apply_figurehead(EffectBus* bus, Kingdom kingdom)`         | Apply figurehead power        |
| `meta_apply_mastery`    | `void meta_apply_mastery(EffectBus* bus, Kingdom kingdom, int level)` | Apply mastery hook            |

#### src/piece.c

**Piece Operations Functions:**

| Function            | Signature                                                       | Purpose                            |
| ------------------- | --------------------------------------------------------------- | ---------------------------------- |
| `piece_value`       | `int piece_value(const PieceState* piece)`                      | Calculate piece value (base + mod) |
| `piece_can_move`    | `bool piece_can_move(const PieceState* piece)`                  | Check if piece can move            |
| `piece_has_moved`   | `bool piece_has_moved(const PieceState* piece)`                 | Check if piece has moved this turn |
| `piece_set_moved`   | `void piece_set_moved(PieceState* piece)`                       | Mark piece as moved                |
| `piece_add_buff`    | `void piece_add_buff(PieceState* piece, const Effect* buff)`    | Add buff to piece                  |
| `piece_remove_buff` | `void piece_remove_buff(PieceState* piece, uint32_t source_id)` | Remove buff from piece             |
| `piece_copy`        | `void piece_copy(PieceState* dest, const PieceState* src)`      | Copy piece state                   |

#### src/recipe.c

**Recipe Operations Functions:**

| Function          | Signature                                                        | Purpose                      |
| ----------------- | ---------------------------------------------------------------- | ---------------------------- |
| `recipe_find`     | `const Recipe* recipe_find(uint16_t a, uint16_t b)`              | Find recipe by ingredients   |
| `recipe_result`   | `uint16_t recipe_result(uint16_t a, uint16_t b)`                 | Get result of recipe         |
| `recipe_is_valid` | `bool recipe_is_valid(const Recipe* recipe, uint64_t forbidden)` | Check if recipe is craftable |
| `recipe_discover` | `void recipe_discover(RunState* run, uint16_t result)`           | Mark recipe as discovered    |

#### src/registry.c

**Registry Accessor Functions:**

| Function              | Signature                                                     | Purpose                        |
| --------------------- | ------------------------------------------------------------- | ------------------------------ |
| `registry_piece`      | `const PieceTemplate* registry_piece(PieceId id)`             | Get piece template by ID       |
| `registry_card`       | `const CardTemplate* registry_card(CardId id)`                | Get card template by ID        |
| `registry_relic`      | `const RelicTemplate* registry_relic(RelicId id)`             | Get relic template by ID       |
| `registry_innate`     | `const InnateTemplate* registry_innate(Kingdom kingdom)`      | Get innate template by kingdom |
| `registry_chain`      | `const Chain* registry_chain(uint8_t level)`                  | Get chain by level             |
| `registry_modifier`   | `const Modifier* registry_modifier(ModifierId id)`            | Get modifier by ID             |
| `registry_trait`      | `const BoardTrait* registry_trait(TraitId id)`                | Get trait by ID                |
| `registry_event`      | `const EventTemplate* registry_event(EventId id)`             | Get event by ID                |
| `registry_overseer`   | `const Overseer* registry_overseer(OverseerId id)`            | Get overseer by ID             |
| `registry_synergy`    | `const Synergy* registry_synergy(SynergyId id)`               | Get synergy by ID              |
| `registry_figurehead` | `const FigureheadPower* registry_figurehead(Kingdom kingdom)` | Get figurehead by kingdom      |
| `registry_archetype`  | `const AIArchetype* registry_archetype(Kingdom kingdom)`      | Get archetype by kingdom       |
| `registry_recipe`     | `const Recipe* registry_recipe(size_t index)`                 | Get recipe by index            |

#### src/rng.c

**Xorshift64 PRNG Functions:**

| Function      | Signature                                                               | Purpose                   |
| ------------- | ----------------------------------------------------------------------- | ------------------------- |
| `rng_init`    | `void rng_init(Rng* rng, uint64_t seed)`                                | Initialize PRNG with seed |
| `rng_next`    | `uint64_t rng_next(Rng* rng)`                                           | Get next random value     |
| `rng_range`   | `uint64_t rng_range(Rng* rng, uint64_t min, uint64_t max)`              | Get random value in range |
| `rng_shuffle` | `void rng_shuffle(Rng* rng, void* arr, size_t count, size_t elem_size)` | Shuffle array in place    |

### 4.2 Data Files (src/data/)

#### src/data/data_archetypes.c

Contains 5 AI archetypes:

| Archetype      | Kingdom   | Key Traits                                                            |
| -------------- | --------- | --------------------------------------------------------------------- |
| Siege Engineer | Longwei   | High value_diff (12), territory (6), low reclaim_priority (0)         |
| Reclaimer      | Harushima | High reclaim_priority (10), combo_chain_bonus (7), low aggression (2) |
| The Tide       | Kewarani  | High aggression (8), moderate value_diff (6)                          |
| Trickster      | Zarqan    | Highest aggression (12), max_piece_cost (9), low save_threshold (3)   |
| The Hammer     | Caelan    | Balanced weights, high territory (8), high value_diff (9)             |

#### src/data/data_caelan.c

Contains 7 Caelan pieces and 8 Caelan cards.

**Pieces:**

- PIECE_PAWN: mg_step (0,1), TIER_DISTRICT, base_value 10
- PIECE_KNIGHT: mg_leap_set (8 knight moves), TIER_DISTRICT, base_value 30
- PIECE_BISHOP: mg_slide_dirs (0xAA diagonal mask), TIER_TOWN, base_value 30
- PIECE_QUEEN: mg_slide_dirs (0xFF all dirs), TIER_PROVINCE, base_value 90
- PIECE_GRYPHON: mg_ca_gryphon (custom), TIER_CAPSTONE, base_value 100
- PIECE_CHANCELLOR: mg_choice (Rook or Knight), TIER_PROVINCE, base_value 70
- PIECE_SOVEREIGN_BANNER: mg_slide_dirs (0xFF), TIER_PROVINCE, base_value 110

**Cards:**

- CARD_CASTLING, CARD_QUEENS_GAMBIT, CARD_VENGEANCE, CARD_QUEENS_DECREE, CARD_CATHEDRAL, CARD_CORONATION, CARD_CRUSADE, CARD_DIVINE_RIGHT

#### src/data/data_chains.c

Contains 3 chain penalties:

- Bronze (level 1): 1 penalty - deducts starting cp
- Silver (level 2): 2 penalties - Bronze + gifts enemy free piece
- Gold (level 3): 3 penalties - Bronze + Silver + Gold (locks kingdom track)

#### src/data/data_events.c

Contains 14 event templates:

- EVENT_UNIVERSAL_WANDERER, EVENT_UNIVERSAL_MARKET, EVENT_UNIVERSAL_AMBUSH
- EVENT_LONGWEI_RIVER_FESTIVAL, EVENT_LONGWEI_SKY_LADDER
- EVENT_HARUSHIMA_HONOR_TRIAL, EVENT_HARUSHIMA_TAKENOKO_SWARM
- EVENT_KEWARANI_PILGRIMAGE_SEASON, EVENT_KEWARANI_BAZAAR_RUMOR
- EVENT_ZARQAN_SANDSTORM, EVENT_ZARQAN_OASIS_DISCOVERY
- EVENT_CAELAN_TOURNAMENT, EVENT_CAELAN_CASTLE_FESTIVAL

#### src/data/data_figureheads.c

Contains 5 figurehead powers:

- FIGUREHEAD_POWER_LONGWEI: Mingzhu's Cannon Salute (spawn free Pao +20 cp elsewhere)
- FIGUREHEAD_POWER_HARUSHIMA: Tomohito's Patience (reclaim cost drops to 10 cp)
- FIGUREHEAD_POWER_KEWARANI: Selassie's March (Double Time active from run start)
- FIGUREHEAD_POWER_ZARQAN: Timur's Royal Substitution (usable twice per battle)
- FIGUREHEAD_POWER_CAELAN: Isabella's Crusade (guaranteed Province-tier Caelan card at turn 1)

#### src/data/data_harushima.c

Contains 9 Harushima pieces and 7 Harushima cards.

**Pieces:**

- PIECE_FUHYO: mg_step (0,1), TIER_DISTRICT, base_value 10
- PIECE_KYOSHA: mg_slide (forward slide), TIER_DISTRICT, base_value 30
- PIECE_GINSHO: mg_hs_ginsho (5-direction step), TIER_TOWN, base_value 30
- PIECE_KINSHO: mg_hs_kinsho (6-direction step), TIER_PROVINCE, base_value 35
- PIECE_SHISHI: mg_hs_shishi (Lion move), TIER_CAPSTONE, base_value 100
- PIECE_HONORABLE_HORSE: mg_leap_set (8 knight moves) + passive (swap to Kinsho on enemy territory entry)
- PIECE_PROMOTED_BISHOP: mg_compound (Bishop + 1-orthogonal), TIER_PROVINCE, base_value 50
- PIECE_DAIMYO: mg_step_set (16 moves, like King), TIER_COUNTRY, base_value 50, immune to flip
- PIECE_DRAGON: mg_compound (Rook + 1-diagonal), TIER_CAPSTONE, base_value 70

**Cards:**

- CARD_RONIN, CARD_RESURRECTION, CARD_GOLD_STANDARD, CARD_PROMOTION, CARD_DUAL_DROP, CARD_FORCE_DROP, CARD_BUSHIDO

#### src/data/data_innates.c

Contains 5 kingdom innate powers:

- INNATE_BULWARK (Longwei): TRIGGER_RESOLVE_DEFENSE
- INNATE_RECLAIM (Harushima): TRIGGER_TURN_START
- INNATE_DOUBLE_TIME (Kewarani): TRIGGER_QUERY_MOVE_COUNT
- INNATE_ROYAL_SUBSTITUTION (Zarqan): TRIGGER_TURN_START
- INNATE_CONQUERORS_REWARD (Caelan): TRIGGER_RESOLVE_FLIP

#### src/data/data_kewarani.c

Contains 7 Kewarani pieces and 7 Kewarani cards.

**Pieces:**

- PIECE_MEDEQ: mg_kw_berolina (-1 forward), TIER_DISTRICT, base_value 10
- PIECE_MAKWANAM: mg_step_set (8 diagonal moves), TIER_DISTRICT, base_value 15
- PIECE_SABA: mg_leap_set (4 corners at distance 2), TIER_TOWN, base_value 20
- PIECE_FARAS: mg_leap_set (4 orthogonal at distance 2), TIER_PROVINCE, base_value 30
- PIECE_NEGUS_GUARD: mg_kw_negus_guard (double king-step), TIER_CAPSTONE, base_value 100, splitter (2 pieces)
- PIECE_MEDEQ_SQUAD: mg_kw_berolina (-1), TIER_TOWN, base_value 20, splitter (2 pieces)
- PIECE_SULTANS_LEVY: mg_kw_negus_guard, TIER_PROVINCE, base_value 110, splitter (3 pieces)

**Cards:**

- CARD_SULTANS_GOLD, CARD_MARCH, CARD_DOUBLE_TIME, CARD_SALT_ROAD, CARD_CARAVAN, CARD_DOUBLESTRIKE, CARD_HAJJ

#### src/data/data_longwei.c

Contains 8 Longwei pieces and 7 Longwei cards.

**Pieces:**

- PIECE_BING: mg_step (0,1), TIER_DISTRICT, base_value 10
- PIECE_XIANG: mg_lw_xiang (2,2 blockable diagonal), TIER_DISTRICT, base_value 20
- PIECE_MA: mg_lw_ma (elbow-blocked knight), TIER_TOWN, base_value 30
- PIECE_PAO: mg_lw_pao (cannon capture), TIER_PROVINCE, base_value 50
- PIECE_LIUBO_DIVINER: mg_lw_liubo (teleport to threatened squares), TIER_CAPSTONE, base_value 100
- PIECE_SANG: mg_lw_sang (ortho then 2 diagonals), TIER_TOWN, base_value 30
- PIECE_NORTHERN_CAVALRY: mg_leap_set (8 knight moves), TIER_PROVINCE, base_value 40
- PIECE_HWACHA: mg_lw_hwacha (8-dir cannon), TIER_COUNTRY, base_value 60

**Cards:**

- CARD_RIVER_WADE, CARD_CHARGE, CARD_FORMATION, CARD_DIVINATION, CARD_CANNON_VOLLEY, CARD_PALACE_DECREE, CARD_MANDATE

#### src/data/data_masteries.c

Contains 5 mastery cards and 15 mastery hooks.

**Mastery Cards:**

- CARD_MINGZHUS_SEAL (Longwei): Province-tier
- CARD_TOMOHITOS_PATIENCE (Harushima): Province-tier
- CARD_SELASSIES_MARCH (Kewarani): Province-tier
- CARD_TIMURS_CONQUEST (Zarqan): Province-tier
- CARD_ISABELLAS_CORONATION (Caelan): Province-tier

**Mastery Hooks (3 levels x 5 kingdoms):**

- Level 1: Innate activation at Town entry
- Level 2: Add figurehead card to cardset
- Level 3: Upgrade starting power

#### src/data/data_modifiers.c

Contains 12 battle modifiers:

**Economy:**

- MODIFIER_LEAN_TIMES: Starting cp -20
- MODIFIER_WINDFALL: Both sides start with +30 cp
- MODIFIER_OPEN_MARKET: All pieces cost 50% this battle

**Meter:**

- MODIFIER_GLASS_CANNON: Both meters start at 50% of calculated value
- MODIFIER_BLOODBATH: Each flip triggers 2 pieces instead of 1
- MODIFIER_IRON_WILL: Meters floor at 10 until a single attack would push below 0

**Cards:**

- MODIFIER_RICH_HAND: Draw 4 cards per turn
- MODIFIER_SPARSE_HAND: Draw 2 cards per turn
- MODIFIER_KINGDOM_PURITY: Only cards matching the region are drawn

**Board:**

- MODIFIER_FOG_OF_WAR: Enemy piece values hidden until they attack
- MODIFIER_DENSE_TERRAIN: 20% of squares impassable
- MODIFIER_EXTENDED_FRONT: Board +2 columns wide

#### src/data/data_overseers.c

Contains 6 overseers:

- OVERSEER_IRON_STRATEGIST (Longwei)
- OVERSEER_ETERNAL_RECURSION (Harushima)
- OVERSEER_CARAVAN_OF_CONQUEST (Kewarani)
- OVERSEER_MANY_FACED_KING (Zarqan)
- OVERSEER_CROWNED_HERETIC (Caelan)
- OVERSEER_VORATH (final boss)

#### src/data/data_recipes.c

Contains 15 combination recipes:

| Recipe | Ingredients          | Result           | Steps |
| ------ | -------------------- | ---------------- | ----- |
| 1      | Xiang + Ma           | Sang             | 1     |
| 2      | Ma + Pao             | Northern Cavalry | 1     |
| 3      | Bing + Pao           | Hwacha           | 1     |
| 4      | Ginsho + Kinsho      | Shishi           | 1     |
| 5      | Kyosha + Fuhyo       | Honorable Horse  | 1     |
| 6      | Bishop + Rook        | Promoted Bishop  | 1     |
| 7      | Knight + Rook        | Dragon           | 1     |
| 8      | Makwanam + Saba      | Faras            | 1     |
| 9      | Faras + Negus Guard  | Medeq Squad      | 1     |
| 10     | Wazir + Jamal        | Cataphract       | 1     |
| 11     | Talliya + Wazir      | Ziraafa          | 1     |
| 12     | Cataphract + Ziraafa | Shahzadeh        | 1     |
| 13     | Wazir + Old King     | War Elephant     | 1     |
| 14     | Knight + Bishop      | Chancellor       | 1     |
| 15     | Bishop + Knight      | Gryphon          | 1     |

#### src/data/data_relics.c

Contains 26 relics across 5 categories:

**Economy (6):**

- RELIC_MERCHANTS_LEDGER: Card sell values +5 cp
- RELIC_MINTED_COIN: +5 cp at start of every turn
- RELIC_TAX_STAMP: +10 cp whenever you play a card with a play cost
- RELIC_BULK_DISCOUNT: When buying 3+ pieces in one turn, the cheapest is free
- RELIC_WAR_CHEST: Unspent cp at end of turn adds 20% to meter
- RELIC_TRADE_ROUTES: Foreign kingdom markup removed

**Meter (6):**

- RELIC_SOUL_SHARD: Gaining a flipped piece adds 30 bonus to meter
- RELIC_VETERANS_BOND: Pieces with value 50+ contribute 20 extra to meter
- RELIC_DEAD_MANS_PACT: First time meter would empty, reset to 20 instead
- RELIC_IRON_KING: King contributes 20 to meter instead of 10
- RELIC_BLOODTHIRST: At start of each turn where meter exceeds enemy, gain 5 to meter
- RELIC_LAST_BREATH: When a friendly piece flips, deal damage equal to its value to enemy meter

**Cards (5):**

- RELIC_TACTICIANS_SCROLL: Draw 4 cards per turn
- RELIC_LIBRARIANS_NOTES: Once per turn, before drawing, see top card and skip it
- RELIC_COUNTRY_SEAL: Selling a Country-tier card gives +20 extra cp
- RELIC_DEEP_HAND: Once per battle, draw 2 extra cards on a turn of your choice
- RELIC_GILDED_ARCHIVE: District-tier cards sell for +10 cp

**Combinations (4):**

- RELIC_ALCHEMISTS_KIT: Combinations cost 0 actions
- RELIC_MASTERS_NOTES: Archive nodes reveal 2 recipes instead of 1
- RELIC_PHILOSOPHERS_STONE: Once per battle, a combined piece permanently gains +20 value
- RELIC_INHERITED_POWER: All combined pieces gain +5 value above normal result

**Board (5):**

- RELIC_EAGLE_EYE: Enemy piece values always visible
- RELIC_SURVEYORS_MAP: One random battle node's modifier pre-revealed per map
- RELIC_FORWARD_COMMAND: Pieces deal +5 damage when occupying enemy territory
- RELIC_FORTIFIED_LINE: Pieces that did not move this turn deal +5 damage
- RELIC_WARLORDS_BANNER: Pieces adjacent to your king deal +5 damage

#### src/data/data_synergies.c

Contains 5 inter-kingdom synergies:

- SYNERGY_LONGWEI (Pao Vanguard): Pao +10 damage in Harushima
- SYNERGY_HARUSHIMA (Reclaimed Honor): Caelan cards draw 1 in Caelan
- SYNERGY_KEWARANI (Caravan Discount): Kewarani pieces -10 cp in Zarqan
- SYNERGY_ZARQAN (Imperial Tribute): Ziraafa/Talliya +5 value in Longwei
- SYNERGY_CAELAN (Golden Sultanate): Sultan's Gold +10 cp in Kewarani

#### src/data/data_traits.c

Contains 10 board traits (2 per kingdom):

**Longwei:**

- TRAIT_RIVER_CROSSING: A row bisects the board. Xiang cannot cross it. Bing gains sideways step immediately upon crossing. Pao attacks freely through any piece on the river row.
- TRAIT_PALACE: A 3x3 zone near the enemy king. Enemy king cannot voluntarily leave it.

**Harushima:**

- TRAIT_FOG_COAST: 3 farthest columns show enemy piece values as "?" until they attack.
- TRAIT_ISLAND_CHAIN: 3-4 impassable column gaps force pieces to route around.

**Kewarani:**

- TRAIT_TRADE_ROUTE: A diagonal path crosses the board. Any piece on it gains +1 movement per action.
- TRAIT_CONTESTED_MARKET: Start of each player turn: 1 random District piece appears at the board edge. The first piece to reach it claims it.

**Zarqan:**

- TRAIT_SANDSTORM: On even turns, all sliders move at most 3 squares.
- TRAIT_MIRAGE: 5 random squares appear occupied but are not. Movement attempts waste the action.

**Caelan:**

- TRAIT_CASTLE_CORNERS: The four 2x2 corner zones grant Bulwark to any piece standing in them.
- TRAIT_SIEGE_TRENCH: A row of impassable squares crosses near the center with a 2-square gap.

#### src/data/data_universal.c

Contains the King piece and 12 universal cards.

**King Piece:**

- PIECE_KING: mg_step_set (8 directions, 1 step), TIER_KING, base_value 10

**Universal Cards:**

- CARD_PAWN_STORM: Buy up to 3 pawns this turn; the third is free
- CARD_REVITALIZE: Restore 50 to your meter
- CARD_HOSTAGE: Passive. Next flip to your side: meter gains 20 bonus
- CARD_LAST_STAND: This turn, your meter cannot trigger a flip
- CARD_SACRIFICE: Remove one of your pieces. Gain its value x2 as meter
- CARD_REFORGE: Passive. Next time one of your pieces flips, the cost for the first piece of that type is discounted 30% next turn
- CARD_MERCY: Target which piece flips next
- CARD_BLOODLETTING: This turn, all your attacks deal extra damage equal to 50% of your missing meter
- CARD_COUNTER_COUP: This turn, all damage you take echoes back at 50% to enemy meter
- CARD_SPITE: Passive. Next time you lose a piece, deal its value x3 to enemy meter
- CARD_CHAIN_BREAK: Force-flip one enemy piece of your choice
- CARD_HYDRA: Passive. Next time one of your pieces flips, 2 friendly pawns spawn adjacent to your king

#### src/data/data_zarqan.c

Contains 9 Zarqan pieces and 8 Zarqan cards.

**Pieces:**

- PIECE_WAZIR: mg_step_set (4 cardinal dirs), TIER_DISTRICT, base_value 15
- PIECE_JAMAL: mg_leap_set (81x3 moves), TIER_DISTRICT, base_value 20
- PIECE_TALLIYA: mg_slide_dirs (0xAA diagonal), TIER_TOWN, base_value 30
- PIECE_ZIRAAFA: mg_zq_ziraafa (diag then ortho slide), TIER_PROVINCE, base_value 35, custom threat
- PIECE_SHAHZADEH: mg_step_set (8 dirs), TIER_CAPSTONE, base_value 100
- PIECE_OLD_KING: mg_step_set (16 moves), TIER_TOWN, base_value 30
- PIECE_CATAPHRACT: mg_choice (Knight or Jamal), TIER_PROVINCE, base_value 40
- PIECE_ROOK: mg_slide_dirs (0x55 cardinal), TIER_CAPSTONE, base_value 50
- PIECE_WAR_ELEPHANT: mg_zq_ziraafa, TIER_COUNTRY, base_value 50, custom threat

**Cards:**

- CARD_COUNSEL, CARD_PILLAGE, CARD_ROYAL_DECOY, CARD_BAZAAR, CARD_STEPPE_RIDERS, CARD_AMBITION, CARD_CITADEL, CARD_CONQUEST

### 4.3 Effects Files (src/effects/)

#### src/effects/eff_card.c

| Function           | Signature                                                                    | Purpose                               |
| ------------------ | ---------------------------------------------------------------------------- | ------------------------------------- |
| `eff_draw_extra`   | `void eff_draw_extra(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Increase draw count this turn         |
| `eff_discard_card` | `void eff_discard_card(EffectCtx* ctx, const EffectArg* args, size_t count)` | Remove card from hand without selling |
| `eff_peek_cards`   | `void eff_peek_cards(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Peek at top N cards of cardset        |

#### src/effects/eff_chain.c

| Function           | Signature                                                                    | Purpose                                 |
| ------------------ | ---------------------------------------------------------------------------- | --------------------------------------- |
| `eff_chain_bronze` | `void eff_chain_bronze(EffectCtx* ctx, const EffectArg* args, size_t count)` | Reduces player CP by 15 at battle start |
| `eff_chain_silver` | `void eff_chain_silver(EffectCtx* ctx, const EffectArg* args, size_t count)` | Sets chain_silver_pending flag          |
| `eff_chain_gold`   | `void eff_chain_gold(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Campaign-layer liberation lock          |

#### src/effects/eff_damage.c

| Function             | Signature                                                                      | Purpose                                   |
| -------------------- | ------------------------------------------------------------------------------ | ----------------------------------------- |
| `eff_damage_mult`    | `void eff_damage_mult(EffectCtx* ctx, const EffectArg* args, size_t count)`    | Multiply damage output (int basis-points) |
| `eff_damage_reduce`  | `void eff_damage_reduce(EffectCtx* ctx, const EffectArg* args, size_t count)`  | Reduce incoming damage by flat amount     |
| `eff_grant_immunity` | `void eff_grant_immunity(EffectCtx* ctx, const EffectArg* args, size_t count)` | Grant damage immunity to piece            |
| `eff_deal_damage`    | `void eff_deal_damage(EffectCtx* ctx, const EffectArg* args, size_t count)`    | Deal damage directly                      |

#### src/effects/eff_economy.c

| Function         | Signature                                                                  | Purpose                                         |
| ---------------- | -------------------------------------------------------------------------- | ----------------------------------------------- |
| `eff_cp_add`     | `void eff_cp_add(EffectCtx* ctx, const EffectArg* args, size_t count)`     | Add delta to CP, triggers cp-changed event      |
| `eff_cp_set`     | `void eff_cp_set(EffectCtx* ctx, const EffectArg* args, size_t count)`     | Set CP to specific value                        |
| `eff_cost_mod`   | `void eff_cost_mod(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Increase piece cost (positive = more expensive) |
| `eff_sell_mod`   | `void eff_sell_mod(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Increase sell value                             |
| `eff_income_add` | `void eff_income_add(EffectCtx* ctx, const EffectArg* args, size_t count)` | Increase turn income                            |

#### src/effects/eff_figurehead.c

| Function          | Signature                                                                   | Purpose                         |
| ----------------- | --------------------------------------------------------------------------- | ------------------------------- |
| `eff_fh_mingzhu`  | `void eff_fh_mingzhu(EffectCtx* ctx, const EffectArg* args, size_t count)`  | Spawn Pao at (0,1) or +20 CP    |
| `eff_fh_tomohito` | `void eff_fh_tomohito(EffectCtx* ctx, const EffectArg* args, size_t count)` | Set reclaim cost to 5 or 10     |
| `eff_fh_selassie` | `void eff_fh_selassie(EffectCtx* ctx, const EffectArg* args, size_t count)` | Apply Kewarani innate           |
| `eff_fh_timur`    | `void eff_fh_timur(EffectCtx* ctx, const EffectArg* args, size_t count)`    | Set royal_sub_count to 2 or 3   |
| `eff_fh_isabella` | `void eff_fh_isabella(EffectCtx* ctx, const EffectArg* args, size_t count)` | Insert Castling card at hand[0] |

#### src/effects/eff_flip.c

| Function                    | Signature                                                                             | Purpose                                          |
| --------------------------- | ------------------------------------------------------------------------------------- | ------------------------------------------------ |
| `eff_splitter_medeq`        | `void eff_splitter_medeq(EffectCtx* ctx, const EffectArg* args, size_t count)`        | Kewarani splitter: spawn 2-3 Medeq pieces        |
| `eff_grant_flip_immunity`   | `void eff_grant_flip_immunity(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Grant immunity to one flip                       |
| `eff_flip_destroy_adjacent` | `void eff_flip_destroy_adjacent(EffectCtx* ctx, const EffectArg* args, size_t count)` | On flip: destroy one random adjacent enemy piece |

#### src/effects/eff_innate.c

| Function                       | Signature                                                                                | Purpose                                                       |
| ------------------------------ | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------- |
| `eff_innate_bulwark`           | `void eff_innate_bulwark(EffectCtx* ctx, const EffectArg* args, size_t count)`           | +50 damage reduction when Longwei piece has friendly neighbor |
| `eff_innate_reclaim`           | `void eff_innate_reclaim(EffectCtx* ctx, const EffectArg* args, size_t count)`           | Exposes battle_action_reclaim via reclaim_cost_override       |
| `eff_innate_double_time`       | `void eff_innate_double_time(EffectCtx* ctx, const EffectArg* args, size_t count)`       | +1 move to Kewarani pieces                                    |
| `eff_innate_royal_sub`         | `void eff_innate_royal_sub(EffectCtx* ctx, const EffectArg* args, size_t count)`         | Exposes battle_action_substitute                              |
| `eff_innate_conquerors_reward` | `void eff_innate_conquerors_reward(EffectCtx* ctx, const EffectArg* args, size_t count)` | +50% value_mod to attacker on successful flip                 |

#### src/effects/eff_mastery.c

| Function     | Signature                                                              | Purpose                                              |
| ------------ | ---------------------------------------------------------------------- | ---------------------------------------------------- |
| `mastery_l1` | `void mastery_l1(EffectCtx* ctx, const EffectArg* args, size_t count)` | Apply kingdom's innate immediately on Town-map entry |
| `mastery_l2` | `void mastery_l2(EffectCtx* ctx, const EffectArg* args, size_t count)` | Append figurehead card to cardset                    |
| `mastery_l3` | `void mastery_l3(EffectCtx* ctx, const EffectArg* args, size_t count)` | Set mastery_l3[kingdom] = 1 flag                     |

#### src/effects/eff_meter.c

| Function                 | Signature                                                                          | Purpose                               |
| ------------------------ | ---------------------------------------------------------------------------------- | ------------------------------------- |
| `eff_meter_add`          | `void eff_meter_add(EffectCtx* ctx, const EffectArg* args, size_t count)`          | Add delta, clamp to meter_cap         |
| `eff_meter_set`          | `void eff_meter_set(EffectCtx* ctx, const EffectArg* args, size_t count)`          | Set to specific value                 |
| `eff_meter_cap_up`       | `void eff_meter_cap_up(EffectCtx* ctx, const EffectArg* args, size_t count)`       | Increase capacity (not current meter) |
| `eff_meter_overflow_up`  | `void eff_meter_overflow_up(EffectCtx* ctx, const EffectArg* args, size_t count)`  | Increase overflow capacity            |
| `eff_meter_overflow_add` | `void eff_meter_overflow_add(EffectCtx* ctx, const EffectArg* args, size_t count)` | Add to overflow meter                 |
| `eff_meter_refill`       | `void eff_meter_refill(EffectCtx* ctx, const EffectArg* args, size_t count)`       | Fill meter to capacity                |

#### src/effects/eff_movement.c

| Function                  | Signature                                                                           | Purpose                              |
| ------------------------- | ----------------------------------------------------------------------------------- | ------------------------------------ |
| `eff_swap_movegen`        | `void eff_swap_movegen(EffectCtx* ctx, const EffectArg* args, size_t count)`        | Swap piece's movegen to new pattern  |
| `eff_grant_extra_step`    | `void eff_grant_extra_step(EffectCtx* ctx, const EffectArg* args, size_t count)`    | Grant extra move step                |
| `eff_grant_friendly_pass` | `void eff_grant_friendly_pass(EffectCtx* ctx, const EffectArg* args, size_t count)` | Allow moving through friendly pieces |

#### src/effects/eff_overseer.c

| Function                           | Signature                                                                                    | Purpose                                     |
| ---------------------------------- | -------------------------------------------------------------------------------------------- | ------------------------------------------- |
| `eff_overseer_iron_strategist`     | `void eff_overseer_iron_strategist(EffectCtx* ctx, const EffectArg* args, size_t count)`     | Spawn 3 Pao + 2 Wazir for enemy             |
| `eff_overseer_eternal_recursion`   | `void eff_overseer_eternal_recursion(EffectCtx* ctx, const EffectArg* args, size_t count)`   | On enemy→player flip: re-flip back to enemy |
| `eff_overseer_caravan_of_conquest` | `void eff_overseer_caravan_of_conquest(EffectCtx* ctx, const EffectArg* args, size_t count)` | On odd turns: spawn Faras at top-center     |
| `eff_overseer_many_faced_king`     | `void eff_overseer_many_faced_king(EffectCtx* ctx, const EffectArg* args, size_t count)`     | Spawn 3 Shahzadeh + King for enemy          |
| `eff_overseer_crowned_heretic`     | `void eff_overseer_crowned_heretic(EffectCtx* ctx, const EffectArg* args, size_t count)`     | Spawn full Caelan starting army             |

#### src/effects/eff_piece.c

| Function           | Signature                                                                    | Purpose                           |
| ------------------ | ---------------------------------------------------------------------------- | --------------------------------- |
| `eff_spawn_piece`  | `void eff_spawn_piece(EffectCtx* ctx, const EffectArg* args, size_t count)`  | Spawn piece template at position  |
| `eff_remove_piece` | `void eff_remove_piece(EffectCtx* ctx, const EffectArg* args, size_t count)` | Remove piece from board           |
| `eff_swap_pieces`  | `void eff_swap_pieces(EffectCtx* ctx, const EffectArg* args, size_t count)`  | Swap two pieces' positions        |
| `eff_force_flip`   | `void eff_force_flip(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Force-flip piece to opposite side |

#### src/effects/eff_registry.c

| Function            | Signature                      | Purpose                                          |
| ------------------- | ------------------------------ | ------------------------------------------------ |
| `eff_registry_init` | `void eff_registry_init(void)` | Maps EffectFuncId enum to live function pointers |

#### src/effects/eff_relic.c

Contains 26 relic implementations:

| Function                 | Relic                    |
| ------------------------ | ------------------------ |
| `eff_merchants_ledger`   | RELIC_MERCHANTS_LEDGER   |
| `eff_minted_coin`        | RELIC_MINTED_COIN        |
| `eff_tax_stamp`          | RELIC_TAX_STAMP          |
| `eff_bulk_discount`      | RELIC_BULK_DISCOUNT      |
| `eff_war_chest`          | RELIC_WAR_CHEST          |
| `eff_trade_routes`       | RELIC_TRADE_ROUTES       |
| `eff_soul_shard`         | RELIC_SOUL_SHARD         |
| `eff_veterans_bond`      | RELIC_VETERANS_BOND      |
| `eff_dead_mans_pact`     | RELIC_DEAD_MANS_PACT     |
| `eff_iron_king`          | RELIC_IRON_KING          |
| `eff_bloodthirst`        | RELIC_BLOODTHIRST        |
| `eff_last_breath`        | RELIC_LAST_BREATH        |
| `eff_tacticians_scroll`  | RELIC_TACTICIANS_SCROLL  |
| `eff_librarians_notes`   | RELIC_LIBRARIANS_NOTES   |
| `eff_country_seal`       | RELIC_COUNTRY_SEAL       |
| `eff_deep_hand`          | RELIC_DEEP_HAND          |
| `eff_gilded_archive`     | RELIC_GILDED_ARCHIVE     |
| `eff_alchemists_kit`     | RELIC_ALCHEMISTS_KIT     |
| `eff_masters_notes`      | RELIC_MASTERS_NOTES      |
| `eff_philosophers_stone` | RELIC_PHILOSOPHERS_STONE |
| `eff_inherited_power`    | RELIC_INHERITED_POWER    |
| `eff_eagle_eye`          | RELIC_EAGLE_EYE          |
| `eff_surveyors_map`      | RELIC_SURVEYORS_MAP      |
| `eff_forward_command`    | RELIC_FORWARD_COMMAND    |
| `eff_fortified_line`     | RELIC_FORTIFIED_LINE     |
| `eff_warlords_banner`    | RELIC_WARLORDS_BANNER    |

#### src/effects/eff_run.c

| Function            | Signature                                                                     | Purpose                                         |
| ------------------- | ----------------------------------------------------------------------------- | ----------------------------------------------- |
| `eff_apply_relic`   | `void eff_apply_relic(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Copies relic effects onto battle's EffectBus    |
| `eff_apply_chain`   | `void eff_apply_chain(EffectCtx* ctx, const EffectArg* args, size_t count)`   | Applies penalty chain effects                   |
| `eff_apply_synergy` | `void eff_apply_synergy(EffectCtx* ctx, const EffectArg* args, size_t count)` | Activates synergy bonus after Overseer victory  |
| `eff_todo`          | `void eff_todo(EffectCtx* ctx, const EffectArg* args, size_t count)`          | Universal placeholder for unimplemented effects |

#### src/effects/eff_synergy.c

| Function            | Signature                                                                     | Purpose                                |
| ------------------- | ----------------------------------------------------------------------------- | -------------------------------------- |
| `eff_syn_longwei`   | `void eff_syn_longwei(EffectCtx* ctx, const EffectArg* args, size_t count)`   | +10 damage when attacker is Pao        |
| `eff_syn_harushima` | `void eff_syn_harushima(EffectCtx* ctx, const EffectArg* args, size_t count)` | Draw one card when playing Caelan card |
| `eff_syn_kewarani`  | `void eff_syn_kewarani(EffectCtx* ctx, const EffectArg* args, size_t count)`  | -10 cost for Kewarani pieces           |
| `eff_syn_zarqan`    | `void eff_syn_zarqan(EffectCtx* ctx, const EffectArg* args, size_t count)`    | +5 value_mod to Ziraafa and Talliya    |
| `eff_syn_caelan`    | `void eff_syn_caelan(EffectCtx* ctx, const EffectArg* args, size_t count)`    | +10 CP when playing Sultan's Gold      |

#### src/effects/eff_vorath_memory.c

| Function                  | Signature                                                                           | Purpose                                         |
| ------------------------- | ----------------------------------------------------------------------------------- | ----------------------------------------------- |
| `eff_vorath_memory_tally` | `void eff_vorath_memory_tally(EffectCtx* ctx, const EffectArg* args, size_t count)` | Increments Profile.vorath_memory[template_id]   |
| `eff_vorath_memory_apply` | `void eff_vorath_memory_apply(EffectCtx* ctx, const EffectArg* args, size_t count)` | Finds most-played piece type, spawns two copies |

### 4.4 Engine Files (src/engine/)

#### src/engine/engine.c

| Function                  | Signature                                                        | Purpose                                                                           |
| ------------------------- | ---------------------------------------------------------------- | --------------------------------------------------------------------------------- |
| `engine_init`             | `EngineState* engine_init(FILE* in, FILE* out)`                  | Initializes engine with stdin/stdout, loads profile, enters title screen          |
| `engine_destroy`          | `void engine_destroy(EngineState* engine)`                       | Saves profile, frees run/battle/profile memory                                    |
| `engine_handle_line`      | `void engine_handle_line(EngineState* engine, const char* line)` | Parses protocol verb, dispatches to screen handlers, applies transitions          |
| `screen_apply_transition` | `void screen_apply_transition(EngineState* engine)`              | Calls leave() on old screen, sets current, calls enter() and emit() on new screen |

#### src/engine/main.c

**Purpose**: Headless engine entry point. CLI subcommands for testing:

- `--new-profile` - Create fresh profile
- `--dump-save <path>` - Pretty-print save file header/chunks
- `--test-save` - Round-trip test RunState through save/load
- `--debug-map <kingdom> <tier>` - Generate and print map node table
- `--debug-jump-results [win|loss]` - Simulate run completion

Main loop: reads frontend verbs from stdin, writes `< ` lines to stdout.

#### src/engine/platform_posix.c

| Function              | Signature                                                                     | Purpose                                |
| --------------------- | ----------------------------------------------------------------------------- | -------------------------------------- |
| `platform_pref_path`  | `const char* platform_pref_path(void)`                                        | Get application data directory         |
| `platform_time_ns`    | `uint64_t platform_time_ns(void)`                                             | Nanosecond timestamp (CLOCK_MONOTONIC) |
| `platform_open_read`  | `PlatformStream* platform_open_read(const char* path)`                        | Open file for reading                  |
| `platform_open_write` | `PlatformStream* platform_open_write(const char* path)`                       | Open file for writing                  |
| `platform_read`       | `size_t platform_read(PlatformStream* stream, void* buf, size_t size)`        | Binary read                            |
| `platform_write`      | `size_t platform_write(PlatformStream* stream, const void* buf, size_t size)` | Binary write                           |
| `platform_size`       | `size_t platform_size(PlatformStream* stream)`                                | Get file size                          |
| `platform_close`      | `void platform_close(PlatformStream* stream)`                                 | Close stream                           |
| `platform_remove`     | `int platform_remove(const char* path)`                                       | Delete file                            |

#### src/engine/profile.c

| Function       | Signature                            | Purpose                             |
| -------------- | ------------------------------------ | ----------------------------------- |
| `profile_new`  | `Profile* profile_new(void)`         | Zero-init with current save version |
| `profile_load` | `Profile* profile_load(void)`        | Load from profile.regsav            |
| `profile_save` | `int profile_save(Profile* profile)` | Write to save file                  |

#### src/engine/protocol.c

| Function              | Signature                                                                            | Purpose                           |
| --------------------- | ------------------------------------------------------------------------------------ | --------------------------------- |
| `protocol_parse`      | `void protocol_parse(const char* line, ProtocolVerb* out)`                           | Tokenizes line into verb + tail   |
| `protocol_arg_str`    | `const char* protocol_arg_str(const ProtocolVerb* verb, const char* key)`            | Extract string value from tail    |
| `protocol_arg_int`    | `int protocol_arg_int(const ProtocolVerb* verb, const char* key, int def)`           | Extract int value from tail       |
| `protocol_arg_xy`     | `bool protocol_arg_xy(const ProtocolVerb* verb, const char* key, Position* out)`     | Extract position from tail        |
| `protocol_emit_show`  | `void protocol_emit_show(EngineState* engine, const char* screen, ...)`              | Emit `< SHOW <screen> [params]`   |
| `protocol_emit_state` | `void protocol_emit_state(EngineState* engine, const char* topic, ...)`              | Emit `< STATE <topic> [params]`   |
| `protocol_emit_popup` | `void protocol_emit_popup(EngineState* engine, const char* name, ...)`               | Emit `< POPUP <name> [params]`    |
| `protocol_emit_log`   | `void protocol_emit_log(EngineState* engine, ProtocolSeverity sev, const char* msg)` | Emit `< LOG <severity> <message>` |
| `protocol_emit_quit`  | `void protocol_emit_quit(EngineState* engine)`                                       | Emit `< QUIT`                     |

#### src/engine/run.c

| Function              | Signature                                                        | Purpose                                              |
| --------------------- | ---------------------------------------------------------------- | ---------------------------------------------------- |
| `run_init`            | `RunState* run_init(uint64_t seed, Profile* profile)`            | Zero-init with seed                                  |
| `run_add_relic`       | `int run_add_relic(RunState* run, RelicId id)`                   | Add relic to run                                     |
| `run_remove_relic`    | `int run_remove_relic(RunState* run, RelicId id)`                | Remove relic from run                                |
| `run_relic_at`        | `const RelicTemplate* run_relic_at(RunState* run, size_t index)` | Get relic template by index                          |
| `run_kingdom_cleared` | `bool run_kingdom_cleared(RunState* run, Kingdom kingdom)`       | Check kingdom completion                             |
| `run_load`            | `RunState* run_load(Profile* profile)`                           | Load from save file                                  |
| `run_save`            | `int run_save(RunState* run)`                                    | Save to file                                         |
| `run_delete`          | `int run_delete(void)`                                           | Delete save file                                     |
| `run_finalize`        | `void run_finalize(RunState* run, RunEnd end)`                   | Handle win/loss, update profile mastery, save/delete |

#### src/engine/save.c

**Writer Functions:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `save_writer_init` | `void save_writer_init(SaveWriter* writer)` | Initialize writer |
| `save_write_u8` | `void save_write_u8(SaveWriter* writer, uint8_t val)` | Write uint8 |
| `save_write_u16` | `void save_write_u16(SaveWriter* writer, uint16_t val)` | Write uint16 |
| `save_write_u32` | `void save_write_u32(SaveWriter* writer, uint32_t val)` | Write uint32 |
| `save_write_u64` | `void save_write_u64(SaveWriter* writer, uint64_t val)` | Write uint64 |
| `save_write_bool` | `void save_write_bool(SaveWriter* writer, bool val)` | Write bool |
| `save_write_bytes` | `void save_write_bytes(SaveWriter* writer, const void* data, size_t len)` | Write bytes |
| `save_write_header` | `void save_write_header(SaveWriter* writer)` | Write magic + version |
| `save_write_chunk_begin` | `void save_write_chunk_begin(SaveWriter* writer, SaveChunkId id)` | Begin chunk |
| `save_write_chunk_end` | `void save_write_chunk_end(SaveWriter* writer)` | End chunk |
| `save_writer_flush` | `int save_writer_flush(SaveWriter* writer, FILE* out)` | Flush to file |

**Reader Functions:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `save_reader_open` | `int save_reader_open(SaveReader* reader, FILE* in)` | Open save file |
| `save_read_u8` | `uint8_t save_read_u8(SaveReader* reader)` | Read uint8 |
| `save_read_u16` | `uint16_t save_read_u16(SaveReader* reader)` | Read uint16 |
| `save_read_u32` | `uint32_t save_read_u32(SaveReader* reader)` | Read uint32 |
| `save_read_u64` | `uint64_t save_read_u64(SaveReader* reader)` | Read uint64 |
| `save_read_bool` | `bool save_read_bool(SaveReader* reader)` | Read bool |
| `save_read_bytes` | `void save_read_bytes(SaveReader* reader, void* data, size_t len)` | Read bytes |
| `save_read_chunk_header` | `SaveChunkId save_read_chunk_header(SaveReader* reader, uint32_t* length)` | Read chunk header |
| `save_skip` | `void save_skip(SaveReader* reader, size_t bytes)` | Skip bytes |

**Utility Functions:**
| Function | Signature | Purpose |
|----------|-----------|---------|
| `crc32_ieee` | `uint32_t crc32_ieee(const void* data, size_t len)` | Calculate CRC32 |
| `save_dump` | `void save_dump(FILE* out, const char* path)` | Pretty-print save file |

#### src/engine/screen.c

| Function               | Signature                         | Purpose                                   |
| ---------------------- | --------------------------------- | ----------------------------------------- |
| `screen_registry_init` | `void screen_registry_init(void)` | Maps ScreenId to concrete Screen v-tables |

#### src/engine/screen_battle.c

| Function        | Signature                                                           | Purpose                                         |
| --------------- | ------------------------------------------------------------------- | ----------------------------------------------- |
| `battle_enter`  | `void battle_enter(EngineState* engine)`                            | Called when entering battle screen              |
| `battle_leave`  | `void battle_leave(EngineState* engine)`                            | Called when leaving battle screen               |
| `battle_handle` | `void battle_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle frontend verb (stub)                     |
| `battle_emit`   | `void battle_emit(EngineState* engine)`                             | Emit turn number, active side, CP, meter values |

#### src/engine/screen_codex.c

| Function       | Signature                                                          | Purpose                           |
| -------------- | ------------------------------------------------------------------ | --------------------------------- |
| `codex_enter`  | `void codex_enter(EngineState* engine)`                            | Called when entering codex screen |
| `codex_leave`  | `void codex_leave(EngineState* engine)`                            | Called when leaving codex screen  |
| `codex_handle` | `void codex_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle frontend verb              |
| `codex_emit`   | `void codex_emit(EngineState* engine)`                             | Emit codex state                  |

#### src/engine/screen_event.c

| Function       | Signature                                                          | Purpose                           |
| -------------- | ------------------------------------------------------------------ | --------------------------------- |
| `event_enter`  | `void event_enter(EngineState* engine)`                            | Called when entering event screen |
| `event_leave`  | `void event_leave(EngineState* engine)`                            | Called when leaving event screen  |
| `event_handle` | `void event_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle frontend verb              |
| `event_emit`   | `void event_emit(EngineState* engine)`                             | Emit event state                  |

#### src/engine/screen_map.c

| Function     | Signature                                                        | Purpose                           |
| ------------ | ---------------------------------------------------------------- | --------------------------------- |
| `map_enter`  | `void map_enter(EngineState* engine)`                            | Called when entering map screen   |
| `map_leave`  | `void map_leave(EngineState* engine)`                            | Called when leaving map screen    |
| `map_handle` | `void map_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle select_node verb           |
| `map_emit`   | `void map_emit(EngineState* engine)`                             | Emit STATE map.node for each node |

#### src/engine/screen_mastery.c

| Function         | Signature                                                            | Purpose                             |
| ---------------- | -------------------------------------------------------------------- | ----------------------------------- |
| `mastery_enter`  | `void mastery_enter(EngineState* engine)`                            | Called when entering mastery screen |
| `mastery_leave`  | `void mastery_leave(EngineState* engine)`                            | Called when leaving mastery screen  |
| `mastery_handle` | `void mastery_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle frontend verb                |
| `mastery_emit`   | `void mastery_emit(EngineState* engine)`                             | Emit per-kingdom mastery levels     |

#### src/engine/screen_results.c

| Function         | Signature                                                            | Purpose                                       |
| ---------------- | -------------------------------------------------------------------- | --------------------------------------------- |
| `results_enter`  | `void results_enter(EngineState* engine)`                            | Calls run_finalize with outcome, detaches run |
| `results_leave`  | `void results_leave(EngineState* engine)`                            | Called when leaving results screen            |
| `results_handle` | `void results_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle continue/ack verbs                     |
| `results_emit`   | `void results_emit(EngineState* engine)`                             | Emit win/loss outcome                         |

#### src/engine/screen_settings.c

| Function          | Signature                                                             | Purpose                              |
| ----------------- | --------------------------------------------------------------------- | ------------------------------------ |
| `settings_enter`  | `void settings_enter(EngineState* engine)`                            | Called when entering settings screen |
| `settings_leave`  | `void settings_leave(EngineState* engine)`                            | Called when leaving settings screen  |
| `settings_handle` | `void settings_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle frontend verb                 |
| `settings_emit`   | `void settings_emit(EngineState* engine)`                             | Emit settings state                  |

#### src/engine/screen_title.c

| Function        | Signature                                                          | Purpose                                                                                 |
| --------------- | ------------------------------------------------------------------ | --------------------------------------------------------------------------------------- |
| `title_enter`   | `void title_enter(EngineState* engine)`                            | Checks if saved run exists for resume                                                   |
| `title_leave`   | `void title_leave(EngineState* engine)`                            | Called when leaving title screen                                                        |
| `title_handle`  | `void title_handle(EngineState* engine, const ProtocolVerb* verb)` | Handle new_run and load_run verbs                                                       |
| `title_emit`    | `void title_emit(EngineState* engine)`                             | Emit title state                                                                        |
| `start_new_run` | `void start_new_run(EngineState* engine)`                          | Creates new run with time-based seed, generates first map, saves and transitions to MAP |
| `continue_run`  | `void continue_run(EngineState* engine)`                           | Loads saved run from disk, attaches profile, transitions to MAP                         |

### 4.5 Move Generator Files (src/movegens/)

#### src/movegens/mg_basics.c

**Primitive Patterns:**

| Function                  | Signature                                                                                                                              | Purpose                                   |
| ------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------- |
| `mg_step`                 | `void mg_step(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`                 | Single-step by (dx, dy) offset            |
| `mg_step_set`             | `void mg_step_set(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`             | Multiple single-step pairs                |
| `mg_slide`                | `void mg_slide(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`                | Linear slide in one direction             |
| `mg_slide_dirs`           | `void mg_slide_dirs(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`           | Multi-direction slide via bit mask        |
| `mg_leap_set`             | `void mg_leap_set(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`             | Capture-only leaps                        |
| `mg_blockable_leap`       | `void mg_blockable_leap(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`       | Leap requiring empty intermediate squares |
| `mg_compound`             | `void mg_compound(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`             | Union of two patterns                     |
| `mg_choice`               | `void mg_choice(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`               | Two pattern options                       |
| `mg_double_act`           | `void mg_double_act(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`           | Two consecutive moves (stub)              |
| `mg_territory_restricted` | `void mg_territory_restricted(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)` | Territory-filtered movement (stub)        |
| `mg_attack_only_subset`   | `void mg_attack_only_subset(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`   | Separate move/attack patterns (stub)      |
| `mg_todo`                 | `void mg_todo(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`                 | Empty stub for unimplemented pieces       |

**Helper Functions:**

| Function               | Signature                                                                                    | Purpose                                 |
| ---------------------- | -------------------------------------------------------------------------------------------- | --------------------------------------- |
| `ml_push`              | `void ml_push(MoveList* list, Position pos)`                                                 | Append destination to MoveList (max 64) |
| `is_friendly`          | `bool is_friendly(const PieceState* piece, Side owner)`                                      | Check if piece shares owner             |
| `is_enemy`             | `bool is_enemy(const PieceState* piece, Side owner)`                                         | Check if piece is opposing side         |
| `can_move_to`          | `bool can_move_to(const BattleState* battle, Position pos)`                                  | Bounds + friendly-occupancy check       |
| `can_capture`          | `bool can_capture(const BattleState* battle, Position pos)`                                  | Enemy occupancy check                   |
| `can_capture_or_empty` | `bool can_capture_or_empty(const BattleState* battle, Position pos)`                         | Empty or enemy check                    |
| `mg_generate`          | `void mg_generate(const PieceState* piece, const BattleState* battle, MoveList* out)`        | Fill MoveList with legal relocations    |
| `mg_generate_threat`   | `void mg_generate_threat(const PieceState* piece, const BattleState* battle, MoveList* out)` | Fill MoveList with threat squares       |

#### src/movegens/mg_caelan.c

| Function        | Signature                                                                                                                    | Purpose                                 |
| --------------- | ---------------------------------------------------------------------------------------------------------------------------- | --------------------------------------- |
| `mg_ca_gryphon` | `void mg_ca_gryphon(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)` | One diagonal step then orthogonal slide |

#### src/movegens/mg_harushima.c

| Function                | Signature                                                                                                                            | Purpose                                  |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------- |
| `mg_hs_kinsho`          | `void mg_hs_kinsho(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`          | Six-direction step set                   |
| `mg_hs_ginsho`          | `void mg_hs_ginsho(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`          | Five-direction step set                  |
| `mg_hs_honorable_horse` | `void mg_hs_honorable_horse(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)` | Asymmetric leap (2 forward + 1 sideways) |
| `mg_hs_shishi`          | `void mg_hs_shishi(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`          | Lion move (capstone)                     |

#### src/movegens/mg_kewarani.c

| Function            | Signature                                                                                                                        | Purpose                                           |
| ------------------- | -------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------- |
| `mg_kw_berolina`    | `void mg_kw_berolina(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`    | Berolina pawn (diagonal relocate, forward attack) |
| `mg_kw_negus_guard` | `void mg_kw_negus_guard(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)` | Double-act king-step                              |

#### src/movegens/mg_longwei.c

| Function       | Signature                                                                                                                   | Purpose                        |
| -------------- | --------------------------------------------------------------------------------------------------------------------------- | ------------------------------ |
| `mg_lw_ma`     | `void mg_lw_ma(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`     | Knight-L with elbow blocking   |
| `mg_lw_xiang`  | `void mg_lw_xiang(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`  | (2,2) blockable diagonal leap  |
| `mg_lw_pao`    | `void mg_lw_pao(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`    | Xiangqi cannon capture         |
| `mg_lw_hwacha` | `void mg_lw_hwacha(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)` | 8-direction cannon             |
| `mg_lw_sang`   | `void mg_lw_sang(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`   | Ortho then 2 diagonals         |
| `mg_lw_liubo`  | `void mg_lw_liubo(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`  | Teleport to threatened squares |

#### src/movegens/mg_zarqan.c

| Function               | Signature                                                                                                                           | Purpose                          |
| ---------------------- | ----------------------------------------------------------------------------------------------------------------------------------- | -------------------------------- |
| `mg_zq_ziraafa`        | `void mg_zq_ziraafa(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`        | Diagonal then orthogonal walk    |
| `mg_zq_swap_with_king` | `void mg_zq_swap_with_king(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)` | Returns friendly king's position |
| `mg_zq_war_elephant`   | `void mg_zq_war_elephant(const PieceState* piece, const BattleState* battle, const EffectArg* args, size_t count, MoveList* out)`   | Adjacent threat pattern          |

### 4.6 SDL Frontend Files (src/sdl/)

#### src/sdl/app.c

| Function      | Signature                    | Purpose            |
| ------------- | ---------------------------- | ------------------ |
| `app_init`    | `App* app_init(void)`        | Initialize SDL app |
| `app_run`     | `int app_run(App* app)`      | Main app loop      |
| `app_destroy` | `void app_destroy(App* app)` | Cleanup app        |

#### src/sdl/input.c

| Function             | Signature                                                       | Purpose                              |
| -------------------- | --------------------------------------------------------------- | ------------------------------------ |
| `input_update`       | `void input_update(Input* input)`                               | Update keyboard state                |
| `input_key_down`     | `bool input_key_down(const Input* input, SDL_Scancode key)`     | Check if key is down                 |
| `input_key_pressed`  | `bool input_key_pressed(const Input* input, SDL_Scancode key)`  | Check if key was pressed this frame  |
| `input_key_released` | `bool input_key_released(const Input* input, SDL_Scancode key)` | Check if key was released this frame |

#### src/sdl/main.c

**Purpose**: SDL entry point.

#### src/sdl/platform_sdl.c

| Function                   | Signature                                                                      | Purpose             |
| -------------------------- | ------------------------------------------------------------------------------ | ------------------- |
| `platform_window_create`   | `SDL_Window* platform_window_create(const char* title, int width, int height)` | Create SDL window   |
| `platform_window_destroy`  | `void platform_window_destroy(SDL_Window* window)`                             | Destroy SDL window  |
| `platform_renderer_create` | `SDL_Renderer* platform_renderer_create(SDL_Window* window)`                   | Create SDL renderer |

#### src/sdl/ui.c

Contains UI drawing functions (ui*draw*\* functions).

---

## 5. Backend API (Protocol Communication)

The engine communicates with the frontend via line-oriented stdio grammar. The engine writes lines prefixed with `< ` to stdout and reads frontend verbs prefixed with `> ` (prefix is optional) from stdin. Lines end with `\n` and are capped at `PROTOCOL_LINE_BYTES` (1024 bytes).

### 5.1 Constants

```c
#define PROTOCOL_LINE_BYTES  1024   // Max line length
#define PROTOCOL_VERB_BYTES  64     // Max verb length
#define PROTOCOL_VALUE_BYTES 64      // Max value length in key=value pairs
```

### 5.2 Protocol Severity Levels

```c
typedef enum ProtocolSeverity {
    PROTOCOL_LOG_INFO,  // Informational log
    PROTOCOL_LOG_WARN,   // Warning log
    PROTOCOL_LOG_ERR     // Error log
} ProtocolSeverity;
```

### 5.3 Types

**ProtocolVerb Struct** - Parsed frontend command:

```c
typedef struct ProtocolVerb {
    char        verb[PROTOCOL_VERB_BYTES];  // First whitespace-delimited token
    const char* tail;                        // Pointer to rest of line after verb
} ProtocolVerb;
```

When a line is blank or only whitespace, `verb[0] == '\0'`.

---

### 5.4 Engine Output (Frontend Input Verbs)

#### General Structure

```
> <verb> [key=value ...]
```

The engine reads these lines from stdin. The `> ` prefix is optional.

#### Complete Verb Reference

| Verb          | Parameters                         | Description                                                 |
| ------------- | ---------------------------------- | ----------------------------------------------------------- |
| `new_run`     | -                                  | Start a fresh run, generate first map, transition to MAP    |
| `load_run`    | -                                  | Load saved run from disk, transition to MAP                 |
| `goto`        | `<screen>`                         | Jump to specified screen                                    |
| `open_codex`  | `<kind>`                           | Open codex popup with kind (e.g., "piece", "card", "relic") |
| `close_popup` | -                                  | Close any open popup/codex                                  |
| `select_node` | `<id>`                             | Move map highlight to node ID                               |
| `play_card`   | `hand_index=N [target=piece:<id>]` | Play card at hand index, optionally targeting piece         |
| `sell_card`   | `hand_index=N`                     | Sell card at hand index                                     |
| `move`        | `piece=<id> to=(<x>,<y>)`          | Move piece to position                                      |
| `buy`         | `template=<name> at=(<x>,<y>)`     | Buy piece template, place at position                       |
| `combine`     | `a=<id> b=<id>`                    | Combine two pieces                                          |
| `end_turn`    | -                                  | End current turn                                            |
| `quit`        | -                                  | Quit the application                                        |
| `continue`    | -                                  | Continue from results screen                                |
| `ack`         | -                                  | Acknowledge and continue (alias for continue)               |

---

### 5.5 Engine Input (Frontend Output)

#### General Structure

```
< <type> <screen/topic> [key=value ...]
```

The engine writes these lines to stdout.

#### SHOW Lines

Emitted when entering a new screen or on screen transition.

**Format:**

```
< SHOW <screen> [key=value ...]
```

**Screens:**

| Screen     | Emitted Parameters                                    | Description                                |
| ---------- | ----------------------------------------------------- | ------------------------------------------ |
| `title`    | `resume=<0/1>`                                        | Title screen. resume=1 if saved run exists |
| `map`      | `kingdom=<0-4> tier=<0-2> node_cursor=<id>`           | Map screen with current kingdom/tier       |
| `battle`   | `turn=<N> active=<player/enemy> cp=<N> meter=<N>/<N>` | Battle screen header                       |
| `event`    | -                                                     | Event screen (stub)                        |
| `results`  | `outcome=<win/loss>`                                  | End-of-run summary                         |
| `codex`    | `kind=<piece/card/relic/recipe>`                      | Codex browser                              |
| `mastery`  | -                                                     | Mastery progression screen                 |
| `settings` | -                                                     | Settings screen                            |

#### STATE Lines

Emitted for state updates, typically one per entity.

**Format:**

```
< STATE <topic> [key=value ...]
```

**Topics:**

| Topic      | Emitted Parameters                            | Description                           |
| ---------- | --------------------------------------------- | ------------------------------------- |
| `map.node` | `id=<id> type=<0-6> payload=<id> x=<x> y=<y>` | Map node data (emitted once per node) |

Node types (NodeType enum):

| Value | Type                  | Description        |
| ----- | --------------------- | ------------------ |
| 0     | NODE_BATTLE           | Standard battle    |
| 1     | NODE_ELITE            | Elite battle       |
| 2     | NODE_ARCHIVE          | Recipe reveal      |
| 3     | NODE_OFFERING         | Card removal       |
| 4     | NODE_EVENT            | Narrative event    |
| 5     | NODE_OVERSEER         | Boss fight         |
| 6     | NODE_LIBERATION_TRIAL | Liberation attempt |

#### POPUP Lines

Emitted to show or hide popup overlays.

**Format:**

```
< POPUP <name> [key=value ...]
```

| Name    | Emitted Parameters | Description      |
| ------- | ------------------ | ---------------- |
| `codex` | `kind=<type>`      | Open codex popup |
| `close` | (empty)            | Close any popup  |

#### LOG Lines

Emitted for logging and debugging.

**Format:**

```
< LOG <level> <message>
```

| Level             | Tag    | Description           |
| ----------------- | ------ | --------------------- |
| PROTOCOL_LOG_INFO | `info` | Informational message |
| PROTOCOL_LOG_WARN | `warn` | Warning message       |
| PROTOCOL_LOG_ERR  | `err`  | Error message         |

#### QUIT Line

Emitted when the engine is shutting down.

**Format:**

```
< QUIT
```

---

### 5.6 Screen V-Table Hooks

Each screen implements the Screen v-table with four hooks:

```c
typedef struct Screen {
    void (*enter)(EngineState*);     // Called when entering screen
    void (*leave)(EngineState*);     // Called when leaving screen
    void (*handle)(EngineState*, const ProtocolVerb*);  // Handle frontend verb
    void (*emit)(EngineState*);      // Emit state to frontend
} Screen;
```

#### Title Screen (SCREEN_TITLE_V)

| Hook   | Function       | Purpose                                         |
| ------ | -------------- | ----------------------------------------------- |
| enter  | `title_enter`  | Checks if saved run exists (`resume_available`) |
| leave  | -              | No-op                                           |
| handle | `title_handle` | Handles `new_run`, `load_run`                   |
| emit   | `title_emit`   | Emits `resume=N`                                |

**Flow:**

- `new_run`: Allocates RunState, generates seed from `platform_time_ns()`, initializes Harushima Town map, saves run, transitions to MAP
- `load_run`: Loads saved run from disk, attaches profile, transitions to MAP

#### Map Screen (SCREEN_MAP_V)

| Hook   | Function     | Purpose                                   |
| ------ | ------------ | ----------------------------------------- |
| enter  | -            | No-op                                     |
| leave  | -            | No-op                                     |
| handle | `map_handle` | Handles `select_node`                     |
| emit   | `map_emit`   | Emits SHOW + STATE map.node for each node |

**Emit Output Example:**

```
< SHOW map kingdom=1 tier=1 node_cursor=0
< STATE map.node id=0 type=0 payload=0 x=0 y=0
< STATE map.node id=1 type=1 payload=0 x=1 y=0
...
```

#### Battle Screen (SCREEN_BATTLE_V)

| Hook   | Function        | Purpose                            |
| ------ | --------------- | ---------------------------------- |
| enter  | -               | No-op                              |
| leave  | -               | No-op                              |
| handle | `battle_handle` | Stub (no verbs handled yet)        |
| emit   | `battle_emit`   | Emits turn, active side, CP, meter |

**Emit Output:**

```
< SHOW battle turn=1 active=player cp=30 meter=50/100
```

#### Results Screen (SCREEN_RESULTS_V)

| Hook   | Function         | Purpose                               |
| ------ | ---------------- | ------------------------------------- |
| enter  | `results_enter`  | Calls `run_finalize`, detaches run    |
| leave  | -                | No-op                                 |
| handle | `results_handle` | Handles `continue`, `ack`             |
| emit   | `results_emit`   | Emits `outcome=win` or `outcome=loss` |

#### Codex Screen (SCREEN_CODEX_V)

| Hook   | Function       | Purpose             |
| ------ | -------------- | ------------------- |
| enter  | -              | No-op               |
| leave  | -              | No-op               |
| handle | `codex_handle` | Stub                |
| emit   | `codex_emit`   | Emits `kind=<type>` |

---

### 5.7 Engine Verb Dispatch Flow

The engine's `engine_handle_line()` function processes each frontend input:

```
1. protocol_parse(line) → ProtocolVerb {verb, tail}
2. If verb is empty/whitespace → ignore
3. If verb == "quit" → set quitting=true, emit QUIT, return
4. If verb == "goto" → find screen by name, call screen_goto()
5. If verb == "open_codex" → set codex.kind, emit POPUP codex
6. If verb == "close_popup" → set codex.open=false, emit POPUP close
7. Else → forward to active screen's handle() hook
8. Apply any pending screen transition
9. fflush(stdout)
```

---

### 5.8 Argument Parsing Helpers

The protocol module provides helpers to extract typed values from verb tails:

| Function             | Signature                                                                              | Purpose                                       |
| -------------------- | -------------------------------------------------------------------------------------- | --------------------------------------------- |
| `protocol_read_line` | `bool protocol_read_line(FILE* in, char* buffer, size_t capacity)`                     | Read one newline-terminated line, strip CR/LF |
| `protocol_parse`     | `void protocol_parse(const char* line, ProtocolVerb* out)`                             | Parse line into verb + tail                   |
| `protocol_arg_str`   | `bool protocol_arg_str(const char* tail, const char* key, char* out, size_t capacity)` | Extract string value for key=                 |
| `protocol_arg_int`   | `bool protocol_arg_int(const char* tail, const char* key, int* out)`                   | Extract integer value for key=                |
| `protocol_arg_xy`    | `bool protocol_arg_xy(const char* tail, const char* key, int* x, int* y)`              | Extract position (x,y) for key=               |

**Example Usage:**

```
// Frontend sends: "move piece=42 to=(3,5)"
// Handler extracts:
int piece_id;
int x, y;
protocol_arg_int(verb->tail, "piece", &piece_id);  // piece_id = 42
protocol_arg_xy(verb->tail, "to", &x, &y);         // x=3, y=5
```

---

### 5.9 Screen Name Mapping

| ScreenId        | Name String | Source File       |
| --------------- | ----------- | ----------------- |
| SCREEN_TITLE    | "title"     | screen_title.c    |
| SCREEN_MAP      | "map"       | screen_map.c      |
| SCREEN_BATTLE   | "battle"    | screen_battle.c   |
| SCREEN_EVENT    | "event"     | screen_event.c    |
| SCREEN_RESULTS  | "results"   | screen_results.c  |
| SCREEN_CODEX    | "codex"     | screen_codex.c    |
| SCREEN_MASTERY  | "mastery"   | screen_mastery.c  |
| SCREEN_SETTINGS | "settings"  | screen_settings.c |

The `screen_name()` function converts ScreenId to string, used by `protocol_emit_show()`.

---

### 5.10 Example Protocol Session

**Frontend → Engine:**

```
> new_run
```

**Engine → Frontend:**

```
< SHOW title resume=0
< SHOW map kingdom=1 tier=1 node_cursor=0
< STATE map.node id=0 type=0 payload=0 x=0 y=0
< STATE map.node id=1 type=2 payload=0 x=1 y=0
< STATE map.node id=2 type=0 payload=0 x=2 y=0
...
```

**Frontend → Engine:**

```
> select_node 2
```

**Engine → Frontend:**

```
< SHOW map kingdom=1 tier=1 node_cursor=2
```

**Frontend → Engine:**

```
> goto battle
```

**Engine → Frontend:**

```
< SHOW battle turn=0 active=none
```

---

### 5.11 Battle Actions (Future)

These verbs are defined in protocol.h but battle_handle() is currently a stub:

```
> play_card <hand_index> [target=piece:<id>]
> sell_card <hand_index>
> move piece=<id> to=(<x>,<y>)
> buy template=<name> at=(<x>,<y>)
> combine a=<id> b=<id>
> end_turn
```

When implemented, these will call:

- `battle_play_card()`
- `battle_sell_card()`
- `battle_action_move()`
- `battle_action_buy()`
- `battle_action_combine()`

---

### 5.12 Key Files

| File                    | Description                             |
| ----------------------- | --------------------------------------- |
| `incl/protocol.h`       | Protocol header with full documentation |
| `src/engine/protocol.c` | Protocol implementation                 |
| `src/engine/engine.c`   | Verb dispatch and screen transitions    |
| `src/engine/screen_*.c` | Individual screen implementations       |

---

## 6. Implementation Status vs GDD

### 6.1 Phase 1: Core Loop ✅ IMPLEMENTED

| Feature               | Status | Notes                                 |
| --------------------- | ------ | ------------------------------------- |
| 1 kingdom (Harushima) | ✅     | All 5 kingdoms implemented            |
| Town map              | ✅     | All 3 map tiers per kingdom           |
| 10-turn battles       | ✅     | Configurable max_turns                |
| Meter mechanic        | ✅     | Implemented in battle.c + eff_meter.c |
| End-of-turn damage    | ✅     | Implemented in battle_resolve()       |
| Flips                 | ✅     | Implemented in eff_flip.c             |
| Placement             | ✅     | Implemented in battle.c               |
| Combination           | ✅     | Implemented in recipe.c               |
| Card consumption      | ✅     | Implemented in card.c                 |
| 8 pieces              | ✅     | Harushima has 9 pieces                |
| 10 cards              | ✅     | Harushima has 7 cards + 12 universal  |
| 1 board layout        | ✅     | Variable board sizes                  |
| 1 AI archetype        | ✅     | 5 archetypes implemented              |

### 6.2 Phase 2: Single Kingdom Vertical Slice ✅ IMPLEMENTED

| Feature               | Status | Notes                   |
| --------------------- | ------ | ----------------------- |
| 1 kingdom, all 3 maps | ✅     | All 5 kingdoms complete |
| Full piece roster     | ✅     | 47 pieces total         |
| Combination tree      | ✅     | 15 recipes              |
| All Universal cards   | ✅     | 12 cards                |
| 3 AI archetypes       | ✅     | 5 archetypes            |
| Campaign map          | ✅     | All node types          |
| Overseer              | ✅     | All 5 implemented       |

### 6.3 Phase 3: Full Content ✅ IMPLEMENTED

| Feature              | Status | Notes                |
| -------------------- | ------ | -------------------- |
| All 5 kingdoms       | ✅     | Complete             |
| All Overseers        | ✅     | All 5 + Vorath       |
| Full piece roster    | ✅     | 47 pieces            |
| All combinations     | ✅     | 15 recipes           |
| All cards            | ✅     | 46 cards + 5 mastery |
| Chain penalty system | ✅     | Bronze/Silver/Gold   |
| Liberation Trial     | ✅     | Implemented          |
| Vorath               | ✅     | Implemented          |

### 6.4 Phase 4: Engagement Systems ⚠️ PARTIAL

| Feature           | Status | Notes                                  |
| ----------------- | ------ | -------------------------------------- |
| Battle Modifiers  | ✅     | 12 modifiers implemented               |
| Figurehead Powers | ✅     | All 5 implemented                      |
| Relics            | ✅     | All 26 implemented                     |
| Narrative Events  | ✅     | 14 events implemented                  |
| Board Traits      | ✅     | 10 traits implemented                  |
| AI Archetypes     | ✅     | 5 archetypes, partial AI logic         |
| Kingdom Synergies | ✅     | All 5 implemented                      |
| Meta-Progression  | ✅     | Codex + Mastery implemented            |
| Prestige          | ⚠️     | Flag storage only, effects not applied |
| Challenge Runs    | ❌     | Not implemented                        |
| Daily Conquest    | ❌     | Not implemented                        |

### 6.5 Known Gaps

| Feature                   | Priority | Notes                                        |
| ------------------------- | -------- | -------------------------------------------- |
| Complete AI logic         | High     | ai_score_move() and ai_score_buy() are stubs |
| Divination card           | Medium   | Enemy AI move storage not implemented        |
| Meter UI projection       | Medium   | Projected damage display not implemented     |
| Irregular board generator | Medium   | Currently uses fixed board sizes             |
| Data-driven movement      | Medium   | Some patterns hardcoded                      |
| All card effects          | Medium   | Several cards marked eff_todo                |
| SDL UI rendering          | High     | UI stubs present, needs completion           |
| Battle screen handling    | High     | battle_handle() is stub                      |

### 6.6 Card Implementation Status

| Kingdom   | Cards | Implemented | STUB |
| --------- | ----- | ----------- | ---- |
| Universal | 12    | 6           | 6    |
| Longwei   | 7     | 4           | 3    |
| Harushima | 7     | 2           | 5    |
| Kewarani  | 7     | 2           | 5    |
| Zarqan    | 8     | 3           | 5    |
| Caelan    | 8     | 3           | 5    |
| Mastery   | 5     | 0           | 5    |

### 6.7 Piece Passives Implementation Status

| Kingdom   | Pieces | Implemented | STUB |
| --------- | ------ | ----------- | ---- |
| Longwei   | 8      | 3           | 5    |
| Harushima | 9      | 3           | 6    |
| Kewarani  | 7      | 4           | 3    |
| Zarqan    | 9      | 2           | 7    |
| Caelan    | 8      | 2           | 6    |

### 6.8 Overseer Implementation Status

| Overseer            | Status | Notes                 |
| ------------------- | ------ | --------------------- |
| Iron Strategist     | ✅     | Pao + screen mechanic |
| Eternal Recursion   | ✅     | Re-flip mechanic      |
| Caravan of Conquest | ✅     | Spawn mechanic        |
| Many-Faced King     | ✅     | Prince swap mechanic  |
| Crowned Heretic     | ✅     | Ghost mechanic        |
| Vorath              | ✅     | Quadrant mechanic     |

---

## 7. Key Relationships

```
Profile (persistent)
    ↓ loaded/saved
RunState (campaign)
    ↓ contains
    ├── MapState (current map)
    │   └── MapNode[] (nodes with edges)
    ├── RelicId[] (held relics)
    └── flags (run-wide effects)
        ↓ used to initialize
BattleState (per battle)
    ├── BattleConfig (setup params)
    ├── Board (piece positions)
    │   └── PieceState[] (pieces on board)
    │       └── PieceTemplate (immutable data)
    ├── CardInstance[][] (hands)
    │   └── CardTemplate (immutable data)
    ├── EffectBus (active effects)
    │   └── Effect[] (triggered behaviors)
    └── Rng (random state)
        ↓ displayed via
EngineState (orchestrator)
    ├── Profile* (persistent data)
    ├── RunState* (current run)
    ├── BattleState* (active battle)
    └── ScreenId (current screen)
        ↓ communicates via
Protocol (stdio grammar)
        ↓ rendered by
App (SDL frontend)
```

---

## 8. File Count Summary

| Category                        | Files   |
| ------------------------------- | ------- |
| Headers (incl/)                 | 31      |
| Core sources (src/)             | 13      |
| Data files (src/data/)          | 18      |
| Effects (src/effects/)          | 18      |
| Engine (src/engine/)            | 17      |
| Move generators (src/movegens/) | 6       |
| SDL frontend (src/sdl/)         | 5       |
| **Total**                       | **108** |
