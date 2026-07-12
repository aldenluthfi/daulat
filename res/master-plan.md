# Daulat — Full Reimplementation Plan (rev 3)

> On approval: copy this plan verbatim to `res/master-plan.md`, then apply
> the rev 3 rework (Phase 2.5 in §10) before starting Phase 3.

## Rev 3 corrections (user session 2026-07-12, after Phases 0–2 shipped)

1. **Seat model.** Singleplayer: human always plays against the AI. The
   human ALTERNATES between white and black each battle; white always
   moves first. The AI never receives relics or any other altering items
   — its only tools are its cards and pieces. Consequence: "white = the
   player" is wrong everywhere it was assumed. battle.c tracks the
   human's seat (from `run->battles_fought` parity: even = white); the
   run-walk attaches human-scoped effects (relics, chain penalties,
   event rewards, human innates) to the HUMAN seat's list, whichever
   color that is this battle.
2. **effect_fire walks ONE player list.** List membership IS the side
   scoping: `effect_fire(battle, side, trigger, x)` walks the fired
   side's `PlayerState.effects` only, then every live piece's embedded
   effects (pieces self-filter via context). No fired-side getter is
   needed — a Minted Coin in the human's list simply never fires during
   the AI's income query. Effects that must observe the OTHER side's
   events attach to the observed side's list with the beneficiary in
   their context (e.g. Conqueror's Reward attaches its flip observer to
   the enemy list). Battlefield-wide items (modifiers, board traits)
   attach one copy to EACH list.
3. **Effects spawn effects.** Dynamic attachment mid-battle is a core
   pattern, not an exception: a flipping Kewarani splitter or the
   Many-Faced King attaches (for example) a QUERY_METER_DAMAGE_TAKEN
   effect onto the attacked player's list to nullify the pending damage.
   battle_flip therefore stays `void` — no return-the-pick signature is
   needed; meter corrections flow through attached effects, and the
   cascade measures reality via meter_max deltas (§6).
4. **Shop = the in-battle buy menu**, not a meta screen. Once unlocked
   (capstones unlock via overseer defeat), capstone pieces ARE buyable —
   their `tier = TIER_COUNTRY` is real, not a placeholder. The ONLY
   pieces excluded from buying are combination results; battle_buy
   rejects ids that appear as a recipe result.
5. **Sentinel convention.** `*_SENTINEL` entries rename to `*_NONE` and
   sit at the END of the enum, after `*_COUNT` when present:
   `PIECE_NONE`, `CHALLENGE_NONE`, `KINGDOM_NONE`, and new
   `DIFFICULTY_NONE` (EngineState.cleared for a fresh profile). Ordinal
   "none" members whose position carries meaning stay FIRST:
   `MASTERY_NONE` (level ordering), `CHAIN_NONE` (registry slot 0).
6. **Flip trigger naming.** `ON_PIECE_FLIP`/`ON_PIECE_FLIPPED` were
   confusing. Three triggers now: `ON_PIECE_FLIP_PRE` (pre-toggle,
   x = `PieceInfo**`, redirect/consume), `ON_PIECE_FLIP` (fired right
   after the side toggle, x = `PieceInfo*`), `ON_PIECE_FLIP_POST`
   (after the cascade step settles — refill applied, consume side
   effects like Medeq spawns done; x = `PieceInfo*`).
7. **Effect firing is logged, applied fires only.** `struct Effect`
   gains `char* name` and func returns `bool`: true when the effect
   actually applied, false when it self-filtered (walk-all dispatch
   means every trigger match is invoked; only the effect body knows
   whether it acted). effect_fire emits `log effect name="..."
   trigger=...` through protocol_emit only for applied fires, so piped
   sessions show real effect activity without considered-but-filtered
   noise. Trigger names come from a static `TRIGGER_NAME[]` table in
   effect.c. eff_noop returns false.

## Context

Old implementation (95 files, ~13k lines, preserved at git `a816a99`) was
deleted as overbloated. User handwrote lean headers (`incl/`) as the contract
and wants the game reimplemented strictly against them. `res/GDD.md` is the
single source of truth — no invented content. Headers are user-owned: every
delta in §1 was explicitly approved (prior 4-round Q&A + this session's 4
answers). Rev 2 folds in the user's corrections to rev 1:

- **Effect model**: `func(EffectContext* context, void* x)` — `context` is
  per-EffectItem-instance private scratch (independent per item; filled with
  whatever THAT item needs, sometimes nothing); `x` is the actual value being
  queried, mutating in place as it passes through every matching effect. No
  global payload convention. Arg order may be canonized per small group only.
- **Movegen**: exactly 3 generics — `mg_leap`, `mg_slide`, `mg_compound`
  (compound copies other pieces' patterns). Direction/vector args reuse
  `Square`. Special pieces write bespoke fns. No offset/mask registry tables.
- **Rev 2 Q&A**: `mv`/`at` gain `PieceInfo* self` param; spawn =
  heap-copy of Piece template with `kingdom`/`tier`/`class`/`value` fields;
  Board = `PieceInfo*` cells + width/height + `Side` on PieceInfo;
  `effect_fire(battle, side, trigger, x)` walks the fired side's player
  list + every live piece's embedded effects (rev 3 narrowed the walk
  from both lists to one); 5 new triggers (rev 3 renames the flip pair
  and adds a third stage).

Keep from old impl (ideas, not layer count): parameterized movegen
primitives, per-kingdom data files, trigger-driven battle loop, line
protocol. Drop: EffectBus, registry ladders, TLV codec, SDL layer,
16 effect files, 30 headers.

## 0. Code style (mandatory, every file)

- <= 80 chars/line. C23 (`nullptr` like existing files). `make debug`
  (`-Wall -Wextra -Werror`) zero warnings, nothing silenced.
- File header comment only: `//! file.c` + >= 3-line description +
  `Created:` / `Author : Alden Luthfi`.
- `///` doc comment on every fn/struct/enum: >= 2-line description,
  mandatory `Params:` (name: type -> desc) when params exist, mandatory
  `Return:` when non-void, optional `Notes:`.
- `/*---*\ ... \*---*/` full-width section banners between file regions and
  for IMPORTANT FUNC PART emphasis inside long fns.
- NO comments inside function bodies (banners are the only exception).
- No unused variables. Descriptive names, even in macros.
- ALL structs/enums/declarations hoisted to headers; `.c` files hold only
  implementations and variable definitions. Fns with many args: one per
  line, closing `)` on its own line.
- Mimic `src/data_structure/linked_list.c` exactly (include order:
  `<forward.h>` then `<prelude.h>`; extra libc includes above them).

## 1. Header deltas (all approved; apply in Phase 0)

User already applied by hand (do NOT redo): `CARD_COUNT`, Piece
`EFFECT_ITEM_BASE` dedup, PieceInfo flag removal, `Card* hand[]`,
`RunState.cards[]`.

### representation.h

Fixes:
- `#define MAX_BOARD_SIZE (20 * 20)` — parenthesize.
- `MasteryLevel`: prepend `MASTERY_NONE` (fresh profile must encode "no
  mastery"; currently MASTERY_LEVEL_1 = 0 grants everyone mastery 1).

New enums (+ forward.h typedefs):
- `enum Side { SIDE_WHITE, SIDE_BLACK, SIDE_NEUTRAL }` (NEUTRAL:
  Contested Market spawns).
- `enum MoveClass { MOVE_LEAPER, MOVE_SLIDER, MOVE_SPECIAL }` (backs
  Sovereign Banner aura + Sandstorm "sliders max 3"; assignments §5).

`EffectTrigger` insertions (grouped, each doc-commented with its x type):
- `QUERY_CARD_CAN_DRAW` after QUERY_CARD_SELL_COST — x: `bool*`
  (Kingdom Purity, Liberation Trial filter, Counsel discard mark).
- `QUERY_PIECE_MOVES`, `QUERY_PIECE_ATTACKS` after QUERY_PIECE_CAN_ATTACK
  — x: `Square*` list (pawn double-step, Sovereign Banner, Sandstorm,
  traits edit the list in place).
- `QUERY_METER_REFILL` after QUERY_METER_DAMAGE_TAKEN — x: `int*`
  (Glass Cannon halves cascade refill).
- Flip triggers, three stages replacing the old FLIP/FLIPPED pair:
  `ON_PIECE_FLIP_PRE` — x: `PieceInfo**`, pre-toggle; effects may
  redirect the flip (Many-Faced King) or consume it after side effects
  (Kewarani splitters set `*x = nullptr`; a consumed flip skips the
  toggle but the cascade step still settles).
  `ON_PIECE_FLIP` — x: `PieceInfo*`, fired immediately after the side
  toggle; damagers register live during resolve (Soul Shard/Hostage/
  Spite/Hydra/Ronin/Bushido/Conqueror's/Eternal Recursion/Heretic).
  `ON_PIECE_FLIP_POST` — x: `PieceInfo*`, fired after the cascade step
  settles: refill applied and consume side effects (Medeq spawns) done;
  for observers that need the final board/meter state (Last Breath/
  Overflow/Reforge).

COUNT sentinels appended: `MODIFIER_COUNT`, `BOARD_TRAIT_COUNT`,
`EVENT_COUNT`, `CHAIN_PENALTY_COUNT`, `OVERSEER_COUNT`, `CHALLENGE_COUNT`,
`AI_ARCHETYPE_COUNT` (CARD_COUNT already done; PIECE_COUNT exists).
COUNT never doubles as "none". `*_NONE` sentinels sit after their COUNT:
`PIECE_NONE` (renamed from PIECE_SENTINEL), `CHALLENGE_NONE` (no
challenge), `KINGDOM_NONE` (kingdomless pieces: the King), and
`DIFFICULTY_NONE` appended to Difficulty (fresh profile has beaten
nothing; `engine_init` sets `cleared = DIFFICULTY_NONE`, gating logic
compares explicitly). Ordinal "none" members stay first where position
carries meaning: `MASTERY_NONE`, `CHAIN_NONE`. `CardTier` renamed
`UnlockTier` — it gates pieces AND cards by map progression.

`MapNodeID` += `MAP_NODE_OVERSEER`, `MAP_NODE_LIBERATION`.

Struct changes:
```c
struct Board {
    PieceInfo*  piece_board[MAX_BOARD_SIZE];   /* stride 20          */
    BoardTrait* trait;
    int8_t      width;
    int8_t      height;
};  /* cell: nullptr = empty, &VOID_CELL sentinel = missing square */

struct PieceInfo {
    Piece* piece;   /* owned heap copy of the template */
    Square square;
    Side   side;
};  /* heap per live piece; pointer-stable identity; flip = side toggle */

struct Piece {
    Square* (*at)(BattleState* battle_state, PieceInfo* self);
    Square* (*mv)(BattleState* battle_state, PieceInfo* self);

    EFFECT_ITEM_BASE;

    PieceID    id;
    KingdomID  kingdom;  /* KINGDOM_NONE for the King */
    UnlockTier tier;     /* in-battle shop tier; capstones TIER_COUNTRY,
                            buyable once unlocked by overseer defeat;
                            recipe results never buyable regardless */
    MoveClass  class;
    int        value;    /* mutable on the heap copy: Conqueror's,
                            Philosopher's Stone, Overflow, Inherited */
};

struct Effect { /* += */
    bool  (*func)(EffectContext* context, void* x);
                         /* return = whether the effect applied; false
                            on self-filter, gates the log line */
    char* name;          /* shown by the effect-fire log line; set in
                            templates ("Double Step") and at attach */
};

struct Card { /* += */
    KingdomID kingdom;   /* combo chains, Kingdom Purity, climax count */
    int       play_cost; /* 0 = free */
    int       sell_cost;
};

struct KingdomState { /* += */ bool ever_chained; };
/* mastery demands "never chained across the run"; chain ptr returning
   to nullptr after a win erases history without this */

struct BattleState { /* += */ MapNode* node; };
/* battle location: kingdom pricing, trait, modifier, elite/overseer */

struct RunState { /* += */
    EventState     events[EVENT_COUNT];      /* choice_taken, NO_CHOICE
                                                default; one-shot rewards
                                                reset after applying */
    size_t         seed;
    size_t         battles_fought;
    size_t         liberation_at[KINGDOM_COUNT]; /* trial respawn: locked
                                                until battles_fought >=
                                                liberation_at[k] */
    ChallengeRunID challenge;             /* CHALLENGE_NONE = none */
};

struct EngineState { /* += */
    Difficulty   cleared;   /* highest beaten; DIFFICULTY_NONE fresh */
    BattleState* battle;    /* nullptr outside battle */
};
```

Defines: `#define SQUARE_END ((Square) { -1, -1 })` — terminator for
move lists ONLY. Offset/direction arrays fed to mg_leap/mg_slide
terminate with the zero vector `{ 0, 0 }` instead: `{ -1, -1 }` is the
legitimate up-left offset, while no piece has a zero offset. `PIECE_NONE`
terminates PieceID lists for mg_compound.

Generic direction sets (externs from battle.c, declared in
representation.h): `ORTHOGONAL_DIRECTIONS`, `DIAGONAL_DIRECTIONS`,
`ALL_DIRECTIONS` — zero-vector terminated unit vectors used both as
slide directions and as single-step leap offsets (King = leap over
ALL_DIRECTIONS). These are the ONLY named movement tables. Piece-
specific patterns (Pawn steps, Knight leaps, compound part lists) are
never named globals: they appear inline as compound literals
(`(const Square[]) { ... }`, `(const PieceID[]) { ... }`) at the piece's
generator. When mv and at share a pattern, a static `<piece>_gen(battle,
self, threat)` holds the literal once and both call it.

Function declarations + registry externs: full spec in §2.

### data_structure.h
- `void dag_rand(DirectedGraph* graph, size_t vertices, size_t edges,
  size_t seed);` — seed param (currently hardcodes RNG_SEED: every map
  identical).

### protocol.h
- `SCREEN_REGISTRY` loses `const` (Screen.prev/cursor mutate at runtime
  for back-nav).
- `#define PROTOCOL_LINE_MAX 256`, `#define PROTOCOL_MAX_ARGS 16`.
- Decls: `protocol_bind`, `protocol_run`, `protocol_emit`, `arg_value`,
  `arg_long`, `screen_goto` (§2).

### forward.h
- typedefs for `Side`, `MoveClass`.

### prelude.h — untouched. `.c` files needing `string.h`/`ctype.h`/
`time.h` include them locally above `prelude.h`.

REJECTED (do not implement): run-scoped effect LinkedList in RunState.
Battle start walks run state (relics/synergies/events/chain/masteries/
innates) and attaches effects from const registries. `PlayerState.effects`
+ live pieces' embedded arrays are the only live effect stores; all
triggers are battle-scoped. ENTIRE_RUN-duration effects = re-attached each
battle from the walk.

## 2. Declaration set added to representation.h (grouped by .c)

```c
/* effect.c */
Effect* effect_attach(LinkedList* list, const Effect* effect);
void    effect_fire(BattleState* battle, Side side,
                    EffectTrigger trigger, void* x);
void    effect_tick(LinkedList* list);
void    effect_clear(LinkedList* list);
bool    eff_noop(EffectContext* context, void* x);   /* marker body */
Effect* effect_find_mark(LinkedList* list, uintptr_t tag, void* subject);

/* battle.c — lifecycle + actions */
void       battle_begin(EngineState* engine, MapNode* node);
void       battle_free(BattleState* battle);
bool       battle_move(BattleState* battle, Square from, Square to);
bool       battle_buy(BattleState* battle, PieceID id, Square at);
bool       battle_combine(BattleState* battle, Square a, Square b);
bool       battle_play(BattleState* battle, size_t hand, long a, long b);
bool       battle_sell(BattleState* battle, size_t hand);
bool       battle_reclaim(BattleState* battle, Square at);
void       battle_end_turn(BattleState* battle);
/* battle.c — queries + mutation helpers (effects/AI/protocol use) */
Square*    battle_moves(BattleState* battle, PieceInfo* piece);
Square*    battle_attacks(BattleState* battle, PieceInfo* piece);
int        battle_value(BattleState* battle, PieceInfo* piece,
                        PieceInfo* victim);
int        battle_meter_max(BattleState* battle, Side side);
Side       battle_territory(BattleState* battle, Square square);
PieceInfo* battle_at(BattleState* battle, Square square);
bool       battle_in_bounds(BattleState* battle, Square square);
PieceInfo* battle_spawn(BattleState* battle, PieceID id, Square at,
                        Side side);
void       battle_flip(BattleState* battle, PieceInfo* piece);
void       battle_remove(BattleState* battle, PieceInfo* piece);
/* battle.c — subject registers (statics behind getters; set by the
   emission points so effects can self-filter without payload slots) */
BattleState* battle_current(void);
PieceInfo*   battle_subject(void);       /* movegen / pricing / damage */
PieceInfo*   battle_victim(void);        /* damage queries, else NULL  */
Card*        battle_subject_card(void);  /* card queries               */
Square       battle_move_from(void);     /* ON_PIECE_MOVE origin       */
PieceInfo**  battle_damagers(void);      /* NULL-terminated, resolve   */
/* battle.c — movegen kit (3 generics; kingdom files compose these) */
void       mg_begin(void);
void       mg_push(Square square);
Square*    mg_end(void);
void       mg_leap(BattleState* battle, PieceInfo* self,
                   const Square* offsets, bool threat);
void       mg_slide(BattleState* battle, PieceInfo* self,
                    const Square* directions, int8_t min, int8_t max,
                    bool threat);
void       mg_compound(BattleState* battle, PieceInfo* self,
                       const PieceID* parts, bool threat);
extern const PieceInfo VOID_CELL;
extern const Square ORTHOGONAL_DIRECTIONS[];  /* generic dir sets */
extern const Square DIAGONAL_DIRECTIONS[];
extern const Square ALL_DIRECTIONS[];

/* ai.c */
void ai_take_turn(BattleState* battle);
void ai_plan(BattleState* battle);          /* Divination preview */

/* run.c */
size_t rng_mix(size_t seed, size_t salt);
void   run_new(EngineState* engine, size_t seed, Difficulty difficulty,
               ChallengeRunID challenge);
void   run_free(RunState* run);
void   run_enter_map(EngineState* engine, KingdomID kingdom);
bool   run_select_node(EngineState* engine, size_t index);
void   run_battle_result(EngineState* engine, bool won);
void   run_event_choose(EngineState* engine, EventChoice choice);
void   run_offering(EngineState* engine, CardID card);
void   run_relic_pick(EngineState* engine, RelicID relic);
size_t run_pressure(RunState* run, KingdomID kingdom);

/* engine.c */
void engine_init(EngineState* engine);
void engine_free(EngineState* engine);
bool engine_save(EngineState* engine, const char* path);
bool engine_load(EngineState* engine, const char* path);
void engine_finalize_run(EngineState* engine, bool vorath_won);

/* relic.c */
extern const Relic RELIC_REGISTRY[RELIC_COUNT];

/* kingdom/<k>.c exports (x5: longwei kewarani zarqan harushima caelan) */
extern const Piece      LONGWEI_PIECES[];   /* … per kingdom */
extern const Card       LONGWEI_CARDS[];
extern const BoardTrait LONGWEI_TRAITS[];
void longwei_innate(BattleState* battle, Side side, MasteryLevel level);
void longwei_climax(BattleState* battle, Side side);
void longwei_overseer(BattleState* battle);
void longwei_event(EngineState* engine, EventID id, EventChoice choice);

/* kingdom/universal.c */
extern const Piece          UNIVERSAL_PIECES[];  /* King */
extern const Card           UNIVERSAL_CARDS[];   /* 12 + 5 mastery */
extern const BattleModifier MODIFIER_REGISTRY[MODIFIER_COUNT];
extern const ChainPenalty   CHAIN_REGISTRY[CHAIN_PENALTY_COUNT];
extern const Piece* const   PIECE_REGISTRY[PIECE_COUNT];
extern const Card* const    CARD_REGISTRY[CARD_COUNT];
extern const BoardTrait* const TRAIT_REGISTRY[BOARD_TRAIT_COUNT];
extern const KingdomID      KINGDOM_ADJACENT[KINGDOM_COUNT];
extern const char* const    EVENT_NAME[EVENT_COUNT];
extern const char* const    EVENT_TEXT[EVENT_COUNT];
extern const char* const    EVENT_OPTION_A[EVENT_COUNT];
extern const char* const    EVENT_OPTION_B[EVENT_COUNT];
void vorath_setup(BattleState* battle);
/* dispatch tables aggregating the kingdom fns */
extern void (*const KINGDOM_INNATE[KINGDOM_COUNT])(BattleState*, Side,
                                                   MasteryLevel);
extern void (*const KINGDOM_CLIMAX[KINGDOM_COUNT])(BattleState*, Side);
extern void (*const KINGDOM_OVERSEER[KINGDOM_COUNT])(BattleState*);
extern void (*const KINGDOM_EVENT[KINGDOM_COUNT])(EngineState*, EventID,
                                                  EventChoice);
```

protocol.h decls:
```c
void        protocol_bind(Protocol* protocol);  /* static sink for emit */
void        protocol_run(Protocol* protocol);   /* stdio loop */
void        protocol_emit(const char* format, ...);
const char* arg_value(int argc, char** argv, const char* key);
long        arg_long(int argc, char** argv, const char* key,
                     long fallback);
void        screen_goto(EngineState* engine, ScreenID id);
```
`Screen.handle` variadic convention (documented at decl): invoked as
`handle(engine, (int) argc, (char**) argv)`; handlers va_arg exactly that
pair. Screen handler fns are static in screen.c (registry initializer is
the export).

Registry style (approved): fully const. Kingdom arrays exported as
`extern const`; registries are `const T* const` designated-initializer
pointer tables in universal.c. Zero startup mutation. Relic/modifier/chain
registries are direct const arrays in their owning file.

## 3. File inventory

| File | Contents | ~lines |
|---|---|---|
| src/representation/effect.c | attach (heap copy + fresh calloc'd context), fire (walk the FIRED side's list + every board piece's effects[], filtered by trigger, `log effect` line per non-noop fire via TRIGGER_NAME table), tick (TURNS_N decrement through contiguous enum, reap + free context), clear, eff_noop, effect_find_mark | 250 |
| src/representation/battle.c | VOID_CELL def; static battle RNG; HUMAN_SIDE + ACTING_SIDE seat statics; subject registers + getters; scratch + mg trio + push/begin/end; board setup (size per tier ± Extended/Compressed, Dense Terrain voids, kings, pressure/chain/elite/liberation/Traitor's Gambit free pieces); run-walk effect attachment per rev 3 placement rules; spawn (heap Piece copy + per-effect context alloc); actions (move incl. free-move marks + king-swap dests, buy incl. recipe-result rejection + home 40%/foreign +20% pricing + human-seat challenge guards, combine recipe match, play/sell, reclaim); seat-generalized round engine (income, draw pipeline w/ QUERY_CARD_CAN_DRAW + Lucky Strike special case, force-sell, resolve = coverage filtered to enemy-occupied, cascade w/ meter_max-delta accounting, tick, AI halves per seat, turn-10 territory); combo-chain count + climax dispatch; value/meter/territory/at/in_bounds | 980 |
| src/representation/ai.c | ai_take_turn/ai_plan; shared scoring; 5 GDD archetypes + behind-fallbacks (behind = own meter < 50% enemy's) | 420 |
| src/representation/run.c | rng_mix; static layout tables = GDD node lists verbatim (names, types, content ids, elite relic pairs); map_generate (dag_rand + connectivity patch: every node i>0 gets incoming edge); node select/enter per type; unlock schedule (idempotent per-tier); run_battle_result (chains, ever_chained, liberation break/respawn, Vorath counter thresholds incl. forbidding revealed recipes, overseer rewards incl. capstone + synergy flag, Vorath win -> finalize); event dispatch via KINGDOM_EVENT; relic offers; offering/archive | 650 |
| src/representation/engine.c | engine_init/free; text save/load codec (§8); finalize (mastery advance where !ever_chained && vorath won; cleared bump) | 320 |
| src/representation/relic.c | RELIC_REGISTRY: 26 relics, 1–2 static effect bodies each, exactly per GDD | 460 |
| src/representation/kingdom/universal.c | King; 12 universal + 5 mastery-2 cards; 18 modifiers; 4 chain penalties; registry pointer tables + KINGDOM_ADJACENT + EVENT_* strings + dispatch tables; 4 universal events; vorath_setup + Vorath effects (quadrant tally, Grand King reset/overshoot) | 700 |
| src/representation/kingdom/longwei.c | Bing/Xiang/Ma/Pao/Liubo/Sang/N.Cavalry/Hwacha (Ma elbow, Pao/Hwacha screens, Sang legs, Liubo copy-out union); 7 cards; Bulwark innate (M3 60%); climax; Iron Strategist; River Crossing + The Palace; 5 events | 560 |
| src/representation/kingdom/kewarani.c | Medeq/Makwanam/Saba/Faras/Negus Guard/Medeq Squad/Sultan's Levy (Berolina mv/at split, splitter flip-consume, double-move); 7 cards; Double Time innate (pricing rewrite, M3 20%); climax; Caravan of Conquest; Trade Route + Contested Market; 5 events | 520 |
| src/representation/kingdom/zarqan.c | Wazir/Jamal/Talliya/Ziraafa/Shahzadeh/Old King/Cataphract/Rook/War Elephant (Ziraafa bent line, multi-hit at); 8 cards; Royal Substitution innate (M3 twice); climax; Many-Faced King (prince redirect); Sandstorm + Mirage; 5 events | 560 |
| src/representation/kingdom/harushima.c | Fuhyo/Kyosha/Ginsho/Kinsho/Shishi/Honorable Horse/Promoted Bishop/Daimyo/Dragon; 7 cards; Reclaim innate (M3 20 cp); climax; Eternal Recursion (return marks); Fog Coast + Island Chain; 5 events | 560 |
| src/representation/kingdom/caelan.c | Pawn/Knight/Bishop/Queen/Gryphon/Chancellor/Sovereign Banner (double-step effect, Gryphon bent slide, Banner delta-transform aura); 8 cards; Conqueror's Reward innate (M3 60%); climax; Crowned Heretic (ghosts); Castle Corners + Siege Trench; 6 events | 540 |
| src/protocol/protocol.c | bind/run/emit; line read, quote-aware tokenizer, argv dispatch to engine->screen->handle; error line on unknown verb | 230 |
| src/protocol/screen.c | SCREEN_REGISTRY (mutable) + 6 static handlers + emission helpers (board rows, hand, nodes, event panels, Blind Draft/Fog masking) | 620 |
| src/main.c | parse --seed/--save; engine_init; Protocol{engine,stdin,stdout}; protocol_run | 40 |

Total new ≈ 7,350 lines / 15 files (old: 13k / 95). Existing
data_structure/*.c: only dag_rand signature updated.

System→home check: innates/climaxes/overseers/traits/events → kingdom
files; Vorath/modifiers/chains/universal cards/data tables → universal.c;
relics → relic.c; territory, meter 200% overflow clamp, pricing, recipes,
combo chains → battle.c; chains/liberation/unlocks/Vorath counter +
Pressure/archives/offerings/synergies → run.c; masteries → engine.c +
kingdom files; difficulties → run.c/battle.c checks; challenges →
battle.c guards + screen.c masking + daily seed; AI → ai.c.

## 4. Effect conventions

Fire: `bool func(EffectContext* context, void* x)` — returns whether
the effect applied; self-filtered invocations return false and are not
logged.

- `context` — that effect INSTANCE's private heap scratch (16 slots).
  Independent per EffectItem; no cross-item convention. Filled at
  attach/spawn time with what the effect needs, or lazily by the effect
  itself on first fire. Effects mutate their own context freely (pawn
  disables its own double-step).
- `x` — the value the trigger is computing. It starts at the base value
  and mutates in place through every matching effect (e.g.
  QUERY_CARD_SELL_COST: x = `int*` starting at the card's printed sell
  value). x's type is defined per trigger and documented verbatim at the
  EffectTrigger enum. Nothing else is global.

Group canons (small, local — not a universal payload):
- Piece-embedded effects: `args[0]` = owning `PieceInfo*` (set at spawn).
- Player-list effects: `args[0]` = beneficiary `Side` cast through
  uintptr_t (set at attach). Usually the list's own side; for cross-side
  observers it names who profits (Conqueror's Reward in the enemy list
  stores the human's side).
- Mark effects (func = eff_noop): `args[0]` = subject ptr, `args[1]` =
  tag, `args[2..]` = payload; found via effect_find_mark.
- Everything past those slots is that effect's own business.

Side/subject discovery: `effect_fire(battle, side, trigger, x)` walks
the FIRED side's player list only, then every live piece's embedded
effects[]. List membership is the side scoping — no fired-side getter
exists or is needed. Placement rules:
- Human-scoped items (relics, chain penalties, event rewards, human
  innates) attach to the human's seat list, whichever color the human
  plays this battle (§ seat model, rev 3.1).
- Battlefield-wide items (modifiers, board traits) attach one copy to
  EACH list; every copy fires only for its own side.
- Cross-side observers attach to the list of the side whose events they
  watch, beneficiary side in their context (Conqueror's Reward observer
  lives in the ENEMY list: flips fire with side = losing side).
- Piece-embedded effects fire on every walk and self-filter via context
  (`battle_subject() == self`, own side vs subject side, etc.).
Subject registers (`battle_subject()`, `battle_victim()`,
`battle_subject_card()`, `battle_move_from()`, `battle_damagers()`,
`battle_current()`) provide the rest. Effects that need no battle data
use none of it.

Dynamic attachment (core pattern): effects may effect_attach new effects
mid-battle onto either list — a flipping splitter or the Many-Faced King
attaches a QUERY_METER_DAMAGE_TAKEN nullifier onto the attacked player's
list. ll_push appends to the tail, so an effect attached during a firing
of the SAME trigger fires later in that same walk; attachments for other
triggers fire from their next emission. Marks attached during fire are
found by effect_find_mark immediately.

Fire logging: for every effect whose func runs (eff_noop marks excluded),
effect_fire emits `log effect name="<effect->name>" trigger=<name>`
through protocol_emit, using a static `TRIGGER_NAME[]` string table in
effect.c kept in enum order. Templates set `.name` per effect; attach
copies it; marks get their name at attach.

Per-trigger x (reproduced verbatim as the enum's doc comment):

| Trigger | x | base value / notes |
|---|---|---|
| QUERY_CARD_DRAW_COUNT | int* | 3 (Rich 4, Sparse 2, Tactician's 4) |
| QUERY_CARD_PLAY_COST | int* | card play_cost; subject card register |
| QUERY_CARD_SELL_COST | int* | card sell_cost; Devalued/Merchant's/Gilded/Country Seal/Tax Collector |
| QUERY_CARD_CAN_DRAW | bool* | true; Purity/Liberation/Counsel mark |
| QUERY_PIECE_ACTION_COST_MOVE | int* | 1; free-move marks -> 0 |
| QUERY_PIECE_ACTION_COST_BUY | int* | 1; Pawn Storm 0 |
| QUERY_PIECE_ACTION_COST_COMBINE | int* | 1; Alchemist's Kit 0 |
| QUERY_PIECE_CP_COST_BUY | int* | effective value ± home/foreign rule pre-applied; Open Market/events/synergy/Double Time pricing |
| QUERY_PIECE_CP_COST_RECLAIM | int* | 30; M3 20, Tomohito's 15 |
| QUERY_PIECE_CAN_FLIP | bool* | true; subject = candidate (Daimyo once, Citadel, immunity zones, Generals gate) |
| QUERY_PIECE_CAN_MOVE | bool* | true; Citadel, Mingzhu's Seal |
| QUERY_PIECE_CAN_ATTACK | bool* | true; Citadel, Xiang territory rule |
| QUERY_PIECE_MOVES | Square* | list from mv; edit in place (filter = compact, append = insert before SQUARE_END) |
| QUERY_PIECE_ATTACKS | Square* | list from at; same editing rule |
| QUERY_PIECE_DAMAGE_DEALT | int* | subject->piece->value; victim register nullptr = effective-value query (meter/pricing/display) — Veteran's Bond/Iron King act only then; Formation/Bloodletting/Queen's Decree/Forward Command/Fortified Line/Warlord's Banner act only victim != nullptr; value deltas act in both |
| QUERY_PIECE_DAMAGE_TAKEN | int* | post-offense damage; subject = victim (Bulwark, Cathedral, N.Cavalry grant) |
| QUERY_METER_DAMAGE_TAKEN | int* | total resolve damage; side = receiver (Dead Man's Pact intercept, Counter Coup echo, Iron Will/Mirror observers) |
| QUERY_METER_REFILL | int* | battle_meter_max result (Glass Cannon 50%) |
| QUERY_CP_INCOME | int* | 10 (Minted +5, Salt Road +10) |
| ON_PIECE_FLIP_PRE | PieceInfo** | pre-toggle; side = losing side; mutate *x to redirect (Many-Faced), or do side effects + `*x = nullptr` to consume (splitters); nullptr result skips toggle, cascade step still settles |
| ON_PIECE_FLIP | PieceInfo* | right after the side toggle; damagers register live during resolve, nullptr outside |
| ON_PIECE_FLIP_POST | PieceInfo* | after the cascade step settles (refill applied, consume spawns done); final-state observers |
| ON_PIECE_BUY | PieceInfo* | price paid via register-free convention: effects needing it track ON_CARD/QUERY themselves; Bulk Discount counts buys in own context |
| ON_PIECE_MOVE | PieceInfo* | origin via battle_move_from(); Honorable Horse, Fog reveal, Vengeance marks, Fortified Line bookkeeping |
| ON_PIECE_COMBINE | PieceInfo* | result piece; Philosopher's/Inherited mutate value |
| ON_CARD_PLAY | Card* | Tax Stamp, synergy Harushima->Caelan draw; combo counting in battle.c |
| ON_CARD_SELL | Card* | post-sale observers |
| ON_TURN_START | uintptr turn | Bloodthirst, ghosts, Contested Market, Caravan +1/2 turns |
| ON_TURN_END | uintptr turn | War Chest, Eternal Recursion returns |
| ON_BATTLE_START | MapNode* | Queen's Favor cp, Vorath counter +20 meters |
| ON_BATTLE_END | uintptr winner Side | cleanup observers |

Card target encoding (battle_play a/b longs): square = `y*20+x`; piece =
its square; piece type = `PieceID + 1000`; hand index = 0..9; card =
`CardID + 2000`. Each card's doc comment states expected targets.

Marks: bookkeeping effects with `func = eff_noop` in the owner's player
list (MARK_MOVED TURNS_2, MARK_BLOODED, MARK_FREE_MOVE, MARK_SKIP_CARD…),
searched via effect_find_mark. Attach = heap copy into
`PlayerState.effects`; expiry frees effect + context.

Lifetimes: spawn heap-copies the Piece template, then for each non-empty
effect slot allocates a zeroed EffectContext and sets args[0] = self.
battle_remove / battle_free free contexts, piece copy, PieceInfo.
effect_tick runs only on player lists (piece-embedded effects are
ENTIRE_BATTLE by nature); durational card effects targeting pieces live
in player lists with the target in their context.

## 5. Movegen

Three generics, vector args reuse Square. Move lists are SQUARE_END
terminated; offset/direction argument arrays are zero-vector terminated
(see §1 defines note):

- `mg_leap(battle, self, offsets, threat)` — for each offset (dy mirrored
  for SIDE_BLACK; tables written white-perspective, forward = -y):
  destination in bounds, non-void; threat=false (mv): must be EMPTY;
  threat=true (at): must not hold a friendly.
- `mg_slide(battle, self, directions, min, max, threat)` — walk each
  direction from step 1 to max (127 = unbounded); blockers stop the walk
  regardless of min (Talliya min 2 still blocked at distance 1);
  threat=false: emit empty squares at distance >= min; threat=true: emit
  covered squares >= min including the first occupant when not friendly.
- `mg_compound(battle, self, parts, threat)` — parts = PieceID array
  terminated by PIECE_NONE; calls PIECE_REGISTRY[part]->mv/at
  (matching threat) with the SAME self: the copied pattern generates from
  self's square/side. Backs Queen-from-parts pieces (Cataphract,
  Chancellor, Promoted Bishop, Dragon, Medeq Squad, Sultan's Levy, War
  Elephant mv) AND Ambition/Conquest (their effect resets scratch via
  mg_begin then re-generates through the copied piece's registry entry,
  in place, during QUERY_PIECE_MOVES).

Scratch: static `Square SCRATCH[MAX_BOARD_SIZE + 1]` in battle.c;
mg_begin resets cursor, mg_push appends, mg_end writes SQUARE_END and
returns SCRATCH (sentinel is overwritten by the next push, so sequential
generator calls concatenate). Valid until next battle_moves/attacks call
— NO nesting: bespoke fns that need other pieces' lists (Liubo, AI)
copy each battle_attacks result to a local buffer, then mg_begin +
mg_push the union (documented convention at mg_begin's decl).

at() = attack COVERAGE: every square the piece threatens, empty or
enemy-occupied, never friendly-occupied. Resolve filters the list to
enemy-occupied squares for damage; Liubo filters to empty squares for
teleport targets; movement never captures (GDD).

Pipeline (all consumers go through these; raw piece->mv is internal):
```
battle_moves(battle, piece):
  subject register = piece; mg_begin()
  QUERY_PIECE_CAN_MOVE gate (x = &bool) -> empty list when false
  list = piece->mv(battle, piece)
  effect_fire(battle, piece->side, QUERY_PIECE_MOVES, list)
  return list          /* battle_attacks mirrors with at/CAN_ATTACK */
```
Piece fn shape (offsets inline as a compound literal, no named table;
mv/at sharing a pattern factor into a `<piece>_gen(battle, self,
threat)` holding the literal once):
```c
static void knight_gen(BattleState* battle, PieceInfo* self,
                       bool threat) {
    mg_leap(battle, self, (const Square[]) {
        { -1, -2 }, { 1, -2 }, { -2, -1 }, { 2, -1 },
        { -2,  1 }, { 2,  1 }, { -1,  2 }, { 1,  2 },
        {  0,  0 },
    }, threat);
}
Square* knight_mv(BattleState* battle, PieceInfo* self) {
    knight_gen(battle, self, false);
    return mg_end();
}
```

Piece table (mv; at = same fn unless noted; X = bespoke fn; class L/SL/S
= MOVE_LEAPER/MOVE_SLIDER/MOVE_SPECIAL):

| Piece | Pattern | Class |
|---|---|---|
| KING | leap 8-king | L |
| BING | leap {0,-1}; resolve observer sets MARK_BLOODED -> moves-effect adds {±1,0} | L |
| XIANG | leap {±2,±2} (GDD gives no block clause); can-attack false in enemy territory | L |
| MA | X blockable knight, orthogonal elbow blocks | S |
| PAO | mv slide orth 1..127; X at = exactly-one-screen coverage per line (empties behind screen included) | SL |
| LIUBO_DIVINER | X mv = union of enemy battle_attacks, empty squares only, copy-out + dedup; at = leap 8-king (interpretation) | S |
| SANG | X 1 orth + 2 diag chained, both intermediates empty | S |
| NORTHERN_CAVALRY | leap knight-8; embedded QUERY_PIECE_DAMAGE_TAKEN effect: orth-adjacent allies -50% | L |
| HWACHA | X Pao rule on all 8 directions | SL |
| FUHYO | leap {0,-1} | L |
| KYOSHA | slide {0,-1} 1..127 | SL |
| GINSHO | leap {0,-1},{±1,-1},{±1,1} | L |
| KINSHO | leap gold set | L |
| SHISHI | leap 8-king + embedded free-move-per-move-action effect (double act) | L |
| HONORABLE_HORSE | leap {±1,-2}; ON_PIECE_MOVE into enemy territory sets context flag -> moves-effect regenerates as gold set | L |
| PROMOTED_BISHOP | compound {BISHOP} + leap orth-1 | SL |
| DAIMYO | leap 8-king; embedded can-flip once-false (context counter) | L |
| DRAGON | compound {ROOK} + leap diag-1 | SL |
| MEDEQ | mv leap {±1,-1}; at leap {0,-1} (Berolina) | L |
| MAKWANAM | leap {±1,±1} | L |
| SABA | leap {±2,±2} | L |
| FARAS | leap {0,±2},{±2,0} | L |
| NEGUS_GUARD | leap 8-king + free-move effect; ON_PIECE_FLIP consume -> remove + 2 Medeq adjacent to king | L |
| MEDEQ_SQUAD | compound {MEDEQ} (mv+at); consume -> 2 Medeq adjacent to own square | L |
| SULTANS_LEVY | leap 8-king + free-move effect; consume -> 3 Medeq adjacent to king | L |
| WAZIR | leap {0,±1},{±1,0} | L |
| JAMAL | leap (1,3)-set | L |
| TALLIYA | slide diag min 2 | SL |
| ZIRAAFA | X 1 diag then straight >= 3 outward, intermediates empty | S |
| SHAHZADEH | leap 8-king; embedded moves-effect appends own king's square = swap (once per battle, context counter); battle_move detects king-square dest -> swap; at = king set | L |
| OLD_KING | leap 8-king | L |
| CATAPHRACT | compound {KNIGHT, JAMAL} | L |
| ROOK | slide orth 1..127 | SL |
| WAR_ELEPHANT | mv compound {ZIRAAFA}; X at = all adjacent squares (multi-hit coverage) | S |
| PAWN | mv leap {0,-1} + embedded double-step effect (context: spawn square + active flag; square != spawn -> flag off forever; flag on -> append forward-2 when both squares empty); X at leap {±1,-1} | L |
| KNIGHT | leap knight-8 | L |
| BISHOP | slide diag 1..127 | SL |
| QUEEN | slide 8 dirs 1..127 | SL |
| GRYPHON | X 1 diag step then orth slide outward | S |
| CHANCELLOR | compound {ROOK, KNIGHT} | SL |
| SOVEREIGN_BANNER | compound {QUEEN}; embedded aura on QUERY_PIECE_MOVES (below) | SL |

Sovereign Banner aura (delta-transform — no offset tables exist to read):
fires on every QUERY_PIECE_MOVES; if subject is an adjacent ally:
MOVE_SLIDER/MOVE_SPECIAL untouched; MOVE_LEAPER: for each list entry,
delta = entry - subject square; all |components| <= 1 (stepper) -> append
entry + delta if empty/valid (blocked extension stays blocked since entry
had to be reachable); else (leaper) grow the larger |component| by 1 keep
sign -> append subject square + new delta if empty/valid.

Sandstorm: QUERY_PIECE_MOVES effect; even turns, subject class ==
MOVE_SLIDER -> remove entries with Chebyshev(delta) > 3. Bent movers are
MOVE_SPECIAL, exempt (flagged §11).

## 6. Battle pipeline (trigger emission points)

Seats: `HUMAN_SIDE` static in battle.c, set at battle_begin from
`run->battles_fought` parity (even = SIDE_WHITE). White always moves
first regardless of who sits there. `ACTING_SIDE` static tracks whose
actions are legal; battle_move/buy/etc. act for ACTING_SIDE. run->pieces
unlock gating and challenge guards apply to the HUMAN seat only; the AI
buys from its kingdom's arsenal freely. State emission, `result won=`,
and run_battle_result are all relative to HUMAN_SIDE.

```
battle_begin(engine, node):
  HUMAN_SIDE = battles_fought % 2 ? BLACK : WHITE
  board size per tier (± Extended/Compressed), Dense Terrain voids,
  kings, free enemy pieces (pressure + silver chain + elite + node type)
  run-walk attach, all to the HUMAN seat's list: relics, chain penalty,
    synergies (node->kingdom == KINGDOM_ADJACENT[cleared]), human innate
    (per progress/mastery), pending event rewards, challenge
  AI seat's list: overseer setup via KINGDOM_OVERSEER / vorath_setup,
    enemy innate per Enslaved, Vorath-counter effects
  both lists (one copy each): modifier, trait
  meters = battle_meter_max (victim-nullptr value queries); cp = 20 ±
    modifier/chain/event deltas (their ON_BATTLE_START bodies mutate cp)
  ON_BATTLE_START (white) then (black)
  turn_start(white); white == AI -> ai_take_turn + white half_turn,
    then turn_start(black)   /* human always ends up in control */

turn_start(side):
  actions = 3; income = 10 -> QUERY_CP_INCOME -> cp += income
  ON_TURN_START
  n = 3 -> QUERY_CARD_DRAW_COUNT; sample n from run->cards gated per
    card by QUERY_CARD_CAN_DRAW (subject card register); Lucky Strike =
    battle.c special case (highest tier first, flagged §11); AI hand
    drawn from its kingdom's cards

actions (human via protocol; AI via ai_take_turn; both for ACTING_SIDE):
  move:    battle_moves -> dest in list -> QUERY_PIECE_ACTION_COST_MOVE
           (0 if MARK_FREE_MOVE, consume mark) -> apply ->
           ON_PIECE_MOVE (origin in register); king-square dest for
           Shahzadeh/Royal Substitution = swap
  buy:     recipe-result ids rejected; HUMAN seat: run->pieces +
           challenge guards -> battle_value(victim nullptr) ->
           home/foreign rule -> QUERY_PIECE_CP_COST_BUY ->
           QUERY_PIECE_ACTION_COST_BUY -> battle_spawn -> ON_PIECE_BUY
  combine: recipe match + result unlocked -> ACTION_COST_COMBINE ->
           remove parents, spawn result at second square ->
           ON_PIECE_COMBINE
  play:    QUERY_CARD_PLAY_COST -> pay -> card's effects: immediate ones
           run now, durational ones attach -> ON_CARD_PLAY ->
           same-kingdom count: 2nd -> +15 cp, 3rd -> KINGDOM_CLIMAX
  sell:    QUERY_CARD_SELL_COST -> cp += value -> ON_CARD_SELL

half_turn(side):                 /* one side's end-of-turn settlement */
  force-sell remaining hand; ON_TURN_END (War Chest, Eternal Recursion)
  resolve: for each of side's attackers with QUERY_PIECE_CAN_ATTACK:
    coverage = battle_attacks(attacker)
    for each covered square holding enemy e:
      d = QUERY_PIECE_DAMAGE_DEALT (subject attacker, victim e)
      d = QUERY_PIECE_DAMAGE_TAKEN (subject e, attacker in register)
      total += d; record attacker in damagers
  QUERY_METER_DAMAGE_TAKEN (side = receiver); meter -= total
  cascade while receiver meter <= 0:
    deficit = -meter
    candidates = receiver non-king where QUERY_PIECE_CAN_FLIP
    none + no non-kings -> king flips -> battle over
    none + non-kings all immune -> meter = 0, break (Last Stand)
    pick = Mercy mark if candidate else seeded random
           (Bloodbath: two picks)
    gainer_max_before = battle_meter_max(gaining side)
    battle_flip(pick): ON_PIECE_FLIP_PRE (x = &pick) -> redirect/
      consume -> if pick: side toggle, ON_PIECE_FLIP
    refill = battle_meter_max(receiver) -> QUERY_METER_REFILL
    meter = refill - deficit
    gaining side meter += battle_meter_max(gaining side) -
      gainer_max_before        /* measures what ACTUALLY changed:
                                  redirected piece, consume spawns, or
                                  nothing — no flip return value needed */
    clamp both meters <= 2 * own meter_max
    ON_PIECE_FLIP_POST (x = flipped piece) when a toggle happened
  effect_tick(side's list)

battle_end_turn(battle):         /* human pressed end */
  human == WHITE:
    half_turn(white); over -> finish
    turn_start(black); ai_take_turn; half_turn(black); over -> finish
    turn++; turn > 10 -> territory finish
    turn_start(white)
  human == BLACK:                /* white half already ran this round */
    half_turn(black); over -> finish
    turn++; turn > 10 -> territory finish
    turn_start(white); ai_take_turn; half_turn(white); over -> finish
    turn_start(black)
  territory finish: count squares by battle_territory (Chebyshev), more
    squares wins, tie = human loss
end: ON_BATTLE_END, effect_clear both + free pieces, run_battle_result
```

Order note when human plays black: rounds are white-first as always;
battle_begin plays the AI's opening white turn before the human's first
black turn, and each subsequent `end` settles black, advances the turn,
then plays the AI's next white turn before returning control.

GDD resolve order note: defense on QUERY_PIECE_DAMAGE_TAKEN, offense on
QUERY_PIECE_DAMAGE_DEALT; multiplicative order commutes, rounding =
floor except Conqueror's explicit round-up.

## 7. Campaign

Layout tables (run.c static): per (kingdom, tier), rows in GDD order —
`{ MapNodeID type; const char* name; int content; }` (content = EventID /
archive result PieceID / packed elite RelicID pair / OverseerTypeID).
Node index == layout row index, so MapNode needs no name/content fields —
run.c resolves via (kingdom, tier, index).

DAG: `dag_rand(&graph, n, n + n/2, map_seed)` + patch pass (every node
i>0 gets incoming edge `i-1 -> i` if missing). `DGNode.data` = heap
MapNode. Selectable = node 0 or successor of cleared. Overseer row last =
highest index. Modifier per node = `rng_mix(map_seed, index)` draw; trait
on 50% of in-kingdom battles; Surveyor's Map pre-reveals one battle
node's modifier per map.

Progression: all 15 maps generated at run_new (deterministic from seed).
Active map per kingdom = lowest uncleared tier. run_enter_map idempotent
unlocks per GDD schedule (town piece/cards, province + innate, country
cards, capstone on overseer defeat; universal tier cards on first entry
of any kingdom). Vorath battle = static MapNode in run.c, enabled when
all 5 overseers dead. Liberation: Gold chain spawns virtual
MAP_NODE_LIBERATION in adjacent kingdom's map; win breaks Gold; loss
sets `liberation_at[k] = battles_fought + 3`.

Seeds: run->seed from `new seed=`, else RNG_SEED; Daily Conquest =
YYYYMMDD from time(NULL). map_seed = rng_mix(seed, kingdom*3 + tier);
battle stream = rng_mix(seed, battles_fought + 1). rng_mix =
splitmix-style hash over rand_r.

Vorath counter: +1 per loss; each upward crossing of 2 -> enemy meters
+20 baseline (battle-start effect); of 4 -> forbid random revealed recipe
(run->pieces[result] = false). Events decrement; only upward crossings
fire.

## 8. Protocol grammar

Input: one command/line, `verb [key=value|positional]...`, whitespace
split, `"` quoting, `#` comments. Unknown/bad -> `error msg=...`, no
state change. Success ends `ok`; screen change emits `screen <name>` +
state dump. `quit` everywhere -> `bye`.

| Screen | Commands |
|---|---|
| title | `new [seed=N] [difficulty=0..3] [challenge=0..5]`, `load`, `settings`, `quit` |
| campaign | `kingdoms` (list: id/name/mastery/chain/tier), `enter id=K`, `vorath` (when eligible), `codex`, `settings`, `save`, `title` |
| codex | `pieces`/`cards`/`relics`, `info piece=N\|card=N\|relic=N`, `back` |
| map | `nodes` (i/type/name/revealed/cleared/selectable/modifier), `select i=N` — battle-likes -> `screen battle`; event -> panel + `choose a\|b`; offering -> `remove card=N`; archive -> auto `log recipe=...`; relic offers -> `offer a=N b=N` + `relic id=N`; `back` |
| battle | `state`, `board` (`row y=0 cells=....X..k..`, `.` empty `X` void, letters per piece, uppercase white, Fog masks values), `hand` (Blind Draft masks), `moves x= y=`, `attacks x= y=`, `move fx= fy= tx= ty=`, `buy piece= x= y=`, `combine ax= ay= bx= by=`, `play i= [a=] [b=]`, `sell i=`, `reclaim x= y=`, `end` (resolve + AI, `log` lines, next state or `result won=0\|1`), `concede` |
| settings | `show`, `reset run`, `reset all`, `back` (Screen.prev) |

`log ...` = free-form narration (flips, cascades, climaxes, AI actions,
Divination via ai_plan).

## 9. Save format (text, path ./daulat.sav, --save overrides)

```
daulat version=1
masteries=0,1,0,0,2
cleared=1                  # DIFFICULTY_NONE = nothing beaten yet
run=1                      # 0 => nothing below read
seed=3735928559
difficulty=1
challenge=7                # CHALLENGE_NONE = none
vorath=3
battles=14
relics=00110100...         # RELIC_COUNT bits
pieces=1111100...          # PIECE_COUNT bits
cards=111011...            # CARD_COUNT bits
synergies=01000
chains=-1,0,2,-1,-1        # -1 none, else ChainPenaltyID
ever=0,1,1,0,0
liberation=0,0,17,0,0
map k=0 t=0 cleared=11111011000 revealed=11111111100
...                        # 15 lines, bit per node in layout order
event i=27 choice=1        # only non-NO_CHOICE written
```

Load: rebuild 15 maps from seed, apply bits + events. Missing file =
fresh profile. Saving only outside SCREEN_BATTLE (battles never saved).

## 10. Phases (gate: `make debug` zero warnings + piped session)

0. **Headers + stubs.** DONE 2026-07-12. Apply §1 deltas + §2
   declarations; stub all functions; empty registries; dag_rand seed
   param. Verified: clean build; `printf 'quit\n' | bin/daulat` ->
   `screen title` `bye`.
1. **Effect core + board + movegen.** DONE 2026-07-12. effect.c;
   battle.c setup + subject registers + mg trio + moves/attacks +
   territory; universal.c King + registries; caelan.c pieces. Verified:
   king shows 8 moves, pawn double-step appears then disappears after
   moving, bishop slide blocked correctly (24-check scratch harness).
2. **Vertical slice.** DONE 2026-07-12 (against the rev 2 white=human
   assumption — reworked in 2.5). Actions, resolve cascade, meters,
   turn loop; Hammer AI; protocol.c + title/battle screens; `new` ->
   synthetic battle. Verified: piped session buys/moves/ends, damage +
   flip cascade logs, result by king flip.
2.5. **Rev 3 rework (before Phase 3).**
   - Header: rename ON_PIECE_FLIP/ON_PIECE_FLIPPED to the PRE/FLIP/POST
     trio; PIECE_SENTINEL -> PIECE_NONE (CHALLENGE/KINGDOM_NONE renames
     from the SENTINEL interim names); append DIFFICULTY_NONE; add
     `char* name` to Effect; update EffectTrigger doc table.
   - effect.c: fire walks the fired side's list only; add TRIGGER_NAME
     table + `log effect` emission (skip eff_noop).
   - battle.c: HUMAN_SIDE from battles_fought parity; generalize round
     engine per §6 (human-black order incl. AI opening turn); buy =
     recipe-result rejection + human-seat-only unlock gating; cascade
     meter_max-delta accounting for the gaining side; ON_PIECE_FLIP_POST
     emission; engine_init cleared = DIFFICULTY_NONE.
   - caelan.c/universal.c: name every template effect; screen.c: state
     emission + result relative to HUMAN_SIDE.
   - Verify: seeded session as white AND as black (battles_fought
     parity forced via two-battle script once campaign exists; until
     then a temporary `new` seat override or direct parity check);
     `log effect` lines appear for double-step and income once named
     effects exist; regression: Phase 1 harness + Phase 2 piped script.
3. **Cards + combos.** DONE 2026-07-12. Universal + Caelan cards, hand
   pipeline, combo refund + climax, marks. Verified: 24-check card
   harness (climax +50%, refund +15, Revitalize/Sacrifice/Coronation/
   Divine-Right/Chain-Break/Last-Stand/Hostage); piped seed=2 session
   (QG mid-turn draw, combo refund line, force-sell autosells). Design:
   battle_play is generic — a card's effects[] slot with trigger
   ON_CARD_PLAY runs immediately (temp context, x = Card*), any other
   trigger attaches to the acting player's list (args[0] = beneficiary
   Side, args[1..2] = encoded a/b targets); eff_noop slots become marks
   (args[1] = card->id tag, args[2] = payload). battle.c reads three
   marks inline (no header change, tag = CardID): Pawn Storm and Reforge
   in battle_buy (id-filtered pricing/action), and Last Stand routes
   through the existing all-immune cascade branch via a
   QUERY_PIECE_CAN_FLIP veto (no cascade coupling). Mercy/Spite/Hydra/
   Reforge observe own-side losses through ON_PIECE_FLIP_PRE (pre-toggle
   fires on the losing side's own list); Hostage/climax via own-list
   fires. Queen's Gambit's 3-card draw is battle.c-side (needs engine);
   only the human draws a hand (AI plays no cards this build). run_new
   unlocks the 5 implemented District cards; battle_draw null-guards
   CARD_REGISTRY so Phase 5 kingdom cards light up automatically.
4. **Campaign + save.** DONE 2026-07-12. run.c 15 verbatim GDD layout
   tables + map generation (dag_rand + i-1->i patch, heap MapNode per
   vertex, per-battle modifier via rng_mix), node select/enter dispatch,
   idempotent tier unlock schedule, chain add/remove, overseer capstone
   reward + synergy, event dispatch (choice recorded, bodies deferred),
   offering, archive reveal, relic offer, Vorath eligibility + finalize
   wiring; engine.c text save/load codec + finalize (mastery advance
   where !ever_chained, cleared bump); screen.c campaign/map/codex/
   settings handlers; battle end returns to map via battle_finish ->
   run_battle_result. Header adds (user-approved): MapNode gains
   name/content fields — the static layout tables are MapNode arrays
   (designated init), no separate row struct; run_emit_kingdoms /
   run_emit_map / run_enter_vorath, battle_concede. Verified: 18-check
   campaign harness (unlock schedule,
   archive/offering/event dispatch, town-clear -> province unlock);
   piped select->battle->concede returns to map; save -> load ->
   identical `nodes` across sessions; chain 0->1 on loss; regressions
   (phase1 24, phase3 24, black-seat 8) green; 80-col clean. Design
   notes: `new` now enters the campaign screen (synthetic-battle path
   retired); battle START transition done by screen.c on seeing
   engine->battle, battle END transition done by run.c (called from
   battle_finish); MapNode self-describes — name/content authored in the
   static MapNode layout tables and copied in at generation (map_generate
   does *node = LAYOUTS[k][t][i] then sets runtime fields), so no
   reverse index lookup. DEFERRED to their phases: liberation node
   injection +
   Liberation Trial, Vorath-counter threshold battle effects, pressure
   scaling, difficulty/challenge battle effects (all Phase 6); event
   effect bodies (kingdom phases); overseer + Vorath battle armies
   (Phase 7). run_battle_result increments the Vorath counter and forbids
   a recipe every 4 losses (state only); board sizes by tier
   (town 12 / province 14 / country 16); campaign battles are king-only
   until pressure/free-piece setup lands (Phase 6).
5. **Remaining kingdoms.** 4 kingdom files complete + recipes + combine.
   Verify: Pao screen capture, Ma elbow block, Berolina split, Ziraafa
   bent line via `moves`/`attacks` spot-checks.
6. **Run-scale systems.** relic.c bodies; modifiers; synergies;
   liberation; Vorath counter; pressure; difficulties; masteries + M2
   cards + M3 params; challenges. Verify: two scripted losses -> bronze
   chain in `kingdoms`; Offering removal reflected in seeded draws.
7. **Overseers + Vorath + AI.** 5 overseer setups, vorath_setup, 4
   remaining archetypes + fallbacks, ai_plan/Divination. Verify:
   overseer entry shows bespoke army; full seeded run end-to-end.

## 11. Flagged interpretations (GDD silent; coded as stated, easy to flip)

Liubo threat (its at) = king-adjacent; at() = coverage incl. empty
squares (needed by Liubo + resolve filters occupied); Shishi =
free-second-move entitlement per move action; Xiang leap unblockable;
turn-10 territory tie = player loss; all-immune / Last Stand cascade ->
meter clamps 0, no flip; floor rounding (Conqueror's rounds up);
Sovereign Banner delta-transform (stepper = all offsets Chebyshev 1;
MOVE_SPECIAL untouched); Sandstorm hits MOVE_SLIDER only — bent movers
(Ziraafa/Gryphon/War Elephant) are MOVE_SPECIAL and exempt (Ziraafa min
leg 4 would otherwise freeze on even turns); Lucky Strike = draw-code
special case (sampling bias, not a per-card filter); overseer "+2 cards"
= re-add up to 2 Offering-removed kingdom cards; Iron Strategist split =
boss meter damage / boss piece count, Generals = value-10 markers
needing 3 consecutive adjacent-attack turns; Crowned Heretic flips
become ghosts (never join player), meter = army sum; Vorath quadrants =
four 10x10 corners, center priority; elite offer holding-collision ->
lowest unheld relic substitutes; Kingdom Purity pads with universal when
short; Clockwork soft-enforced (auto-end past 30 s); event elites count
as normal losses; Queen's Favor lasts current map; pawn double-step =
spawn-square compare + self-clearing flag (movegen previews cannot burn
it). Phase 3 card interpretations: card-dealt meter damage
(Spite/Vengeance/Crusade/Counter Coup echo) lowers the enemy meter
without an immediate cascade — flips resolve on the next cascade (no
public battle-scoped damage helper); Vengeance approximates "moved
adjacent last turn" as "currently adjacent to a friendly piece" (no
per-turn move history yet); Crusade resolves its Knight's three attacks
as value x3 to the enemy meter; Reforge's "next turn" discount is a mark
active until used (TURNS_2 approximation of the turn boundary); draws
sample with replacement (duplicate cards in a hand allowed).

## 12. Risks

Per-trigger x semantics live in doc comments — mitigated: §4 table
verbatim at the enum. Shared movegen scratch forbids nesting — Liubo/AI
copy out; convention documented at mg_begin. Subject registers are
hidden state — set/cleared only inside battle.c emission points, getters
documented. Single-list walk means a misplaced attachment silently
never fires — mitigated: §4 placement rules + `log effect` lines make
missing fires visible in piped sessions. Dynamic attachment during a
walk appends to the tail — same-trigger attachments fire in the same
walk; documented at effect_attach. MAX_EFFECT_COUNT 8 sufficient (max
audited need 3). Variadic Screen.handle safe only with uniform
(int, char**) extraction — shared prologue macro. Overseer bespoke
logic largest complexity — isolated in kingdom files.
