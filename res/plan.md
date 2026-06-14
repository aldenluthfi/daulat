# Regnum — Battle-Mechanic Boilerplate Plan

## 1. Context

The repo is bootstrapped (Makefile, `.clangd`, empty `src/` `incl/`
`bin/`, three `.aseprite` source files) and `res/GDD.md` describes the
full game. No engine code exists yet. SDL3 integration is explicitly
deferred — this plan delivers a **pure-logic battle API** that an SDL3
front-end will later consume.

The deliverable is three artifacts:

1. A compiling C11 boilerplate under `src/` and `incl/` that
   implements the battle mechanic with every game element registered
   as data and every behavior dispatched through function pointers.
2. `res/report_elements.md` — exhaustive mapping of every piece, card,
   relic, mastery card, innate, penalty chain (plus board traits,
   battle modifiers, events, figurehead powers, overseers, synergies)
   to the function pointer(s) and argument lists that implement them.
3. `res/report_expansion.md` — a guide for adding new content
   (pieces, cards, effects, movegens, kingdoms, AI archetypes,
   triggers, save/load, an SDL3 front-end).

The overriding constraint, in the user's words: **no exceptions for
any element**. King, capstones, Kewarani splitters, multi-step combos,
overseer mechanics — *all* are data records, never `if (piece->is_X)`
branches in engine code.

## 2. Architectural Principle — Universal `(Func, Args)` Tuple

One pattern carries the entire game. Three runtime tuples, all
isomorphic:

```c
/* Effect: a behavior keyed to an event, with typed arguments. */
typedef struct Effect {
    EffectTrigger trigger;
    EffectFunc    apply;
    EffectArg     args[MAX_EFFECT_ARGS];
    uint8_t       arg_count;
    int16_t       duration_turns;   /* 0=local, n=turns, -1=run */
    Side          owner;
    uint32_t      source_id;
} Effect;

/* MoveGen: a movement pattern with typed parameters. */
typedef struct MoveGen {
    MoveGenFunc func;
    EffectArg   params[MAX_MOVE_PARAMS];
    uint8_t     param_count;
} MoveGen;

/* AIArchetype: a decision policy with weighted parameters. */
typedef struct AIArchetype {
    AIPickFunc pick;
    AIWeights  primary;
    AIWeights  fallback;
    int        fallback_threshold;
} AIArchetype;
```

Every game element compiles down to one or more of these tuples
placed in arrays scoped at a particular lifetime:

| Scope                | Container                       | Lifetime         |
|----------------------|---------------------------------|------------------|
| This-turn local      | `BattleState.bus` (dur=0)       | until turn end   |
| Multi-turn timed     | `BattleState.bus` (dur=n)       | until n turns    |
| Whole-battle         | `BattleState.bus` (dur=-1)      | until battle end |
| Piece passive        | `PieceTemplate.passives[]`      | piece lifetime   |
| Piece buff           | `PieceState.buffs[]`            | until expiry     |
| Card on-play         | `CardTemplate.on_play[]`        | emitted to bus   |
| Card on-sell         | `CardTemplate.on_sell[]`        | emitted to bus   |
| Run-wide (relic)     | `RunState.relic_effects[]`      | run              |
| Kingdom innate       | `Innate.effects[]`              | activates @ map  |
| Mastery              | `Mastery.effects[]`             | applied @ run    |
| Penalty chain        | `Chain.penalties[]`             | until kgdm win   |
| Board trait          | `BoardTrait.effects[]`          | this battle      |
| Battle modifier      | `Modifier.effects[]`            | this battle      |
| Figurehead power     | `FigureheadPower.effects[]`     | whole run        |

All routing happens through a single **EffectBus** keyed by
`EffectTrigger`. Adding a behavior never edits dispatcher code —
only adds an `Effect` entry to a template or run state.

## 3. Code Style — Save to Memory FIRST

These rules MUST be saved as a `feedback` memory entry as Step 0 of
execution so every future session enforces them automatically. They
take precedence over every default style heuristic.

* **80-char hard line limit** — never exceeded. Priority #1.
* File header: `//!` style, 3+ lines of description, then `Created:`
  and `Author : Alden Luthfi` lines.
* Doc comments above every func/struct/enum/type: `///`, 2+ lines.
* Comments inside functions ONLY at columns 80–120 using `/* ... */`
  block syntax. No `//` inside function bodies. No left-margin
  comments inside functions.
* Section dividers inside any source file:

  ```
  /*------------------------------------------------------------------------*\
                                 SECTION TITLE
  \*------------------------------------------------------------------------*/
  ```

* No unused variables. No silenced compiler warnings. Build with
  `-Wall -Wextra` and treat warnings as bugs.
* Descriptive variable names even inside macros (`piece_id` not `p`).
* **No project-name prefix on macros or constants** — use
  `MAX_PIECES`, not `REGNUM_MAX_PIECES`.
* **Constants are `#define`s, not enum members.** Enums are reserved
  for discriminated value sets (Side, Kingdom, Tier, ids, triggers).
* **Headers live flat in `incl/`**, not in a project-named subdir.
  Include lines read `#include "piece.h"`.

## 4. File & Directory Layout

```
incl/
  regnum.h          /* umbrella include — pulls every other header */
  defs.h            /* #define constants (MAX_*, version), id enums */
  types.h           /* Side, Kingdom, Tier, Position, MoveList */
  effect.h          /* Effect, EffectArg, EffectTrigger, EffectBus */
  movegen.h         /* MoveGen + primitive prototypes */
  piece.h           /* PieceTemplate, PieceState */
  card.h            /* CardTemplate, CardInstance, Hand */
  recipe.h          /* Recipe, recipe_lookup */
  meta.h            /* Relic, Innate, Mastery, Chain, BoardTrait,
                       Modifier, FigureheadPower, Event, Synergy,
                       Overseer */
  board.h           /* Board, territory queries, threat map */
  ai.h              /* AIArchetype, AIWeights, AIPickFunc */
  registry.h        /* access to all static tables by id */
  battle.h          /* BattleState, turn API, action API */
  run.h             /* RunState skeleton (relics, chains, mastery) */
  rng.h             /* deterministic xorshift */
  log.h             /* minimal logger */

src/
  main.c            /* demo: init battle, run 2 turns, print */
  effect.c          /* EffectBus dispatch + duration tick */
  movegen.c         /* MoveGen invocation, threat queries */
  piece.c           /* spawn, combine, flip, query */
  card.c            /* draw, play, sell, hand mgmt */
  recipe.c          /* recipe table lookup */
  meta.c            /* relic/innate/mastery/chain apply */
  board.c           /* bounds, territory, threat map */
  ai.c              /* archetype dispatcher + default scorer */
  registry.c        /* template id → pointer */
  battle.c          /* turn loop, resolve order */
  run.c             /* run-level state (stubs for now) */
  rng.c             /* xorshift64 */
  log.c             /* printf wrapper */
  effects/
    eff_meter.c     /* meter +/-, refill, cap, overflow */
    eff_economy.c   /* cp +/-, cost mods, sell mods, income */
    eff_damage.c    /* damage multipliers, reductions, immunity */
    eff_movement.c  /* movegen swap, free moves, extra steps */
    eff_piece.c     /* spawn, remove, swap, force flip */
    eff_card.c      /* draw extra, skip, peek, target removal */
    eff_flip.c      /* on-flip handlers, splitter spawns */
    eff_run.c       /* relic / chain / synergy hooks */
  movegens/
    mg_basics.c     /* parameterised primitives + composition:
                       step, step_set, slide, slide_dirs, leap_set,
                       blockable_leap, compound, choice, double_act,
                       territory_restricted, attack_only_subset,
                       todo */
    mg_longwei.c    /* bespoke Longwei patterns: ma, xiang_alfil,
                       pao, hwacha, sang, liubo_teleport */
    mg_harushima.c  /* kinsho, ginsho, kyosha, honorable_horse,
                       shishi (compound forms like promoted_bishop
                       and dragon use mg_compound from basics) */
    mg_kewarani.c   /* berolina (medeq), negus_guard_double
                       (Medeq Squad / Sultan's Levy reuse the
                       relevant primitives/wrappers) */
    mg_zarqan.c     /* ziraafa, swap_with_king (Shahzadeh free
                       action), war_elephant_threat (multi-target
                       attack on Ziraafa relocate) */
    mg_caelan.c     /* gryphon (Pawn/Knight/Bishop/Rook/Queen/King
                       resolve to mg_basics primitives directly,
                       Chancellor uses mg_choice from basics) */
  data/
    data_longwei.c     /* 5 base + 3 combos, 7 cards */
    data_harushima.c   /* 5 base + 4 combos, 7 cards */
    data_kewarani.c    /* 5 base + 2 combos, 7 cards */
    data_zarqan.c      /* 5 base + 4 combos, 8 cards */
    data_caelan.c      /* 5 base + 2 combos, 8 cards */
    data_universal.c   /* king + 12 universal cards */
    data_recipes.c     /* combination table */
    data_relics.c      /* 26 relics */
    data_innates.c     /* 5 innates */
    data_masteries.c   /* 5 mastery cards + 5 lv3 upgrades + 5
                          lv1 innate-shift hooks */
    data_chains.c      /* Bronze / Silver / Gold */
    data_modifiers.c   /* economy / meter / cards / board */
    data_traits.c      /* 10 board traits */
    data_events.c      /* narrative events */
    data_figureheads.c /* 5 starting powers */
    data_overseers.c   /* 5 overseers + Vorath */
    data_synergies.c   /* 5 cleared-kingdom bonuses */
    data_archetypes.c  /* 5 AI archetypes */
```

`.clangd` already adds `-Iincl`; flat layout works as-is. The
Makefile's `$(wildcard src/*.c) $(wildcard src/**/*.c)` covers the
single-subdir-deep tree used here (root + `effects/` + `movegens/`
+ `data/`).

## 5. Core Types (sketches)

Project size limits are `#define`s with no project prefix.

`incl/defs.h`:
```c
#define MAX_PIECES           128
#define MAX_PIECES_PER_SIDE   64
#define MAX_HAND              12
#define MAX_CARDSET           96
#define MAX_EFFECTS          256
#define MAX_EFFECT_ARGS        6
#define MAX_PIECE_BUFFS        8
#define MAX_PIECE_PASSIVES     4
#define MAX_CARD_EFFECTS       6
#define MAX_MOVE_PARAMS        8
#define MAX_SUB_MOVEGENS       4
#define MAX_MOVES             64
#define MAX_BOARD_DIM         20
#define MAX_RELICS_HELD       26
```

Enums stay enums for discriminated value sets (each value has
semantic identity, not a size).

`incl/types.h`:
```c
typedef enum { SIDE_PLAYER, SIDE_ENEMY, SIDE_NEUTRAL } Side;

typedef enum {
    KINGDOM_LONGWEI, KINGDOM_HARUSHIMA, KINGDOM_KEWARANI,
    KINGDOM_ZARQAN,  KINGDOM_CAELAN,    KINGDOM_NONE
} Kingdom;

typedef enum {
    TIER_DISTRICT, TIER_TOWN, TIER_PROVINCE, TIER_COUNTRY,
    TIER_CAPSTONE, TIER_KING
} Tier;

typedef struct { int8_t x, y; } Position;

typedef struct {
    Position squares[MAX_MOVES];
    uint8_t  count;
} MoveList;
```

Id enums (each value is a discrete name, kept in `incl/defs.h`):
```c
typedef enum {
    PIECE_KING = 0,
    /* Longwei */
    PIECE_BING, PIECE_XIANG, PIECE_MA, PIECE_PAO,
    PIECE_LIUBO_DIVINER, PIECE_SANG, PIECE_NORTHERN_CAVALRY,
    PIECE_HWACHA,
    /* Harushima */
    PIECE_FUHYO, PIECE_KYOSHA, PIECE_GINSHO, PIECE_KINSHO,
    PIECE_SHISHI, PIECE_HONORABLE_HORSE, PIECE_PROMOTED_BISHOP,
    PIECE_DAIMYO, PIECE_DRAGON,
    /* ...all 41 piece ids... */
    PIECE_ID_COUNT
} PieceId;

typedef enum {
    CARD_PAWN_STORM, CARD_REVITALIZE, /* ...all 49 cards... */
    CARD_ID_COUNT
} CardId;

typedef enum {
    RELIC_MERCHANTS_LEDGER, /* ...all 26 relics... */
    RELIC_ID_COUNT
} RelicId;

/* INNATE_*, CHAIN_*, MODIFIER_*, TRAIT_*, EVENT_*, OVERSEER_*,
   SYNERGY_*, MASTERY_*, FIGUREHEAD_POWER_*, ARCHETYPE_*  */
```

`incl/effect.h`:
```c
typedef enum {
    /* lifecycle */
    TRIGGER_BATTLE_START, TRIGGER_BATTLE_END,
    TRIGGER_TURN_START,   TRIGGER_TURN_END,
    /* resolve */
    TRIGGER_RESOLVE_DEFENSE, TRIGGER_RESOLVE_ATTACK,
    TRIGGER_RESOLVE_DAMAGE,  TRIGGER_RESOLVE_FLIP,
    /* card events */
    TRIGGER_CARD_DRAWN, TRIGGER_CARD_PLAYED, TRIGGER_CARD_SOLD,
    TRIGGER_COMBO_CHAIN_2, TRIGGER_COMBO_CHAIN_3,
    /* piece events */
    TRIGGER_PIECE_PLACED, TRIGGER_PIECE_MOVED,
    TRIGGER_PIECE_COMBINED, TRIGGER_PIECE_DEALT_DAMAGE,
    TRIGGER_PIECE_ENTERED_ENEMY_TERR,
    TRIGGER_PIECE_FLIPPED, TRIGGER_PIECE_REMOVED,
    /* queries (mutate out-params) */
    TRIGGER_QUERY_PIECE_COST, TRIGGER_QUERY_SELL_VALUE,
    TRIGGER_QUERY_DRAW_COUNT, TRIGGER_QUERY_TURN_INCOME,
    TRIGGER_QUERY_MOVE_COUNT, TRIGGER_QUERY_DAMAGE_MULT,
    TRIGGER_QUERY_METER_CAP,
    /* meta */
    TRIGGER_MAP_ENTERED, TRIGGER_RUN_START,
    TRIGGER_COUNT
} EffectTrigger;

typedef enum {
    EARG_INT, EARG_FLOAT, EARG_PIECE_REF, EARG_CARD_REF,
    EARG_TMPL_REF, EARG_KINGDOM, EARG_TIER, EARG_POS, EARG_SIDE,
    EARG_MOVEGEN_REF
} EffectArgType;

typedef struct EffectArg {
    EffectArgType type;
    union {
        int      i;
        float    f;
        uint32_t piece_id;
        uint32_t card_id;
        uint16_t tmpl_id;
        Kingdom  kingdom;
        Tier     tier;
        Position pos;
        Side     side;
        uint16_t movegen_id;
    } v;
} EffectArg;

typedef struct EffectCtx EffectCtx;   /* defined in effect.c */
typedef void (*EffectFunc)(EffectCtx *ctx,
                           const EffectArg *args, size_t n);
```

`EffectCtx` is a union of per-trigger evidence (attacker / target
pointers, mutable out-params for queries, card pointer, etc.) so
the bus signature is uniform regardless of trigger.

`incl/piece.h`:
```c
typedef struct PieceTemplate {
    uint16_t      id;
    const char   *name;
    Kingdom       kingdom;
    Tier          tier;
    int           base_value;
    MoveGen       move;
    MoveGen       threat;     /* .func==NULL → reuse move */
    Effect        passives[MAX_PIECE_PASSIVES];
    uint8_t       passive_count;
} PieceTemplate;

typedef struct PieceState {
    uint32_t             id;
    const PieceTemplate *tmpl;
    MoveGen              move_override;     /* .func==NULL = none */
    MoveGen              threat_override;
    Side                 owner;
    Position             pos;
    int                  value_mod;         /* Conqueror's Reward */
    Effect               buffs[MAX_PIECE_BUFFS];
    uint8_t              buff_count;
    uint8_t              moves_used;        /* per action */
    uint16_t             flags;             /* HAS_MOVED, etc. */
    uint8_t              streak_attack;     /* Iron Strategist */
} PieceState;
```

`incl/card.h`:
```c
typedef struct CardTemplate {
    uint16_t      id;
    const char   *name;
    Kingdom       kingdom;
    Tier          tier;
    int           play_cost;        /* -1 if no play action */
    int           sell_value;
    Effect        on_play[MAX_CARD_EFFECTS];
    uint8_t       play_effect_count;
    Effect        on_sell[MAX_CARD_EFFECTS];
    uint8_t       sell_effect_count;
    TargetFunc    target_query;     /* enumerate valid targets */
} CardTemplate;
```

`incl/battle.h`:
```c
typedef struct BattleState {
    Board               board;
    BattleConfig        config;
    Side                active_side;
    uint16_t            turn_no;
    uint16_t            max_turns;
    int                 cp[2];
    int                 meter[2];
    int                 meter_cap[2];
    int                 meter_overflow_cap[2];
    PieceState          pieces[MAX_PIECES];
    uint16_t            piece_count;
    uint32_t            next_piece_id;
    CardInstance        hand[2][MAX_HAND];
    uint8_t             hand_count[2];
    const CardTemplate *cardset[2][MAX_CARDSET];
    uint16_t            cardset_count[2];
    uint8_t             actions_left;
    EffectBus           bus;
    Rng                 rng;
} BattleState;
```

## 6. SDL3-Facing API Surface

The SDL3 front-end (built later, not in this boilerplate) consumes
`BattleState` as the model and drives interaction through five
groups of functions. None of them touch SDL — they are pure C
APIs. Everything below is what a renderer/input layer needs.

### 6.1 Lifecycle

```c
void          battle_init(BattleState *bs,
                          const BattleConfig *cfg);
void          battle_destroy(BattleState *bs);
BattleResult  battle_check_end(const BattleState *bs);
void          battle_turn_start(BattleState *bs);
void          battle_turn_end(BattleState *bs);
```

`BattleConfig` carries board dimensions, max turns, starting cp,
RNG seed, active battle modifiers, active board traits, and a
pointer to the `RunState` (for relics / chains / mastery /
figurehead powers / synergies).

### 6.2 Read-only state accessors

```c
const PieceState   *battle_piece_at(const BattleState *bs,
                                    Position p);
const PieceState   *battle_piece_by_id(const BattleState *bs,
                                       uint32_t piece_id);
size_t              battle_pieces(const BattleState *bs, Side side,
                                  const PieceState **out,
                                  size_t cap);
Side                battle_territory(const BattleState *bs,
                                     Position p);
int                 battle_threat_count(const BattleState *bs,
                                        Position p, Side attacker);
size_t              battle_hand(const BattleState *bs, Side side,
                                const CardInstance **out,
                                size_t cap);
const CardTemplate *battle_card_tmpl(uint16_t id);
const PieceTemplate*battle_piece_tmpl(uint16_t id);
```

Any field of `BattleState` is also directly readable since the
struct is public — these helpers exist for the common queries the
UI will ask repeatedly.

### 6.3 Affordance enumeration (UI legality)

The renderer uses these to highlight legal squares, show buy
ghosts, show valid combine partners, list valid card targets.

```c
size_t battle_legal_moves(const BattleState *bs, uint32_t piece_id,
                          MoveList *out);
size_t battle_valid_buy_squares(const BattleState *bs, Side side,
                                uint16_t tmpl_id,
                                Position *out, size_t cap);
size_t battle_valid_combinations(const BattleState *bs, Side side,
                                 uint32_t (*out_pairs)[2],
                                 size_t cap);
size_t battle_card_targets(const BattleState *bs, Side side,
                           uint8_t hand_idx,
                           TargetSpec *out, size_t cap);
const Recipe *battle_recipe_preview(uint16_t a, uint16_t b);
```

### 6.4 Projection queries (UI previews)

For the meter-projection panel the GDD calls out as a must-have:

```c
int  battle_query_cost(const BattleState *bs, Side side,
                       uint16_t tmpl_id);
int  battle_query_sell_value(const BattleState *bs, Side side,
                             const CardInstance *card);
int  battle_query_draw_count(const BattleState *bs, Side side);
int  battle_query_turn_income(const BattleState *bs, Side side);
int  battle_projected_damage(const BattleState *bs,
                             Side attacker);
int  battle_projected_flips(const BattleState *bs, Side attacker);
```

`battle_projected_damage` and `_flips` simulate the resolve step
on a *copy* of the EffectBus so the UI can show "this turn will
deal 110 damage; one enemy piece will flip" without committing.

### 6.5 Action API (input handling)

Each action has a `_can` companion so the UI can pre-validate
without attempting the mutation.

```c
bool battle_can_play_card(const BattleState *bs, uint8_t idx,
                          const TargetSpec *tgt);
bool battle_play_card(BattleState *bs, uint8_t idx,
                      const TargetSpec *tgt);

bool battle_can_sell_card(const BattleState *bs, uint8_t idx);
bool battle_sell_card(BattleState *bs, uint8_t idx);

bool battle_can_move(const BattleState *bs, uint32_t piece_id,
                     Position to);
bool battle_action_move(BattleState *bs, uint32_t piece_id,
                        Position to);

bool battle_can_buy(const BattleState *bs, uint16_t tmpl_id,
                    Position at);
bool battle_action_buy(BattleState *bs, uint16_t tmpl_id,
                       Position at);

bool battle_can_combine(const BattleState *bs, uint32_t a,
                        uint32_t b);
bool battle_action_combine(BattleState *bs, uint32_t a,
                           uint32_t b);

void battle_end_player_turn(BattleState *bs);  /* commit + AI */
```

### 6.6 AI driver

```c
void   ai_play_turn(BattleState *bs);    /* full half-turn */
Action ai_pick_one(BattleState *bs);     /* one action; debug */
```

### 6.7 Event log (animation sync)

State-mutating operations push events onto a ring buffer in
`BattleState`. The SDL3 layer drains events each frame and uses
them to schedule animations / sounds.

```c
typedef enum {
    EVT_TURN_STARTED,    EVT_TURN_ENDED,
    EVT_RESOLVE_BEGAN,   EVT_RESOLVE_ENDED,
    EVT_PIECE_PLACED,    EVT_PIECE_MOVED,
    EVT_PIECE_COMBINED,  EVT_PIECE_REMOVED,
    EVT_PIECE_FLIPPED,   EVT_PIECE_DEALT_DAMAGE,
    EVT_METER_CHANGED,   EVT_CP_CHANGED,
    EVT_CARD_DRAWN,      EVT_CARD_PLAYED,
    EVT_CARD_SOLD,       EVT_EFFECT_APPLIED,
    EVT_BATTLE_ENDED,    EVT_COUNT
} EventKind;

typedef struct Event {
    EventKind kind;
    uint16_t  turn_no;
    union {
        struct { uint32_t piece_id; Position from, to; } moved;
        struct { uint32_t piece_id; Position pos;
                 uint16_t tmpl_id; Side owner; }         placed;
        struct { uint32_t piece_id; Side new_owner; }    flipped;
        struct { uint32_t attacker;
                 Side victim_side; int dmg; }            dealt_damage;
        struct { Side s; int old_val, new_val; }         meter;
        struct { Side s; int old_val, new_val; }         cp;
        struct { Side s; uint16_t card_tmpl_id; }        card;
        struct { uint32_t effect_source_id;
                 EffectTrigger trigger; }                effect;
    } as;
} Event;

size_t battle_drain_events(BattleState *bs, Event *out,
                           size_t cap);
void   battle_clear_events(BattleState *bs);
```

`EVT_PIECE_DEALT_DAMAGE` records that an attacker contributed `dmg`
to `victim_side`'s meter. Pieces have no HP — damage always pools to
the side meter, never to an individual piece, so the payload names
the destination side, not a target piece.

`EVT_PIECE_REMOVED` fires only for: combination ingredient
consumption, Sacrifice/Mandate cards, and splitter substitution.
There is no removal-by-movement.

### 6.8 Summary

The SDL3 layer never reaches into `EffectBus` or `MoveGen` directly
— it reads `BattleState`, calls `battle_*` accessors, calls
`battle_action_*` for input, and drains events for animation. This
keeps the boundary clean: when SDL3 is plugged in, the game logic
stays exactly as-is.

## 7. Effect System

`src/effect.c` implements:

```c
void   bus_init(EffectBus *bus);
void   bus_register(EffectBus *bus, const Effect *e);
void   bus_emit(EffectBus *bus, BattleState *bs,
                EffectTrigger trigger, EffectCtx *ctx);
void   bus_tick_turn_end(EffectBus *bus);    /* expire dur=0 */
void   bus_tick_turn_start(EffectBus *bus);  /* decrement dur>0 */
size_t bus_query_count(const EffectBus *bus, EffectTrigger t);
```

Dispatch is a linear scan over `slots[]` filtered by trigger. With
≤256 slots and ~25 triggers, this is fine and obvious. A trigger
index can be added later if needed.

Effects of every scope (turn, battle, run, piece buff, board,
modifier) go through the same bus. Piece passives are copied into
the bus at piece-spawn time so they participate in dispatch
identically. When the piece is removed (flipped + not splitter,
combined-into-result), its passives are evicted from the bus by
`source_id` match.

## 8. Movement System — Full Catalog

Only two file families exist: `mg_basics.c` (parameterised
primitives + composition wrappers) and `mg_<kingdom>.c` (bespoke
patterns that can't be expressed by parameterising a primitive).
Each `PieceTemplate` references its movegens by function pointer in
its data entry. Generic chess movements (knight, bishop, rook,
queen, king, pawn) are not standalone functions — they are *uses
of basics with the right parameters* in the data file.

```c
typedef void (*MoveGenFunc)(const PieceState *piece,
                            const BattleState *bs,
                            const EffectArg *params, size_t n,
                            MoveList *out);
```

Each template carries `MoveGen move` (relocation) and `MoveGen
threat` (squares the piece menaces). If `threat.func == NULL` the
threat pattern equals the move pattern — true for most pieces. Pawn,
Medeq, Pao, Hwacha have distinct threat patterns.

### 8.1 `mg_basics.c` — primitives + composition

| Function                  | Params                                |
|---------------------------|---------------------------------------|
| `mg_step`                 | dx, dy                                |
| `mg_step_set`             | n × (dx,dy) — drives ferz, wazir,     |
|                           | king-set, kinsho, ginsho, etc.        |
| `mg_slide`                | dx, dy, min_dist, max_dist (drives    |
|                           | kyosha)                               |
| `mg_slide_dirs`           | dir-mask, min, max (drives bishop,    |
|                           | rook, queen, talliya)                 |
| `mg_leap_set`             | n × (dx,dy) — drives knight, alfil,   |
|                           | camel, dababbah, honorable horse      |
| `mg_blockable_leap`       | dx, dy, list of intermediate offsets  |
| `mg_compound`             | n × MoveGen (union of sub-patterns —  |
|                           | drives promoted bishop, dragon)       |
| `mg_choice`               | n × MoveGen (player picks per action  |
|                           | — drives cataphract, chancellor)      |
| `mg_double_act`           | MoveGen (apply twice — drives shishi  |
|                           | and the negus-guard wrapper below)    |
| `mg_territory_restricted` | MoveGen, allowed-territory mask       |
| `mg_attack_only_subset`   | MoveGen for relocate, MoveGen for     |
|                           | attack — drives pawn, medeq           |
| `mg_todo`                 | placeholder; returns empty MoveList   |

These cover every piece whose movement is "parameterised chess".
Pawn (Caelan): `mg_attack_only_subset` wrapping `mg_step` (forward)
and `mg_step_set` (diagonal attacks), with first-move-2 handled by
a passive Effect that reads `HAS_MOVED` flag. Northern Cavalry:
`mg_leap_set` with knight offsets. Bishop, Rook, Queen: same
treatment via `mg_slide_dirs`. Daimyo and Old King: `mg_step_set`
with 8 king directions.

### 8.2 `mg_longwei.c`

Bespoke Longwei patterns that don't fit basics:

| Function              | Used by                                 |
|-----------------------|-----------------------------------------|
| `mg_lw_ma`            | Ma (knight L with elbow-square block    |
|                       | check that basics can't express)        |
| `mg_lw_xiang`         | Xiang ((2,2) blockable leap + cannot    |
|                       | *attack* in enemy territory: writes a   |
|                       | partial threat list)                    |
| `mg_lw_pao`           | Pao (rook-line move, attack only with   |
|                       | exactly one screen on the line)         |
| `mg_lw_hwacha`        | Hwacha (Pao screen rule on rook +       |
|                       | bishop lines)                           |
| `mg_lw_sang`          | Sang (1 ortho then 2 diagonal,          |
|                       | blockable at either intermediate)       |
| `mg_lw_liubo`         | Liubo Diviner (teleport to any square   |
|                       | currently threatened by any enemy)      |

Bing uses `mg_basics:mg_step (0,1)`. Northern Cavalry uses
`mg_basics:mg_leap_set` with knight offsets — its Bulwark grant is
a passive Effect, not a movegen.

### 8.3 `mg_harushima.c`

| Function                  | Used by                              |
|---------------------------|--------------------------------------|
| `mg_hs_kinsho`            | Kinsho — 6-direction step set (no    |
|                           | diag-back). Expressible as           |
|                           | `mg_step_set`, but kept named for    |
|                           | clarity since Honorable Horse swaps  |
|                           | into it on territory entry.          |
| `mg_hs_ginsho`            | Ginsho — 5-direction step set        |
|                           | (fwd + diag). Same naming rationale. |
| `mg_hs_honorable_horse`   | Asymmetric 2-fwd-1-side leap. A      |
|                           | passive Effect listens for           |
|                           | TRIGGER_PIECE_ENTERED_ENEMY_TERR and |
|                           | swaps `move_override` to             |
|                           | `mg_hs_kinsho`.                      |
| `mg_hs_shishi`            | Move-then-move OR move-then-attack-  |
|                           | without-moving on king-set. Cannot   |
|                           | be expressed as `mg_double_act`      |
|                           | alone (the "attack without moving"   |
|                           | variant is special).                 |

Fuhyo uses `mg_basics:mg_step (0,1)`. Kyosha uses
`mg_basics:mg_slide` forward. Daimyo uses `mg_basics:mg_step_set`
with king directions; its immune-once flip is a passive Effect.
Promoted Bishop = `mg_basics:mg_compound` over bishop slide + wazir
step. Dragon = `mg_basics:mg_compound` over rook slide + ferz step.

### 8.4 `mg_kewarani.c`

| Function                  | Used by                              |
|---------------------------|--------------------------------------|
| `mg_kw_berolina`          | Medeq — diagonal 1 step, attack      |
|                           | forward 1. Specific enough to keep   |
|                           | named (also reused for the splitter  |
|                           | piece templates).                    |
| `mg_kw_negus_guard`       | Negus Guard double-act wrapper.      |
|                           | Could be `mg_double_act(mg_step_set  |
|                           | king-directions)`, but kept named    |
|                           | because Sultan's Levy reuses it.     |

Makwanam → `mg_step_set` ferz. Saba → `mg_leap_set` alfil (no
territory restriction). Faras → `mg_leap_set` dababbah offsets.
Medeq Squad → `mg_kw_berolina` (its on-flip spawn is a passive
Effect). Sultan's Levy → `mg_kw_negus_guard` (its on-flip spawn is
a passive Effect).

### 8.5 `mg_zarqan.c`

| Function                  | Used by                              |
|---------------------------|--------------------------------------|
| `mg_zq_ziraafa`           | Ziraafa — 1 diagonal step then 3+    |
|                           | squares straight. Two-stage path     |
|                           | check requires bespoke walk.         |
| `mg_zq_swap_with_king`    | Shahzadeh's free swap (surfaced as   |
|                           | an alt action MoveGen returning the  |
|                           | king's square; the action API treats |
|                           | it specially via a flag).            |
| `mg_zq_war_elephant`      | Ziraafa relocate + multi-target      |
|                           | threat: the threat MoveGen produces  |
|                           | two adjacent enemies simultaneously. |

Wazir → `mg_basics:mg_step_set` orthogonals. Jamal → `mg_leap_set`
(1,3). Talliya → `mg_slide_dirs` diagonal mask with min=2. Old King
→ `mg_step_set` king-set. Cataphract → `mg_choice` over knight
leap_set + camel leap_set. Rook → `mg_slide_dirs` orthogonal mask.

### 8.6 `mg_caelan.c`

| Function                  | Used by                              |
|---------------------------|--------------------------------------|
| `mg_ca_gryphon`           | Gryphon — 1 diagonal step then any   |
|                           | orthogonal distance. Two-stage walk. |

Pawn → `mg_attack_only_subset` wrapping forward step and diagonal
step_set; first-move-2 via passive Effect. Knight, Bishop, Rook,
Queen → basics. Chancellor → `mg_choice` over rook slide + knight
leap_set. Sovereign Banner → same MoveGen as Queen; its adjacency
buff is a passive Effect.

### 8.7 Runtime movement overrides

Cards that change movement (Promotion, Gold Standard, Ambition,
Conquest, Crusade, Divine Right) install an Effect whose `apply`
writes into `PieceState.move_override` (and/or `threat_override`).
A null override falls back to the template; this is the *only*
path for runtime movement changes and applies uniformly.

## 9. Pieces

Templates are immutable `const PieceTemplate` literals in
`src/data/data_<kingdom>.c`. State is per-instance. A piece is
spawned by:

1. Copy template pointer into `PieceState.tmpl`.
2. Copy `template->passives[]` into `bus.slots[]` with
   `source_id = piece.id`.
3. Place at requested position.
4. Emit `TRIGGER_PIECE_PLACED`.

Flipping a piece:

1. Emit `TRIGGER_PIECE_FLIPPED`. Context carries the piece pointer
   and its current (old) owner. Effects on the piece's owner with
   this trigger fire first (so Kewarani splitters intercept and
   transform the flip into spawn-Medeq + remove-self).
2. If the flip was not consumed, switch the piece's owner.
3. Recalculate threatened squares + meter caps.

`FLIPPED` is one event, not three. Cards that care about a flip on
their side (Hostage's "next flip to your side", Reforge's "next time
one of your pieces flips") filter by side in the effect handler. No
LOST/GAINED perspective triggers exist — they were redundant.

Removing a piece (Sacrifice card, Mandate card, splitter
substitution, combination ingredient consumption) emits
`TRIGGER_PIECE_REMOVED`. There is no removal-by-movement — no
"capture" mechanic exists. Cards that need "you lose a piece"
semantics (Spite) install **two** passive effects: one on
`TRIGGER_PIECE_FLIPPED` with side filter, one on
`TRIGGER_PIECE_REMOVED` with side filter.

Kewarani splitter behavior is therefore *just* a passive effect on
PIECE_MEDEQ_SQUAD, PIECE_SULTANS_LEVY, and PIECE_NEGUS_GUARD that
listens for `TRIGGER_PIECE_FLIPPED`, spawns Medeq pawns, and emits
`TRIGGER_PIECE_REMOVED` for self. No engine code knows about
splitters; the splitter set is identified entirely by these three
template ids carrying the same passive.

## 10. Cards

Drawing: `card_draw(bs, side)` invokes `bus_emit(
TRIGGER_QUERY_DRAW_COUNT, ...)` with an int out-param to compute
count, then samples that many `CardTemplate*` from the side's
cardset, places them in `hand[side]`.

Playing: `battle_play_card(bs, idx, target)` deducts `play_cost`,
copies each `on_play[]` effect into the bus (timestamped this
turn), emits `TRIGGER_CARD_PLAYED`, decrements hand. If 2nd
same-kingdom card this turn → `TRIGGER_COMBO_CHAIN_2`; 3rd →
`TRIGGER_COMBO_CHAIN_3`.

Selling: copy `on_sell[]` to bus (often empty), credit
`battle_query_sell_value()`, emit `TRIGGER_CARD_SOLD`.

End of turn: any unplayed/unsold card auto-sells.

## 11. Recipes (Combinations)

```c
typedef struct Recipe {
    uint16_t ingredient_a;
    uint16_t ingredient_b;
    uint16_t result;
    uint8_t  steps;        /* 1 direct, 2 multi-step */
} Recipe;

extern const Recipe RECIPES[];
extern const size_t RECIPES_COUNT;

const Recipe *recipe_find(uint16_t a, uint16_t b);
```

`battle_action_combine(bs, piece_a, piece_b)` looks up the recipe
(`a` and `b` order-agnostic), removes piece A, transforms piece B
into the result template at piece B's square, evicts old passives
from the bus and registers new ones, emits `TRIGGER_PIECE_COMBINED`.

## 12. Meta Layer

Each meta concept is a header + template type that owns an
`Effect[]`. All are added/removed from the bus when their
activation condition fires. The run-level container (`RunState`)
holds the master lists for the current run.

| Meta concept         | Template type     | Activation             |
|----------------------|-------------------|------------------------|
| Relic                | `RelicTemplate`   | On acquisition         |
| Innate               | `InnateTemplate`  | Province map entered   |
| Mastery card         | `MasteryCard`     | Added to set on run    |
| Mastery innate-shift | `MasteryHook`     | Run start (level 1+)   |
| Mastery startup-up   | `MasteryHook`     | Run start (level 3)    |
| Penalty chain        | `ChainTemplate`   | Lose battle in kgdm    |
| Battle modifier      | `Modifier`        | Battle start           |
| Board trait          | `BoardTrait`      | Battle start           |
| Figurehead power     | `FigureheadPower` | Run start              |
| Synergy              | `Synergy`         | Overseer cleared       |
| Event choice         | `EventOption`     | Player selects on map  |

`RunState` is a stub in this boilerplate (full campaign loop is out
of scope) — it just needs to exist so `BattleState` can read relic
effects when initializing each battle.

## 13. Battle Loop — Turn Flow & Resolve Order

```
battle_init(bs, cfg):
    populate board, set kings, set cp=20 (mod relics), set meters,
    copy run-wide effects into bus,
    emit TRIGGER_BATTLE_START.

battle_turn_start(bs):
    bus_tick_turn_start(bus);    /* decrement durations */
    emit TRIGGER_TURN_START;
    cp[active] += query_turn_income();
    draw card_draw(bs, active) times.

(player or AI loop)
    while actions_left > 0 and not ended:
        battle_action_* or battle_play_card or battle_sell_card

battle_turn_end(bs):
    auto-sell unsold/unplayed cards.
    battle_resolve(bs):
        emit TRIGGER_RESOLVE_DEFENSE  (defender side)
        emit TRIGGER_RESOLVE_ATTACK   (attacker side)
        for each attacker piece on active side:
            compute threatened enemy squares (uses threat MoveGen);
            for each threatened square with enemy piece:
                damage = attacker.value
                       * query_damage_mult(attacker)
                       * query_damage_reduction(target);
                accumulate damage into total[passive_side];
        apply_damage_with_cascading_flips(bs, total[passive_side]);
    emit TRIGGER_TURN_END.
    bus_tick_turn_end(bus);      /* expire dur=0 */
    swap active_side;
    if active == player: turn_no++;
    if turn_no >= max_turns: settle_by_territory().

apply_damage_with_cascading_flips(bs, dmg):
    while dmg > 0:
        if meter <= dmg:
            dmg -= meter;
            meter = 0;
            flip a random non-king (mod TRIGGER_RESOLVE_FLIP);
            meter = recompute_cap();   /* "fills up again" */
        else:
            meter -= dmg;
            dmg = 0;
        if only enemy king remains and meter==0:
            mark battle_won.
```

Defensive effects (Bulwark, Cathedral, Counter Coup) fire during
`TRIGGER_RESOLVE_DEFENSE`. Offensive effects (Bloodletting,
Formation, Queen's Decree) fire during `TRIGGER_RESOLVE_ATTACK`.
Both modify per-piece multiplier registers held in `EffectCtx`.

Both sides' effects participate in both phases (Counter Coup needs
to read enemy damage). Reading order is deterministic by
`source_id`.

## 14. AI Archetypes

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

typedef struct AIArchetype {
    const char *name;
    Kingdom     kingdom;
    AIPickFunc  pick;
    AIWeights   primary;
    AIWeights   fallback;
    int         fallback_meter_diff;
} AIArchetype;
```

`AIPickFunc` returns one `Action` (move / buy / combine / play /
sell / end). The default `ai_pick_default` scores candidate
actions by a dot product of features against weights and picks the
argmax. Each kingdom can override `pick` with a custom function if
its archetype needs distinctive logic (Reclaimer, Trickster) — but
the *normal* path is weights-only.

Five archetypes are registered in `data_archetypes.c`. Bosses get
their own archetypes registered alongside.

## 15. Registry & Data Files

`incl/registry.h` exposes:

```c
const PieceTemplate     *piece_template(uint16_t id);
const CardTemplate      *card_template(uint16_t id);
const Recipe            *recipe_find(uint16_t a, uint16_t b);
const RelicTemplate     *relic_template(uint16_t id);
const InnateTemplate    *innate_template(Kingdom k);
const Chain             *chain_template(uint8_t level);
const BoardTrait        *trait_template(uint16_t id);
const Modifier          *modifier_template(uint16_t id);
const FigureheadPower   *figurehead_power(Kingdom k);
const Synergy           *synergy_template(Kingdom cleared);
const AIArchetype       *ai_archetype(Kingdom k);
```

Each lookup hits a single static array defined in
`src/data/data_*.c`. The arrays are dense and indexed by id so
`piece_template(PIECE_BING)` is `O(1)`.

All id enums (`PieceId`, `CardId`, `RelicId`, `InnateId`, etc.) live
in `incl/defs.h`; this is the single source of truth for ids.

## 16. `main.c` — Demo Battle Round

`src/main.c` performs:

1. `battle_init` on a 12×12 board, two kings only, cp=20.
2. Add one extra piece per side for free (sanity placement).
3. Run `battle_turn_start`, print drawn cards.
4. Issue a couple of API actions (move pawn, sell a card,
   end turn).
5. Run the AI half-turn via `ai_pick_default`.
6. `battle_turn_end`, print resolve numbers (damage, meter, flips).
7. Repeat once more for a full 2-turn trace.
8. Tear down.

This validates the API end-to-end without rendering. It also acts
as the smoke test: `make debug && ./bin/regnum` should print a
deterministic battle trace.

## 17. Makefile Adjustments

The current Makefile globs `src/*.c` and `src/**/*.c`. GNU Make's
`wildcard` doesn't recurse, so the second pattern only catches
single-level subdirs. With our `effects/`, `movegens/`, `data/`
(all one level deep), it works as-is.

Adjustments:

* Add `-std=c11` to both debug and release.
* Add `-Werror` to debug (matches "no silenced warnings" rule).
* Add `-MMD -MP` for header dependency tracking so editing a
  header rebuilds dependents.
* Append a `format` and `lint` target (cpplint already configured).

## 18. Reports to Generate

### 18.1 `res/report_elements.md`

Exhaustive catalog organized as:

```
## Pieces
### Universal
### Longwei
  - Bing
    | Behavior                | Mechanism                          |
    |-------------------------|------------------------------------|
    | Forward 1 step          | mg_step, args [dx=0, dy=1]         |
    | Sideways step after dmg | passive Effect on
    |                         | TRIGGER_PIECE_DEALT_DAMAGE →
    |                         | eff_grant_sideways_step(self)      |
  - Xiang
  - Ma
  - Pao
  - Liubo Diviner
  - Sang (combo)
  - Northern Cavalry (combo)
  - Hwacha (combo)
### Harushima  (5 base + 4 combos)
  - Base:  Fuhyo, Kyosha, Ginsho, Kinsho, Shishi
  - Combo: Honorable Horse, Promoted Bishop, Daimyo, Dragon
### Kewarani   (5 base + 2 combos)
  - Base:  Medeq, Makwanam, Saba, Faras, Negus Guard
  - Combo: Medeq Squad, Sultan's Levy
### Zarqan     (5 base + 4 combos)
  - Base:  Wazir, Jamal, Talliya, Ziraafa, Shahzadeh
  - Combo: Old King, Cataphract, Rook, War Elephant
### Caelan     (5 base + 2 combos)
  - Base:  Pawn, Knight, Bishop, Queen, Gryphon
  - Combo: Chancellor, Sovereign Banner

## Cards
### Universal  (12)
  - Pawn Storm
    | Behavior                | Mechanism                          |
    |-------------------------|------------------------------------|
    | Up to 3 pawns this turn,| on_play Effect with dur=0 on       |
    | 3rd is free             | TRIGGER_QUERY_PIECE_COST →         |
    |                         | eff_pawn_storm_pricing(counter)    |
  - District:  Revitalize, Hostage
  - Town:      Last Stand, Sacrifice, Reforge
  - Province:  Mercy, Bloodletting, Counter Coup
  - Country:   Spite, Chain Break, Hydra
### Longwei    (7)
  - District:  River Wade, Charge
  - Town:      Formation, Divination
  - Province:  Cannon Volley, Palace Decree
  - Country:   Mandate
### Harushima  (7)
  - District:  Ronin
  - Town:      Resurrection, Gold Standard, Promotion
  - Province:  Dual Drop, Force Drop
  - Country:   Bushido
### Kewarani   (7)
  - District:  Sultan's Gold, March
  - Town:      Double Time, Salt Road
  - Province:  Caravan, Doublestrike
  - Country:   Hajj
### Zarqan     (8)
  - District:  Counsel, Pillage
  - Town:      Royal Decoy, Bazaar, Steppe Riders
  - Province:  Ambition, Citadel
  - Country:   Conquest
### Caelan     (8)
  - District:  Castling, Queen's Gambit
  - Town:      Vengeance, Queen's Decree
  - Province:  Cathedral, Coronation
  - Country:   Crusade, Divine Right

## Relics  (26)
  - Economy:      Merchant's Ledger, Minted Coin, Tax Stamp,
                  Bulk Discount, War Chest, Trade Routes
  - Meter:        Soul Shard, Veteran's Bond, Dead Man's Pact,
                  Iron King, Bloodthirst, Last Breath
  - Cards:        Tactician's Scroll, Librarian's Notes,
                  Country Seal, Deep Hand, Gilded Archive
  - Combinations: Alchemist's Kit, Master's Notes,
                  Philosopher's Stone, Inherited Power
  - Board:        Eagle Eye, Surveyor's Map, Forward Command,
                  Fortified Line, Warlord's Banner

## Innates (5: Bulwark, Reclaim, Double Time, Royal Substitution,
            Conqueror's Reward)

## Mastery
### Mastery cards (5 figurehead cards)
The five Mastery-2 cards are proper `CardTemplate`s in the
`CARD_*` enum: `CARD_MINGZHUS_SEAL` (Longwei), `CARD_TOMOHITOS_PATIENCE`
(Harushima), `CARD_SELASSIES_MARCH` (Kewarani),
`CARD_TIMURS_CONQUEST` (Zarqan), `CARD_ISABELLAS_CORONATION`
(Caelan). The Mastery-2 hook for kingdom K injects the matching
card into the player's cardset when Mastery-2 is reached. There is
no separate `MasteryCard` struct — they are normal cards with
normal data.
### Mastery level 1 — innate-shift hooks (5)
### Mastery level 3 — starting-power upgrades (5)

## Penalty Chains (Bronze / Silver / Gold)

## Battle Modifiers
  - Economy: Lean Times, Windfall, Open Market
  - Meter:   Glass Cannon, Bloodbath, Iron Will
  - Cards:   Rich Hand, Sparse Hand, Kingdom Purity
  - Board:   Fog of War, Dense Terrain, Extended Front

## Board Traits (River Crossing, Palace, Fog Coast, Island Chain,
                 Trade Route, Contested Market, Sandstorm, Mirage,
                 Castle Corners, Siege Trench)

## Figurehead Powers (5)

## Overseers (Iron Strategist, Eternal Recursion, Caravan of
              Conquest, Many-Faced King, Crowned Heretic)

## Vorath
The final boss occupies a 20×20 board partitioned into 5
quadrants. Each quadrant holds a Minor King; the Grand King sits
in the centre quadrant. The Grand King's meter resets to full at
the start of every turn **unless** the player attacked at least
one Minor King in each of the 5 quadrants on the immediately
preceding turn. The run-end win condition is reducing the Grand
King's meter to zero in a turn where all 5 quadrants were struck.

## Kingdom Synergies (5)

## Events  (per-kingdom + universal)

## Combo Chain Climaxes (5)

## AI Archetypes (5)
One archetype per kingdom: Siege Engineer (Longwei), Reclaimer
(Harushima), The Tide (Kewarani), Trickster (Zarqan), The Hammer
(Caelan). Each is an `(AIPickFunc, AIWeights primary, AIWeights
fallback, int fallback_threshold)` tuple in `data_archetypes.c`.
```

Every row pins the element to one or more
`(EffectFunc | MoveGenFunc, EffectArg[])` tuples plus the trigger
and scope. Where a behavior is not yet implemented in the
boilerplate, the row still names the function it *would* call (so
the report doubles as a TODO list).

### 18.2 `res/report_expansion.md`

Step-by-step guides:

1. Adding a new piece (template + movegen + recipe entry).
2. Adding a new card (template + on_play / on_sell + targeting).
3. Adding a new effect (function signature, ctx evidence, args).
4. Adding a new movegen primitive.
5. Adding a new trigger (extending `EffectTrigger` + dispatcher).
6. Adding a new kingdom (data file + id range + AI archetype).
7. Adding a relic / chain / mastery (registry append + activation).
8. Wiring board traits and battle modifiers.
9. Wiring AI archetypes (weights vs custom pick).
10. Plugging in SDL3 (BattleState as model, draw loop reads
    `pieces[]` and `bus`, action API for input).
11. Adding save/load (binary blob of BattleState + RunState; bus
    needs effect-id table since function pointers don't serialize —
    each Effect's `apply` resolves through an id at load).
12. Adding unit tests (per-effect, per-movegen, per-resolve).
13. Adding multiplayer / replay (deterministic RNG seed + action
    log).

## 19. Implementation Order

Execution order once plan is approved:

1. **Save code-style rules to memory** as a `feedback` entry (this
   includes the new layout/naming/macro rules from Section 3).
2. `incl/defs.h`, `types.h`, `effect.h`, `movegen.h`, `piece.h`,
   `card.h`, `recipe.h`, `meta.h`, `board.h`, `ai.h`, `registry.h`,
   `battle.h`, `run.h`, `rng.h`, `log.h`, `regnum.h` (umbrella).
3. `src/effect.c` (bus + dispatch).
4. `src/board.c`, `rng.c`, `log.c`.
5. `src/movegens/mg_basics.c` (primitives + composition + todo).
6. `src/movegens/mg_longwei.c`, `mg_harushima.c`, `mg_kewarani.c`,
   `mg_zarqan.c`, `mg_caelan.c` (bespoke per-kingdom patterns;
   bodies for the most-used, `mg_todo` for the rest).
7. `src/effects/eff_*.c` (signatures wired; bodies for
   resolve-path essentials: meter, damage, flip, draw, economy,
   cost-mod).
8. `src/piece.c`, `card.c`, `recipe.c`, `meta.c`.
9. `src/data/data_*.c` (all elements registered, with `eff_todo`
   markers in `apply` slots for unimplemented behaviors).
10. `src/battle.c` (turn loop, resolve, cascading flips).
11. `src/ai.c` + `data_archetypes.c` (default scorer).
12. `src/main.c` (demo).
13. Update `Makefile` (`-std=c11`, `-MMD -MP`, `-Werror` on debug).
14. `make debug` clean.
15. Run `./bin/regnum`, capture trace.
16. Write `res/report_elements.md` (exhaustive).
17. Write `res/report_expansion.md`.

## 20. Verification

* `make clean && make debug` builds with zero warnings under
  `-Wall -Wextra -Werror`.
* `./bin/regnum` runs the demo, prints a deterministic trace
  containing: turn start banner, drawn card names, action acks,
  resolve damage breakdown, end-of-turn summary. Re-running gives
  byte-identical output (deterministic RNG seed).
* `make release` builds at `-O3` with no warnings.
* `wc -L` over all `.c`/`.h` files confirms every line ≤ 80 chars.
* Grep for `//` at the start of indented lines inside function
  bodies returns nothing.
* Every `.c`/`.h` file starts with a `//!` header block of ≥ 3
  description lines.
* Grep for `REGNUM_` returns nothing (no project prefix on macros).
* `res/report_elements.md` mentions every entity from the GDD
  (cross-check by grepping piece/card names against the GDD).
* `res/report_expansion.md` exists with all 13 guide sections.

## 21. Out of Scope (this boilerplate)

* SDL3 rendering, audio, input.
* Save/load serialization (planned in expansion report).
* Full campaign map state machine (only `RunState` stub).
* Animations / juice / particle systems.
* Network play.
* Localization.
* Tooling for the `.aseprite` sources.
* Asset pipeline.
* Unit-test framework integration (left to expansion report).

## 22. Open Decisions / Defaults

These defaults are committed; flag any to change before approval:

| Decision                       | Default                           |
|--------------------------------|-----------------------------------|
| Memory model                   | Static arrays, compile-time max   |
| C standard                     | C11 (designated initializers)     |
| RNG                            | xorshift64 seeded from config     |
| Per-side action count          | 3 (per GDD)                       |
| Turn semantics                 | 1 turn = one side's full cycle;   |
|                                | round = 2 turns; "10 turns"       |
|                                | counted per player turn           |
| Compiler warnings              | `-Wall -Wextra -Werror` on debug  |
| Header include style           | flat: `#include "piece.h"`        |
| Constant style                 | `#define`, no project prefix      |
| Logging                        | Plain `fprintf` behind `log_*`    |
| Demo `main.c` scope            | 2 turns, prints trace             |
| Test framework                 | None in boilerplate               |
| Element data file granularity  | One file per kingdom + 1 file     |
|                                | per meta category                 |
| Unimplemented effects          | Registered with `eff_todo`        |
|                                | placeholder, logs and no-ops      |
| Unimplemented movegens         | Registered with `mg_todo`,        |
|                                | returns empty `MoveList`          |
| Report exhaustiveness          | Every GDD-listed element gets a   |
|                                | row, including extras beyond the  |
|                                | strict list (modifiers, traits,   |
|                                | events, powers, overseers,        |
|                                | synergies)                        |

## 23. Header Rewrite — Complete Header DAG

### 23.1 Problem Statement

The current header graph has cycles that prevent compilation:
- `effect.h` → `movegen.h` → `effect.h` (cycle via EffectArg)
- `board.h` → `piece.h` → `effect.h` → `movegen.h` → `effect.h`
- `battle.h` has duplicate includes and circular deps

### 23.2 Solution: Single Prelude + DAG Ordering

Strategy: One `prelude.h` that all `.c` files include. Headers are written
as standalone units that only include their dependencies in declaration order.
No header includes another header that hasn't been fully parsed yet.

**Key insight**: `struct` forward declarations inside struct bodies are valid.
`PieceState*` in `EffectCtx` is fine because the body only contains pointers.

### 23.3 Header Dependency DAG

```
Layer 0 (no deps):
  core.h      — Tier, Kingdom, Side, BattleResult, Position, EffectArgType, EffectArg
  rng.h       — Rng, rng_* API
  log.h       — log_info/warn/err

Layer 1 (depends on core):
  defs.h      — #defines, id enums (PieceId, CardId, etc.)

Layer 2 (depends on defs + core):
  types.h     — CardInstance, CardInstanceFlags, PieceStateFlags, TargetSpec,
                inline helpers (side_opposite, pos_equal, pos_in_bounds)
                NOTE: Position already defined in core.h — do NOT redefine

Layer 3 (forward-decl only, depends on defs):
  movegen.h   — MoveList, MoveGenFunc typedef, MoveGen struct,
                ALL function prototypes
                NOTE: Uses PieceState* and BattleState* as forward decls
                NOTE: Does NOT include effect.h

  effect.h    — EffectTrigger enum, EffectFunc typedef, Effect struct,
                EffectSlot struct, EffectBus struct,
                EffectCtx struct (PieceState* forward decl inside)
                NOTE: Does NOT include movegen.h — MoveGen is in movegen.h

Layer 4 (depends on Layer 3):
  piece.h     — PieceTemplate, PieceState (full), piece API
                Includes: effect.h, movegen.h

  card.h      — CardTemplate, card API
                Includes: types.h, forward-declares BattleState for callbacks

  recipe.h    — Recipe struct, recipe API
                Includes: defs.h

  meta.h      — All meta types (RelicTemplate, InnateTemplate, Chain, etc.)
                Includes: defs.h, types.h, forward-declares Effect

  board.h     — Board struct, board API
                Includes: types.h, forward-declares PieceState
                NOTE: Does NOT include piece.h — PieceState* is a forward ptr

  ai.h        — AIWeights, Action, AIPickFunc, AIArchetype, ai API
                Includes: types.h, forward-declares BattleState

  run.h       — RunState, run API
                Includes: types.h

Layer 5 (depends on Layer 4):
  registry.h  — All *_template() and *_count() accessors
                Includes: all template headers

  battle.h    — BattleConfig, EventKind, Event, BattleState, all battle API
                Includes: rng.h, run.h, types.h, movegen.h, piece.h,
                          card.h, effect.h, board.h, meta.h, ai.h, recipe.h
                NOTE: No duplicate includes

Layer 6 (umbrella):
  prelude.h   — Includes all headers in Layer 0→5 order
                No duplicates
```

### 23.4 Rules for Headers

1. **Never redefine a type from a lower layer.** Position is in core.h.
   types.h must NOT re-typedef Position.
2. **Forward-declare structs for use in function pointer typedefs.**
   `struct PieceState;` before `MoveGenFunc` typedef.
3. **Forward-declare structs inside struct bodies when only used as pointers.**
   `PieceState *attacker;` inside `EffectCtx` — no need to include piece.h.
4. **No duplicate includes.** battle.h must not list board.h twice.
5. **effect.h does NOT include movegen.h.** EffectCtx has `MoveGen *move_out`
   but MoveGen is a complete type from movegen.h (opaque pointer is fine).
6. **movegen.h does NOT include effect.h.** Uses `EffectArg` from core.h.

### 23.5 Implementation Steps

1. **Delete all existing headers** in `incl/`
2. **Write new headers in DAG order** (Layer 0 → Layer 6)
3. **Update all .c files**: replace all `#include` with single `#include "prelude.h"`
4. **Verify**: `make debug` compiles clean, `./bin/regnum` runs

### 23.6 Key Header Contents

**core.h** (Layer 0): Tier, Kingdom, Side, BattleResult, Position, EffectArgType,
EffectArg. Standalone — no internal deps. Position is defined HERE, not types.h.

**rng.h** (Layer 0): Standalone, `<stdint.h>`, Rng, rng API.

**log.h** (Layer 0): Standalone, `<stdio.h>`, log_info/warn/err.

**defs.h** (Layer 1): `#include "core.h"`, then all `#define`s and id enums.

**types.h** (Layer 2): `#include "defs.h"`, then CardInstance, flags, TargetSpec,
inline helpers. **DOES NOT redefine Position** — gets it from core.h.

**movegen.h** (Layer 3): `#include "defs.h"`, forward-declares `struct PieceState`
and `struct BattleState`, then MoveList, MoveGenFunc, MoveGen, all prototypes.
**DOES NOT include effect.h.**

**effect.h** (Layer 3): `#include "defs.h"`, forward-declares `struct PieceState`,
then EffectTrigger, EffectFunc, Effect, EffectSlot, EffectCtx (with PieceState*
forward ptrs), EffectBus, bus API. **DOES NOT include movegen.h.**

**piece.h** (Layer 4): `#include "effect.h"` + `#include "movegen.h"`,
then PieceTemplate, PieceState (full), piece API.

**card.h** (Layer 4): `#include "types.h"`, forward-declares `struct BattleState`,
then CardTemplate, card API.

**recipe.h** (Layer 4): `#include "defs.h"`, then Recipe, recipe API.

**meta.h** (Layer 4): `#include "defs.h"` + `#include "types.h"`,
forward-declares `struct Effect`, then all meta types.

**board.h** (Layer 4): `#include "types.h"`, forward-declares `struct PieceState`,
then Board struct (with PieceState* pointers), board API.
**DOES NOT include piece.h.**

**ai.h** (Layer 4): `#include "types.h"`, forward-declares `struct BattleState`,
then AIWeights, Action, AIPickFunc, AIArchetype, ai API.

**run.h** (Layer 4): `#include "types.h"`, then RunState, run API.

**registry.h** (Layer 5): includes all template headers, declares all accessors.

**battle.h** (Layer 5): includes in order: rng.h, run.h, types.h, movegen.h,
piece.h, card.h, effect.h, board.h, meta.h, ai.h, recipe.h.
Then BattleConfig, EventKind, Event, BattleState, all battle API.
**NO duplicate includes.**

**prelude.h** (Layer 6): includes all headers in dependency order (Layer 0→5).

### 23.7 .c File Pattern

Every `.c` file starts with:
```c
#include "prelude.h"
```

No individual `#include` statements. No forward declarations. All types and
APIs come through prelude.h.

### 23.8 Verification

1. `make clean && make debug` — zero warnings, zero errors
2. `./bin/regnum` — runs demo, prints trace
3. `wc -L **/*.c **/*.h | sort -n | tail -5` — confirms ≤80 chars
4. No `//` at column 0 inside function bodies
5. No `REGNUM_` prefix on any macro
