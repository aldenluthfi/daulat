# Regnum — Boilerplate Expansion Guide

**Created**: 2026-06-14
**Purpose**: Step-by-step instructions for adding new content to the Regnum
boilerplate without editing engine code. Every addition is a data record;
engine behavior is dispatched through function pointers.

---

## Conventions

- All new files go in `src/data/` for templates or `src/movegens/` /
  `src/effects/` for function implementations.
- Every `.c` file starts with `#include "prelude.h"`.
- New enum values go in `incl/defs.h` before `*_COUNT`.
- All identifiers use underscores, no project prefix.
- Every function has a `//!` header block with description, params, and
  return.
- Line length ≤ 80 chars.

---

## 1. Adding a New Piece

**Files to touch**: `incl/defs.h`, `src/data/data_<kingdom>.c`,
`src/movegens/mg_basics.c` or `mg_<kingdom>.c`, `src/registry.c`

### Step 1 — Declare the piece id

In `incl/defs.h`, add the new `PIECE_*` value to the appropriate kingdom
group, before `PIECE_ID_COUNT`:

```c
typedef enum {
    // ...
    PIECE_WAZIR, PIECE_JAMAL, PIECE_TALLIYA,
    PIECE_ZIRAAFA, PIECE_SHAHZADEH, PIECE_MY_NEW_PIECE, /* add here */
    PIECE_ID_COUNT
} PieceId;
```

### Step 2 — Implement or reuse a MoveGen

**If the piece uses existing primitives**: In the kingdom's `data_<kingdom>.c`,
reference the existing `MoveGenFunc`:

```c
const PieceTemplate PIECES_ZARQAN[] = {
    // ...
    [PIECE_MY_NEW_PIECE] =
        {
            .id = PIECE_MY_NEW_PIECE,
            .name = "My New Piece",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_TOWN,
            .base_value = 3,
            .move = {
                .func = mg_slide_dirs,
                .params = {
                    {.type = EARG_INT, .v.i = DIRMask(DIR_N | DIR_S)},
                    {.type = EARG_INT, .v.i = 1},
                    {.type = EARG_INT, .v.i = UNBOUNDED}
                },
                .param_count = 3,
            },
            .passives = {},
            .passive_count = 0,
        },
};
```

**If the piece needs a new movement pattern**: Add a new function to the
appropriate `mg_<kingdom>.c` file:

```c
/// mg_zq_my_pattern
///
/// [description of the movement].
void
mg_zq_my_pattern(const PieceState *piece, const BattleState *bs,
                 const EffectArg *params, size_t n, MoveList *out)
{
    // implement movement logic
}
```

Declare the prototype in `incl/movegen.h` under the Kewarani section.

### Step 3 — Add passive effects

In the `passives[]` array of the `PieceTemplate`, add one or more effects:

```c
.passives = {
    [0] = {
        .trigger = TRIGGER_RESOLVE_DEFENSE,
        .apply = eff_bulwark,
        .args = {
            {.type = EARG_INT, .v.i = 2}
        },
        .arg_count = 1,
        .duration_turns = 0,
        .owner = SIDE_NEUTRAL,
        .source_id = 0,
    },
},
.passive_count = 1,
```

### Step 4 — Register in the recipe table (if combo result)

If the piece is a combination result, add an entry to `src/data/data_recipes.c`:

```c
[RECIPE_MY_NEW_PIECE] =
    {
        .ingredient_a = PIECE_KYOSHA,
        .ingredient_b = PIECE_WAZIR,
        .result = PIECE_MY_NEW_PIECE,
        .steps = 1,
    },
```

Add the `RECIPE_*` id to `incl/defs.h` and bump `RECIPES_COUNT`.

### Step 5 — Update registry bounds

In `src/registry.c`, verify the kingdom range in `piece_template()` still
covers the new id. If the new piece is at the end of a kingdom range, no
change is needed.

---

## 2. Adding a New Card

**Files to touch**: `incl/defs.h`, `src/data/data_<kingdom>.c`,
`src/effects/` (new or existing), `src/registry.c`

### Step 1 — Declare the card id

In `incl/defs.h`, add the new `CARD_*` value to the appropriate kingdom
group:

```c
typedef enum {
    // ...
    CARD_COUNSEL, CARD_PILLAGE, CARD_MY_NEW_CARD, /* add here */
    CARD_CONQUEST,
    CARD_ID_COUNT
} CardId;
```

### Step 2 — Implement on_play effects

**If using an existing effect function**: Reference it in the card template.

**If the card needs new behavior**: Add a new `EffectFunc` to the
appropriate `src/effects/eff_*.c` file:

```c
/// eff_my_new_effect
///
/// [one-line description].
void
eff_my_new_effect(EffectCtx *ctx, const EffectArg *args, size_t n)
{
    BattleState *bs = ctx->bs;
    // implement effect logic
}
```

Declare the prototype in `incl/effect.h` under the appropriate section.

### Step 3 — Declare the card template

In the kingdom's `data_<kingdom>.c`:

```c
const CardTemplate CARDS_ZARQAN[] = {
    // ...
    [CARD_MY_NEW_CARD] =
        {
            .id = CARD_MY_NEW_CARD,
            .name = "My New Card",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_TOWN,
            .play_cost = 4,
            .sell_value = 2,
            .on_play = {
                [0] = {
                    .trigger = TRIGGER_CARD_PLAYED,
                    .apply = eff_my_new_effect,
                    .args = {},
                    .arg_count = 0,
                    .duration_turns = 0,
                    .owner = SIDE_NEUTRAL,
                    .source_id = 0,
                },
            },
            .play_effect_count = 1,
            .on_sell = {},
            .sell_effect_count = 0,
        },
};
```

### Step 4 — Update registry bounds

In `src/registry.c`, verify the kingdom range in `card_template()` covers
the new id. If the new card is at the end of a kingdom range, no change
is needed.

---

## 3. Adding a New Effect Function

**Files to touch**: `src/effects/eff_<category>.c`, `incl/effect.h`

### Step 1 — Choose the file

| File | Handles |
|------|---------|
| `eff_meter.c` | Meter adjustments, overflow, cap |
| `eff_economy.c` | CP adjustments, cost/sell modifiers, income |
| `eff_damage.c` | Damage multipliers, reductions, immunities |
| `eff_movement.c` | Movegen swaps, extra steps, free moves |
| `eff_piece.c` | Spawn, remove, swap, force flip |
| `eff_card.c` | Draw extra, skip, peek, target removal |
| `eff_flip.c` | On-flip handlers, splitter spawns |
| `eff_run.c` | Relic / chain / synergy hooks |

If the effect doesn't fit an existing file, create `eff_<name>.c` with the
appropriate content and add it to the Makefile's `EFFECT_SRCS`.

### Step 2 — Implement the function

```c
/// eff_my_effect
///
/// [full description of behavior].
void
eff_my_effect(EffectCtx *ctx, const EffectArg *args, size_t n)
{
    // Access the BattleState
    BattleState *bs = ctx->bs;

    // Access per-trigger context data
    if (ctx->trigger == TRIGGER_RESOLVE_DEFENSE) {
        const PieceState *attacker = ctx->attacker;
        const PieceState *defender = ctx->defender;
        int *damage_mult = &ctx->out_damage_mult;
        // modify *damage_mult
    }

    // Access typed arguments
    // if (n > 0 && args[0].type == EARG_INT) { int x = args[0].v.i; }
}
```

### Step 3 — Declare the prototype

In `incl/effect.h`, add the declaration under the `/* effect function prototypes */`
section.

---

## 4. Adding a New MoveGen Primitive

**Files to touch**: `src/movegens/mg_basics.c` or `mg_<kingdom>.c`,
`incl/movegen.h`

### When to add a new primitive

Add a new primitive in `mg_basics.c` only when:
1. The pattern is reusable across multiple kingdoms, AND
2. It cannot be expressed as a parameterization of an existing primitive.

Otherwise, add it to `mg_<kingdom>.c` for the kingdom that uses it.

### Implementation pattern

```c
/// mg_my_pattern
///
/// [description]. Used by [piece list].
///
void
mg_my_pattern(const PieceState *piece, const BattleState *bs,
              const EffectArg *params, size_t n, MoveList *out)
{
    out->count = 0;
    // Walk the board, check legality (no self, in bounds, not blocked),
    // add to out->squares[]
}
```

Key helpers available in all movegens:
- `pos_in_bounds(bs, pos)` — check board bounds
- `battle_piece_at(bs, pos)` — get piece at position (NULL = empty)
- `piece->owner` — owner of moving piece
- `bs->board` — raw board array

### Declaration

In `incl/movegen.h`:

```c
void mg_my_pattern(const PieceState *piece, const BattleState *bs,
                   const EffectArg *params, size_t n, MoveList *out);
```

---

## 5. Adding a New Trigger

**Files to touch**: `incl/effect.h`, `incl/battle.h` (if new event),
`src/effect.c`

### Step 1 — Add the trigger enum value

In `incl/effect.h`:

```c
typedef enum {
    // ... existing triggers ...
    TRIGGER_MY_NEW_TRIGGER,  /* add before TRIGGER_COUNT */
    // TRIGGER_COUNT must remain last
} EffectTrigger;
```

### Step 2 — Extend EffectCtx (if new context needed)

If the new trigger needs new evidence fields, add them to the `EffectCtx`
union in `incl/effect.h`:

```c
typedef struct EffectCtx {
    EffectTrigger trigger;
    BattleState  *bs;
    // ... existing fields ...
    // add new fields for your trigger's evidence
    struct {
        uint32_t my_piece_id;
        int      my_value;
    } my_trigger;
} EffectCtx;
```

### Step 3 — Emit the trigger in engine code

In the appropriate engine file (`src/piece.c`, `src/battle.c`, etc.):

```c
EffectCtx ctx = {
    .trigger = TRIGGER_MY_NEW_TRIGGER,
    .bs = bs,
    .my_trigger.my_piece_id = piece->id,
    .my_trigger.my_value = value,
};
bus_emit(&bs->bus, bs, TRIGGER_MY_NEW_TRIGGER, &ctx);
```

### Step 4 — Add to trigger index in report_elements.md

Add an entry to the Trigger Index section documenting the new trigger's
semantics and context fields.

---

## 6. Adding a New Kingdom

**Files to touch**: `incl/defs.h`, `incl/types.h`, `src/data/`,
`src/movegens/`, `src/effects/`, `src/registry.c`, `src/ai.c`,
`src/data/data_archetypes.c`

### Step 1 — Add the kingdom id

In `incl/defs.h`:

```c
typedef enum {
    KINGDOM_LONGWEI, KINGDOM_HARUSHIMA, KINGDOM_KEWARANI,
    KINGDOM_ZARQAN, KINGDOM_CAELAN, KINGDOM_NEW_KINGDOM, /* add */
    KINGDOM_NONE
} Kingdom;
```

### Step 2 — Add piece ids

Add the kingdom's base and combo pieces to `PIECE_*` enum in `incl/defs.h`,
in a contiguous range. Create `src/data/data_new_kingdom.c` following the
pattern of existing kingdom data files.

### Step 3 — Add card ids

Add the kingdom's cards to `CARD_*` enum in `incl/defs.h`, in a contiguous
range. Add card template definitions to the new `data_new_kingdom.c`.

### Step 4 — Register in registry.c

Add a new range check in `piece_template()` and `card_template()`:

```c
if (id >= PIECE_NEW_BASE && id <= PIECE_NEW_CAPSTONE) {
    return &PIECES_NEW_KINGDOM[id - PIECE_NEW_BASE];
}
```

Add `extern const PieceTemplate PIECES_NEW_KINGDOM[]` to `incl/data.h`.

### Step 5 — Add AI archetype

Add to `ARCHETYPE_*` enum and `data_archetypes.c`. Implement `AIPickFunc`
if the archetype needs custom decision logic, otherwise use `NULL` for the
default scorer.

### Step 6 — Add innate

Add to `data_innates.c` with the kingdom's starting passive effect.

### Step 7 — Add figurehead power

Add to `data_figureheads.c` for the kingdom's starting power.

---

## 7. Adding a Relic / Chain / Mastery

**Files to touch**: `incl/defs.h`, `src/data/`, `src/registry.c`

### Adding a Relic

1. Add `RELIC_*` id to `incl/defs.h`
2. Add template to `src/data/data_relics.c`:

```c
[RELIC_MY_RELIC] =
    {
        .id = RELIC_MY_RELIC,
        .name = "My Relic",
        .description = "Effect description.",
        .effects = {
            [0] = {
                .trigger = TRIGGER_RUN_START,
                .apply = eff_my_effect,
                .args = {},
                .arg_count = 0,
                .duration_turns = -1,
                .owner = SIDE_NEUTRAL,
                .source_id = 0,
            },
        },
        .effect_count = 1,
    },
```

3. Add `extern const RelicTemplate RELICS[]` / `RELICS_COUNT` to `incl/data.h`
   if not already present.
4. Update registry bounds in `src/registry.c`.

### Adding a Penalty Chain

1. Add `CHAIN_*` id to `incl/defs.h`
2. Add template to `src/data/data_chains.c`:

```c
[CHAIN_MY_CHAIN] =
    {
        .level = 2,
        .name = "My Chain",
        .penalties = {
            [0] = {
                .trigger = TRIGGER_BATTLE_START,
                .apply = eff_cp_penalty,
                .args = {{.type = EARG_INT, .v.i = -5}},
                .arg_count = 1,
                .duration_turns = -1,
                .owner = SIDE_NEUTRAL,
                .source_id = 0,
            },
        },
        .penalty_count = 1,
    },
```

### Adding a Mastery Card

1. Add `CARD_*` id to `incl/defs.h` (mastery cards are in the main card enum)
2. Add to `src/data/data_masteries.c` in `MASTERY_CARDS[]` with the figurehead
   card's template
3. Update `MASTERY_HOOKS[]` if adding new level hooks

---

## 8. Wiring Battle Modifiers and Board Traits

**Files to touch**: `incl/defs.h`, `src/data/data_modifiers.c`,
`src/data/data_traits.c`, `src/battle.c`

### Adding a Battle Modifier

1. Add `MODIFIER_*` id to `incl/defs.h`
2. Add to `src/data/data_modifiers.c`:

```c
[MODIFIER_MY_MODIFIER] =
    {
        .id = MODIFIER_MY_MODIFIER,
        .name = "My Modifier",
        .description = "Effect description.",
        .type = MODIFIER_MY_MODIFIER,
        .effects = {
            [0] = {
                .trigger = TRIGGER_BATTLE_START,
                .apply = eff_my_effect,
                .args = {},
                .arg_count = 0,
                .duration_turns = -1,
                .owner = SIDE_NEUTRAL,
                .source_id = 0,
            },
        },
        .effect_count = 1,
    },
```

3. In `src/battle.c`'s `battle_init()`, add the modifier's effects to the
   bus when the modifier is active.

### Adding a Board Trait

1. Add `TRAIT_*` id to `incl/defs.h`
2. Add to `src/data/data_traits.c` with terrain description and effects
3. In `src/battle.c`'s `battle_init()`, wire the trait's effects when the
   board has this trait

---

## 9. Wiring AI Archetypes

**Files to touch**: `incl/defs.h`, `src/data/data_archetypes.c`,
`src/ai.c`

### Tuning an Existing Archetype

Edit the `AIWeights` values in `data_archetypes.c`. Each weight field:

| Field | Meaning |
|-------|---------|
| `value_diff_w` | Weight for piece value difference |
| `territory_w` | Weight for territory control |
| `aggression_w` | Weight for offensive play |
| `sell_threshold` | Sell if hand value exceeds this |
| `save_threshold` | Keep if value exceeds this |
| `max_piece_cost` | Don't buy above this cost |
| `combo_chain_bonus` | Bonus for chain combos |
| `reclaim_priority` | Priority for Harushima reclaim behavior |

### Adding a Custom Pick Function

If an archetype needs behavior that can't be expressed through weights,
implement `AIPickFunc`:

```c
/// ai_pick_my_archetype
///
/// Custom pick function for [archetype description].
Action
ai_pick_my_archetype(BattleState *bs)
{
    // Examine bs for valid actions
    // Score and return best Action
}
```

In `data_archetypes.c`:

```c
{
    .name = "My Archetype",
    .kingdom = KINGDOM_NEW_KINGDOM,
    .pick = ai_pick_my_archetype,
    // ... weights ...
}
```

---

## 10. Plugging in SDL3

**Files to touch**: `src/sdl3/` (new), `incl/`

### Architecture

The SDL3 front-end treats `BattleState` as read-only model data and drives
interaction through the battle API. No engine code is modified.

```
+------------------+
|   SDL3 Renderer  |  (src/sdl3/renderer.c)
+------------------+
         |
         v
+------------------+     +------------------+
|  BattleState     |<--->|  Input Handler   |
|  (model, read-    |     |  (src/sdl3/input.c)
|   only)           |     +------------------+
+------------------+              |
         ^                          v
         |                  +------------------+
         |                  |  battle_action_* |
         |                  |  (src/battle.c)  |
         |                  +------------------+
         |
         v
+------------------+
|  Event Drain     |  battle_drain_events()
|  (animation sync)|  src/battle.c
+------------------+
```

### Render loop

1. Call `battle_drain_events(bs, events, cap)` each frame
2. For each event, schedule animation / sound
3. Read `bs->pieces[]`, `bs->meter[]`, `bs->cp[]` to draw board
4. Call `battle_legal_moves()` to highlight legal squares
5. Call `battle_valid_buy_squares()` to show buy ghosts
6. On player input → call `battle_action_*` → loop

### Projection queries

Before committing, use projection queries to preview outcomes:

```c
int projected = battle_projected_damage(bs, SIDE_PLAYER);
// Simulate on a copy of the bus without mutating state
```

---

## 11. Adding Save/Load

**Files to touch**: `src/run.c`, `incl/run.h`

### Serialization Strategy

`BattleState` and `RunState` contain:
- Plain data (arrays, ints) — serialize directly
- Pointers to templates — resolve via id at load time
- Function pointers in `EffectBus.slots[]` — **cannot serialize directly**

### Effect Function Pointers

Since function pointers cannot be serialized, each `Effect` must carry an
id that maps back to the function at load time:

```c
typedef struct Effect {
    uint16_t      effect_id;  /* index into function table */
    EffectTrigger trigger;
    EffectArg     args[MAX_EFFECT_ARGS];
    uint8_t       arg_count;
    int16_t       duration_turns;
    Side          owner;
    uint32_t      source_id;
} Effect;
```

At load time, `effect_id` is looked up in a static function table:

```c
static const EffectFunc EFFECT_TABLE[] = {
    [EFFECT_TODO]       = eff_todo,
    [EFFECT_BULWARK]    = eff_bulwark,
    [EFFECT_BLOODLETTING] = eff_bloodletting,
    // ...
};
#define EFFECT_COUNT (sizeof(EFFECT_TABLE) / sizeof(EFFECT_TABLE[0]))
```

`src/run.c`:
```c
bool run_save(const RunState *rs, FILE *fp);
bool run_load(RunState *rs, FILE *fp);

bool battle_save(const BattleState *bs, FILE *fp);
bool battle_load(BattleState *bs, FILE *fp);
```

### Save format

```
[RunState binary blob]
[BattleState binary blob]
[Event log for replay]  /* optional, for deterministic replay */
```

Use `fwrite`/`fread` for integers and fixed-size arrays. For dynamic
length arrays, write length prefix first.

---

## 12. Adding Unit Tests

**Files to touch**: `tests/` (new directory)

### Test structure

```
tests/
  test_effect.c
  test_movegen.c
  test_piece.c
  test_card.c
  test_battle.c
  test_meter.c
  fixtures.h     /* shared test helpers */
```

### Test pattern

```c
/// test_my_effect
///
/// Verify eff_my_effect modifies meter correctly.
void test_my_effect(void)
{
    BattleState bs;
    battle_init(&bs, &TEST_CONFIG);

    EffectCtx ctx = {
        .trigger = TRIGGER_TURN_START,
        .bs = &bs,
    };
    EffectArg args[] = {
        {.type = EARG_INT, .v.i = 10}
    };
    eff_my_effect(&ctx, args, 1);

    assert(bs.meter[SIDE_PLAYER] == 30);  /* after +10 from effect */

    battle_destroy(&bs);
}
```

### Running tests

```makefile
test: $(TEST_SRCS)
	$(CC) $(CFLAGS) -Iincl $^ -o $@
	./test
```

### Coverage targets

| Area | What to test |
|------|-------------|
| `eff_meter.c` | Meter cap, overflow, cascade |
| `eff_damage.c` | Multipliers, reductions, immunity |
| `eff_piece.c` | Spawn, flip, remove |
| `mg_*.c` | Each movegen covers legal squares |
| `piece.c` | Flip cascades, splitter intercept |
| `battle.c` | Turn flow, resolve order |

---

## 13. Adding Multiplayer / Replay

**Files to touch**: `src/battle.c`, `src/run.c`

### Replay system

The RNG is seeded from `BattleConfig.seed`. Actions are deterministic:
same seed + same action sequence = same outcome.

```c
typedef struct ActionLog {
    uint32_t turn_no;
    Side     side;
    Action   action;    /* union of all action types */
} ActionLog;
```

At end of battle, serialize `ActionLog[]` to file. To replay:
1. Load initial `BattleState` from save
2. Replay `ActionLog[]` by calling `battle_action_*` in order
3. Skip rendering during replay; capture final state

### Network play

The battle engine is stateless between actions. A simple TCP relay:

```
Player A <---> Game Server <---> Player B
              (validates actions, relays)
```

- Server holds authoritative `BattleState`
- Each client sends actions; server validates and broadcasts
- Both clients render the same `BattleState` after each action
- Use UDP for low-latency relay with server reconciliation

---

## 14. Makefile Checklist

When adding new files:

- **`DATA_SRCS`** in `Makefile`: add `src/data/data_new.c`
- **`MOVE_SRCS`**: add `src/movegens/mg_new.c` if creating new file
- **`EFFECT_SRCS`**: add `src/effects/eff_new.c` if creating new file
- **`TEST_SRCS`**: add `tests/test_new.c` if creating test file

Headers: `-MMD -MP` (already in Makefile) auto-generates `.d` files
so editing any header rebuilds its dependents.

---

## 15. Code Review Checklist

Before submitting a new element:

- [ ] Id added to enum in `incl/defs.h` before `*_COUNT`
- [ ] Template defined in correct `data_<kingdom>.c`
- [ ] `passive_count` / `play_effect_count` matches array length
- [ ] `owner` field set to `SIDE_NEUTRAL` for passive/sell effects
- [ ] `duration_turns` correct: `0` = local, `n` = turns, `-1` = battle/run
- [ ] All arguments have correct `EffectArgType`
- [ ] Registry range in `registry.c` covers the new id
- [ ] Build with `make debug` — zero warnings, zero errors
- [ ] Run `./bin/regnum` — demo still works
- [ ] Document the new element in `report_elements.md`
