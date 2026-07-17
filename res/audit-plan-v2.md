# Daulat — Effect Composition & Engine Simplification Audit v2

## Context

`res/audit-plan-v1.md` asked whether authored game elements bypassed the
shared effect system. Its rectification phases moved most card, relic, piece,
modifier, trait, chain, difficulty, challenge, event, innate, synergy, and
climax behavior into effects.

This audit asks a different question:

> Does the effect system itself compose cleanly, or did generic engine code
> become a second layer of special cases, duplicate primitives, hidden state,
> and order-sensitive conventions?

Current answer: authored item behavior is much cleaner, but effect plumbing is
not. `battle.c` is 2,993 lines and owns actions, effect context, setup, turn
order, card targeting, resolution, cascade, querying, board mutation, and
several run lookups. `run.c` is 1,414 lines and combines map generation,
progression, event suspension, rewards, battle outcomes, and protocol
navigation. `effect.c` is small, but callers compensate for its narrow API
with repeated context manipulation and direct effect invocation.

This plan simplifies those structures without removing behavior. It does not
reopen the v1 item backlog except where unfinished work depends on an engine
seam. `res/GDD.md` remains mechanics source of truth. No new card, piece,
relic, place, event, or rule is invented here.

Crusade is one explicit replacement requested by the user: remove the
one-card `battle_lunge` primitive and express its three moves through ordinary
movement, action-cost, and resolution composition.

---

## Executive finding

Recommended architecture has five rules:

1. One nested-safe effect fire frame, not a bank of manually managed globals.
2. One effect invocation path and one attachment path.
3. One generic engine primitive per operation: relocate, strike, damage,
   cascade, target-step, turn advance, and pending run interaction.
4. Trigger families fold by payload type and operation kind. `x` remains the
   mutable value; auxiliary metadata never replaces it with a wrapper.
5. Function names expose role: pure lookup, effectful evaluation, mutation,
   projection, or lifecycle orchestration.

Keep current source/header file count. Simplify inside existing files with
shared helpers, clearer sections, and smaller functions. Creating new modules
would move complexity instead of removing it.

---

## Evidence baseline

- **Trigger surface:** 50 `EffectTrigger` values
  (`incl/representation.h:128-193`). Similar operations require parallel
  triggers and duplicate effect bodies.
- **Trigger names:** positional `TRIGGER_NAME[]` (`effect.c:13-78`). Enum
  insertion can silently mislabel logs.
- **Dispatch:** `effect_fire` walks one seat list, then every live piece and
  all eight slots (`effect.c:153-181`). Order and board position affect
  composition.
- **Fire context:** eight effect-facing registers plus six other battle
  globals (`battle.c:19-33`). Callers manually save, set, fire, and restore
  hidden state.
- **Fire sites:** 53 `effect_fire(` occurrences and 36 assignments matching
  `CURRENT_BATTLE *= *battle` with flexible whitespace. Context setup is
  repeated and inconsistent.
- **Piece effects:** `MAX_EFFECT_COUNT` is eight
  (`representation.h:758-762`). Runtime facts and temporary grants compete
  with authored behavior.
- **Context:** `void *args[MAX_EFFECT_ARGS]`, where `MAX_EFFECT_ARGS` is 16
  (`representation.h:752-781`). Meaning depends on numeric slots.
- **Attachment:** `effect_attach` discards baked template context
  (`effect.c:119-137`). Event effects need a special argument-copy loop.
- **Immediate invocation:** card play, card targeting, and event choice call
  effect bodies directly. Matching and logging have multiple implementations.
- **Crusade:** `eff_crusade_pick` calls `battle_lunge` three times
  (`caelan.c:847-907`). One card owns a second movement/damage pipeline.
- **Resolution:** coverage-to-damage loops exist in `battle_resolve` and
  `battle_lunge` (`battle.c:1549-1592`, `1847-1903`). Fixes can diverge.
- **Meter damage:** half-turn damage is inline; direct damage uses
  `battle_damage` (`battle.c:1766-1785`, `1807-1831`). Two mutation and
  cascade entry paths exist.
- **Cascade:** `FLIPPED_PIECE` bridges flip and cascade
  (`battle.c:32`, `1676-1681`). Nested flips can overwrite result.
- **Run state:** nine file-static navigation/reward values
  (`run.c:394-402`). State ownership is implicit and singleton-only.
- **Event completion:** completion occurs in three functions
  (`run.c:1163-1170`, `1194-1205`, `1226-1231`). New deferred paths must
  update all completion sites.
- **Tests:** `makefile` declares `test` phony but defines no recipe.
  Verification depends on clean builds and protocol traces.

---

## Audit record format

Every implementation item below uses the same eight questions:

1. **Current shape** — what exists, with `file:line` evidence.
2. **Composition failure** — duplication, hidden state, ordering, lifetime,
   or false abstraction.
3. **Simplified shape** — one recommended replacement.
4. **Preserved behavior** — exact GDD and engine invariants.
5. **Affected mechanics** — concrete consumers.
6. **Dependency** — what must land first.
7. **Risk** — ways an apparently mechanical change can alter behavior.
8. **Verification** — build, protocol, and static evidence.

---

# Part 0 — Composition-completeness gate

## Gate rule

No simplification phase starts while an effect item remains deferred, stubbed,
hardcoded, incorrectly composed, or runtime-unverified because its required
semantic seam does not exist.

Feature completion comes first. Simplification then has a complete behavior
surface to preserve instead of optimizing around placeholders.

Completion gate covers:

- every ⚠️, ❌, and ⬜ effect-item row from `res/analysis.md`;
- every deferred composition item in `res/audit-plan-v1.md:784-856`;
- every mismatch found by `res/audit-baseline-v2.md`;
- all deep-progression verification gaps for implemented effect items;
- missing active, preview, run-side, map-side, boss, and recipe seams.

Only genuinely non-effect systems remain outside gate: Daily Conquest seed
selection, Clockwork real-time timer, full AI personality/archetype expansion,
and cosmetic mastery titles.

## Completion inventory

| Group | Required before simplification | Completion condition |
|---|---|---|
| Baseline defects | Counter Coup, Vorath capacity, Timur's Conquest, Bulk Discount | GDD traces pass |
| Run interaction | Event target dialog and all dynamic event choices | No fixed representative/no-op choice remains |
| Archive composition | Master's Notes and generic archive reveal count | No relic identity in `run_select_node` |
| Hand preview | deterministic next hand, Counsel, Librarian's Notes | Preview and selected skip/discard persist into draw |
| Active relic | Deep Hand | player may activate once per battle on chosen turn |
| Map visibility | hidden nodes, modifier visibility, event reveals, Surveyor's Map | All reveal mechanics observable through map protocol |
| AI projection | `ai_plan` and Divination | intended moves/cards emitted from stored plan |
| Chain system | Gold chain and Liberation Trial | lock, injected node, retry, restricted draw, and break work |
| Vorath run rule | recipe forbid every four losses | forbidden recipe stored and combine vetoed through effect query |
| Event elites | Mirage, Pretender, Tournament armies/rules | each event battle matches GDD, not standard army fallback |
| Overseers | five kingdom bosses and Vorath finale | bespoke rules live in effect/data items, not battle identity checks |
| Mastery gaps | Double Time cost penalty and Royal Substitution usage upgrade | M0-M3 behavior matches GDD |
| Verification | tier-gated cards, relics, traits, events, bosses | runtime traces exist through public protocol fixtures |

---

## F0.5. Confirm deterministic fixture reachability

Before F10 accepts any feature, prove a public-protocol fixture can execute its
deep mechanics. F0.5 records route contracts up front; each owning F phase makes
its routes executable before its gate. Save-file patches may establish starting
state, but they are not behavior evidence; each trace must drive the real
transition through protocol commands.

Record one deterministic route for:

- every event option, target continuation, event-elite result, and reward;
- Gold's Liberation Trial loss, three-battle retry delay, respawn, and win;
- Caravan of Conquest's reinforcement across consecutive turn pairs;
- Crowned Heretic accumulation of multiple Ghosts and later human-turn damage;
- every other Overseer setup, recurring mechanic, win, loss, and reward;
- Vorath exact-zero, overshoot, liberation, and final-run result;
- every tier, mastery, chain, map, active-item, and save/load gate named below.

If a route is missing, its owning F phase must add a deterministic
protocol-visible fixture seam inside existing files. Do not accept patched end
state, code inspection, or one isolated boss turn as proof.

Gate: every F10 behavior row has an executable fixture recipe before that
feature can claim completion.

---

## F1. Fix baseline defects before adding more features

### Counter Coup

Current `TURNS_1` copy expires at end of playing side's half-turn, before
opponent can deal normal attack damage. Do not fix this by making every
one-turn offensive buff last longer.

Add an owner-turn duration meaning:

- attached during owner's current turn;
- survives opponent half-turn;
- removed at start of owner's next turn.

Counter Coup uses this duration. Its receiver-side meter observer remains on
owner seat, echoes 50% through ordinary `battle_damage`, and expires before
owner takes another turn. Trace both human colors because black crosses round
boundary before next opponent half.

### Vorath pressure capacity

GDD says every two losses add 20 enemy meter capacity
(`res/GDD.md:659-664`). Current effect adds transient meter after maximum is
computed, then 200% clamp cuts lone-King gain to 10.

Introduce maximum/capacity operation in meter query family before general
trigger folding:

1. attach Vorath run effect before initial meter calculation;
2. `battle_meter_max` folds base piece sum through maximum query;
3. effect adds 20 per two losses to enemy maximum;
4. initial meter starts at modified maximum;
5. refill and 200% overflow use same modified maximum;
6. remove post-meter `battle_meter_gain` pressure path.

Later trigger consolidation keeps this as `QUERY_METER_AMOUNT` kind MAXIMUM.

### Timur's Conquest

Implement real meter-threshold state now:

- meter damage records whether owner crossed from at/above 20 to below 20;
- cascade preserves crossing through refills and redirects;
- one `ON_CASCADE_END` event fires after full damage/cascade transaction;
- Timur watcher observes owner-side crossing and triggers Royal Substitution;
- watcher does not depend on flipped piece's new side;
- once-per-battle usage remains explicit.

This stable cascade event remains during simplification; do not build another
intermediate post-flip trigger.

### Bulk Discount

Fix both deferred defects:

- shared reset/buy state, not two unrelated effect contexts;
- actual GDD settlement: when three or more pieces were bought this turn,
  cheapest purchase is free.

Track committed purchase costs during turn. At turn end, refund cheapest paid
cost when count is at least three, then clear state. Failed/evaluated buys do
not count. This avoids order-dependent "third purchase free" approximation.

This settlement supersedes v1 item H13's shared `eff_noop` mark proposal. A
shared mark would fix reset ownership but preserve the wrong third-buy-free
interpretation. Do not implement both fixes.

### Mastery effect gaps

Complete effect-backed innate and mastery behavior before simplification.
Double Time's movement effects exist, but its base pricing rule was never
implemented (`res/analysis.md:379`). F1 therefore adds net-new base behavior,
not only a mastery gate:

- while Double Time is active, Kewarani home pieces lose the 40% home discount
  and foreign pieces use a 40% markup (`res/GDD.md:236`);
- Selassie mastery 3 reduces only that foreign markup to the normal 20%
  (`res/GDD.md:979`);
- Royal Substitution is once per battle normally and twice at Timur mastery 3
  (`res/GDD.md:980`);
- pricing and use counters fire through generic queries or the active-power
  path, never kingdom checks in `battle.c`.

Gate: all six fixes have public protocol traces and GDD matrix entries.

---

## F2. Complete run-side effect dispatch and event targeting

### Typed run interaction

Implement one pending run interaction inside existing run/engine state before
adding dynamic choices. It owns:

- event and selected A/B option;
- current target step;
- typed target list;
- prior picks;
- pending node;
- reward/removal continuation;
- battle suspension and resume state.

Use same typed target vocabulary where domains match:

- `TARGET_CARD`;
- `TARGET_RELIC`;
- `TARGET_PIECE_TYPE`;
- `TARGET_NODE`.

Protocol flow becomes:

1. select event node;
2. choose A/B;
3. engine emits next target list when required;
4. protocol returns `target i=N`;
5. effect resolves after final pick;
6. one completion path clears node when no continuation remains.

Remove event-specific `choose`/`remove`/`relic` continuation branches after all
callers use target interaction. Keep generic relic acquisition and card removal
mutators internally.

### Dynamic event choices

Complete currently approximated/no-op choices:

- Dragon Court Tribute and other removal events target actual held card(s);
- Janggi Elder targets one Longwei piece type, not fixed Xiang;
- Forge Master targets one piece type, not fixed Kyosha;
- skip-battle choices target an actual selectable battle node, not a
  `skip_next_battle` flag;
- relic choices target one of emitted offers;
- node-reveal choices target or deterministically reveal valid hidden nodes;
- chain-removal choices target eligible figurehead when GDD says "any".

All 60 event options must finish through same interaction/resume path.

### Minimum node-visibility seam

Node-reveal targeting cannot precede real hidden-node state. F2 therefore owns
the minimum durable visibility primitive needed by every event target:

1. generated future nodes have explicit visible/hidden state;
2. clearing or selecting a node reveals reachable successors;
3. ordinary node selection rejects hidden nodes;
4. `run_emit_map` masks hidden identity and content;
5. reveal target queries enumerate valid hidden nodes only;
6. event reveal effects mutate that same state;
7. save/load preserves visibility.

F5 extends this primitive with separate modifier visibility and Surveyor's Map.
It must not replace an F2 no-op flag or temporary reveal representation.

### Run-side effect firing

Add generic run item dispatch for operations that occur without battle. It
walks held run effect items and invokes matching effect through shared
invocation/log path. `x` remains mutable value; run operation metadata lives in
run fire context/accessors.

Required initial run queries/events:

- archive reveal count;
- map entry/reveal;
- recipe availability;
- run loss thresholds;
- overseer defeat reward;
- event choice/target completion.

Gate: engine run code names operations, never specific relic/event/chain IDs.

---

## F3. Complete archive composition and Master's Notes

Replace archive hardcode with generic reveal-count fold:

1. base archive reveal count is one;
2. fire run-side `QUERY_ARCHIVE_REVEAL_COUNT` with `int *`;
3. Master's Notes adds one;
4. archive reveals up to final count of still-hidden recipes;
5. deterministic registry order or run RNG order is documented;
6. protocol emits every revealed result;
7. engine contains no `RELIC_MASTERS_NOTES` identity check.

This is preferable to an item-specific archive event because additional future
reveal modifiers compose on same count.

Gate: normal archive reveals one, Master's Notes reveals two, exhausted pool
stops safely, save/load preserves revealed recipes.

---

## F4. Complete deterministic next-hand pipeline

### Pre-roll model

Before current turn ends or before next draw interaction begins:

- build eligible pool through existing card eligibility query;
- deterministically roll upcoming hand once;
- store projected hand in `BattleState`;
- preview effects edit that stored hand;
- next draw consumes exact edited projection instead of rerolling;
- bonus draws after projection use same deterministic stream.

`QUERY_NEXT_HAND` receives mutable projected card list. Count and owner live in
fire context/accessors, not wrapper `x`.

### Counsel

Implement GDD literally (`res/GDD.md:314`):

- preview next turn's three cards;
- advertise those cards as `TARGET_CARD` choices;
- selected card is discarded from upcoming hand;
- remaining projected cards persist into next draw;
- no `QUERY_CARD_CAN_DRAW` veto approximation remains.

Required user decision before F4 implementation: GDD says discard one card
from the upcoming hand but does not say whether the vacated slot refills. Record
one approved rule before coding:

- next turn begins with the two remaining projected cards; or
- deterministic stream advances once and refills the third slot.

This plan chooses neither interpretation. Do not bake a default into projection
or draw-count ordering until the decision is recorded. Verification must cover
the approved rule with Rich Hand, Sparse Hand, and bonus draws.

### Librarian's Notes

Once per turn before draw:

- show top projected card;
- offer keep/skip interaction;
- skip removes top and advances deterministic stream once;
- decision persists into realized draw;
- no automatic skip and no reroll.

### Deep Hand

Provide generic active relic interaction, not a Deep Hand command:

- battle protocol lists activatable held effect items;
- player activates by generic item index/id;
- dispatch invokes held item's activation effect;
- Deep Hand draws two immediately on chosen turn;
- once-per-battle state prevents second use;
- extra cards obey play-or-sell requirement;
- activation costs no action unless GDD says otherwise.

Gate: previewed/edited hand equals realized hand for both human seat colors and
under Rich Hand, Sparse Hand, Kingdom Purity, and Lucky Strike.

---

## F5. Complete map visibility and Surveyor's Map

F2 already establishes persisted node visibility, successor reveal, hidden-node
targeting, selection rejection, and protocol masking. Current
`node->revealed = true` still conflates node and modifier knowledge. Extend the
F2 primitive rather than replacing it.

Implementation requirements:

1. add modifier visibility separate from node visibility;
2. keep battle modifier hidden until its normal reveal point;
3. Defector and similar effects reveal requested modifier data through generic
   run targets;
4. preserve F2 node-reveal event behavior unchanged;
5. save/load persists both visibility dimensions.

Surveyor's Map (`res/GDD.md:823`) effect:

- on first entry to each map, choose one random battle node deterministically;
- reveal that node's modifier, not every node and not node itself unless
  already visible;
- store per-map application so re-entering does not reveal another modifier.

Gate: map protocol demonstrates hidden, newly reachable, event-revealed, and
Surveyor-pre-revealed states.

---

## F6. Complete AI projection and Divination

Full AI archetype personalities remain outside effect composition gate, but
Divination cannot remain a no-op.

Implement minimum authoritative `AIPlan` used by actual current AI execution:

- planned buys;
- planned moves with from/to squares;
- planned card plays/sells when AI card logic exists;
- execution order;
- invalidation/replan rule when board changes.

AI stores plan before human preview point, then executes same plan unless a
specified invalidation occurs. Divination (`res/GDD.md:293`) reveals next-turn
intended moves and cards through effect/projection path. It does not calculate a
separate fake preview.

Gate: revealed plan and following AI turn match for deterministic unchanged
board; invalidation emits replan reason.

---

## F7. Complete Gold chain and recipe-forbid composition

### Gold chain

Implement complete GDD chain state (`res/GDD.md:653-657`):

- Gold/Subjugated kingdom track is locked;
- Liberation Trial node injects into lore-adjacent kingdom track;
- trial enemy starts with +2 pieces through army-count query;
- only subjugated kingdom cards are drawable through card eligibility query;
- win breaks Gold chain and unlocks track;
- loss schedules node respawn after three battles using `liberation_at`;
- other kingdoms' normal battles remain unaffected.

Map/entry engine fires generic run queries. Gold chain item supplies effects.
No `if chain == GOLD` behavior in generic map/battle code.

### Recipe forbid

Replace every-four-loss stub with real run effect:

- run loss event increments Vorath counter;
- at each new multiple of four, deterministically choose an allowed recipe;
- store forbidden recipe for rest of run;
- protocol emits identity;
- `battle_combine` fires `QUERY_RECIPE_ALLOWED` before commit;
- query frame exposes candidate recipe/result while `x` remains `bool *`;
- save/load persists forbidden set;
- exhausted recipe pool becomes safe no-op.

Global Vorath Counter is a run effect item containing both capacity and recipe
threshold effects. Engine fires loss and amount queries, never threshold
identity logic.

Gate: Gold trial lifecycle and 4/8/12-loss recipe bans pass save/load traces.

---

## F8. Complete event-elite armies

Current event elites reuse standard regional army. Replace with effect/data
setup owned by event option:

- **Mirage:** enemy pieces begin with Citadel immunity/immobility per GDD;
- **Pretender:** enemy setup mirrors player's piece identities and positions
  according to legal opposite-side transform;
- **Tournament:** fixed mid-tier Caelan army from GDD, not random region army.

Event option starts battle with a generic battle setup effect item/preset
pointer carried by pending run interaction. `battle_begin` consumes generic
setup data and never checks event ID.

Reward continuation remains typed run interaction from F2.

Gate: board snapshots prove each bespoke army/rule and both win/loss resume
paths.

---

## F9. Complete Overseer and Vorath effect items

Every boss uses existing source/header file count. Kingdom data files own
kingdom Overseer effect items; `universal.c` owns Vorath data. Generic battle
setup attaches them through same item path.

### Iron Strategist

- setup 3 Pao triangle and 2 Generals;
- split incoming meter damage equally across boss pieces through damage
  operation effects;
- track direct adjacent hits on Generals for three consecutive turns;
- each Pao owns once-per-battle active 3x3 salvo effect;
- King cannot collapse before both Generals satisfy/flip.

### Caravan of Conquest

- edge reinforcement every two turns through turn-phase effect;
- Salt Road diagonal board trait doubles movement for both sides;
- all boss pieces receive Double Time;
- scaling stops when battle ends.

### Many-Faced King

- setup three Prince king candidates;
- pre-flip redirect consumes would-be King flip into one remaining Prince;
- king status moves until all three are flipped;
- overflow redirect targets Prince share through effect state;
- battle ends only after all real-king candidates are exhausted.

### Eternal Recursion

- setup four Kinsho fortress;
- enemy pieces flipped away schedule return at end of next turn;
- scheduled identity/position restoration survives occupancy conflicts by
  documented placement rule;
- King flip still ends battle before recursion can restore it.

### Crowned Heretic

- setup full 8 Pawn, 2 Knight, 2 Bishop, 2 Rook, Queen, King army;
- each lost boss piece increments Ghost count;
- Ghost effect deals 10 per Ghost at start of human turn;
- Ghosts are off-board effect state, not fake pieces.

### Vorath finale

- 20x20 board through board-dimension query;
- 5 Minor Kings, Grand King, mixed five-kingdom army, and one free player
  piece per unchained figurehead;
- quadrant attack marks reset each turn;
- exact-zero Grand King rule;
- undershoot/overshoot distinction;
- overshoot flips random Minor King and resets Grand meter;
- successful exact zero after all five quadrants schedules next-turn Grand King
  attack that flips all Minor Kings;
- final result uses explicit battle origin, not pointer sentinel.

F9 must reuse F1's `QUERY_METER_AMOUNT` kind MAXIMUM seam and centralized meter
clamp for Grand meter initialization, exact-zero comparison, overshoot reset,
and later refill. Do not add a Vorath-only capacity or clamp path; modified
maximum must mean the same value throughout the transaction.

Boss rewards fire generic overseer-defeat run event:

- unlock capstone;
- add correct kingdom cards;
- unchain figurehead;
- persist progression.

No boss name/ID check enters `battle.c` or generic `run_battle_result`.

Gate: six deterministic boss traces cover setup, core mechanic, win, loss, and
reward persistence.

---

## F10. Composition-complete verification gate

Before simplification starts:

- every F0.5 fixture recipe reaches and executes its real transition;
- every effect-item row in `res/analysis.md` is ✅;
- no ⚠️, ❌, or ⬜ remains for any effect-composed card, relic, trait,
  modifier, chain, power, event, difficulty, challenge, Overseer, or Vorath
  item; Daily Conquest and Clockwork retain documented category-G non-effect
  status;
- all 60 event choices have runtime traces or generated public-protocol
  fixtures;
- all tier/mastery-gated cards and deep relics have traces;
- every boss and event elite has deterministic board/result evidence;
- save/load round-trips next hand, visibility, forbidden recipes, Gold chain,
  and progression state;
- grep finds no specific item identity in agnostic engine paths;
- `res/audit-baseline-v2.md` expands with post-fix evidence;
- behavior freeze is recorded before any simplification rename/fold begins.

This is hard gate. Simplification cannot use "feature still deferred" as reason
to choose an abstraction.

---

# Part I — Effect kernel

## E1. Replace subject-register choreography with one fire frame

### Current shape

`battle.c:19-33` stores current battle, subject, victim, card, buy template,
move origin, owner square, damagers, acting side, flipped piece, and shared
damager storage as file globals.

Effectful queries then reproduce a save/set/fire/restore protocol:

- move generation: `battle.c:2777-2799`;
- attack generation: `battle.c:2815-2837`;
- damage/value: `battle.c:2854-2878`;
- territory: `battle.c:2918-2954`;
- draw pool and view projection use the same pattern;
- action functions often set fields and reset them to null instead of
  restoring their previous values.

Nested paths already exist. `battle_lunge` can run inside a card effect, then
call `battle_attacks`, `battle_value`, `battle_damage`, and cascade. Some
functions therefore save prior values while others assume top-level entry.

### Composition failure

Context correctness depends on every caller remembering all relevant globals
and restoring them in every early return. A missed field leaks into a later
fire. A reset-to-null destroys an outer fire's context. Adding one new piece
of metadata requires editing many call sites.

The register names also hide trigger kind. `SUBJECT` means mover, flip
candidate, damage dealer, damage victim, or queried history owner depending on
the current trigger.

### Simplified shape

Define one `EffectFrame` type in `incl/representation.h`. It contains only
per-fire metadata:

- previous frame pointer;
- battle;
- operation kind or lifecycle phase;
- piece subject and secondary piece;
- card or buy template;
- move origin;
- queried owner/territory square;
- damager list;
- committed/evaluation mode where needed.

Use one nested-safe battle fire helper:

1. copy or link previous frame;
2. install the new frame;
3. call core dispatch;
4. restore the exact previous frame.

Existing accessors such as `battle_subject()` may remain during migration, but
they read the current frame. Add narrow accessors for operation kind and
phase. Do not pass a frame through `x`; `x` remains the value being folded.

### Preserved behavior

- Existing effect function signature remains usable.
- Nested fires see inner metadata, then outer metadata returns unchanged.
- Existing side-list selection and board-piece walk order remain initially.
- Protocol logs still name the applied effect and trigger family.
- Effects continue to reach battle through `battle_current()`.

### Affected mechanics

Every trigger consumer. Highest-risk examples are damage dealt/taken subject
swapping, move origin, Citadel gates, Trade Routes pricing, card target
selection, cascade damager attribution, and territory effects.

### Dependency

First code phase after baseline traces. Trigger folds and cascade cleanup both
need a stable metadata channel.

### Risk

A helper that resets fields rather than restoring the previous frame repeats
the current bug class under a new name. A single mutable global frame without
a previous-frame link remains non-reentrant.

### Verification

Add a deterministic trace where an effect query calls another effectful query.
Assert subject, victim, card, origin, and operation kind before the nested
call, during it, and after it. Grep should remove manual
`CURRENT_BATTLE = battle` choreography from ordinary call sites.

---

## E2. Use one effect invocation and logging path

### Current shape

`effect_run` performs trigger matching, calls the effect, and emits the effect
log (`effect.c:80-105`). Three callers reproduce part of that behavior:

- immediate `ON_CARD_PLAY` effects in `battle_play`
  (`battle.c:1312-1338`);
- `ON_CARD_TARGET_SELECTED` effects in `battle_card_target`
  (`battle.c:1427-1444`);
- `ON_EVENT_CHOOSE` effects in `run_event_choose`
  (`run.c:1143-1159`).

Each direct path constructs its own context and hand-writes the log string.

### Composition failure

An effect has two execution semantics: attached dispatch or direct template
invocation. Changes to applied-result logging, trigger naming, context copying,
or error handling must be repeated. Direct paths can silently diverge from
`effect_run`.

### Simplified shape

Expose one `effect_invoke` primitive that accepts an effect, trigger, context,
and `x`. It performs:

- function and trigger matching;
- effect body call;
- applied-result handling;
- protocol logging through the same trigger-name lookup.

`effect_fire` uses it while walking attached effects. Card and run paths use it
for one-shot template effects. Keep traversal separate from invocation.

### Preserved behavior

Applied effects log once. Self-filtered or unnamed effects stay silent.
One-shot event effects still use a stack copy of template state unless they
intentionally persist.

### Affected mechanics

All immediate cards, all targeted cards, and all 60 event choices.

### Dependency

Independent of trigger folds, but easier after E1 because one-shot calls can
install a proper frame.

### Risk

Calling one-shot effects through full seat dispatch would incorrectly fire
other attached effects with the same trigger. Reuse invocation, not traversal.

### Verification

Compare protocol logs before and after for one non-target card, one targeted
card, one self-filtered effect, and one event option.

---

## E3. Use one attachment primitive and copy template state correctly

### Current shape

Near-identical effect-slot loops exist in:

- `battle_walk_run` (`battle.c:1914-1943`);
- `battle_walk_run_effects` (`battle.c:1956-2006`);
- chain attachment in `battle_setup_armies` (`battle.c:2116-2133`);
- `battle_attach_power` (`battle.c:2167-2188`);
- `battle_walk_modifiers` (`battle.c:2321-2344`);
- `battle_walk_traits` (`battle.c:2427-2450`);
- `battle_attach_rule` (`battle.c:2462-2477`).

`effect_attach` copies `Effect`, then always allocates a zeroed context. It
does not copy a template's baked context (`effect.c:119-137`). Event
attachment compensates by manually copying `args[1..15]`
(`battle.c:1990-1996`). Other callers then set owner, target, mastery, or flags
by numeric index.

### Composition failure

Adding a new effect item category creates another loop. Template parameters
survive only when a caller knows to copy them. Ownership setup, duration
handling, and allocation failure policy vary by caller.

### Simplified shape

Make `effect_attach` clone baked template context when present, otherwise
create a zeroed context. Add one array attachment helper that receives:

- destination list;
- effect array;
- runtime owner/beneficiary;
- optional runtime mastery or target metadata;
- attachment mode when a category needs special consumption bookkeeping.

Event consumption remains event policy outside the generic helper. Template
parameter copying does not.

### Preserved behavior

- Owner remains in canonical runtime field/slot.
- Event parameters survive.
- Power mastery survives.
- Both-seat modifier and trait copies remain independent.
- Effects retain their authored duration.

### Affected mechanics

Relics, events, chains, modifiers, traits, rules, synergies, innates, climaxes,
Vorath capacity, and card-applied temporary effects.

### Dependency

Land with E2 or immediately after it.

### Risk

Blindly cloning `args[0]` can retain a static template placeholder where the
runtime owner must overwrite it. Document which fields are authored and which
are runtime-owned.

### Verification

Trace one baked event value effect, one mastery-scaled innate, one both-seat
modifier, and one temporary card effect. Grep should leave one generic slot
loop plus category selection logic.

---

## E4. Define attachment failure and dispatch-time mutation semantics

### Current shape

`effect_attach` correctly returns null when its own effect/context allocations
fail. After those succeed, it calls `ll_push`, which returns `void` and
silently exits when node allocation fails (`linked_list.c:37-58`). In that
specific failure path, `effect_attach` returns a non-null unlinked effect, its
allocations leak, caller believes attachment succeeded, and gameplay omits the
effect.

`battle_spawn` allocates each embedded effect context and immediately writes
`args[0]` without checking allocation (`battle.c:561-568`).

`effect_fire` advances with `node = node->next` after invoking an effect
(`effect.c:162-168`). If an effect appends another effect to the same list,
the new tail can become reachable in the same dispatch. This behavior is not
documented.

### Composition failure

Effect presence can depend on allocator failure without a controlled result.
Effects that spawn effects may run the new effect immediately or on the next
fire based on list topology rather than authored semantics.

### Simplified shape

- Make list insertion report success.
- Make attachment atomic: either list owns the complete effect/context pair or
  caller receives failure and nothing leaks.
- Make battle setup fail loudly or unwind when mandatory effect allocation
  fails; do not continue with a partially authored battle.
- Snapshot dispatch boundary at fire start. Effects attached during a fire
  become eligible on the next fire, not the current one.

### Preserved behavior

Effects may still spawn effects. New effects begin after their creator's
current trigger has finished, which removes accidental recursive eligibility.

### Affected mechanics

Free-move grants, redirects, refunds, adopted movement, Timur's Conquest,
Mingzhu's Seal, and any effect that attaches another effect.

### Dependency

Core effect cleanup. Establish before testing trigger folds.

### Risk

Some current mechanic may accidentally rely on same-fire eligibility. Audit
all `effect_attach` calls inside effect bodies before changing traversal.

### Verification

Unit-level allocator injection would be ideal. Until a test harness exists,
add focused list tests and traces proving newly attached effects wait until the
next matching fire.

---

## E5. Remove trigger-name positional coupling

### Current shape

`TRIGGER_NAME[]` is a plain positional array aligned manually with
`EffectTrigger` (`effect.c:13-78`, `representation.h:128-193`).

### Composition failure

Inserting or moving one enum without changing the string table at the same
position produces valid code and wrong logs.

### Simplified shape

Use designated initializers:

```c
[QUERY_CARD_DRAW_COUNT] = "QUERY_CARD_DRAW_COUNT",
```

Add a compile-time count assertion and a safe fallback for missing entries.

### Preserved behavior

Protocol names remain exact.

### Affected mechanics

All effect logs; no mechanics change.

### Dependency

Do before trigger consolidation.

### Risk

None beyond missing initializers, which the assertion/fallback exposes.

### Verification

Enumerate every trigger and assert a non-null name before running gameplay
traces.

---

## E6. Move runtime facts out of fixed piece effect slots

### Current shape

Move count, flip count, last move turn, and last flip turn are `eff_noop`
entries embedded in `Piece.effects[8]` (`battle.c:35-38`, `575-635`).
`piece_embed_effect` also stores one-shot grants in the same array and returns
null when full (`piece.c:260-282`).

A moved and flipped piece already spends four slots on facts. Authored piece
behavior exists in the array before runtime grants. Spent free-move effects
remain `ENTIRE_BATTLE` and occupy slots (`piece.c:313-335`).

### Composition failure

Bookkeeping competes with behavior. Failure is silent. The representation can
store counts but not a movement path, which Crusade now needs. No effect body
uses `QUERY_PIECE_HAS_MOVED` or `QUERY_PIECE_HAS_FLIPPED`, so those trigger
fires add no authored fold. Their accessor functions do have live consumers,
including Vengeance and Harushima reclaim/Fog Coast logic; those readers must
remain as pure typed-history reads.

### Simplified shape

Add typed runtime facts to `PieceInfo`:

- move count;
- flip count;
- last move turn;
- last flip turn;
- bounded current-turn landing history where a mechanic requests it.

Delete history mark tags and their piece-slot helpers. Make ordinary history
reads pure. Keep `effect_find_mark` only for genuine effect-to-effect shared
state, then consolidate repeated tag/subject lookup behind one named helper.
Attach temporary behavior to owning seat list with target pointer and real
duration, not to a fixed piece slot. If embedded runtime effects remain, give
them removal/ticking semantics and capacity failure visible to caller.

### Preserved behavior

All history-based mechanics observe the same counts and turns. Ordinary moves
and flips update facts exactly once after committed mutation.

### Affected mechanics

Vengeance, free moves, Double Time, movement copies, flip-count consumers,
Crusade, and any effect using `piece_embed_effect`.

### Dependency

Before Crusade and before deleting history triggers.

### Risk

`battle_lunge` currently skips move facts. Removing it changes Crusade to the
requested ordinary-move behavior. Other forced relocation paths must state
whether they count as moves.

### Verification

Trace ordinary move, free move, double move, flip redirect, consumed flip,
piece removal, and repeated grants. Confirm history never depends on effect
slot availability.

---

## E7. Reduce untyped context-slot coupling

### Current shape

Every effect receives `EffectContext { void *args[16]; }`. Shared conventions
exist only in prose: owner often uses `args[0]`, mastery often `args[1]`, and
mechanic-local state uses whichever later index is convenient.

### Composition failure

Slot meaning changes per effect. Copying, attachment, and review cannot tell
which fields are authored parameters, runtime ownership, target identity, or
mutable state. Wrong-slot bugs compile, including Bulk Discount's known reset
bug recorded in v1.

### Simplified shape

Do not replace every effect with a bespoke heap type. Keep compact private
state, but formalize common fields and accessors:

- owner/beneficiary;
- authored parameter range;
- runtime target;
- mutable flags/counters.

Use named enums or helpers instead of raw numeric indices in shared effects.
Mechanic-specific effects may retain local slots when naming them in the
function's header is clearer than a new global type.

### Preserved behavior

No payload or persistence change.

### Affected mechanics

All effects, especially shared helpers and event templates.

### Dependency

Can migrate gradually after E3.

### Risk

A total type-erasure rewrite would create more code than it removes. Keep this
as targeted normalization, not a new object system.

### Verification

Static search for raw `args[N]` in shared engine helpers should shrink. Every
remaining use must have named meaning in its function header.

---

# Part II — Trigger consolidation

## Folding rule

A trigger family may fold only when:

1. `x` keeps one concrete type across the family;
2. operation kind can live in `EffectFrame`;
3. subject orientation is explicit;
4. existing firing order can be reproduced;
5. effects do not need a wrapper payload to recover lost data.

Fewer enum values alone is not success. A false fold that makes every effect
inspect unrelated payloads is worse than two clear triggers.

## Complete migration matrix

| Current trigger(s) | Recommended result | Class |
|---|---|---|
| `QUERY_CARD_DRAW_COUNT` | Rename to hand/player draw-count query; keep distinct | Retain |
| `QUERY_CARD_PLAY_COST`, `QUERY_CARD_SELL_COST` | `QUERY_CARD_AMOUNT`, kind = play/sell | Fold |
| `QUERY_CARD_CAN_DRAW`, `QUERY_CARD_CAN_PLAY` | `QUERY_CARD_ALLOWED`, kind = draw/play | Fold |
| `QUERY_CARD_TARGETS` | Keep target enumeration | Retain |
| `QUERY_PIECE_ACTION_COST_MOVE/BUY/COMBINE` | `QUERY_ACTION_COST`, action kind | Fold |
| `QUERY_PIECE_CP_COST_BUY/RECLAIM` | Join piece numeric query | Fold |
| `QUERY_PIECE_CAN_FLIP/MOVE/ATTACK/BUY` | `QUERY_PIECE_ALLOWED`, capability kind | Fold |
| `QUERY_PIECE_MOVES`, `QUERY_PIECE_ATTACKS` | `QUERY_PIECE_PATTERN`, pattern kind | Fold |
| `QUERY_PIECE_DAMAGE_DEALT/TAKEN`, `QUERY_PIECE_VALUE` | Join piece numeric query | Fold |
| `QUERY_PIECE_HAS_MOVED/HAS_FLIPPED` | Delete; typed facts are pure | Delete |
| `QUERY_METER_DAMAGE_TAKEN`, `QUERY_METER_REFILL` | `QUERY_METER_AMOUNT`, amount kind | Fold |
| `QUERY_FLIP_COUNT` | Rename cascade flip-count query | Retain |
| `QUERY_CP_INCOME` | Keep player income query | Retain |
| `QUERY_ENEMY_ARMY_COUNT` | Keep setup army-count query | Retain |
| `QUERY_SQUARE_OWNER` | Keep territory query | Retain |
| `QUERY_BOARD_DIMENSION` | Keep board geometry query | Retain |
| `QUERY_BOARD_STATE` | Keep board projection query | Retain |
| `QUERY_HAND_STATE` | Keep hand projection query | Retain |
| `ON_BOARD_BUILD` | Keep board mutation phase | Retain |
| `ON_PIECE_FLIP_PRE` | Keep redirect/consume phase | Retain |
| `ON_PIECE_FLIP` | Keep immediate post-toggle event | Retain |
| `ON_PIECE_FLIP_POST` | Replace with `ON_CASCADE_END` | Reframe |
| `ON_PIECE_BUY/MOVE/COMBINE` | `ON_PIECE_ACTION`, action kind | Fold |
| `ON_CARD_PLAY`, `ON_CARD_SELL` | `ON_CARD_ACTION`, action kind | Fold |
| `ON_CARDS_DRAWN` | Keep realized-hand event | Retain |
| `ON_CARD_TARGET_SELECTED` | Keep target completion | Retain |
| `ON_COMBO_DOUBLE`, `ON_COMBO_CLIMAX` | `ON_COMBO`, combo count in frame | Fold |
| `ON_TURN_START`, `ON_TURN_END` | `ON_TURN_PHASE`, phase in frame | Fold |
| `ON_BATTLE_SETUP`, `ON_BATTLE_START` | `ON_BATTLE_PHASE`, explicit phase | Fold |
| `ON_BATTLE_END` | Keep winner event | Retain |
| `ON_EVENT_CHOOSE` | Keep run-immediate event | Retain |

Result reduces 50 triggers to a smaller set while preserving payload types and
lifecycle phases. Exact enum names can be chosen during implementation, but
family boundaries above are fixed by composition semantics.

---

## T1. Piece numeric query

### Current shape

Piece-related `int *` transforms are split across spawn value, outgoing
damage, incoming damage, buy CP cost, and reclaim CP cost. Effects wanting a
shared "piece amount" policy need multiple entries or target one stage only.

`battle_price` pre-applies home 60% or foreign 120% policy
(`battle.c:891-904`), then `QUERY_PIECE_CP_COST_BUY` runs
(`battle.c:983-995`). Trade Routes reverses the foreign multiplier in an
effect. Spawn value runs separately in `battle_spawn`
(`battle.c:541-555`). Damage runs offense then defense in `battle_value`
(`battle.c:2854-2872`).

### Composition failure

All five queries fold an `int *` around piece-related context, but they do not
share subject polarity: outgoing damage belongs to attacker side, incoming
damage belongs to victim side, and price/value queries may use a template with
no victim. Operation kind, firing side, and primary/secondary subject must stay
explicit; separate enum names are not needed once frame carries them.

### Simplified shape

Use one piece numeric query. `x` remains `int *`. Frame kind identifies:

- spawn/base value;
- outgoing damage;
- incoming damage;
- buy CP cost;
- reclaim CP cost.

Frame exposes primary piece/template, secondary piece, base amount, home
kingdom relationship, and evaluation/commit mode through accessors. The base
amount may be stored in frame metadata, but the mutable current value remains
`*x`.

### Preserved behavior

- Event value bonuses modify spawned runtime value before meter calculation.
- Outgoing effects run before incoming defense.
- Effective value with no victim remains distinguishable.
- Home/foreign pricing remains engine policy.
- Trade Routes removes foreign markup only.
- Reclaim base remains 30 before effects.

### Affected mechanics

Event value/cost bonuses, Queen's Decree, Cathedral, Caelan climax, Trade
Routes, Bulk Discount, damage modifiers, and display/meter reads.

### Dependency

E1 first. Migrate all consumers atomically because leaving old and new triggers
would double-apply effects.

### Risk

Additive and multiplicative effects are non-commutative. A value bonus before a
percentage damage bonus differs from the reverse. Preserve existing stage and
list order first; improve reducer rules only with explicit traces.

### Verification

Trace one base-value bonus, one buy discount, one outgoing multiplier, one
incoming reduction, one effective-value meter read, and one reclaim discount.

---

## T2. Piece capability query

### Current shape

Move, attack, flip, and buy vetoes are four boolean triggers. Citadel attaches
three copies of the same gate body for move, attack, and flip
(`zarqan.c:869-888`).

### Composition failure

"This piece cannot act" is represented as parallel effects. New capability
kinds multiply effect entries and attachment work.

### Simplified shape

Use one `bool *` capability query. Frame kind identifies move, attack, flip,
or buy. One effect may reject one kind or a set of kinds.

### Preserved behavior

- Veto begins from `true`.
- Effects only move toward denial; later effects must not re-enable a veto
  unless a mechanic explicitly owns override authority.
- Cascade still distinguishes non-kings that exist but cannot flip from a side
  with no non-king pieces.

### Affected mechanics

Citadel, Mingzhu's Seal, Last Stand, Pacifist, Solo Vanguard, and movement or
attack locks.

### Dependency

E1 and a boolean reducer contract.

### Risk

Current effects can overwrite `false` with `true`. Define veto composition as
logical AND to eliminate order-dependent re-enabling.

### Verification

Trace every capability on a Citadel target and confirm unaffected capabilities
on ordinary pieces.

---

## T3. Action-cost query

### Current shape

Move, buy, and combine action costs each have an `int *` trigger. Free-move and
double-move effects write zero. Other mechanics may add or replace cost.

### Composition failure

Shared "next action is free" behavior needs operation-specific effect
instances. Absolute writes depend on attachment order. Query evaluation may
consume a grant before action commit.

### Simplified shape

Use one action-cost query with action kind in frame. Separate evaluation from
commit:

1. evaluate cost without mutating effect state;
2. verify resources and all action preconditions;
3. commit action;
4. notify the effect that its grant was consumed through the committed action
   event.

Define cost reducer as base plus modifiers followed by one final clamp at zero.
An explicit exact-cost mechanic may replace the final result only in a named
stage.

### Preserved behavior

Normal actions cost one. Free grants cost zero once. Double Time alternates as
currently intended. Failed actions do not spend a grant.

### Affected mechanics

Free Move, Double Time, Pawn Storm, Reforge-related purchases, combination
cost effects, Harushima reclaim flows, and Crusade.

### Dependency

Committed/evaluation mode in E1, then piece action event fold.

### Risk

Changing when a grant arms or spends can alter turn sequencing. Build traces
before migration.

### Verification

Try affordable, unaffordable, illegal, and legal actions under each free-action
mechanic. Confirm only committed actions consume state.

---

## T4. Piece pattern query

### Current shape

`battle_moves` and `battle_attacks` duplicate:

- frame save/set;
- capability gate;
- scratch reset;
- base generator call;
- list-edit trigger;
- frame restore.

Movement and attack copying also exist as paired effect bodies in
`piece.c:367-437`.

### Composition failure

Every movement-copy mechanic needs a move half and attack half. Scratch-buffer
rules are repeated. Callers cannot request attack coverage from a virtual
square, blocking clean Crusade path resolution.

### Simplified shape

Create one pattern evaluator parameterized by move/attack kind and origin
square. Public wrappers may remain for readability. A virtual-origin request
uses a local/shadow `PieceInfo`, never temporary board mutation. Result
ownership and scratch lifetime are explicit.

Fold list-edit triggers into one `Square *` pattern query with kind in frame.
Shared copy effects inspect kind and call one helper.

### Preserved behavior

- Move lists contain legal empty destinations.
- Attack lists contain empty and enemy squares, never friendly squares.
- Existing replacement and append effects retain order.
- Shared scratch remains valid only until next pattern evaluation unless the
  implementation replaces it with caller-owned storage.

### Affected mechanics

All pieces, copied movement, Gold Standard, Promotion, Conquest, Ambition,
Divine Right, River Crossing, Trade Route, Citadel, and Crusade.

### Dependency

E1 and typed operation kind. Crusade depends on virtual-origin attacks.

### Risk

Recursive pattern generation currently resets one global scratch buffer.
Nested copies must use caller-owned temporary storage or an explicit stack.

### Verification

Compare move/attack lists for base, additive copy, replacement copy, virtual
origin, and nested pattern effects.

---

## T5. Delete fake history queries

### Current shape

`battle_piece_moves` and `battle_piece_flips` read mark values, then fire
`QUERY_PIECE_HAS_MOVED` or `QUERY_PIECE_HAS_FLIPPED`. No authored effect body
uses either trigger, but multiple mechanics call the accessor functions.

### Composition failure

Pure facts appear effectful and inherit register/logging/reentrancy cost for no
behavior.

### Simplified shape

Delete both triggers after moving counts into `PieceInfo`. Rename readers as
pure history lookups.

### Preserved behavior

Current move and flip counts remain exact.

### Affected mechanics

Vengeance, Harushima reclaim/flip selection, Fog Coast, and every other caller
of move/flip history accessors. Accessors remain; only empty trigger dispatch
disappears.

### Dependency

E6.

### Risk

Re-scan data files immediately before implementation in case a new consumer
lands.

### Verification

Grep shows no trigger references; history traces remain unchanged.

---

## T6. Card amount and eligibility folds

### Current shape

Play cost and sell value are separate `int *` queries. Draw eligibility and
play eligibility are separate `bool *` queries. Card target enumeration is a
`CardTarget *` protocol and draw count is a player/hand integer.

### Simplified shape

- Fold play and sell into card amount with kind.
- Fold can-draw and can-play into card eligibility with kind.
- Keep draw count distinct because its subject is a hand/turn, not one card.
- Keep targets distinct because it owns a multi-step typed list.

### Preserved behavior

Hand projection and playability dry-runs remain non-consuming. A veto remains
monotonic.

### Affected mechanics

Card costs, sales, Counsel's current draw veto, hand restrictions, and all
card play checks.

### Dependency

E1 and query evaluation rules.

### Risk

`battle_card_can_play` currently fires play cost twice across check and commit.
Any stateful cost effect must be fixed before fold.

### Verification

Trace displayed playability, actual play payment, autosell, manual sell, and
card draw filtering.

---

## T7. Piece action event

### Current shape

Buy, move, and combine each fire a `PieceInfo *` event. Move adds origin via a
register. Combine result semantics and destination are implicit.

### Composition failure

An effect observing any committed piece action needs parallel triggers. Action
cost consumption has no shared committed event.

### Simplified shape

Use one piece action event. `x` remains the committed `PieceInfo *`. Frame kind
identifies buy, move, or combine and carries origin/result metadata.

### Preserved behavior

Events fire only after successful mutation and payment. Move origin remains
available. Combine result remains the new piece on square `b`.

### Affected mechanics

Movement observers, purchase observers, combination observers, free-action
consumption, river crossing, and sequence effects.

### Dependency

Relocation primitive and E1.

### Risk

Do not fire a move event for swaps until every move-sensitive consumer is
classified.

### Verification

Trace failed and successful forms of each action. Only successful actions emit
one event.

---

## T8. Card action event

### Current shape

Play and sell both use `Card *`, while drawn cards and selected targets use
different payloads.

### Simplified shape

Fold play and sell into one card action event with action kind. Keep drawn
cards and target selection distinct.

### Preserved behavior

Immediate card effects still run before durational attachment and generic card
observers at the same points as now. Autosell remains a sell action if current
observers expect it.

### Risk

Current `battle_half_turn` autosell fires sell events. Preserve that distinction
from manual sell only if no effect differentiates them; otherwise frame needs
sell source.

---

## T9. Combo event

### Current shape

The same `KINGDOM_PLAYS[]` counter emits `ON_COMBO_DOUBLE` at count two and
`ON_COMBO_CLIMAX` at count three (`battle.c:1349-1360`). Both carry
`KingdomID *`.

### Simplified shape

Use one combo event. `x` remains `KingdomID *`; frame exposes combo count.
Refund self-filters on count two. Kingdom climaxes self-filter on count three
and kingdom.

### Preserved behavior

Second play grants +15 CP exactly once. Third play fires one matching kingdom
climax. Counter still resets each turn.

### Risk

`KINGDOM_PLAYS` is file-static rather than battle state. Move it into battle
turn state during the same phase.

---

## T10. Turn phase

### Current shape

Start and end carry the same turn number but use separate triggers.

### Simplified shape

Use one turn-phase event. `x` remains the current turn encoded as today; frame
phase identifies start/end.

### Preserved behavior

Income/draw/start effects occur before actions. Autosell and end effects occur
before resolve in current order.

### Risk

Do not treat start and end as interchangeable. This is enum consolidation with
an explicit phase, not lifecycle collapse.

---

## T11. Battle phase

### Current shape

`ON_BATTLE_SETUP` fires for the human before initial meters
(`battle.c:2631-2636`). Baseline Vorath pressure attaches after meter
initialization, then `ON_BATTLE_START` fires for both seats
(`battle.c:2638-2653`). F1 intentionally replaces that pressure gain with a
maximum/capacity effect attached before initial meter calculation.

### Composition failure

Two names encode one lifecycle event family, but folding them naively would
move setup effects across meter initialization and change behavior. Preserving
the pre-F1 Vorath placement would also regress F10 behavior.

### Simplified shape

Use one battle-phase event with explicit pre-meter and post-meter phases.
`x` remains `MapNode *`. Keep two emission points in the ordered setup
schedule. Attach run effects needed by maximum queries before meter
calculation; do not recreate a post-meter pressure step.

### Preserved behavior

- Enslaved and Traitor's Gambit run pre-meter.
- Spawned setup pieces contribute to initial maximum.
- Vorath capacity is attached before and contributes to initial maximum.
- Post-meter start effects run after meter initialization.
- Side selection remains human-only pre-meter and both-seat post-meter unless
  a mechanic explicitly changes it.

### Risk

The trigger count drops, but call count should not. Removing one emission is a
behavior bug.

---

## T12. Meter amount

### Current shape

Damage taken and refill are separate `int *` receiver-side queries. Both alter
meter-related amounts, but occur at different cascade stages.

### Simplified shape

Use one meter amount query with damage/refill kind. Keep cascade flip count
separate because it controls candidate count rather than meter quantity.

### Preserved behavior

Damage interceptors run before subtraction. Refill modifiers run after each
flip step. Meter clamping remains a mutation concern, not an effect query.

### Risk

Effects must self-filter by kind. A damage reducer must never run on refill.

---

## T13. Replace post-flip with real cascade completion

### Current shape

`ON_PIECE_FLIP_POST` is documented as post-cascade-step, then fired once per
toggled piece after refill (`battle.c:1703-1716`). Current sole consumer is
Timur's Conquest (`zarqan.c:927-1008`), which ignores `x` and checks whether
its side's meter is below 20.

With multi-flip, it may run twice for one step. With a large deficit, cascade
may run multiple steps. It is therefore neither simply "piece flip post" nor
"cascade end."

### Composition failure

Name, payload, and frequency disagree. A cascade observer receives arbitrary
piece identity even when it only cares about settled meter state.

### Simplified shape

Delete this per-piece event. Emit `ON_CASCADE_END` once after a complete
cascade call settles or terminates. `x` can remain the receiver's mutable or
readable battle value chosen by the final design, but no wrapper is needed;
receiver and threshold facts live in frame/cascade state.

For Timur's Conquest, track whether its meter crossed below 20 during damage
and cascade. At cascade completion, the effect observes that fact and triggers
Royal Substitution once. This matches GDD wording at `res/GDD.md:972` more
closely than checking whichever intermediate refill happens to be below 20.

### Preserved behavior

- Flip redirection/consumption remains `ON_PIECE_FLIP_PRE`.
- Immediate after-toggle effects remain `ON_PIECE_FLIP`.
- Timur's Conquest remains automatic and once per attached watcher.
- Multi-flip no longer repeats a cascade-level observer accidentally.

### Dependency

Cascade local state and E1.

### Risk

Threshold semantics are currently imperfect. Capture pre-damage meter,
post-damage meter, and cascade settlement so GDD behavior is explicit.

### Verification

Test damage that crosses below 20 and refills above it, damage that never
crosses, multi-flip, redirected flip, consumed flip, and repeated cascades.

---

## Retained trigger distinctions

Do not fold these merely because names look related:

- board dimension, board state, hand state, and board build use different
  payloads and phases;
- card targets use a typed, multi-step list;
- cards drawn uses `Card **`, not one card;
- battle end carries winner rather than `MapNode *`;
- event choose is run-side immediate execution;
- flip pre and immediate flip have redirect versus post-toggle semantics;
- cascade flip count controls number of mutations, not a meter amount;
- CP income, army count, and territory have different subjects and reducers.

A smaller enum is useful only when effect authors gain a clearer generic
operation.

---

# Part III — Query composition and order

## Q1. Current order is part of behavior but not a declared contract

`effect_fire` walks:

1. fired side's player list in linked-list insertion order;
2. every board cell from index 0 through `MAX_BOARD_SIZE - 1`;
3. each live piece's effect slots from 0 through 7.

Baseline battle setup determines player-list order:

1. relics;
2. persistent event effects;
3. modifier;
4. trait;
5. difficulty/challenge rules;
6. chain effects during army setup;
7. innates;
8. synergies;
9. combo effects;
10. Vorath pressure later.

F1 intentionally replaces item 10: Vorath maximum/capacity must attach before
initial meter calculation, and no later pressure gain remains. F10 ordering,
not this pre-fix baseline order, becomes simplification contract.

No central contract states whether remaining order is intended. Additive effects
usually commute. Multipliers, division, clamping, replacement, state
consumption, and list rewriting do not.

## Q2. Required reducer contracts

Document one contract for each consolidated family.

### Boolean veto

Start `true`. Every effect may only preserve or set `false`. Final result is
logical AND. Re-enabling a veto is forbidden unless a distinct override phase
is introduced for a real mechanic.

### Additive count

Start from engine base. Effects add/subtract. Clamp once at operation boundary,
not after each effect. Examples: CP income and army count.

### Cost

Start from base/policy cost. Apply additive and percentage modifiers in a
specified stage order, then clamp to zero. Free-action/free-cost grants apply
at final override stage and consume only on commit.

### Damage/value

Keep explicit stages:

1. runtime base value;
2. outgoing source modifiers;
3. incoming victim modifiers;
4. meter receiver modifiers;
5. subtraction and cascade.

Do not flatten these into one unordered list merely because all use `int *`.
One trigger family may still expose operation stage through frame kind.

### List edit

Specify append, prune, and replace semantics. Replacement resets the list;
later append effects see the replacement. Scratch-buffer ownership must be
safe under nested generation.

### Lifecycle event

Events do not reduce a value, but phase and side are explicit. Effects may
attach future effects; new attachments begin on the next fire.

## Q3. Query evaluation must be non-consuming

A function named or classified as evaluation may not permanently consume an
effect grant. Current danger points:

- `battle_card_can_play` evaluates cost, veto, and all target steps before
  actual play (`battle.c:1217-1273`);
- protocol projection may call move, cost, or view queries repeatedly;
- `battle_meter_max` repeatedly invokes piece damage/value effects;
- cascade computes maxima before and after flips;
- AI planning can evaluate actions that are never committed.

State consumption belongs to committed action/event notification. If an effect
must predict a result, evaluation may use temporary local state and discard
it.

## Q4. Effect logs distinguish evaluation from application

Current query effects log whenever their body returns true, including repeated
reads. This can produce protocol noise and make a pure UI refresh appear to
apply gameplay repeatedly.

Audit each query effect's return value. Recommended rule:

- query evaluation logs only when protocol explicitly needs an explanation;
- committed action/event effects log application;
- repeated projections do not imply repeated consumption.

If existing logs are externally consumed, preserve format during refactor and
change logging policy only in a separate phase.

---

# Part IV — Crusade

## C1. Current implementation

GDD states:

> Target Knight makes 3 consecutive L-moves this turn, each attacking on
> resolution. (`res/GDD.md:345`)

Current card targeting correctly offers friendly Knights
(`caelan.c:827-845`). Resolution then:

1. finds enemy king;
2. repeats three times;
3. generates legal moves;
4. greedily picks the move with shortest Chebyshev distance to king;
5. calls `battle_lunge` (`caelan.c:859-905`).

`battle_lunge` directly moves board pointers, fires `ON_PIECE_MOVE`, resolves
one piece's coverage, applies meter damage, and cascades
(`battle.c:1847-1903`).

## C2. Why it does not compose

- One card owns a public engine primitive.
- Player does not choose the three moves.
- Movement bypasses action-cost query.
- Movement does not stamp move count or last move turn.
- It duplicates strike and damage logic.
- It resolves and cascades immediately instead of using ordinary half-turn
  resolution.
- If no move exists, destination remains current square and coverage still
  resolves, allowing stationary repeated attacks.
- Other move-sensitive effects see an incomplete version of movement.

`battle_lunge` is the clearest example of a special function created where a
small set of temporary effects and ordinary primitives would compose better.

## C3. Recommended effect-composed sequence

### Selection

Keep existing `QUERY_CARD_TARGETS` selection of one friendly Knight. On final
selection, attach one temporary Crusade sequence to acting seat. Sequence
state owns:

- selected stable piece pointer;
- remaining committed moves;
- three landing squares;
- original turn;
- cancellation/completion status.

Do not store sequence state in Knight's fixed effect array.

### Movement gate

On unified piece capability query with kind MOVE:

- allow selected Knight;
- veto every other piece while sequence is active;
- leave non-movement actions unchanged because user restriction concerns
  other moves, not buys, cards, sells, or end-turn commands.

### Action cost

On unified action-cost query with kind MOVE:

- selected Knight's first Crusade move resolves to exactly one action;
- second and third resolve to zero;
- failed or merely previewed moves do not advance sequence;
- move counter advances only from committed piece action event.

The reducer contract must state whether a pre-existing free-move effect can
alter the first cost. User wording says first move costs one, so Crusade's
exact-cost stage wins for that move; later two are exact zero.

### Ordinary movement

Every hop uses normal move legality and committed relocation:

- Knight L-move generator;
- bounds and occupancy;
- action payment;
- board mutation;
- move count and last-turn update;
- move event emission.

No greedy enemy-king path remains. Player selects each landing.

### Path damage

Record each post-move landing. Exclude square occupied before Crusade, because
GDD promises three move attacks, not four positions.

At ordinary half-turn resolution:

- current square contributes third landing's coverage;
- stored first and second landing squares contribute their coverage;
- each landing is evaluated separately;
- if one victim is covered from two landings, its damage is counted twice,
  preserving one attack contribution per move;
- outgoing and incoming damage effects run for every contribution;
- resulting total enters the single meter damage/cascade path once.

Use generic virtual-origin pattern evaluation. Never move the live piece
backward temporarily and never edit board occupancy to fake an old square.

### Completion and cancellation

After third committed move, release movement gate but keep stored path until
that half-turn resolves. Clear sequence after resolve.

If sequence cannot complete:

- never generate a stationary attack;
- allow explicit turn end/cancellation;
- retain only landing squares actually reached;
- unlock other movement when sequence cancels;
- resolution counts only completed Crusade moves.

This policy avoids deadlock while refusing fake attacks. Card target
playability may later gain path lookahead, but lookahead is not required for
the first simplification pass.

If selected Knight flips, is removed, or is replaced before completion,
cancel sequence safely. Deferred freeing means target lifetime must be checked
by board membership, not pointer non-nullness alone.

## C4. Preserved and intentionally changed behavior

Preserved:

- one friendly Knight target;
- up to three consecutive legal L-moves this turn;
- one attack contribution per completed landing;
- ordinary damage modifiers and cascade rules;
- total action cost of one for a complete sequence.

Intentionally changed by user direction:

- moves are player-controlled, not greedy automatic moves;
- damage waits for ordinary resolution;
- moves use normal history and action plumbing;
- stationary boxed attacks disappear;
- `battle_lunge` disappears.

## C5. Verification matrix

| Case | Expected result |
|---|---|
| Open board | Three selected L-moves, action delta 1, three landing attacks at resolve. |
| Other piece move | Rejected while sequence active. |
| Buy/play/sell | Unchanged unless another rule rejects it. |
| Same victim covered twice | Two damage contributions. |
| Boxed before first move | No stationary attack; cancellation/end turn remains possible. |
| Dead end after one move | One landing contribution only; no deadlock. |
| Knight flips | Sequence cancels safely. |
| Knight removed | No stale-pointer dereference. |
| Other free-move effect | Crusade's exact 1/0/0 contract wins. |
| Damage modifier | Applied independently to each landing contribution. |
| Cascade | Runs once from total half-turn damage path. |

---

# Part V — Battle engine simplification

## B1. One committed relocation primitive

### Current shape

Ordinary movement validates generated moves, evaluates cost, mutates board,
fires move event, and stamps history in one function
(`battle.c:812-874`). It also contains a friendly-swap branch
(`battle.c:849-861`). Base move generation excludes friendly occupied squares,
but every `QUERY_PIECE_MOVES` editor must be checked before declaring branch
unreachable.

`battle_swap` directly swaps pieces and emits no move event. `battle_lunge`
implements another relocation. Several card effects remove and respawn pieces
to represent transformation.

### Simplified shape

Separate stages:

1. lookup acting piece;
2. evaluate legal destinations;
3. evaluate cost;
4. commit relocation;
5. update runtime history;
6. emit committed piece action.

One internal relocation mutator changes board pointers and coordinates. Callers
state whether relocation is a move, swap, forced displacement, or setup. Only
committed moves pay cost and update move history.

Audit all base generators and `QUERY_PIECE_MOVES` editors for friendly-square
emission. If none can produce one, delete dead branch. Keep `battle_swap`
semantics until all move observers are audited; then either emit two
relocation events with SWAP kind or explicitly document that swapping is not
movement.

### Affected mechanics

Castling, Royal Decoy, Royal Substitution, Zarqan climax, River Crossing,
Vengeance, free moves, copied movement, and Crusade.

### Risk

Making swaps look like normal movement can trigger effects that never fired
before. That is not a mechanical rename. Audit every move observer first.

---

## B2. One strike evaluator

### Current shape

`battle_resolve` and `battle_lunge` both:

- generate coverage;
- copy shared scratch into local storage;
- find victims;
- skip friendly/neutral pieces;
- call `battle_value`;
- sum damage;
- record damager identity.

### Simplified shape

Create one strike evaluator accepting piece and origin square. It returns
contribution and records attribution in caller-owned resolve state. Full-side
resolve iterates pieces. Crusade iterates additional origins for its selected
piece. No special strike function applies meter damage itself.

### Preserved behavior

Damage from each covered enemy piece uses outgoing then incoming effects.
Neutral and friendly pieces remain non-victims. Damager attribution remains
available through cascade completion.

### Risk

Deduplicating target squares would change multi-origin Crusade behavior. Keep
one contribution per origin/coverage occurrence.

---

## B3. One meter damage path

### Current shape

Half-turn resolve fires meter damage query, subtracts inline, logs, then calls
cascade (`battle.c:1766-1785`). `battle_damage` repeats that pipeline for card
and relic damage (`battle.c:1807-1831`).

### Simplified shape

Use one apply-meter-damage primitive:

1. reject or normalize non-positive base amount;
2. install receiver/damager frame;
3. evaluate meter damage amount;
4. record threshold crossings;
5. subtract once;
6. log once;
7. cascade once;
8. emit cascade completion once.

Allow caller to choose immediate cascade versus deferred half-turn only if an
existing mechanic requires that distinction. Current direct damage expects
immediate cascade; ordinary resolve calls same primitive at half-turn end.

### Preserved behavior

Card/relic damage cascades immediately. Ordinary attack damage cascades at
half-turn resolution. Receiver-side interceptors run before subtraction.

### Risk

`amount <= 0` handling currently differs. Define and test zero/negative results
rather than inheriting divergence.

---

## B4. Cascade as explicit local state

### Current shape

`battle_cascade` mixes:

- deficit tracking;
- candidate scanning;
- capability queries;
- king-loss detection;
- random choice;
- multi-flip count;
- redirected/consumed flip results;
- refill;
- opposing meter gain;
- 200% clamp;
- post-step event emission.

`battle_flip` writes global `FLIPPED_PIECE`, which cascade reads after return.
A nested flip can overwrite it.

### Simplified shape

Use local cascade state containing receiver, deficit, candidates, toggled
pieces, threshold crossings, damagers, and completion reason. Split helpers:

- collect candidates;
- choose and apply one flip batch;
- calculate refill;
- transfer gained maximum;
- clamp meter;
- test continuation/termination;
- emit one completion event.

Make flip return actual toggled piece or null. Remove `FLIPPED_PIECE`.

### Preserved behavior

- `QUERY_PIECE_ALLOWED` can veto flip candidates;
- no non-kings means king flips and battle ends;
- non-kings that all veto remain a non-loss zero-meter state;
- Bloodbath-style multi-flip remains supported;
- redirect and consume remain supported;
- deficit carries through refill;
- gainer meter changes by actual maximum delta;
- meter never exceeds 200% of current maximum.

### Risk

A refill of zero with remaining candidates can loop forever. Add explicit
progress assertion/termination policy and trace it.

---

## B5. Centralize meter mutation and clamping

`battle_meter_gain` clamps at twice maximum (`battle.c:341-349`), while cascade
repeats clamp logic (`battle.c:1695-1701`). Other code adjusts meter directly
when buying, combining, and resolving.

Create narrow primitives for:

- gain with clamp;
- loss through damage pipeline;
- maximum delta after piece ownership/value change;
- initialization without gain semantics.

Do not hide all meter writes behind one vague setter. Name each mutation's
reason.

---

## B6. One card target-step interpreter

### Current shape

Target-step walking appears in:

- `battle_card_can_play` dry-run (`battle.c:1242-1267`);
- initial `battle_play` setup (`battle.c:1304-1308`);
- `battle_card_target` continuation (`battle.c:1410-1423`).

Seven pending-card file statics exist (`battle.c:88-94`): `PENDING_CARD`,
`PENDING_SIDE`, `PENDING_TARGETS`, `PENDING_TARGET_COUNT`, `PENDING_PICKS`,
`PENDING_STEP`, and `PENDING_STEP_COUNT`.

### Simplified shape

Move pending card selection into typed battle interaction state. One iterator:

- builds one step;
- reports no target, one/many targets, or completion;
- accepts a selected index;
- advances over empty optional steps according to one rule;
- supports non-mutating feasibility simulation with a local copy.

Centralize hand-index lookup and card existence validation for play, sell, and
playability.

### Preserved behavior

Multi-step cards retain prior picks. Protocol still receives target lists and
returns an index. Play never parks on an impossible mandatory step.

### Risk

Dry-run must not mutate persistent effect context or global pending state.

---

## B7. Separate action validation from mutation

Move, buy, combine, play, sell, and reclaim each hand-code overlapping checks:
acting side, hand range, card existence, unlock, resources, board occupancy,
and effect veto.

Introduce focused evaluators returning a reason/result, then commit only after
all checks pass. This enables UI/AI to ask the same legality question without
consuming effects.

Do not create one giant generic action union. Reuse small checks:

- acting piece at square;
- hand card at slot;
- unlocked piece identity;
- affordable amount;
- legal empty destination;
- committed action notification.

---

## B8. One round-advance state machine

### Current shape

`battle_begin` contains opening white turn and optional AI white half.
`battle_end_turn` repeats white-first AI/human sequencing
(`battle.c:2655-2667`, `2719-2758`). `ACTING_SIDE` and `HUMAN_SIDE` are hidden
file globals.

### Simplified shape

Store human side, acting side, and round state in `BattleState`. Use one
advance function that:

1. finishes current half;
2. checks battle end;
3. starts/plays remaining side if AI;
4. increments round after black half;
5. checks turn-ten territory finish;
6. starts next white half;
7. continues AI automatically until human control returns.

Opening battle enters the same state machine at "before white start" rather
than reproducing it.

### Preserved behavior

Strict white-first order, alternating human seat by battles fought, AI action
order, resolve timing, and turn-ten territory scoring.

### Risk

Winner passed to `battle_finish` must remain actual winning side. Current code
uses side names in several early returns; verify each path.

---

## B9. Decompose battle setup without changing order

`battle_begin` currently allocates state, chooses human side, sizes board,
seeds RNG, attaches effects, applies dimension, spawns kings, builds board,
sets armies, attaches powers, runs pre-meter effects, initializes meters,
attaches baseline pressure, runs post-meter effects, and starts turn flow
(`battle.c:2567-2668`). F1 removes that post-meter pressure stage.

Split F10 behavior into named stages with one explicit schedule:

1. allocate and bind battle;
2. derive immutable setup inputs from run/node;
3. attach pre-board/run effect sources, including maximum/capacity effects;
4. query dimensions;
5. spawn kings;
6. build board;
7. attach chains and create armies;
8. attach innates, synergies, and combos;
9. fire pre-meter battle phase;
10. initialize meter maxima through the amount query;
11. fire post-meter battle phase;
12. enter round state machine.

Do not convert schedule into an opaque callback array. Named calls make
ordering reviewable.

---

## B10. Remove hidden battle singleton state where ownership is clear

Move into `BattleState`:

- human side;
- acting side;
- RNG state;
- combo counters;
- pending card interaction;
- reap queue if it cannot be local;
- engine or run reference needed during battle.

Keep only the minimal current fire-frame pointer needed by legacy effect
accessors. This makes multiple battle instances and isolated tests possible
without rewriting every effect signature.

---

## B11. Keep battle file count unchanged

Do not split `battle.c` into new modules. Simplification target is fewer
primitives and clearer responsibilities, not more files.

Keep one `battle.c`, but organize it with existing section-header style and
small helpers in dependency order:

1. fire-frame accessors and local state;
2. pure board lookups;
3. effectful evaluations;
4. committed actions;
5. strike, damage, flip, and cascade;
6. setup helpers;
7. turn/lifecycle orchestration;
8. public wrappers.

Delete obsolete helpers after callers move. Do not preserve old and new paths
as compatibility layers. All structs, enums, and public declarations remain
in headers; `battle.c` contains implementations and variable declarations.

---

# Part VI — Run engine simplification

## R1. Delete dead `RUN_ENGINE`

`RUN_ENGINE` is declared at `run.c:394` and assigned at `run.c:753`, with no
read. Delete it. `EngineState *` is already passed to run operations.

Risk: none after grep confirmation.

---

## R2. Replace pending globals with owned interaction state

### Current shape

`run.c:394-402` stores:

- entered kingdom;
- pending node;
- two pending relics;
- relic-offer flag;
- synthetic Vorath node;
- removal countdown;
- event reward integer;
- reward-active flag.

Different functions arm and consume different subsets. `PENDING_RELICS` has
three producers. `EVENT_REWARD` uses magic 0/1. Node identity and reward state
can become stale independently.

### Simplified shape

Add typed run interaction state owned by `RunState` or `EngineState`:

- interaction kind;
- pending node;
- owning event/node source;
- relic choices;
- remaining removal count;
- post-battle reward kind;
- active/complete status.

Use declared enums for interaction and reward kinds. One transition function
moves interaction forward; unrelated commands cannot consume it.

Do not build a generic UI framework. This state only models existing run
suspension: event choice, card removal, relic offer, and event elite result.

### Preserved behavior

Existing protocol commands continue to resolve pending choices. Save/load
scope remains unchanged unless pending interactions are intended to persist.

### Risk

Event elite battles suspend an event across battle lifetime. Preserve the
pending node until both battle result and reward choice finish.

---

## R3. Centralize pending node completion

### Current shape

Event node completion occurs after direct choice, final removal, or relic pick
in three functions. Each site checks a different subset of pending conditions.

### Simplified shape

Use one `run_try_complete_interaction`/node completion helper. It clears a node
only when:

- no battle is active;
- no reward choice is pending;
- no removal remains;
- no required continuation remains.

All event resolution paths call it after changing interaction state.

### Preserved behavior

Nodes remain uncleared while player input or battle reward is pending. Map is
emitted once after completion.

### Risk

A helper that emits map on every state change can duplicate protocol output.
Completion owns the final emit.

---

## R4. Split `run_battle_result`

### Current shape

`run_battle_result` handles final Vorath identity, null-node exit, battle count,
node clear, chain add/remove, overseer reward, Vorath counter, recipe-forbid
stub log, screen navigation, map emission, elite relic offers, and event reward
continuation (`run.c:1044-1116`).

### Simplified shape

Split into:

1. classify battle origin;
2. apply run outcome facts;
3. apply node-specific progression;
4. prepare reward continuation;
5. complete or retain pending interaction;
6. navigate/emit once;
7. finalize run for final battle.

Each helper performs one mutation class. Main function reads as ordered
orchestration.

### Preserved behavior

Battle count, alternating seat, chains, Vorath counter, overseer rewards, elite
relic offers, event rewards, and final-run result remain unchanged.

### Risk

`PENDING_NODE` is currently cleared at function start. Event reward paths still
need node/source context. Typed interaction must replace that temporal trick
before splitting.

---

## R5. Use shared one-shot effect invocation for events

Replace direct event effect calls with E2. Keep event option selection and
persistent-choice recording in run code. One-shot effects receive an event/run
frame and `x = EngineState *` as now.

Do not route run events through battle seat lists. There may be no battle.

---

## R6. Clarify battle/run boundary

### Current shape

Battle setup reads run unlocks, relics, events, difficulty, challenge,
synergies, masteries, pressure, chains, seed, battle count, and Vorath counter
through `BATTLE_ENGINE->run`. Buy and draw gates also read live run arrays.

### Simplified shape

Store an explicit run/engine reference in `BattleState`. At battle start,
derive a setup view containing values that should remain fixed for the battle:

- unlocked pools;
- selected rules;
- map/node identity;
- chain level;
- mastery and power availability;
- seed inputs.

Do not copy authoritative mutable run arrays unnecessarily. Buy/draw unlocks
can remain read-only live references if run cannot change during battle.
Vorath capacity effect should read a named setup value or an explicitly live
value; its current live read must not remain accidental.

### Preserved behavior

Run remains authority. Battle effects do not mutate unlock arrays directly.

### Risk

A broad snapshot can become stale if an in-battle effect legitimately changes
run state. Audit all run mutations callable from battle before choosing copied
versus referenced fields.

---

## R7. Clamp enemy army count before unsigned conversion

`battle_setup_armies` lets effects mutate signed `count`, then casts it to
`size_t` (`battle.c:2135-2151`). A negative result becomes a huge
reinforcement count.

Clamp at zero after all effects and before conversion. This is a correctness
fix and a reducer contract for `QUERY_ENEMY_ARMY_COUNT`.

---

## R8. Fix mixed event-duration ownership

`battle_walk_run_effects` marks a whole event choice consumed if any attached
effect has `ONE_BATTLE` duration (`battle.c:1973-2005`). If one option mixes
`ONE_BATTLE` and `ENTIRE_RUN`, consuming the choice removes both.

Current data may avoid the mix, but representation permits it.

Recommended first step: validate event data and reject mixed persistent
durations in one option. Rich per-effect consumption would require replacing
`EventChoice events[]` with more state and is not justified until GDD contains
such an option.

---

## R9. Remove implicit trait registry layout

`map_generate` chooses trait by `kingdom * 2 + random % 2`
(`run.c:510-513`). This assumes exactly two traits per kingdom in contiguous
registry order.

Replace arithmetic with an explicit per-kingdom trait table or add compile-time
layout declarations. Prefer explicit table because it remains correct if one
kingdom gains a different count.

---

## R10. Replace final battle pointer sentinel

`run_battle_result` identifies Vorath by `node == &VORATH_NODE`
(`run.c:1049`). Pointer identity encodes battle origin.

Store explicit battle origin/node kind in pending run interaction or battle
state. Synthetic final battle data may still exist, but result semantics do
not depend on one static object's address.

---

## R11. Keep run file count unchanged

Do not split `run.c`. Reorder existing file into clear sections after pending
state has one owner:

1. static data and typed run interaction state;
2. map generation and pure lookup helpers;
3. run lifecycle;
4. map projection and selection;
5. event/reward continuation;
6. battle outcome resolution;
7. public run mutations.

Remove dead and transitional helpers instead of moving them into new files.
Goal is smaller call graph inside same file count.

---

# Part VII — API roles and names

## Naming rule

Names should reveal effect behavior:

- **pure lookup/read:** `*_at`, `*_find_*`, `*_is_*`, `*_get_*`;
- **effectful evaluation:** `*_eval_*`;
- **committed mutation:** action verb such as `*_move`, `*_apply_damage`,
  `*_spawn`, `*_remove`;
- **projection:** `*_project_*` or `*_emit_*`;
- **lifecycle orchestration:** `*_begin`, `*_advance`, `*_resolve`,
  `*_finish`.

Do not call an effectful fold a getter.

## Battle API classification and recommended direction

| Current API | Role now | Recommended direction |
|---|---|---|
| `battle_begin` | lifecycle | keep or `battle_start` after setup split |
| `battle_free` | lifecycle cleanup | keep |
| `battle_concede` | lifecycle mutation | keep |
| `battle_move` | committed action | keep after stage split |
| `battle_buy` | committed action | rename `battle_buy_piece` only if accessor conflict removed |
| `battle_combine` | committed action | keep |
| `battle_play` | committed card action | keep |
| `battle_card_can_play` | effectful evaluation | `battle_eval_card_playable` |
| `battle_card_target` | interaction commit | `battle_select_card_target` |
| `battle_pending_picks` | pure interaction read | `battle_get_card_picks` |
| `battle_piece_unlocked` | global-backed lookup | `battle_is_piece_unlocked` with battle arg |
| `battle_piece_moves` | effectful history query today | pure `battle_get_move_count` after E6 |
| `battle_piece_flips` | effectful history query today | pure `battle_get_flip_count` after E6 |
| `battle_piece_move_turn` | pure history read | `battle_get_last_move_turn` |
| `battle_piece_flip_turn` | pure history read | `battle_get_last_flip_turn` |
| `battle_sell` | committed card action | keep |
| `battle_reclaim` | committed action | keep |
| `battle_end_turn` | lifecycle orchestration | `battle_finish_human_turn` or `battle_advance` |
| `battle_moves` | effectful list evaluation | `battle_eval_moves` |
| `battle_attacks` | effectful list evaluation | `battle_eval_attacks` |
| `battle_value` | effectful amount evaluation | `battle_eval_piece_amount`/damage wrapper |
| `battle_meter_max` | effectful aggregate evaluation | `battle_eval_meter_max` |
| `battle_territory` | effectful ownership evaluation | `battle_eval_territory` |
| `battle_at` | pure lookup | `battle_piece_at` |
| `battle_in_bounds` | pure predicate | `battle_is_in_bounds` |
| `battle_spawn` | mutation | keep |
| `battle_is_recipe_result` | pure predicate | keep |
| `battle_flip` | mutation plus events | keep or `battle_apply_flip` internally |
| `battle_swap` | mutation | keep, document event semantics |
| `battle_remove` | deferred mutation | `battle_queue_remove` if behavior stays deferred |
| `battle_scatter_voids` | mutation | keep |
| `battle_board_view` | effectful projection | `battle_project_board` |
| `battle_hand_view` | effectful projection | `battle_project_hand` |
| `battle_reinforce` | setup mutation | keep |
| `battle_attach_power` | effect attachment | internalize under generic attach helper |
| `battle_player` | pure lookup | `battle_player_state` |
| `battle_enemy` | pure mapping | `battle_opponent` |
| `battle_find_king` | pure lookup | keep |
| `battle_meter_gain` | mutation | `battle_apply_meter_gain` |
| `battle_rand` | state mutation/read | `battle_next_random` |
| `battle_draw_pool` | effectful evaluation | `battle_eval_draw_pool` |
| `battle_damage` | mutation pipeline | `battle_apply_meter_damage` |
| `battle_lunge` | one-card special path | delete |
| `battle_current` | fire-frame accessor | keep during migration |
| subject/victim/card accessors | fire-frame accessors | retain with explicit frame docs |
| `battle_draw` | committed mutation | keep |

Exact rename phase comes late. First make each role true; then rename once.

## Run API classification and recommended direction

| Current API | Role now | Recommended direction |
|---|---|---|
| `rng_mix` | pure utility | move to RNG utility or keep |
| `run_new` | lifecycle | keep |
| `run_free` | lifecycle cleanup | keep |
| `run_enter_map` | lifecycle mutation | keep |
| `run_select_node` | interaction orchestration | keep after dispatch split |
| `run_battle_result` | overloaded orchestration | `run_resolve_battle_result` after split |
| `run_event_choose` | interaction commit | `run_select_event_choice` |
| `run_offering` | interaction commit | `run_select_card_removal` |
| `run_relic_pick` | interaction commit | `run_select_relic_reward` |
| `run_pressure` | pure aggregate | `run_measure_pressure` |
| `run_innate_ready` | pure predicate | `run_is_innate_ready` |
| `run_offer_relics` | interaction mutation | internal reward preparation helper |
| `run_begin_removal` | interaction mutation | internal interaction helper |
| `run_reduce_vorath` | mutation | keep |
| `run_remove_chain` | mutation | clarify which kingdom/selection policy |
| `run_skip_battle` | mutation | keep until event-target model replaces flag |
| `run_begin_elite` | cross-lifecycle orchestration | `run_begin_event_elite` |
| `run_emit_kingdoms` | projection | keep |
| `run_emit_map` | projection | keep |
| `run_enter_vorath` | lifecycle | keep after explicit final origin |

---

# Part VIII — Hard-to-compose register

| Hazard | Site | Why composition fails | Disposition |
|---|---|---|---|
| Register leakage | `battle.c:19-33` and fire sites | Manual partial save/restore breaks nesting. | Fold now: E1 |
| Fixed effect slots | `representation.h:762`, `piece.c:260` | Facts and temporary grants compete with authored effects. | Fold now: E6 |
| Silent embed failure | `piece.c:260-281` | Mechanic disappears when 8 slots fill. | Fold now |
| Untyped context slots | `representation.h:779-797` | Wrong slot compiles; ownership unclear. | Normalize gradually |
| Template context loss | `effect.c:119-137` | Generic attach drops authored parameters. | Fold now: E3 |
| List insertion failure | `linked_list.c:37-58` | Caller sees success; effect not linked; memory leaks. | Fix now |
| Same-fire attachment | `effect.c:162-168` | New effect eligibility depends on tail growth. | Define next-fire rule |
| Dispatch order dependence | `effect.c:159-180` | Multipliers, clamps, and setters do not commute. | Reducer contracts |
| Query consumes state | action/card queries | Preview or failed action may spend grant. | Split eval/commit |
| Query log noise | `effect_run` | Repeated reads can look like repeated application. | Review after semantics |
| Scratch non-reentrancy | move generation | Nested pattern queries overwrite results. | Stack/caller storage |
| Stale target pointer | seat-list target effects | Deferred piece free can leave context pointers. | Validate membership/lifetime |
| Deferred free | `battle_remove`, `battle_reap` | Immediate free is unsafe during board walk. | Retain and document |
| `FLIPPED_PIECE` global | `battle.c:32`, cascade | Nested flip overwrites result. | Remove now |
| Duplicate damage path | half-turn vs `battle_damage` | Query/log/cascade behavior can diverge. | Fold now |
| Duplicate move path | move vs `battle_lunge` | History/cost/legality diverge. | Delete lunge |
| Swap event bypass | `battle_swap` | Move observers cannot see relocation. | Audit, then decide |
| Suspected dead friendly swap | `battle.c:849-861` | Base generators exclude friendly squares; effect editors still need audit. | Audit, then delete |
| Cascade progress | `battle_cascade` | Zero refill can loop with candidates. | Explicit guard |
| Meter clamp copies | gain and cascade | 200% rule can diverge. | Centralize |
| Setup order | `battle_begin` | Moving one phase changes initial meter. | Retain explicit schedule |
| White-first order | begin/end turn | Duplicated state machine can drift. | One advance function |
| Card pending globals | `battle.c:88-94` | Dry-run and real selection share mutable singleton state. | Typed battle interaction |
| Run pending globals | `run.c:394-402` | Multiple producers/consumers form temporal coupling. | Typed run interaction |
| Dead `RUN_ENGINE` | `run.c:394`, `753` | Duplicate global with no reader. | Delete |
| Pointer sentinel | `run.c:1049` | Address encodes battle origin. | Explicit origin enum |
| Mixed durations | `battle.c:1973-2005` | One one-battle effect consumes entire option. | Validate data |
| Unsigned army wrap | `battle.c:2135-2151` | Negative effect result becomes huge count. | Clamp now |
| Trait layout arithmetic | `run.c:510-513` | Registry order is hidden schema. | Explicit table |
| Magic reward integer | `run.c:1101-1114`, `1354-1364` | 0/1 meaning is not typed. | Enum |
| Event completion spread | three run functions | New continuation can clear early or never clear. | Centralize |
| GDD threshold mismatch | Timur watcher | Post-step meter check is not true threshold crossing. | Cascade state + end event |
| Crusade partial path | current/new sequence | Boxed state can fake attack or deadlock. | Explicit cancellation |

---

# Part IX — Distinctions that must survive simplification

## D1. `x` remains mutable value

Do not introduce a universal query wrapper. Operation kind and auxiliary
subjects belong in frame accessors. Existing effect bodies should continue to
receive `int *`, `bool *`, `Square *`, `CardTarget *`, `PieceInfo *`, or other
concrete values documented by trigger family.

## D2. Pre-meter and post-meter are separate phases

They may share one battle-phase enum, but both emission points remain.
Collapsing them changes Traitor's Gambit, Enslaved, initial maximum, Vorath
capacity evaluation, and post-meter start effects.

## D3. Flip redirect and immediate post-toggle remain separate

`ON_PIECE_FLIP_PRE` can redirect or consume. `ON_PIECE_FLIP` observes actual
toggle. Cascade completion is a third, cascade-level concept, not a renamed
piece callback.

## D4. Deferred reaping remains

Effects can remove their own board piece while `effect_fire` is walking the
board. Immediate free would invalidate iteration. Simplify ownership and name,
but keep deferred destruction until dispatch becomes snapshot-based.

## D5. Typed target protocol remains

Card target enumeration is already a good composition seam. Simplify its state
machine, not its self-describing target list.

## D6. Domain-specific helpers remain when domains differ

`card_targets_piece`, `card_targets_square`, and
`card_targets_piece_type` traverse different domains. One highly generic
iterator would save little and hide legal-target meaning. Keep these focused
helpers.

---

# Part X — Phased implementation plan

Every phase ends with clean debug/release builds, relevant protocol traces, and
a grep proving removed paths are gone. Source code follows project code style;
non-source evidence documents do not inherit source formatting rules. Header
edits require a clean rebuild. Existing source/header file count stays fixed.

## Session chaining and phase receipts

Treat each F/S phase as one full-session unit. A session starts one named phase,
finishes it, and creates or updates exactly one receipt:

```text
res/audit-v2-phases/<phase>.md
```

Examples: `F1.md`, `F4.md`, `S5.md`. Do not pre-create blank receipts for future
phases. `F0.md` and `F0.5.md` record already-completed baseline work. The next
session reads the previous completed receipt, then the named phase section in
this plan, its F0.5 fixture contract, GDD, and relevant source.

A phase receipt must state:

1. phase name, status (`COMPLETE`, `BLOCKED`, or `IN PROGRESS`), date, and
   starting/ending source-header file count;
2. exact scope completed and explicit statement that no downstream phase began;
3. files changed and why;
4. behavior decisions, GDD citations, and approved intentional differences;
5. public-protocol fixture setup, commands, captured output, and probes;
6. debug/release build commands and exact result;
7. static checks, unresolved defects, deferred work, and any user decision
   required before the next phase;
8. next allowed phase and a copy-ready fresh-session directive.

`COMPLETE` is allowed only when the phase gate in this plan passes. If a session
runs out of time or finds an unresolved failure, write `IN PROGRESS` or `BLOCKED`
in the same receipt. The next session resumes that phase; it must not start a
later one. No receipt may claim behavior based on patched end state, source
inspection, build success, or a direct internal call.

`res/audit-v2-session-prompt.md` is the reusable fresh-session prompt.
`res/audit-v2-phases/TEMPLATE.md` is the receipt format. The phase directory
index records only completed/current receipts; it is not another plan.

## Feature-completion stream

No simplification phase starts until F10 passes.

### F0 — Runtime baseline ✅

`res/audit-baseline-v2.md` records current movement, action costs, setup/start,
damage/cascade, Crusade, Timur, event, and reward behavior plus GDD mismatches.

Gate: complete.

### F0.5 — Fixture reachability contract

- record executable public-protocol routes for every F1-F9 mechanic;
- include Gold's three-battle retry, Caravan's second-turn reinforcement, and
  Crowned Heretic's multi-Ghost accumulation;
- distinguish state-establishing save patches from observed transitions;
- add missing deterministic fixture seams during each owning F phase.

Gate: every F10 behavior row has a real transition route, not inspection-only
or patched-end-state evidence.

### F1 — Baseline defects and mastery gaps

- fix Counter Coup owner-turn lifetime;
- convert Vorath pressure into true capacity increase;
- add threshold crossing and `ON_CASCADE_END` for Timur's Conquest;
- fix Bulk Discount reset and cheapest-purchase settlement, superseding v1 H13;
- add Double Time's missing no-home-discount and +40% foreign-markup base rule;
- reduce only Double Time's foreign markup to +20% at Selassie mastery 3;
- enforce Royal Substitution once/twice usage by mastery;
- add regression traces for both human seat colors.

Gate: every baseline mismatch except approved Crusade redesign passes GDD.

### F2 — Run targeting and event composition

- add typed pending run interaction;
- migrate all 60 event options to A/B plus typed follow-up targets;
- remove fixed Xiang/Kyosha representatives and reveal no-ops;
- establish persisted hidden-node state, successor reveal, protocol masking, and
  valid reveal targeting;
- centralize node completion/resume;
- add generic run-side effect firing.

Gate: every event choice and target path runs through generic item effects and
real persisted state.

### F3 — Archive composition and Master's Notes

- add archive reveal-count query;
- implement Master's Notes without relic identity checks;
- preserve deterministic ordering, exhaustion behavior, and save/load state.

Gate: normal and Master's Notes archive traces expose one and two real reveals.

### F4 — Next hand and active relics

- record user's Counsel refill-versus-two-card decision before coding;
- add deterministic projected next hand and `QUERY_NEXT_HAND`;
- implement full Counsel discard choice under approved slot policy;
- add optional Librarian's Notes top-card skip;
- add generic active effect-item protocol;
- add Deep Hand once-per-battle activation;
- verify interactions with draw modifiers and bonus draws.

Gate: projected hand always equals realized hand after choices.

### F5 — Map visibility and Surveyor's Map

- extend F2 node visibility with separate modifier visibility;
- reveal modifier data through generic run targets;
- persist both visibility dimensions;
- apply one deterministic modifier pre-reveal per map from Surveyor's Map.

Gate: map protocol proves hidden, revealed, and modifier-preview states without
replacing F2 visibility state.

### F6 — AI plan and Divination

- authoritative stored AI plan used by actual AI turn;
- intended move/card projection;
- Divination reveal;
- deterministic invalidation/replan behavior.

Gate: unchanged-board revealed plan equals following AI execution.

### F7 — Gold chain and Vorath recipe forbid

- track lock;
- Liberation Trial injection and three-battle retry;
- +2 trial army and restricted card pool through effects;
- Gold break on win;
- deterministic forbidden recipe at every four losses;
- combine veto and save/load persistence.

Gate: full Gold lifecycle and 4/8/12-loss recipe traces pass.

### F8 — Event-elite armies

- Mirage Citadel army/rules;
- Pretender mirrored setup;
- Tournament fixed Caelan setup;
- generic setup data/effect path;
- typed reward continuation on win/loss.

Gate: no event elite falls back to standard regional army.

### F9 — Kingdom Overseers and Vorath finale

Implement and verify Iron Strategist, Caravan of Conquest, Many-Faced King,
Eternal Recursion, Crowned Heretic, and Vorath. All setup and boss rules live in
effect/data items. Generic battle/run code contains no boss identity branches.

Vorath reuses F1's maximum-meter query and shared clamp for initialization,
exact-zero comparison, overshoot reset, and refill. It also uses explicit final
battle origin and run result instead of pointer identity.

Gate: six boss traces cover setup, recurring/core mechanic, win, loss, reward,
and deterministic Vorath exact-zero/overshoot behavior.

### F10 — Composition-complete freeze

- every F0.5 route executes its real transition;
- every effect item in `res/analysis.md` is ✅;
- all deep/tier/mastery-gated items have runtime evidence;
- all save/load state round-trips;
- no item identities remain in agnostic engine paths;
- post-fix GDD matrix is frozen as simplification baseline.

Gate: user-visible feature set complete. Simplification may begin.

## Simplification stream

### S1 — Typed piece facts and fire frame

- add runtime history fields to `PieceInfo`;
- migrate move/flip stamps and readers;
- delete history mark helpers and unused history triggers;
- add nested-safe `EffectFrame` and frame accessors;
- migrate fire sites without trigger renames.

Gate: F10 behavior freeze remains unchanged.

### S2 — Effect invocation and attachment

- add one invocation/log primitive;
- migrate card, run, and event direct calls;
- make attachment copy baked context;
- add generic array attachment;
- make list insertion/allocation failure explicit;
- define next-fire eligibility for newly attached effects;
- designate trigger-name initializers.

Gate: applied logs and all feature traces match F10.

### S3 — Trigger folds

Migrate one family atomically at a time:

1. capability;
2. action cost;
3. card amount/eligibility;
4. piece pattern;
5. piece numeric;
6. meter amount, including maximum/capacity;
7. piece/card action events;
8. combo and turn phase;
9. battle phase;
10. cascade completion;
11. run-side reveal/recipe/map families where payloads match.

Never leave old/new trigger families active together.

Gate: no old family names remain and every F10 trace matches.

### S4 — Battle primitives

- committed relocation;
- virtual-origin pattern evaluation;
- strike evaluator;
- one meter damage path;
- local cascade state;
- one meter clamp path;
- friendly-swap reachability audit/removal if dead;
- explicit swap event decision.

Gate: battle, boss, event-elite, and Vorath traces match F10.

### S5 — Crusade

- attach sequence from card target selection;
- gate movement to selected Knight;
- enforce exact 1/0/0 costs on commit;
- store landing history outside piece effect slots;
- resolve landing attacks through strike evaluator;
- handle cancel, flip, removal, dead end, and turn end;
- delete `battle_lunge`.

Gate: approved Crusade behavior matrix passes; no other trace changes.

### S6 — Battle orchestration

- typed pending card interaction;
- one target-step iterator;
- shared action validation;
- one round-advance state machine;
- ordered setup stages;
- move owned globals into `BattleState`.

Gate: both human-seat orders, active items, bosses, and turn-ten finish pass.

### S7 — Run-state cleanup

- delete `RUN_ENGINE`;
- consolidate remaining pending/reward state around F2 interaction;
- split battle-result responsibilities into small in-file helpers;
- remove final pointer sentinel;
- clamp army count;
- validate event durations;
- replace trait layout arithmetic;
- clarify battle/run setup boundary.

Gate: events, chains, map reveals, bosses, rewards, and final run match F10.

### S8 — API names and in-file organization

- apply role-based renames after semantics stabilize;
- update callers and header docs;
- keep existing source/header file count;
- reorder `battle.c`/`run.c` sections around final responsibilities;
- remove transitional accessors, aliases, and obsolete helpers.

Gate: no ambiguous public name and no new source/header file.

### S9 — Final audit

- rerun complete GDD matrix;
- grep engine code for item identities;
- compare trigger count and duplicated paths to baseline;
- verify one operation path per mutation;
- update v2 status only after all traces pass.

---

# Verification plan

## Build gates

After source-only changes:

```sh
make debug
make release
```

After any header enum, struct, or prototype change:

```sh
make clean && make debug
make clean && make release
```

Debug uses C23, blocks, `-Wall`, `-Wextra`, and `-Werror`. No warning may be
silenced. Current Makefile has no actual `test` recipe despite listing `test`
as phony; do not report `make test` as coverage.

## Static gates

- source lines at or below 80 columns;
- no comments inside functions;
- no unused variables or warning suppression;
- structs, enums, and public declarations in headers;
- trigger-name table complete;
- no `battle_lunge` after S5;
- no old folded trigger names after each family;
- no manual current-battle register choreography after E1;
- one attachment loop primitive;
- no `RUN_ENGINE`;
- no numeric event reward kind;
- no specific item identity in agnostic battle/run paths;
- no history marks in piece effect slots.

## Protocol traces

### Effect core

- nested query restores outer frame;
- effect attached during fire waits until next fire;
- one-shot card and event logs match attached-effect logs;
- baked event context survives generic attachment;
- allocation failure does not create partial effect state.

### Trigger families

- card play/sell amount;
- draw/play veto;
- move/buy/combine action cost;
- move/attack/flip/buy capability;
- movement/attack pattern append and replace;
- spawn value, outgoing damage, incoming damage, buy/reclaim cost;
- meter maximum, damage, and refill;
- piece/card committed events;
- combo count two and three;
- turn start/end;
- pre-meter/post-meter battle phases.

### Damage and cascade

- ordinary half-turn damage;
- direct card/relic damage;
- zero and fully reduced damage;
- multi-flip;
- redirect;
- consume;
- no eligible candidate with non-kings present;
- king-only loss;
- refill deficit continuation;
- zero-refill progress guard;
- 200% cap;
- Timur crossing below 20 then refilling above it.

### Crusade

- three legal player-selected moves;
- exact action delta one;
- other-piece movement veto;
- duplicate victim coverage across landings;
- no legal first move;
- dead end after one/two moves;
- selected Knight flips;
- selected Knight removed;
- player ends turn early;
- interaction with existing free/double move effects;
- one combined damage/cascade entry at half-turn.

### Battle lifecycle

- human white and human black;
- opening AI half;
- white-first rounds;
- turn-ten territory;
- setup pieces included in initial meter;
- Vorath capacity included in initial maximum with no post-meter pressure gain;
- combo reset each turn;
- deferred piece reaping.

### Run lifecycle

- event immediate completion;
- multi-card removal completion;
- relic choice completion;
- event elite win/loss and reward;
- normal elite relic offer;
- chain add/remove;
- Vorath counter increment/reduction;
- negative army effect clamps to zero;
- final battle result;
- mixed event durations rejected by validation.

## GDD behavior matrix

For every mechanic touched by a phase, record:

- GDD text and line;
- pre-refactor trace;
- post-refactor trace;
- intended difference, if user explicitly requested one;
- result.

Crusade is the only planned intentional design difference in this audit. F1
trace changes are GDD defect corrections and must be recorded as such. Any
other difference blocks completion until explained and approved.

---

# Out of scope

Only systems that are not effect-item composition prerequisites remain outside
this plan:

- Daily Conquest date/seed selection;
- Clockwork real-time 30-second timer;
- full AI personality/archetype differentiation beyond authoritative plan
  needed by Divination;
- cosmetic mastery titles and presentation-only text;
- balance changes not stated by GDD or approved by user.

Event targeting, Master's Notes, next-hand features, Deep Hand, map visibility,
Surveyor's Map, Divination, Gold chain, recipe forbid, event elites, Overseers,
Vorath, Bulk Discount, Counter Coup, and mastery effect gaps are all mandatory
F-stream work before simplification.

---

# Critical files

Primary implementation files. Keep this existing file set; do not add
battle/run source modules or new architecture headers:

- `incl/representation.h`
- `src/representation/effect.c`
- `src/representation/battle.c`
- `src/representation/run.c`
- `src/representation/engine.c`
- `src/representation/ai.c`
- `src/representation/piece.c`
- `src/representation/card.c`
- `src/representation/relic.c`
- `src/data_structure/linked_list.c`
- `src/data/longwei.c`
- `src/data/kewarani.c`
- `src/data/zarqan.c`
- `src/data/harushima.c`
- `src/data/caelan.c`
- `src/data/universal.c`
- `src/data/events.c`
- `src/protocol/protocol.c`
- `src/protocol/screen.c`

Source-of-truth and companion documents:

- `res/GDD.md`
- `res/analysis.md`
- `res/audit-plan-v1.md`
- `res/audit-baseline-v2.md`
- `res/master-plan.md`
- `makefile`

---

# Success criteria

Audit implementation is complete only when:

1. Every F0.5 fixture route executes, and every deferred effect item and
   semantic seam in F1-F9 is implemented.
2. Every effect-item row in `res/analysis.md` is ✅ before S1 starts.
3. Counter Coup, Vorath capacity, Timur, Bulk Discount, and mastery gaps pass
   GDD traces.
4. Event targeting, Master's Notes, next hand, Deep Hand, map reveal,
   Surveyor's Map, Divination, Gold chain, and recipe forbid are live; Counsel
   follows an explicitly approved refill-versus-two-card rule.
5. Event elites, five Overseers, and Vorath finale use effect/data setup and
   have deterministic runtime evidence.
6. `battle_lunge` no longer exists.
7. Crusade uses ordinary movement and ordinary half-turn resolution.
8. One nested-safe frame owns all effect fire metadata.
9. One invocation path and one attachment path exist.
10. Runtime history no longer consumes piece effect slots.
11. Trigger families are folded without wrapper payloads.
12. Query evaluation cannot consume state before commit.
13. One relocation, strike, meter damage, cascade, target-step, and round path
    exists.
14. Run pending state has one typed owner and node completion has one path.
15. Public API names reveal role.
16. All GDD behavior traces pass, except approved Crusade differences.
17. Existing source/header file count is unchanged.
18. Clean debug/release builds produce zero warnings.
