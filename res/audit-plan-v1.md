# Daulat — Philosophy Audit & Effect-Composition Rectification

## Context

The reimplementation (master-plan.md Phases 0–6) works, but its "agnostic"
run/battle functions are riddled with hardcoded special cases for specific
game elements. This violates the core design invariant the whole rewrite
exists to enforce:

> A game element's outcome is modified **with and only with effects**.
> Effects are isolated, composable, and self-filtering. Agnostic engine
> code never names a specific card/relic/modifier/kingdom/difficulty. The
> engine fires triggers; items supply effects.

The user cited three example violations (Lucky Strike in `battle_draw`,
Soul Shard in `battle_walk_run`, the empty `EVENT_*` parallel arrays) and
asked for a **full audit** that finds all of them, plus a plan whose first
execution phase is an exhaustive per-item analysis table in
`res/analysis.md`.

Memory note corrected: the boilerplate headers are no longer sacrosanct
(already changed irreparably). We still respect the boilerplate style and
keep verbosity low, but may make smarter structural decisions — never
sacrificing philosophy for constraints.

---

## The invariant, made concrete (from master-plan §2, §4, rev-3)

- `effect_fire(battle, side, trigger, x)` walks the fired side's player
  list + every live piece's `effects[]`. `x` is the value being computed,
  mutated in place. Effects self-filter and return whether they applied.
- Item placement: human-scoped items → human seat list; battlefield-wide
  items (modifiers, traits) → one copy per seat; cross-side observers →
  observed side's list with beneficiary in context.
- Effects spawn effects (core pattern). Marks (`eff_noop`) carry
  bookkeeping in a list.
- **The current code already proves the pattern works**: relics,
  modifiers, board traits, chain penalties, and pieces are attached
  generically through `battle_walk_*` loops. The violations are the
  places that *bypass* this: stub items whose behaviour was hardcoded
  into engine functions instead of authored as effects.

The master-plan itself already specifies several of the "violations" as
effects and they were then shortcut during implementation — e.g. §4 line
540 lists the Vorath-counter +20 meter as an `ON_BATTLE_START` effect;
§10 Phase 6 explicitly *deferred* "difficulty/challenge battle effects"
and "event effect bodies" and they landed as hardcoded `if` ladders.

---

## Complete EffectItem inventory & compliance status

Every category that is (or should be) an EffectItem, with current state.
`res/analysis.md` will expand each **instance** to its own row.

### A. Already effect items — mostly compliant
| Category | Registry | Count | Status |
|---|---|---|---|
| Cards | `CARD_REGISTRY` | 54 | Effect-driven except: **Queen's Gambit** draw hardcoded (battle.c:1049); **targeting** cards use the blind `a`/`b` codec — a protocol seam (Phase 2.5, §F); **Pawn Storm**/**Reforge** marks read inline by `battle_buy` (⚠️, Phase 4). |
| Relics | `RELIC_REGISTRY` | 26 | Compliant except **Soul Shard** side-selection hardcoded in `battle_walk_run` (battle.c:1551). |
| Pieces | `PIECE_REGISTRY` | 41 | Clean pattern — special behaviour lives in embedded `effects[]`. Reference model for everything else. |

### B. Effect items with STUB instances (behaviour hardcoded in engine)
| Category | Registry | Stub instances (hardcoded site) |
|---|---|---|
| Battle Modifiers | `MODIFIER_REGISTRY` (18) | **Bloodbath** flip-count (battle.c:1288); **Lucky Strike** draw (battle.c:328); **Fog of War** (display); **Dense Terrain** (battle.c:1851); **Extended Front**/**Compressed** board width (battle.c:1938-1942). 12/18 compliant. |
| Board Traits | `TRAIT_REGISTRY` (10) | **Mirage**/**Island Chain** terrain scatter (battle.c:1859-1864); **River Crossing** unimplemented stub; **Fog Coast**, **Contested Market**, **Sandstorm**, **Trade Route**, **Castle Corners**, **Siege Trench** to audit per instance. |
| Chain Penalties | `CHAIN_REGISTRY` (3) | **Bronze** is an effect but its cp magnitude is injected by difficulty (battle.c:1697); **Silver**/**Gold** are stubs handled imperatively in `battle_setup_armies`/run.c. |

### C. Effects, but not full items (inconsistent shape)
| Category | Registry | Issue |
|---|---|---|
| Kingdom Synergies | `SYNERGY_REGISTRY` | Declared `const Effect[]`, not an EffectItem struct with name/desc. Attached generically but shape differs from every other item. |

### D. NOT effect items — function-pointer dispatch (imperative)
| Category | Dispatch | Issue |
|---|---|---|
| Kingdom Innates | `KINGDOM_INNATE[]` fns | Each fn attaches an effect (semi-good) but is function-dispatched, not a data item; mastery scaling passed by arg. User wants these as effect items. |
| Kingdom Climaxes | `KINGDOM_CLIMAX[]` fns | Same as innates. |
| Kingdom Overseers | `KINGDOM_OVERSEER[]` fns | All empty stubs (Phase 7). Bespoke battle setup. |
| Narrative Events | `KINGDOM_EVENT[]` fns | **All empty stubs.** Real behaviour is fully hardcoded in run.c (see E). `EVENT_NAME/TEXT/OPTION_A/OPTION_B` are empty `= {}` parallel arrays (the user's cited violation). |

### E. NOT effect items — fully hardcoded, no registry
| Category | Hardcoded site(s) | GDD |
|---|---|---|
| Event value bonuses | `run_value_bonus` giant id/kingdom ladder (run.c:1258-1309) | "+3/+5 value this run" event rewards |
| Event cost bonuses | `run_cost_bonus` id ladder (run.c:1322-1346) | "cost 15% less this run" event rewards |
| Event immediate actions | `run_event_choose` `VORATH_REDUCTIONS` table + Desert Crossing special-case (run.c:1105-1169) | vorath reduce, chain remove, card removal, relic grant, map reveal |
| Difficulties | Bound +2 pieces (battle.c:1650), Shackled bronze=25 (battle.c:1697), Enslaved enemy innates (battle.c:1768) | §8 Difficulty table |
| Challenge Runs | Pacifist/Solo buy gate (battle.c:702-706), Traitor's Gambit intrusion (battle.c:1670), Blind Draft mask (screen.c:147), Clockwork (timer/display), Daily Conquest (seed) | §8 Challenge Runs |
| Global Vorath Counter | +20 meter/2 losses (battle.c:1980), forbid recipe/4 losses (run.c:1072) | §6 Global Vorath Counter |

### F. Presentation-boundary items — IN scope via state-query triggers
The philosophy extends to the protocol/display seam. `screen.c` currently
reads raw state and masks it by naming a challenge/modifier
(`challenge == CHALLENGE_BLIND_DRAFT` at screen.c:147; Fog of War implied).
That is the same violation wearing a display hat. The fix: the protocol
**queries state through triggers**, and display-affecting items supply
effects that redact / annotate / preview it. This pulls Fog of War, Eagle
Eye, Fog Coast, Blind Draft, Counsel, and Librarian's Notes into the
effect system and deletes the `screen.c` id checks. (Triggers below.)

The same seam appears at **card-play targeting**: `battle_play(hand, a, b)`
forces the caller to pre-encode each card's target, so the protocol must
know every card's encoding. Fix identically — the card advertises its legal
targets via `QUERY_CARD_TARGETS`; the protocol emits them and returns the
pick through `ON_CARD_TARGET_SELECTED` (two-step play, Phase 2.5).

### G. Genuinely out of effect scope (note in analysis, do not convert)
- **AI Archetypes** (`ai.c`, master-plan Phase 7 — decision trees,
  behavioural, not value-mutating).
- **Clockwork** challenge — a real-time 30 s/turn timer, not game state.
- **Daily Conquest** challenge — a seed choice at `run_new`, not an effect.

---

## New triggers / mechanisms the rectification adds

All extend the enum + `TRIGGER_NAME[]` table + the master-plan §4 x-doc
table (header doc discipline is part of the work). Every one keeps the
engine agnostic: it fires a trigger and lets items' effects act.

**`x` is always the mutable value being computed — never a wrapper struct
for auxiliary data.** An effect that needs run/unlock state reaches it
through a purpose-built battle.c helper (the engine pointer is
battle.c-private but exposed via functions like `battle_draw_pool`), the
same way effects already reach the battle via `battle_current()`. So no
trigger fattens `x` to smuggle context.

**Battle-side**
1. **`ON_CARDS_DRAWN`** (x = `Card**`, the hand) — fired at the end of
   `battle_draw` when ≥1 card landed; effects rewrite the drawn cards in
   place. Lucky Strike becomes a Modifier effect that, **once per turn**
   (it stamps `battle_current()->turn` in its own context), replaces
   `hand[0]` with a random highest-tier card from the *available* set
   (GDD "**always the highest-tier available**"; any top-tier pick is valid
   — current code wrongly forces the first). The available set comes from a
   shared `battle_draw_pool(battle, side, out)` helper (`run->cards` gated
   by `QUERY_CARD_CAN_DRAW`) that both `battle_draw` and the effect call —
   no pool passed through `x`. `battle_rand()` supplies the random pick.
2. **`QUERY_BOARD_DIMENSION`** (x = `Square*` {width, height}) +
   **`ON_BOARD_BUILD`** (x = board) — in `battle_begin`. One general
   dimension query, not a width-only one, so it extends to height mods and
   the Vorath 20×20 board. Extended Front/Compressed edit the dimension;
   Dense Terrain/Mirage/Island Chain scatter voids on the built board.
   Kills battle.c:1849-1944.
3. **`QUERY_ENEMY_ARMY_COUNT`** (x = int*) — in `battle_setup_armies`.
   Vorath pressure, Silver chain +1, Elite +1, Liberation +2, Bound +2
   all become effects adding to the count; the increment block dies.
4. **`QUERY_PIECE_CAN_BUY`** (x = bool*) — in `battle_buy`. Pacifist and
   Solo Vanguard become challenge effects; deletes
   `battle_challenge_allows_buy`.
5. **Flip-count mark** — on flip, stamp a per-piece mark counting flips so
   effects read it. Soul Shard: `ON_PIECE_FLIP_POST` on the gaining side
   attaches a battle-long `QUERY_METER_REFILL` +30 to the gainer — no
   engine id check, no enemy-list special placement.
6. **Bloodbath** → cascade-observer effect requesting an extra flip
   instead of `flips = id==BLOODBATH ? 2 : 1` (exact hook settled in
   analysis).

**Presentation-side** (the protocol queries these while emitting; effects
redact/annotate/preview — no `screen.c` id checks)
7. **`QUERY_BOARD_STATE`** (x = a mutable board view: per-cell piece +
   value-visible flag) — the board emitter fires it. Fog of War hides
   enemy values off player-attacked squares; Eagle Eye forces all visible;
   Fog Coast hides the far rows; Mirage marks revealed voids.
8. **`QUERY_HAND_STATE`** (x = a mutable hand view: per-card reveal flag) —
   the hand emitter fires it. Blind Draft clears id/name reveal, leaving
   kingdom + tier.
9. **`QUERY_NEXT_HAND`** (x = the projected upcoming hand) — the preview
   emitter fires it. Counsel peeks and discards one; Librarian's Notes
   previews the top card and skips it. It is the read/modify counterpart
   to `ON_CARDS_DRAWN`, which finalizes the realized hand. (The next hand
   is pre-rolled deterministically so it can be previewed then consumed —
   exact storage settled in analysis.)

**Card-play targeting** (self-describing two-step play; the protocol
enumerates then resolves — no per-card `a`/`b` encoding)
10. **`QUERY_CARD_TARGETS`** (x = `CardTarget*` the card appends to,
    `TARGET_NONE`-terminated like a move list) — fired after a card is
    played when it needs a target. Targets are **typed, not just squares**:
    `struct CardTarget { TargetKind kind; int value; }` with
    `TARGET_SQUARE` / `TARGET_NEXT_CARD` / `TARGET_PIECE_TYPE` — a new kind
    extends the enum, never the protocol. The card advertises its own legal
    targets of whatever kind.
11. **`ON_CARD_TARGET_SELECTED`** (x = `CardTarget*` chosen) — the card
    resolves (immediately, or by attaching a target-bound observer). The
    protocol relays only a plain **index** into the advertised list, so no
    square/codec crosses it. Multi-target cards fire the query per slot
    (kinds may mix — Force Drop = type then square) and resolve on the
    last; Counsel advertises `TARGET_NEXT_CARD`s sourced from
    `QUERY_NEXT_HAND`.

**Run-side** (per **decision 2 — run effect list**)
12. **`RunState.effects`** (LinkedList) + **`run_effect_fire(run, trigger,
    x)`** — walks the run list only. Holds `ENTIRE_RUN` effects; freed on
    run end.
13. **`ON_EVENT_CHOOSE`** (x = engine/choice context) — fired by
    `run_event_choose` for the chosen option's run-immediate effects (card
    removal, Vorath-counter reduce, chain removal, relic grant, map
    reveal, skip battle, next-battle cp).
14. Run-persistent event bonuses ("+3 value this run", "-15% cost this
    run") are `ENTIRE_RUN` effects on `QUERY_PIECE_DAMAGE_DEALT` /
    `QUERY_PIECE_CP_COST_BUY`; `battle_begin` copies battle-relevant
    `ENTIRE_RUN` run effects onto the human seat
    (`battle_walk_run_effects`, mirroring `battle_walk_run`). Deletes
    `run_value_bonus` and `run_cost_bonus` wholesale.

---

## Deliverable 1 (execution Phase 1): `res/analysis.md`

An exhaustive table listing **every EffectItem instance separately**
(every card, relic, piece, modifier, board trait, chain penalty, synergy,
innate, climax, event×2 choices, difficulty, challenge, overseer, plus the
Vorath counter). Columns:

| Column | Content |
|---|---|
| Item | Category + specific name/id |
| GDD | Verbatim intended behaviour (source of truth) |
| Effects / triggers | The effect(s) it should decompose into + trigger(s) |
| Context | Placement (which list), self-filter, args convention, cross-side notes |
| Current impl | Exactly how it behaves today + `file:line` |
| Compliant? | ✅ effect-driven / ⚠️ effect but wrong shape / ❌ hardcoded, with the specific violation |

Grouped by category, ordered to match the registries. This is the gating
artifact: it settles the exact per-item refactor before any code changes.

---

## Refactor approach

Core rule for every fix: **delete the `->id ==` / difficulty / challenge
`if` from the agnostic function; author the behaviour as the item's
effect(s); attach it through a generic `walk` loop.** After each phase,
`grep` for the item's id in `src/representation/*.c` must return nothing.

Reused machinery: the proven `battle_walk_*` attach loops, `effect_attach`,
self-filtering effect bodies, marks. New machinery: the triggers above +
the run effect list.

### Decisions settled (this session)
1. **Full plan now** — analysis is Phase 1, refactors Phases 2–8.
2. **Run-side effect list** — `RunState.effects` + `run_effect_fire`;
   events/difficulties/challenges/Vorath-counter are true effect items
   (run-immediate via `ON_EVENT_CHOOSE`; run-persistent via `ENTIRE_RUN`
   effects copied to the seat at `battle_begin`).
3. **Board-build + presentation triggers** — general `QUERY_BOARD_DIMENSION`
   + `ON_BOARD_BUILD` for geometry; `QUERY_BOARD_STATE` / `QUERY_HAND_STATE`
   / `QUERY_NEXT_HAND` so the protocol queries state through effects.
   Geometry and display items become ordinary effects.
4. **Two-step card targeting** — `battle_play(hand)` then
   `battle_card_target(index)`; cards self-describe a **typed `CardTarget`**
   list (`SQUARE`/`NEXT_CARD`/`PIECE_TYPE`) via `QUERY_CARD_TARGETS` /
   `ON_CARD_TARGET_SELECTED`; the protocol relays only an index — no
   per-card `a`/`b` encoding (Phase 2.5).

### Status (live, 2026-07-13)

> This plan is mirrored at **`res/audit-plan-v1.md`** (in-repo copy). Keep
> the two files in sync on every edit.

**NO-LEGACY MANDATE (user, absolute):** never leave a legacy or dual code
path in the tree — not transiently as a shipped state, not ever. Every
migration ends by deleting the old path.

- **Phase 1** ✅ — `res/analysis.md` written (215 instances catalogued).
- **Phase 2** — **Lucky Strike ✅**: `ON_CARDS_DRAWN` (x = `Card**`) +
  `battle_rand()` + shared `battle_draw_pool()`; `battle_draw` hardcode
  deleted; once-per-turn top-tier, verified seed=1. Rest dependency-blocked
  → moves to where its infra lands: Blind Draft (`QUERY_HAND_STATE`) →
  Phase 5; Counsel (`NEXT_CARD`) → 2.5; Librarian's + next-hand pre-roll
  (`QUERY_NEXT_HAND`) → interactive-preview slice; Fog of War / Eagle Eye /
  Fog Coast (`QUERY_BOARD_STATE`) → Phase 3.
- **Phase 2.5** IN PROGRESS — two-step targeting.
  - **Infra ✅**: `CardTarget {kind,value}` + `TargetKind`;
    `QUERY_CARD_TARGETS` / `ON_CARD_TARGET_SELECTED`; `battle_play` split +
    `battle_card_target(index)` + `PENDING_*`; protocol `target i=N` relays
    only the index; no-target → reject + refund (card stays in hand).
    card.c helpers: `card_target_push/count/at`, `card_targets_own/enemy`.
    Verified end-to-end (Sacrifice buy→play→target→removed).
  - **Migrated ✅** (universal): Sacrifice, Chain Break (immediate), Mercy
    (attach-observer). Patterns proven: immediate, attach-observer,
    embed-on-piece.
  - **Migrated ✅** (longwei): River Wade (own-pawn inline filter),
    Charge (`card_targets_own_all` + observer), Cannon Volley (own-Pao
    inline filter), Mandate (`card_targets_own` + remove), Mingzhu's Seal
    (`card_targets_enemy` + seal). Verified: River Wade seed=1 buy Bing →
    play → `target i=0 kind=0 value=203` → resolve grants sideways steps.
    Palace Decree's `card_square(args[1])` stays (internal zone-center
    packing on an attached observer, non-targeting card).
  - **Migrated ✅** (kewarani single-target): Double Time
    (`card_targets_own_all`, king-inclusive — verified seed=3 emits Medeq
    204 + king 225), Caravan (new `TARGET_FILE` kind, code-verified).
  - **Migrated ✅** (zarqan single-target): Bazaar (`card_targets_own` —
    verified seed=32 sells piece 204, king excluded), Citadel (new
    `card_targets_all`, both sides incl. kings — code-verified, Province
    tier undrawable in 200 opening hands).
  - **Migrated ✅** (harushima single-target): Ronin (verified seed=3
    king 226, `card_targets_own_all`), Bushido, Gold Standard, Promotion
    (own_all), Resurrection (`card_targets_enemy`).
  - **Migrated ✅** (caelan single-target): Queen's Gambit — **battle.c QG
    draw hardcode DELETED**; draws its 3 cards in its own
    `ON_CARD_TARGET_SELECTED` via `battle_draw` (fires `ON_CARDS_DRAWN`, so
    Lucky Strike composes). Verified seed=7: play → target pawn 204 →
    resolve, hand grows 3→5. Castling (Rook), Coronation (Pawn), Crusade
    (Knight) via new `card_targets_own_type`; Vengeance
    (`card_targets_enemy`).
  - **New kinds/helpers ✅**: `TARGET_FILE` (value = column; pick reads
    value directly). `card_targets_own_all` (friendly incl. king, movement
    grants), `card_targets_own_type(id)` (one identity — Pao/Rook/Knight/
    Pawn; Cannon Volley refactored onto it; verified via QG pawn),
    `card_targets_all` (both sides incl. kings). `card_targets_own` stays
    non-king for removal/protection. All enumerators now guard
    `cell->piece` (VOID_CELL has side=WHITE, piece=nullptr — a white-side
    scan would else deref null).
  - **GOTCHA** (see [[daulat-build-gotchas]]): the Makefile does NOT track
    header deps — after editing a header enum, `make clean && make debug`
    or translation units disagree on enum values (a `TARGET_NONE` mismatch
    segfaulted `card_target_count`).
  - **Multi-target infra ✅**: per-step selection. `battle_play` fires only
    step 0's `QUERY_CARD_TARGETS` and parks; `battle_card_target` records
    each pick into `PENDING_PICKS`, builds/emits the next step, and on the
    last pick fires `ON_CARD_TARGET_SELECTED` with `x = PENDING_PICKS` (a
    `TARGET_NONE`-terminated array — single-target pick fns read `x[0]`
    unchanged). A later step reaches earlier picks via
    `battle_pending_picks()`. A later step with 0 targets auto-skips
    (records a `TARGET_NONE` pick → Fuhyo for Dual Drop); step 0 with 0
    rejects+refunds. `battle_emit_targets` shared by play + each step.
  - **`a/b` FULLY STRIPPED** ✅: `battle_play(battle, hand)` — no more
    `a`,`b`; `screen.c` `play i=N` drops `a=`/`b=`; the ON_CARD_PLAY ctx and
    attach ctx no longer smuggle a/b. `card_pack`/`card_square` remain for
    internal square packing only (Palace Decree zone center longwei:1090,
    caelan:386 self-square) — verified those are the only two callers.
  - **Migrated ✅** (multi-target): Royal Decoy (2 own, 2nd excludes 1st —
    **verified seed=1**: play emits 3 incl king, pick king → step2 emits 2
    excl king, pick pawn → swap on board), Ambition & Conquest (own piece +
    `TARGET_PIECE_TYPE` via new `card_targets_piece_types(cap)`; share both
    query steps), Hajj (own piece + empty dest via new
    `card_targets_empty`), Force Drop (piece type ≤50 + empty dest),
    Dual Drop (2 enemy + Fuhyo fill via auto-skip). The last five are
    Province/Country tier → code-verified, behaviorally unverified
    (undrawable in opening hands); infra proven by Royal Decoy.
  - **Dual Drop semantic note**: two-step model requires ≥1 enemy to play
    (step0==0 rejects) and can't opt Fuhyo when 2 enemies exist (must pick
    the second enemy). Minor divergence from the old free a/b encoding;
    flag if the exact "up to 2, choose Fuhyo freely" behavior matters.
  - **New helpers ✅**: `card_targets_piece_types(list, max_value)` (roster,
    optional value cap), `card_targets_empty(list, battle)` (unoccupied
    in-bounds squares).
  - **PHASE 2.5 COMPLETE.** All targeting cards on the two-step protocol;
    zero `a/b`; no legacy path. Build green, 80-col clean.

  - **Phase 2.5b — consolidation + GDD audit (user feedback)** ✅:
    - **TargetKind renamed**: `TARGET_PIECE` (pick a piece), `TARGET_SQUARE`
      (pick a square; a file is its column), `TARGET_CARD`,
      `TARGET_PIECE_TYPE`, `TARGET_NONE`. `TARGET_FILE` deleted — Caravan
      now advertises `TARGET_SQUARE` and derives the file from the column.
    - **card_targets_\* collapsed to ONE per category**, controlled by a
      predicate the card supplies (no more own/own_all/own_type/all/enemy/
      empty/piece_types): `card_targets_piece(list, battle, side, match)`,
      `card_targets_square(..., match)`, `card_targets_piece_type(list,
      match)`. Generic reusable predicates live in card.c (`card_is_own`,
      `card_is_own_or_king`, `card_is_enemy`, `card_is_enemy_or_king`,
      `card_is_any`, `card_is_empty`); card-specific filters (pawn-grade,
      slider-class, Pao/Rook/Knight id, flipped, unlocked-type, exclude-
      first-pick) are tiny predicates in the data files — special cases
      stay in the cards, not hoisted. See [[daulat-effect-design-prefs]].
    - **GDD discrepancies fixed** (each rechecked against res/GDD.md):
      Charge → sliders only, NOT king (was a hallucination — verified
      king no longer offered); Resurrection & Dual Drop → **flipped**
      pieces only, via a new `MARK_FLIPPED` eff_noop embedded on a piece by
      `battle_mark_flipped` on every flip (travels across flips; predicate
      `harushima_flipped` reads it); Queen's Gambit & Coronation → pawn-
      grade any kingdom (`piece_is_pawn`), not `PIECE_PAWN` only; Ambition,
      Conquest, Force Drop → only **unlocked** identities (new
      `battle_piece_unlocked` accessor); Mingzhu → enemy incl. king per
      "target 1 enemy piece". Royal Decoy & Castling now use `battle_swap`.
    - Verified: River Wade/QG/Royal Decoy still green; Charge with only
      kings now rejects; 3 combat flips ran clean (flip-mark path).

  - **Phase 2.5c — de-bloat + triggers (2nd feedback round)** ✅
    (supersedes parts of 2.5b):
    - **NO named predicate helpers.** The card_is_\*/kingdom_own_\* set from
      2.5b is deleted. `card_targets_piece/square/piece_type` now take an
      inline **Clang block** `bool (^match)(...)`; each card writes its
      filter inline capturing side/battle/prior-picks. Toolchain is Apple
      Clang (Makefile `gcc` → clang; nested funcs unsupported), so `-fblocks`
      added to the Makefile. Rule saved to [[daulat-effect-design-prefs]]:
      never a helper for a one-line filter — hardcode or anonymous block.
    - **Magic marks → query triggers + per-piece counter.** `MARK_MOVED`
      and `MARK_FLIPPED` deleted. New `QUERY_PIECE_HAS_MOVED` /
      `QUERY_PIECE_HAS_FLIPPED` read a piece's embedded counter
      (`eff_piece_counter`, subject-filtered) bumped by battle_move /
      battle_flip; count in args[1], last-event turn in args[2].
      Resurrection/Dual Drop filter on `battle_piece_flips > 0` (cumulative).
      **No per-turn reset** (the `battle_reset_moves` global sweep was
      rejected — nothing needs it): Fortified Line ("still this turn") and
      Vengeance ("moved last turn") instead **timestamp**-compare
      `battle_piece_move_turn(piece)` to `battle->turn`.
    - **`QUERY_CARD_CAN_PLAY`** trigger (x = bool*, base true) +
      `battle_card_can_play` — dry-runs **every** target step (greedily picks
      target 0 each step so dependent steps see a prior pick; pending state
      saved/restored). A card that can't complete all its steps is
      unplayable, so a play never parks on a dead later step (verified:
      Royal Decoy with only the king rejects). battle_play gates on it.
    - **Dual Drop made dynamic** (resolves the earlier caveat — no longer a
      divergence): each pick step offers flipped enemies, or empty squares to
      drop a Fuhyo when none remain, so it is always playable and the Fuhyo
      location is chosen. `spawn_fuhyo` (auto-place) deleted.
    - Verified: blocks path (River Wade), Sacrifice king-exclusion (2 tgts),
      Charge only-kings rejected via can_play, QG draws 3, Royal Decoy swap,
      and an 8-turn AI battle (move+flip counters, reset, no crash).

  - **Phase 2.5d — full GDD fidelity audit (all 27 migrated cards)** ✅:
    - **Vengeance was way off** — implemented as a manual-target card
      offering all enemy non-king; GDD ("…an enemy piece that moved adjacent
      to one of your pieces last turn", no "Target") is **automatic**. Now a
      plain ON_CARD_PLAY scan: sums 2×value of every enemy that moved
      (`battle_piece_move_turn == turn-1`) and is adjacent to a friendly,
      one `battle_damage`. Emits no targets. Verified.
    - **Meter-damage primitive**: Cannon Volley, Mandate, Bushido used raw
      `player->meter -=` (no flip cascade) → switched to `battle_damage`.
    - **Force Drop** dropped its spurious `meter += value` ("for free" only).
    - **Double Time** now honours "Kewarani pieces with the innate active
      gain a third move" (2nd free move when the piece carries the innate's
      `eff_double_move`).
    - Rest confirmed faithful: only GDD "Target"/"your choice" cards are
      targeting cards; Sacrifice/Mandate/Bazaar/etc. resolutions match.
      Known pre-existing edge (not from this work): Caravan advances a file
      in board y-order, so a black multi-piece file can self-block — left
      as-is. Fidelity checklist saved to [[daulat-effect-design-prefs]].
  - **TODO (later phases)**: `TARGET_CARD` (Counsel) needs `QUERY_NEXT_HAND`
    (Phase 2 preview slice); optionally surface `battle_card_can_play` as a
    `playable=` flag in `emit_hand` (skipped now to avoid log-spam
    regression). Behavioral coverage for tier-gated multi and flipped-reclaim
    cards when a progression harness exists.
- **Phase 3 — board pipeline through effects** ✅:
  - **Geometry (Part A)**: new `QUERY_BOARD_DIMENSION` (x = `Square*`
    {width,height} in {x,y}) + `ON_BOARD_BUILD` (x = `Board*`).
    `battle_begin` rewired: attach run/modifier/trait effects FIRST, then
    fire dimension (once, for `SIDE_WHITE`, so the single per-seat copy
    applies exactly once — firing both sides would double it), set dims,
    spawn kings, fire build. The width if-ladder and the whole
    `battle_terrain` function are DELETED. `battle_scatter_voids` exposed
    (header decl) and called by effects. Extended Front (`+2` col),
    Compressed (`-2`), Dense Terrain (20% voids) → universal.c effects;
    Mirage (5%) → zarqan.c; Island Chain (10%) → harushima.c. Verified:
    seed=1 baseline 12×12/0 voids (unbroken), seed=6 Extended→14 wide,
    seed=27 Compressed→10 wide, seed=4 Dense→27 voids (~20%).
  - **Presentation (Part B)**: new `QUERY_BOARD_STATE`, x = `Board*`. **No
    wrapper struct** — visibility is baked into `Board` as
    `bool visible[MAX_BOARD_SIZE]` (user rejected an earlier
    `BoardView {value_visible, reveal_all}` struct as bloat, same mistake as
    the cards-drawn wrapper). **The fog hides the PIECE, not its value** —
    per the user, hiding a value is no nerf (it's lookup-able by type);
    hiding the piece itself is the real fog. A hidden enemy piece renders as
    `?` in the `row` line (position known, identity hidden — the player sees
    something is there but not what), empty squares stay `·`; there are NO
    value lines (the board emits letters only, as it did originally).
    `battle_board_view(battle)` resets `visible` all-true and fires the query
    for `HUMAN_SIDE` (only human-seat copies act). **Eagle Eye** (relic) is
    **itself a `QUERY_BOARD_STATE` effect**: on each read it resets the view
    all-visible and sets `func = eff_noop` on **every OTHER
    `QUERY_BOARD_STATE` effect** — human seat list AND every piece's
    `effects[]` — skipping itself. Disables all blinders by trigger, naming
    no item (user rejected a `reveal_all` override flag). It must be the
    query effect, not `ON_BATTLE_START` (which runs once, so a blinder
    attached mid-battle would escape it); resetting the view up front makes
    fire order moot. **Fog of War** (universal.c): builds human attack
    coverage via `battle_attacks`, hides every enemy piece off that coverage.
    **Fog Coast** (harushima.c): hides enemy pieces on the 3 rows farthest
    from the human (top rows when white, bottom when black) whose
    `battle_piece_moves == 0`. All fire `QUERY_BOARD_STATE`.
    Verified: seed=10 Fog of War renders the enemy king as `?` (row y=0
    `······?·····`) while own king stays a letter; multi-turn `board` dumps
    never crash.
    Fog Coast + Eagle Eye code-verified (same view plumbing, compile-clean);
    live-unverified only because a trait/relic needs deep map progression
    (no harness yet) — same convention as the tier-gated cards in 2.5.
  - **Left for later phases** (were tagged Phase 3 in early notes but belong
    elsewhere): Blind Draft hand mask (`screen.c:175`) is `QUERY_HAND_STATE`
    → Phase 5; Surveyor's Map modifier pre-reveal is run/map-side → Phase 7.
  - Analysis rows flipped ✅ (Extended/Compressed/Dense/Mirage/Island Chain/
    Fog of War/Fog Coast/Eagle Eye); grep confirms no geometry or fog id
    checks remain in `src/representation/*` or `src/protocol/*`.
- **Phase 4 — remaining battle stub items** ✅ (the true engine-hardcode
  violations; the 3 unimplemented traits are feature work, see below):
  - **Soul Shard**: deleted the `id==RELIC_SOUL_SHARD ? enemy : human`
    ternary in `battle_walk_run` — now a plain human-seat relic. Works
    because `battle_flip` fires `ON_PIECE_FLIP` for the *gaining* side's
    list (`pick->side` after the toggle), so a human-seat effect runs only
    when a piece flips onto the human. No enemy-list placement, no id check.
  - **Bloodbath**: new **`QUERY_FLIP_COUNT`** (x = `int*`, base 1) fired by
    the meter-empty cascade for the emptied (`receiver`) side. Deleted the
    `id==MODIFIER_BLOODBATH ? 2 : 1`. Bloodbath is a `QUERY_FLIP_COUNT` +1
    effect; modifier sits on both seats, fired for one side → exactly +1.
  - **Silver chain**: new **`QUERY_ENEMY_ARMY_COUNT`** (x = `int*`) fired by
    `battle_setup_armies` for the **human seat** (chain penalties live
    there). Deleted `if (level >= CHAIN_SILVER) count++`. Restructured
    setup: attach chains **cumulatively** (`for c = BRONZE..level`) BEFORE
    computing the count, then fire the query, then `battle_free_army`.
    Silver is a +1 effect on that trigger; Gold has no battle effect (its
    "track locks" is run/map-side). `MAP_NODE_ELITE/LIBERATION` and the
    `DIFFICULTY_BOUND` +2 stay hardcoded (structural / Phase 5).
  - **Pawn Storm + Reforge (magic-mark cleanup)**: deleted the
    `effect_find_mark(CARD_PAWN_STORM/CARD_REFORGE)` inline reads in
    `battle_buy` AND the engine's noop-mark init block (`if func==eff_noop:
    args[1]=card->id, args[2]=3`) — the attach loop now just sets
    `args[0]=ACTING_SIDE` for every effect. Both cards are now fully
    self-contained, coordinating sibling effects via `effect_find_mark`
    **inside their own data-file effects** (effect-land, allowed — the
    engine names nothing):
    - **Pawn Storm** = `ON_CARD_PLAY` setup effect that attaches a counter
      mark (`args[2]`=0) + `QUERY_PIECE_ACTION_COST_BUY` (count<3 → free
      action) + `QUERY_PIECE_CP_COST_BUY` (count==2 → free cp, the 3rd) +
      `ON_PIECE_BUY` (bump count on each pawn actually bought). All TURNS_1.
      Queries never mutate; only the real buy bumps. **Verified seed=18**:
      pawns 1–2 free-action full-cp, 3rd free-action + free-cp, 4th full
      cost + action consumed — exactly the GDD.
    - **Reforge** = `ON_PIECE_FLIP_PRE` one-shot that, on the first friendly
      flip, attaches a mark (type/turn/spent) + `QUERY_PIECE_CP_COST_BUY`
      (30% off while unspent and `turn <= flip_turn+1`) + `ON_PIECE_BUY`
      (spends on the first buy of that type). TURNS_2. Code-verified (needs
      a live flip to exercise; same flip/query plumbing as the King-flip
      cascade that ran clean in the seed=18 auto-battle).
  - **NOT done in Phase 4 (re-scoped):**
    - **Trade Routes relic** (`battle_price` foreign-markup id check) →
      moved to **Phase 7**: reversing a `*120/100` markup post-hoc is lossy;
      it pairs with the `run_cost_bonus` pricing cleanup already scheduled
      there. Analysis row retagged 7.
    - **Stub traits** — were unimplemented features, not violations (no
      engine code names them; empty `.effects`). User chose "build the 2
      simple ones":
      - **River Crossing** ✅ (longwei.c) — `eff_river_crossing`
        (QUERY_PIECE_MOVES) prunes a Xiang's moves that cross the mid-board
        river; `eff_river_bing` (ON_PIECE_MOVE) embeds the River-Wade
        sideways grant (`eff_sideways_grant`, once) when a Bing steps across.
        **Verified seed=2**: Xiang at (6,7) keeps only same-half moves; a
        Bing crossing (6,6)→(6,5) gains sideways (5,5)/(7,5).
      - **Trade Route** ✅ (kewarani.c) — `eff_trade_route`
        (QUERY_PIECE_MOVES) appends the empty king-adjacent squares when the
        piece sits on the board's main diagonal (x==y). **Verified seed=8**:
        diagonal Medeq gains +1 in every direction, non-diagonal unaffected.
        INTERPRETATION FLAG: "+1 movement per action" read as "+1 square of
        reach in all directions" (movement only, no new attacks) — the one
        bounded, implementable reading of an otherwise pattern-dependent
        phrase.
      - **Contested Market** ✅ (kewarani.c) — needed a new composable
        ownership seam: **`QUERY_SQUARE_OWNER`** (x = `Side*`; base = the
        existing Chebyshev nearest-piece read, `SIDE_NEUTRAL` = contested;
        square via new `battle_owner_square()` register accessor).
        `battle_territory` now computes the base then fires it for
        `SIDE_WHITE`, so every ownership read (turn-10 territory scoring,
        Fortified Line relic, the longwei/harushima territory innates) routes
        through one composable path — no dual code. `eff_contested_market`
        (ON_TURN_START) resolves-then-spawns: each neutral market piece is
        claimed by its square's territory holder or removed if contested
        (`battle_remove`), then a random Kewarani Town piece (runtime
        `PIECE_REGISTRY` scan by kingdom+tier — no hardcoded id list) spawns
        `SIDE_NEUTRAL` on a random contested empty square via `battle_rand()`.
        Interpretation flags: "each turn" = each half-turn (ON_TURN_START
        granularity); claim reassigns side only (no meter delta). Only
        Contested Market spawns neutrals, so all board neutrals are market
        pieces. Sandstorm / Castle Corners / Siege Trench already ✅.
  - Analysis rows flipped ✅ (Pawn Storm, Reforge, Soul Shard, Bloodbath,
    Silver); grep confirms no `CARD_PAWN_STORM/REFORGE`, `MODIFIER_BLOODBATH`,
    `RELIC_SOUL_SHARD`, `CHAIN_SILVER`, or `effect_find_mark` reads remain in
    `src/representation/*` or `src/protocol/*` (only the data registries +
    the `effect_find_mark` definition). Clean rebuild, zero warnings.
- **Phase 5 — difficulty + challenge as effect items** ✅:
  - **New structs/registries**: `DifficultyMode` + `ChallengeRun`
    (EFFECT_ITEM_BASE + id) in representation.h; `DIFFICULTY_REGISTRY[
    DIFFICULTY_NONE]` and `CHALLENGE_REGISTRY[CHALLENGE_COUNT]` in
    universal.c. Attached to the **human seat** at battle start by a new
    `battle_walk_rules` (shared `battle_attach_rule` helper). Difficulties
    attach **cumulatively** (`for d = FREE..run->difficulty`) mirroring the
    chain attach — a higher level includes the lower penalties, matching the
    original `>= / <=` range guards; the challenge is a single choice.
    Called in `battle_begin` after `battle_walk_traits` (before setup_armies
    so Bound's QUERY_ENEMY_ARMY_COUNT is present when it fires).
    Registries indexed by `run->difficulty`/`run->challenge` only — engine
    names no specific level (like the relic loop).
  - **3 new triggers** (enum + TRIGGER_NAME[] + §4 x-doc, all index-aligned,
    clean-rebuilt) — Bronze/Shackled reuse the existing `QUERY_CP_INCOME`
    seam instead of a bespoke chain trigger (user call):
    - **`QUERY_PIECE_CAN_BUY`** (x = bool*, base true) fired in `battle_buy`
      for the human (BUY_PIECE set first). `battle_challenge_allows_buy`
      DELETED. Pacifist (`eff_pacifist`, value>20 veto via
      `battle_buy_piece()`) + Solo Vanguard (`eff_solo_vanguard`, veto while
      a non-king human piece stands).
    - **`QUERY_HAND_STATE`** (x = PlayerState*) fired by new
      `battle_hand_view` from `emit_hand`; PlayerState gains
      `bool hand_visible[MAX_DRAWN_CARDS]` (baked in like Board.visible, no
      wrapper struct). Blind Draft (`eff_blind_draft`) clears the flags;
      `emit_hand` is one unified line — `id`/`name` masked (−1/"?") when
      hidden, `kingdom`+`tier` always shown. screen.c
      `challenge == CHALLENGE_BLIND_DRAFT` id check DELETED. (A stray
      hand-edit had reduced emit_hand to a broken `id=%d`←`"?"` ternary that
      dropped kingdom; corrected to the typed unified form.)
    - **`ON_BATTLE_SETUP`** (x = MapNode*) fired for HUMAN_SIDE in
      `battle_begin` after walk_synergies, **before the meters** — the seam
      for setup actions that must fold into the starting maxima. Enslaved
      (`eff_enslaved`, attach region `KINGDOM_INNATE[]` to the enemy) +
      Traitor's Gambit (`eff_traitors_gambit`, 3 enemy pieces in the human
      half via new public `battle_reinforce`). Chosen over ON_BATTLE_START
      (post-meter) because Traitor's pieces must count toward enemy
      meter_max (meter = sum of piece values); the enslaved-innate walk_
      innates block + the Traitor's setup_armies block both DELETED.
    - **Bronze + Shackled on `QUERY_CP_INCOME`** (NO new trigger — user:
      "why use QUERY_CHAIN_PENALTY when QUERY_CP_INCOME already controls
      per-turn *and* opening cp?"). Bronze (chain item) is now a turn-1
      `*income -= 10`; Shackled (difficulty) a turn-1 `*income -= 15` gated
      on `node->kingdom->chain` (so it bites only in a chained region — the
      only self-gate needed since it can't ride Bronze's magnitude). They
      compose additively on the income value → −25. `eff_bronze_chain`'s
      ON_BATTLE_START `cp -=` path + its 0-clamp DELETED; the `battle.c`
      `penalty = Shackled ? 25 : 10` injection DELETED; setup_armies' chain
      attach only sets args[0]. `turn_start` gained a `cp >= 0` clamp after
      income (preserves the old non-negative invariant). **Bonus:** nets
      into cp with no intermediate clamp, so the true −25 is now observable
      (cp 5, not the clamp-masked 10 the QUERY_CHAIN_PENALTY draft showed).
  - **battle_reinforce** exposed (public) — wraps `battle_free_army` +
    `KINGDOM_BASIC[]` so effects reinforce a side/half without the private
    basic-piece table; setup_armies routes its enemy army through it too.
  - **Verified live** (seed-driven protocol): Bound enemy_meter 10→30 (+2
    Bing, 2 `b` in enemy half); Pacifist (Ma 30 rejected, Xiang 20 allowed,
    control buys Ma); Solo (2nd non-king buy rejected); Blind Draft (hand
    id=-1 name="?" keeps kingdom+tier vs full baseline); Traitor's Gambit (3
    `b` in the human bottom half, enemy_meter=40 = king+3 Bing, pre-meter);
    Shackled bronze — concede-chained Longwei, battle 2 logs `Bronze Chain`
    trigger=QUERY_CP_INCOME: baseline income 10−10=0 → cp 20; Shackled
    income 10−10−15=−15 → cp 5 (the true −25, now fully observable); an
    unchained Shackled battle stays cp 30 (Shackled self-filters, silent);
    Bronze fires exactly once (turn 1) across 12 turns. Enslaved attaches,
    cumulative Bound holds. 20 challenge×difficulty combos × 30 AI turns:
    zero crashes. Build clean, zero warnings, 80-col clean.
    grep: no `challenge ==`/`difficulty >=` id checks in
    `src/representation/*` or `src/protocol/*` (only arg-parse + the generic
    registry-indexed walk).
- **Now:** Phase 5 done. Next is Phase 6 (synergy / innate / climax shape
  unification: reshape `SYNERGY_REGISTRY` to the EffectItem form; convert
  `KINGDOM_INNATE`/`KINGDOM_CLIMAX` fn tables into data EffectItem
  registries attached generically, mastery via context). NOTE: Enslaved's
  `eff_enslaved` still calls `KINGDOM_INNATE[]` — when Phase 6 retires that
  table, repoint `eff_enslaved` at the new innate registry.

### Execution phases (gate each: `make debug` zero warnings, 80-col clean, harnesses green)

**Phase 1 — `res/analysis.md`.** The exhaustive per-instance table (spec
above). Gating artifact; no code change. Settles each item's exact
effect/trigger decomposition.

**Phase 2 — Hand/draw pipeline through effects.** `ON_CARDS_DRAWN`,
`QUERY_HAND_STATE`, `QUERY_NEXT_HAND` (+ deterministic next-hand pre-roll).
Items: Lucky Strike, Blind Draft mask, Counsel, Librarian's Notes. Delete
the Lucky Strike hardcode in `battle_draw` and the Blind Draft id check in
`screen.c`.

**Phase 2.5 — Card targeting (two-step play).** Add the typed
`CardTarget {kind, value}` (`TARGET_SQUARE`/`NEXT_CARD`/`PIECE_TYPE`,
`TARGET_NONE`-terminated) + `QUERY_CARD_TARGETS` / `ON_CARD_TARGET_SELECTED`.
Split `battle_play` into play + a new `battle_card_target(index)`, parking a
`PENDING_CARD` (mirrors run.c's `PENDING_NODE`); the protocol goes two-step
— emit the advertised descriptors (`kind` drives rendering), accept a plain
`target i=N` index into that list (no codec crosses the seam). Move each
targeting card's body from the blind
`a`/`b` read to the two triggers, and delete the `CARD_QUEENS_GAMBIT`
hardcode (battle.c:1049) — QG picks its pawn via the query and draws in its
own `ON_CARD_TARGET_SELECTED` (`battle_draw` fires `ON_CARDS_DRAWN`, so
Lucky Strike composes onto QG's bonus cards free). Counsel advertises
`NEXT_CARD` targets; Force Drop mixes `PIECE_TYPE`+`SQUARE`. No-target cards
keep plain `ON_CARD_PLAY`. (Also fold the buy-side ⚠️ marks here or in
Phase 4:
Pawn Storm / Reforge move from `battle_buy` inline reads to
`QUERY_PIECE_ACTION/CP_COST_BUY` effects with a shared counter mark; same
shape fixes the Bulk Discount reset bug. **These are still open — they use
`effect_find_mark(CARD_PAWN_STORM/CARD_REFORGE, ...)` + `args[2..4]`
bookkeeping in `battle_buy` — the same magic-mark pattern replaced for
move/flip; redo them with the `eff_piece_counter`/query-trigger approach or
a purpose effect. Scheduled for Phase 4.**)

**Phase 3 — Board pipeline through effects.** `QUERY_BOARD_DIMENSION`,
`ON_BOARD_BUILD`, `QUERY_BOARD_STATE`; protocol emits board via the state
query. Items: Extended Front, Compressed, Dense Terrain, Mirage, Island
Chain, Fog of War, Eagle Eye, Fog Coast. Delete battle.c:1849-1944 and the
`screen.c` fog hardcode.

**Phase 4 — Remaining battle stub items.** Soul Shard (+ flip-count mark),
Bloodbath, Queen's Gambit, Silver/Gold chain bodies, and the remaining
stub traits (River Crossing, Contested Market, Sandstorm, Trade Route,
Castle Corners, Siege Trench). Delete their engine special-cases
(`battle_walk_run`, `battle_cascade`, `battle_play`).

**Phase 5 — Difficulty + challenge as effect items.** `QUERY_ENEMY_ARMY_
COUNT`, `QUERY_PIECE_CAN_BUY`; `DIFFICULTY_REGISTRY` + `CHALLENGE_REGISTRY`
(EffectItem structs) attached to `RunState.effects` at `run_new` (Traitor's
Gambit, Pacifist, Solo, Bound, Shackled, Enslaved). Delete
`battle_challenge_allows_buy` and the hardcoded blocks in
`battle_setup_armies` / `battle_walk_innates`.

**Phase 6 — Synergy / innate / climax shape unification.** Reshape
`SYNERGY_REGISTRY` to the standard EffectItem form; convert `KINGDOM_INNATE`
/ `KINGDOM_CLIMAX` function tables into data EffectItem registries attached
generically (mastery scaling via the item's context, not a fn arg). Kingdom
`*_innate` / `*_climax` fns retire.

**Phase 7 — Events + run-side model (largest).** `RunState.effects` +
`run_effect_fire` + `ON_EVENT_CHOOSE`; author every event's two choices as
effects; replace `EVENT_NAME/TEXT/OPTION_*` arrays with an `Event`
EffectItem carrying name/desc/option text; delete `run_value_bonus`,
`run_cost_bonus`, and the `VORATH_REDUCTIONS` / Desert-Crossing ladders in
`run_event_choose`; add `battle_walk_run_effects` to fold run-persistent
bonuses into the seat.

**Phase 8 — Vorath counter.** Threshold behaviours as effects: +20 enemy
meter/2 losses via `ON_BATTLE_START` (per §4:540), recipe-forbid/4 losses
via a run effect. Remove battle.c:1980 and run.c:1072 hardcoding.

Left as-is (category G — note in analysis, do not convert): AI archetypes,
Clockwork timer, Daily Conquest seed.

---

## Verification

- **Per item**: the `res/analysis.md` row flips to ✅ and `grep` for the
  id in `src/representation/*.c` (and `run.c`) returns nothing.
- **Behavioural** (extend piped harnesses): Lucky-Strike draws a top-tier
  first card; Soul-Shard flip adds +30; Bound run starts enemies +2;
  Pacifist rejects a >20 buy; an event choice logs its effect
  (`log effect name="..."`) and its run bonus shows in a later battle.
- **Presentation**: emitted board hides enemy value under Fog of War
  (visible under Eagle Eye); emitted hand masks name/id under Blind Draft —
  both via the state-query triggers, with `screen.c` naming no item.
- **Targeting**: playing Sacrifice emits its valid target squares; sending
  a `target` resolves the sacrifice via `ON_CARD_TARGET_SELECTED`; the
  protocol passes no card-specific encoding.
- **Regression**: all existing harnesses (phase1/3/4/5/6 + black-seat)
  stay green after every phase.
- Update master-plan.md §10 with a new "Phase 6.5 — philosophy
  rectification" record on completion (memory index too).

---

## Critical files

- `incl/representation.h` — new triggers, `CardTarget`/`TargetKind`,
  `PENDING_CARD` register, `RunState.effects`, `DIFFICULTY_
  REGISTRY`/`CHALLENGE_REGISTRY`/`Event`/innate/climax item structs, §4-style
  x-doc for new triggers.
- `src/representation/effect.c` — `TRIGGER_NAME[]`, `run_effect_fire`.
- `src/representation/battle.c` — the bulk of deletions (draw, walk_run,
  cascade, play, buy, setup_armies, walk_innates, terrain, begin); split
  `play` → play + `battle_card_target` with a `PENDING_CARD` register.
- `src/representation/run.c` — `run_effect_fire`, `run_event_choose`,
  delete `run_value_bonus`/`run_cost_bonus`.
- `src/data/universal.c` — stub modifier/chain bodies, synergy reshape,
  dispatch-table removal, `DIFFICULTY_/CHALLENGE_` registries, run-immediate
  event effect bodies.
- `src/data/{longwei,kewarani,zarqan,harushima,caelan}.c` — innate/climax
  items, per-kingdom event effect bodies, stub trait bodies.
- `src/protocol/{screen.c,protocol.c}` — board/hand emission fires
  `QUERY_BOARD_STATE` / `QUERY_HAND_STATE` / `QUERY_NEXT_HAND`; delete the
  Blind Draft (and any fog) id checks; two-step card play (emit `targets`,
  accept a `target` command) driving `QUERY_CARD_TARGETS` /
  `ON_CARD_TARGET_SELECTED`.
