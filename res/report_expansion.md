# Regnum — Boilerplate Expansion Guide

**Created**: 2026-06-14
**Updated**: 2026-06-15
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

| File                  | Handles                                      |
| --------------------- | -------------------------------------------- |
| `eff_meter.c`         | Meter adjustments, overflow, cap             |
| `eff_economy.c`       | CP adjustments, cost/sell modifiers, income  |
| `eff_damage.c`        | Damage multipliers, reductions, immunities   |
| `eff_movement.c`      | Movegen swaps, extra steps, free moves       |
| `eff_piece.c`         | Spawn, remove, swap, force flip              |
| `eff_card.c`          | Draw extra, skip, peek, target removal       |
| `eff_flip.c`          | On-flip handlers, splitter spawns            |
| `eff_chain.c`         | Penalty chain effects                        |
| `eff_figurehead.c`    | Figurehead power effects                     |
| `eff_innate.c`        | Innate power effects                         |
| `eff_mastery.c`       | Mastery hook effects                         |
| `eff_overseer.c`      | Overseer mechanic effects                    |
| `eff_relic.c`         | Relic effects                                |
| `eff_synergy.c`       | Kingdom synergy effects                      |
| `eff_vorath_memory.c` | Vorath memory tracking                       |
| `eff_run.c`           | Universal stub (`eff_todo`) and run handlers |

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

### Step 4 — Register in the effect lookup table

In `src/effects/eff_registry.c`, add the new function to the `EFFECT_TABLE[]`
array using its `EffectFuncId` enum value.

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

## 6. Adding a New Screen

**Files to touch**: `incl/screen.h`, `incl/screens.h`,
`src/engine/screen_*.c`, `src/engine/screen.c`,
`src/engine/protocol.c`

Screens live entirely on the engine side. The SDL frontend renders
from the `< SHOW` / `< STATE` / `< POPUP` lines the engine emits
and translates key presses into verbs the engine consumes via
`engine_handle_line`.

### Step 1 — Add the screen id

In `incl/screen.h`, append the new `SCREEN_*` value just before
`SCREEN_COUNT`:

```c
typedef enum {
    SCREEN_TITLE = 0,
    SCREEN_MAP,
    SCREEN_BATTLE,
    SCREEN_EVENT,
    SCREEN_RESULTS,
    SCREEN_CODEX,
    SCREEN_MASTERY,
    SCREEN_SETTINGS,
    SCREEN_MY_NEW_SCREEN,  /* add here */
    SCREEN_COUNT
} ScreenId;
```

### Step 2 — Define the screen v-table

Create `src/engine/screen_my_new.c` with a `Screen` v-table whose
hooks operate on `EngineState`. There is no `tick` or `render`;
the engine emits state lines and the frontend renders.

```c
//! screen_my_new.c
//!
//! Describe the screen here.
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#include "prelude.h"

static void my_new_enter(EngineState* engine) {
    (void)engine;
}

static void my_new_handle(EngineState* engine, const ProtocolVerb* verb) {
    (void)engine;
    (void)verb;
}

static void my_new_emit(EngineState* engine) {
    protocol_emit_show(engine->out, SCREEN_MY_NEW_SCREEN, "");
}

const Screen SCREEN_MY_NEW_V = {
    .enter  = my_new_enter,
    .leave  = NULL,
    .handle = my_new_handle,
    .emit   = my_new_emit,
};
```

### Step 3 — Register in the screen registry

In `incl/screens.h`, declare the new symbol:

```c
extern const Screen SCREEN_MY_NEW_V;
```

In `src/engine/screen.c`, add to the `REGISTRY` array:

```c
static const Screen* const REGISTRY[SCREEN_COUNT] = {
    [SCREEN_TITLE]         = &SCREEN_TITLE_V,
    [SCREEN_MAP]           = &SCREEN_MAP_V,
    [SCREEN_BATTLE]        = &SCREEN_BATTLE_V,
    [SCREEN_EVENT]         = &SCREEN_EVENT_V,
    [SCREEN_RESULTS]       = &SCREEN_RESULTS_V,
    [SCREEN_CODEX]         = &SCREEN_CODEX_V,
    [SCREEN_MASTERY]       = &SCREEN_MASTERY_V,
    [SCREEN_SETTINGS]      = &SCREEN_SETTINGS_V,
    [SCREEN_MY_NEW_SCREEN] = &SCREEN_MY_NEW_V,  /* add here */
};
```

### Step 4 — Wire the protocol token

In `src/engine/protocol.c`, append the wire token to `NAMES[]` so
`< SHOW my_new` and `> goto my_new` round-trip cleanly:

```c
static const char* const NAMES[SCREEN_COUNT] = {
    [SCREEN_TITLE]         = "title",
    ...
    [SCREEN_MY_NEW_SCREEN] = "my_new",
};
```

The frontend can now transition into the screen with:

```
> goto my_new
```

---

## 7. Screen Transition Mechanism

**Files**: `incl/screen.h`, `src/engine/screen.c`

### Deferred Transition Pattern

Transitions are **deferred** to pump boundaries so the current
screen's `emit` finishes before any teardown / setup runs:

```c
void screen_goto(EngineState* engine, ScreenId id) {
    engine->next               = id;
    engine->transition_pending = true;
}

void screen_apply_transition(EngineState* engine) {
    if (!engine->transition_pending) return;

    const Screen* old_screen = screen_get(engine->current);
    const Screen* new_screen = screen_get(engine->next);

    if (old_screen != NULL && old_screen->leave != NULL)
        old_screen->leave(engine);

    engine->current            = engine->next;
    engine->transition_pending = false;

    if (new_screen != NULL && new_screen->enter != NULL)
        new_screen->enter(engine);
    if (new_screen != NULL && new_screen->emit != NULL)
        new_screen->emit(engine);
}
```

### Hook Ordering

1. **Pump start**: `engine_handle_line` parses the verb and
   dispatches to the active screen's `handle`.
2. **Outgoing `leave`**: Called first if non-NULL.
3. **Screen swap**: `engine->current = engine->next`.
4. **Incoming `enter`**: Called after swap if non-NULL.
5. **Incoming `emit`**: Writes the new SHOW + STATE lines so the
   frontend can re-render.

### Leave Hook Usage

The `leave` hook is optional. Use it to save selection state, clear
codex/popup state, or write a parting LOG line.

---

## 8. Profile and Run Persistence

**Files**: `incl/profile.h`, `src/profile.c`, `incl/run.h`, `src/run.c`

### Profile Lifecycle

The `Profile` struct is the player's persistent identity:

```c
typedef struct Profile {
    uint32_t version;
    uint8_t  mastery_levels[KINGDOM_COUNT];
    uint64_t codex_bits[2];
    uint8_t  prestige_tier;
    uint32_t vorath_defeat_count;
    uint16_t vorath_memory[PIECE_ID_COUNT];
    uint32_t total_wins;
    uint32_t total_losses;
} Profile;
```

**Load/Save cycle:**

```c
/* At engine startup (engine_init) */
engine->profile = calloc(1, sizeof(Profile));
if (!profile_load(engine->profile)) profile_new(engine->profile);

/* At engine shutdown (engine_destroy) */
profile_save(engine->profile);
free(engine->profile);
```

### Run State Lifecycle

The `RunState` struct tracks an in-progress campaign run:

```c
typedef struct RunState {
    uint64_t run_seed;
    Kingdom  current_kingdom;
    Tier     current_map_tier;
    MapState current_map;
    RelicId  relic_ids[MAX_RELICS_HELD];
    uint8_t  relic_count;
    uint8_t  chain_levels[KINGDOM_COUNT];
    bool     subjugated[KINGDOM_COUNT];
    uint16_t vorath_counter;
    uint32_t flags;
    struct Profile* profile;
} RunState;
```

**Run Flags:**

- `RUN_FOREIGN_MARKUP_OFF` — Trade Routes relic
- `RUN_DOUBLE_ARCHIVE` — Master's Notes relic
- `RUN_VISION_ENEMY_VALUES` — Eagle Eye relic
- `RUN_PREREVEAL_MODIFIER` — Surveyor's Map relic

### End-of-Run Processing

```c
void run_finalize(RunState *run, RunEnd outcome)
{
    switch (outcome) {
    case RUN_END_VORATH_WIN:
        run->profile->vorath_defeat_count++;
        run->profile->total_wins++;
        run->profile->prestige_tier = max(1, run->profile->prestige_tier);
        for (k = 0; k < KINGDOM_COUNT; k++) {
            if (!run->mastery_disqualified[k])
                run->profile->mastery_levels[k]++;
        }
        break;
    case RUN_END_LOSS:
        run->profile->total_losses++;
        break;
    }
    profile_save(run->profile);
    run_delete();  // Remove run.regsav
}
```

### Battle Loss Handling

```c
void map_on_battle_lost(RunState *run)
{
    Kingdom k = run->current_kingdom;
    run->chain_levels[k]++;
    run->vorath_counter++;
    run->mastery_disqualified[k] = true;

    if (run->chain_levels[k] >= 3)
        run->subjugated[k] = true;
}
```

### Save Codec Pattern

For serializing function pointers:

```c
// Each Effect carries an id that maps to the function at load time
typedef struct Effect {
    uint16_t      effect_id;  /* index into function table */
    EffectTrigger trigger;
    EffectArg     args[MAX_EFFECT_ARGS];
    uint8_t       arg_count;
    int16_t       duration_turns;
    Side          owner;
    uint32_t      source_id;
} Effect;

// At load time, resolve id to function pointer
static const EffectFunc EFFECT_TABLE[] = {
    [EFFECT_TODO]       = eff_todo,
    [EFFECT_BULWARK]    = eff_bulwark,
    // ...
};

EffectFunc eff_lookup(EffectFuncId id)
{
    if (id >= EFFECT_COUNT) return eff_todo;
    return EFFECT_TABLE[id];
}
```

---

## 9. Adding a New Kingdom

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

## 10. Adding a Relic / Chain / Mastery

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

## 11. Wiring Battle Modifiers and Board Traits

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

## 12. Wiring AI Archetypes

**Files to touch**: `incl/defs.h`, `src/data/data_archetypes.c`,
`src/ai.c`

### Tuning an Existing Archetype

Edit the `AIWeights` values in `data_archetypes.c`. Each weight field:

| Field               | Meaning                                 |
| ------------------- | --------------------------------------- |
| `value_diff_w`      | Weight for piece value difference       |
| `territory_w`       | Weight for territory control            |
| `aggression_w`      | Weight for offensive play               |
| `sell_threshold`    | Sell if hand value exceeds this         |
| `save_threshold`    | Keep if value exceeds this              |
| `max_piece_cost`    | Don't buy above this cost               |
| `combo_chain_bonus` | Bonus for chain combos                  |
| `reclaim_priority`  | Priority for Harushima reclaim behavior |

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

## 13. Plugging in SDL3

**Files to touch**: `src/sdl3/` (future), `incl/`

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

## 14. Adding Save/Load

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

## 15. Adding Unit Tests

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

| Area           | What to test                      |
| -------------- | --------------------------------- |
| `eff_meter.c`  | Meter cap, overflow, cascade      |
| `eff_damage.c` | Multipliers, reductions, immunity |
| `eff_piece.c`  | Spawn, flip, remove               |
| `eff_*.c`      | Each effect function              |
| `mg_*.c`       | Each movegen covers legal squares |
| `piece.c`      | Flip cascades, splitter intercept |
| `battle.c`     | Turn flow, resolve order          |

---

## 16. Adding Multiplayer / Replay

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

## 16. Project Structure

This section documents the overall architecture for contributors unfamiliar with the codebase.

### Directory Layout

```
regnum/
├── incl/                    # Public headers (engine-only, no SDL)
│   ├── prelude.h            # Umbrella header — every engine .c includes this
│   ├── core.h               # Base layer: stddef.h, stdint.h, stdbool.h
│   ├── defs.h               # All enums (PieceId, Kingdom, Side, etc.)
│   ├── types.h              # Struct definitions (BattleState, Profile, etc.)
│   ├── effect.h             # Effect system (Effect, EffectBus, triggers)
│   ├── movegen.h            # MoveGen function signatures
│   ├── card.h               # Card templates and hand management
│   ├── recipe.h             # Combination recipes
│   ├── screen.h             # Screen state machine
│   ├── engine.h             # EngineState, run/map state
│   └── ... (30 headers total)
│
├── src/                     # Implementation
│   ├── battle.c             # Battle state, turn loop, resolve
│   ├── board.c              # Board geometry, occupancy, threat
│   ├── card.c               # Card draw, discard, hand management
│   ├── piece.c              # Piece lifecycle, flip, spawn
│   ├── meta.c               # Relic/meta apply
│   ├── rng.c                # Seeded random number generator
│   ├── registry.c           # Id-to-template lookup
│   ├── map.c                # Overworld map generation + campaign loop
│   ├── recipe.c             # Recipe lookup
│   ├── log.c                # Structured logging
│   ├── platform.c           # OS abstraction (paths, files)
│   │
│   ├── engine/              # Engine subsystems
│   │   ├── run.c            # RunState persistence
│   │   ├── profile.c        # Profile persistence
│   │   └── save.c           # Chunked TLV codec (CRC32)
│   │
│   ├── data/                # Content definitions
│   │   ├── data_<kingdom>.c # Piece/card templates per kingdom
│   │   ├── data_universal.c # PIECE_KING, universal cards
│   │   ├── data_recipes.c   # Combination recipe table
│   │   ├── data_figureheads.c # Kingdom figurehead powers
│   │   └── data_innates.c   # Innate abilities
│   │
│   ├── movegens/            # Movement pattern generators
│   │   ├── mg_basics.c      # Universal primitives (slide, step, leap)
│   │   ├── mg_longwei.c     # Bespoke Longwei patterns (Knight-L, cannon)
│   │   ├── mg_caelan.c      # Bespoke Caelan patterns (Gryphon)
│   │   └── mg_zarqan.c      # Bespoke Zarqan patterns (Ziraafa, Shahzadeh)
│   │
│   ├── effects/             # Effect handler bodies
│   │   ├── eff_*.c          # One file per effect (eff_meter, eff_chain, etc.)
│   │   └── eff_run.c        # Run-scope effects + eff_todo placeholder
│   │
│   └── sdl/                 # Frontend layer (outside prelude)
│       ├── main.c           # Entry point, app lifecycle
│       ├── app.c            # SDL window, renderer, event loop
│       ├── ui.c             # Widget primitives
│       ├── input.c         # Keyboard/mouse input
│       └── platform_sdl.c   # SDL-specific platform implementations
│
├── tests/                   # Unit tests
│
├── res/                     # Documentation and assets
│   ├── GDD.md               # Full game design document
│   ├── plan.md              # Planning document
│   ├── report_expansion.md  # This file
│   └── report_elements.md   # Element reference
│
└── Makefile                 # Build system
```

### Include Hierarchy

All engine `.c` files include exactly one header:

```c
#include "prelude.h"  // includes all 25 engine headers in dependency order
```

The SDL layer (`src/sdl/`) intentionally opts out and includes headers directly.

**Layer 0**: `core.h` — only standard library headers (stdbool.h, stddef.h, stdint.h)
**Layer 1+**: All other headers include `core.h` and/or other domain headers

Most-depended headers:

- `defs.h` — included by 11 other headers
- `types.h` — included by 7 other headers
- `effect.h`, `screen.h`, `recipe.h`, `piece.h`, `movegen.h`, `meta.h`, `card.h`, `ai.h` — each included by 4 headers

### Naming Conventions

| Category              | Convention                 | Example                          |
| --------------------- | -------------------------- | -------------------------------- |
| Files                 | lowercase_underscores      | `eff_meter.c`, `mg_basics.c`     |
| Types (structs/enums) | PascalCase                 | `BattleState`, `PieceTemplate`   |
| Functions             | lowercase_with_underscores | `battle_resolve`, `piece_flip`   |
| Enum values           | UPPER_SNAKE_CASE           | `SIDE_PLAYER`, `KINGDOM_LONGWEI` |
| Macros                | UPPER_SNAKE_CASE           | `MAX_PIECES`, `DIR_N`            |
| Variables             | lowercase_with_underscores | `active_side`, `piece_count`     |

### Code Organization Patterns

**One effect file per effect**:

```
src/effects/
├── eff_meter.c        # Meter +/-, cap, overflow, refill
├── eff_economy.c      # CP +/-, cost/sell/income modifiers
├── eff_chain.c        # Bronze/Silver/Gold penalty chains
└── ...
```

**One movegen file per kingdom**:

```
src/movegens/
├── mg_basics.c        # Universal primitives
├── mg_longwei.c       # Bespoke Longwei movement
├── mg_caelan.c        # Bespoke Caelan movement
└── mg_zarqan.c        # Bespoke Zarqan movement
```

**Data files per kingdom**:

```
src/data/
├── data_longwei.c     # Longwei pieces, cards
├── data_harushima.c   # Harushima pieces, cards
├── data_kewarani.c    # Kewarani pieces, cards
├── data_zarqan.c     # Zarqan pieces, cards
├── data_caelan.c      # Caelan pieces, cards
├── data_universal.c   # PIECE_KING, universal cards
├── data_recipes.c     # All combination recipes
├── data_figureheads.c # Figurehead powers
└── data_innates.c     # Innate abilities
```

### Key Architecture Decisions

**Function-pointer dispatch**: The engine never special-cases content by name. Pieces reference `MoveGenFunc` pointers; effects reference `EffectFunc` pointers. Adding new behavior requires only a new function and updating data templates.

**EffectBus**: Effects are registered into a per-battle `EffectBus` that fires triggers at appropriate moments (battle start, turn start, resolve, etc.). Handlers are stateful within a battle.

**Stateless battle engine**: The battle engine holds no network or save state. Persistence flows through separate `RunState` and `Profile` objects.

**Chunked save format**: Save files use a TLV layout with magic + version header and CRC32 footer. Writers buffer everything atomically; readers validate CRC up-front.

### Prelude Enforcement

Every `.c` file under `src/` (except `src/sdl/`) must:

```c
#include "prelude.h"  // only include; no other internal headers
```

SDL files (`src/sdl/`) are frontend-only and may include headers directly since they live outside the engine layer.

---

## 17. Makefile Checklist

When adding new files:

- **`DATA_SRCS`** in `Makefile`: add `src/data/data_new.c`
- **`MOVE_SRCS`**: add `src/movegens/mg_new.c` if creating new file
- **`EFFECT_SRCS`**: add `src/effects/eff_new.c` if creating new file
- **`SCREEN_SRCS`**: add `src/screens/screen_new.c` if creating new screen
- **`TEST_SRCS`**: add `tests/test_new.c` if creating test file

Headers: `-MMD -MP` (already in Makefile) auto-generates `.d` files
so editing any header rebuilds its dependents.

---

## 18. Code Review Checklist

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
- [ ] Add "Descriptive Behaviour" row from GDD vision
