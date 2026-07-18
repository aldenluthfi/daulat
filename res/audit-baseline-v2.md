# Daulat — Audit v2 Phase 0 Runtime Baseline

## Status

Phase 0 captured on 2026-07-16.

No Phase 1 refactor started. No source/header file added, removed, or edited.
This file is runtime evidence only.

## Method

- No project-local `.claude/skills/` verifier or run skill exists.
- Runtime handle: clean debug build, then `bin/daulat` through its public
  stdin/stdout protocol.
- Build setup: `make clean && make debug` completed with zero warnings.
- No unit tests, internal function calls, or import-and-call harnesses used.
- Deep progression fixtures used public `save`/`load` surface:
  1. app generated a valid text save;
  2. temporary save copied into an isolated temporary directory;
  3. only unlock bits, battle parity, Vorath count, or map-cleared bits were
     patched;
  4. all gameplay after load used normal protocol commands.
- Temporary fixtures were deleted after each trace.

Patched saves are setup, not evidence. Evidence below is app output produced by
real `new`, `load`, `enter`, `select`, `buy`, `move`, `play`, `target`, `end`,
`choose`, `remove`, and `relic` commands.

---

# Battle baselines

## B0. Protocol cold start

Command:

```text
new seed=2 difficulty=0 challenge=7
enter id=0
select i=0
state
board
hand
quit
```

Observed surface:

```text
screen title
screen campaign
screen map
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=3
row y=0 cells=······k·····
row y=11 cells=······K·····
card i=0 id=3 name="Last Stand" kingdom=6 tier=1 play=0 sell=30
card i=1 id=33 name="Counsel" kingdom=2 tier=0 play=0 sell=15
card i=2 id=33 name="Counsel" kingdom=2 tier=0 play=0 sell=15
bye
```

Baseline handle confirmed: deterministic seed, public battle state, board,
hand, and action commands all reachable.

---

## B1. Ordinary move and buy action costs

Command core:

```text
state
moves x=6 y=11
move fx=6 fy=11 tx=6 ty=10
state
move fx=6 fy=10 tx=6 ty=8
state
buy piece=1 x=5 y=10
state
```

Observed:

```text
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=3
square x=5 y=10
square x=6 y=10
square x=7 y=10
square x=5 y=11
square x=7 y=11
ok
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=2
error msg="illegal move"
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=2
log effect name="Open Market" trigger=QUERY_PIECE_CP_COST_BUY
state side=white turn=1 cp=27 meter=20 enemy_meter=10 actions=1
```

Observation:

- committed move consumed one action;
- illegal move consumed nothing;
- buy consumed one action;
- Open Market modified buy CP cost through effect query;
- buying Bing added its value to meter.

Probe:

- invalid two-square King move returned a clean protocol error and preserved
  actions.

---

## B2. Temporary free move

Fixture:

- seed 14;
- all card unlock bits enabled;
- Kewarani Town node 0;
- initial hand contains Double Time at slot 1.

Command core:

```text
play i=1
target i=0
state
move fx=6 fy=11 tx=6 ty=10
state
move fx=6 fy=10 tx=6 ty=9
state
```

Observed:

```text
target i=0 kind=0 value=226
log effect name="Double Time" trigger=ON_CARD_TARGET_SELECTED
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=3
log effect name="Extra Move" trigger=QUERY_PIECE_ACTION_COST_MOVE
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=3
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=2
```

Observation:

- card play cost no action;
- first committed move after grant cost zero;
- next move cost one;
- action-cost effect logs only on free move application.

This trace is baseline for action-cost trigger consolidation.

---

## B3. Setup versus start ordering

### Bound: enemy army before initial meter

Command:

```text
new seed=2 difficulty=1 challenge=7
enter id=0
select i=0
```

Observed:

```text
log effect name="Bound" trigger=QUERY_ENEMY_ARMY_COUNT
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=30 actions=3
```

Two free enemy pieces contributed to initial enemy meter.

### Traitor's Gambit: setup pieces before initial meter

Command:

```text
new seed=2 difficulty=0 challenge=4
enter id=0
select i=0
```

Observed:

```text
log effect name="The Traitor's Gambit" trigger=ON_BATTLE_SETUP
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=40 actions=3
```

Three enemy pieces spawned in setup and contributed to initial meter.

### Enslaved setup before enemy innate start

Command:

```text
new seed=43 difficulty=3 challenge=7
enter id=1
select i=0
```

Observed order:

```text
log effect name="Bound" trigger=QUERY_ENEMY_ARMY_COUNT
log effect name="Enslaved" trigger=ON_BATTLE_SETUP
log effect name="Double Time" trigger=ON_BATTLE_START
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=30 actions=3
```

`ON_BATTLE_SETUP` attached enemy innate before `ON_BATTLE_START` fired it.

### Vorath pressure after initial maximum

Fixture:

- seed 2;
- `vorath=2`.

Observed:

```text
log effect name="Vorath Pressure" trigger=ON_BATTLE_START
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=20 actions=3
```

Base enemy maximum was 10. Pressure attempted +20, but meter gain clamped at
200% maximum, producing 20 rather than 30. This is current behavior and a GDD
comparison finding, not a Phase 0 change.

---

## B4. Ordinary half-turn damage and non-king cascade

Fixture:

- seed 2;
- Bound difficulty;
- all piece unlock bits enabled;
- human bought Ma at `(4,2)`.

Command core:

```text
buy piece=3 x=4 y=2
state
attacks x=4 y=2
end
state
board
```

Observed:

```text
log effect name="Bound" trigger=QUERY_ENEMY_ARMY_COUNT
state side=white turn=1 cp=30 meter=10 enemy_meter=30 actions=3
log effect name="Open Market" trigger=QUERY_PIECE_CP_COST_BUY
state side=white turn=1 cp=21 meter=40 enemy_meter=30 actions=2
square x=2 y=1
log damage side=black amount=30
log flip piece=Bing x=8 y=5
log damage side=white amount=30
state side=white turn=2 cp=91 meter=20 enemy_meter=50 actions=3
row y=5 cells=········B···
```

Observation:

- Ma threatened enemy Bing and dealt 30 at half-turn resolution;
- emptied enemy meter flipped a non-king Bing;
- flipped Bing joined human side and increased human maximum/meter;
- AI half-turn then dealt 30 back through ordinary resolve;
- battle continued after non-king cascade.

---

## B5. Multi-step cascade to King

Fixture:

- seed 2;
- Longwei Town elite node 5 made selectable;
- all piece unlock bits enabled;
- human bought Ma at `(5,2)`.

Observed:

```text
state side=white turn=1 cp=30 meter=10 enemy_meter=20 actions=3
log damage side=black amount=30
log flip piece=Bing x=8 y=5
log flip piece=King x=6 y=0
result won=1
```

Observation:

30 damage against 20 meter carried deficit through first refill, flipped Bing,
continued cascade at zero, then flipped lone King and ended battle.

This trace is baseline for deficit carry, repeated cascade steps, and King-loss
termination.

---

## B6. Current Crusade behavior

### Fixture

- seed 11;
- Bound difficulty;
- `vorath=2`;
- all card and piece unlock bits enabled;
- Caelan Town node 0;
- Crusade at hand slot 0;
- Knight bought at `(4,9)`.

Before play:

```text
log effect name="Bound" trigger=QUERY_ENEMY_ARMY_COUNT
log effect name="Vorath Pressure" trigger=ON_BATTLE_START
state side=white turn=1 cp=30 meter=10 enemy_meter=50 actions=3
row y=0 cells=p·····k·····
row y=5 cells=····p·······
state side=white turn=1 cp=12 meter=40 enemy_meter=50 actions=2
```

Command:

```text
play i=0
target i=0
state
board
```

Observed:

```text
target i=0 kind=0 value=184
log effect name="Mirror" trigger=QUERY_METER_DAMAGE_TAKEN
log damage side=black amount=30
log effect name="Mirror" trigger=QUERY_METER_DAMAGE_TAKEN
log damage side=black amount=30
log flip piece=Pawn x=0 y=0
log effect name="Crusade" trigger=ON_CARD_TARGET_SELECTED
state side=white turn=1 cp=12 meter=64 enemy_meter=10 actions=2
row y=0 cells=P·····k·····
row y=3 cells=···N········
```

Current runtime facts:

- one target selection automatically performed all three Knight moves;
- player selected no landing squares;
- Crusade consumed no action beyond prior Knight buy;
- damage applied immediately during target resolution, not at half-turn end;
- two hops produced separate 30-damage calls;
- first call reduced meter; second call caused cascade during sequence;
- cascade flipped Pawn before Crusade outer effect logged completion;
- final Knight square was `(3,3)`;
- intermediate landing squares were not exposed by protocol.

This is intentional-change baseline. Planned Crusade behavior must differ only
as approved in audit v2.

### Nested effect-fire observation

Crusade target resolution is one outer card effect. During that effect,
`battle_lunge` entered meter damage queries twice, Mirror applied twice, one
cascade ran, then outer Crusade invocation returned and logged its correct
`ON_CARD_TARGET_SELECTED` trigger.

This proves a real nested path currently completes without visible context
corruption. Exact subject/frame restoration is not observable through current
protocol; instrumentation was not added during baseline.

### Blocked edge

No public protocol fixture produced a fully boxed Knight while retaining
Crusade in same turn without adding test-only engine hooks. Stationary boxed
attack remains source-confirmed but runtime-unobserved in Phase 0.

---

## B7. Timur's Conquest threshold behavior

### Fixture

- seed 4;
- Bound difficulty;
- `battles=1`, making human side black;
- all card and piece unlock bits enabled;
- Timur's Conquest at slot 0;
- human bought Wazir at `(3,5)`;
- human meter after buy: 25.

Command core:

```text
sell i=1
buy piece=25 x=3 y=5
play i=0
end
state
end
state
```

Observed:

```text
log effect name="Timur's Conquest" trigger=ON_CARD_PLAY
state side=black turn=1 cp=31 meter=25 enemy_meter=40 actions=2
log damage side=white amount=15
log damage side=black amount=15
state side=black turn=2 cp=71 meter=10 enemy_meter=55 actions=3
log damage side=white amount=15
log damage side=black amount=15
log flip piece=Wazir x=3 y=5
state side=black turn=3 cp=216 meter=5 enemy_meter=55 actions=3
```

No later `Timur's Conquest` effect log appeared.

Runtime finding:

- meter crossed from 25 to 10, below GDD threshold 20;
- automatic Royal Substitution did not trigger;
- next damage caused Wazir flip and cascade settlement;
- automatic Royal Substitution still did not trigger;
- King remained at original square.

Current implementation does not satisfy GDD line 972 in this reachable case.
This is baseline bug evidence for replacing per-piece post-flip trigger with a
real cascade/threshold event.

---

## B8. Counter Coup adjacent probe

GDD line 281: all damage taken this turn should echo 50% to enemy meter.

Fixture:

- seed 77;
- Bound difficulty;
- `battles=1`, human black;
- all card/piece unlock bits enabled;
- Counter Coup at slot 0;
- human bought Wazir at `(6,6)` and played Counter Coup.

Observed:

```text
state side=black turn=1 cp=21 meter=25 enemy_meter=40 actions=2
log effect name="Mirror" trigger=QUERY_METER_DAMAGE_TAKEN
log damage side=white amount=15
log effect name="Mirror" trigger=QUERY_METER_DAMAGE_TAKEN
log damage side=black amount=15
state side=black turn=2 cp=111 meter=13 enemy_meter=58 actions=3
```

No `Counter Coup` fire appeared. `TURNS_1` effect ticked at end of playing
side's half-turn before opponent damage arrived. This adjacent runtime finding
belongs in later duration/query-order review; Phase 0 makes no fix.

---

# Run baselines

## R1. Event card-removal continuation

Fixture:

- seed 2;
- Longwei Town nodes 0-7 marked cleared;
- Dragon Court Tribute node 8 selectable.

Command core:

```text
select i=8
choose a
nodes
remove card=0
nodes
```

Observed:

```text
node i=8 type=4 name="The Governor's Courtyard" cleared=0 selectable=1

event id=1 name="The Governor's Courtyard"
offer remove=1
log event id=1 choice=1
node i=8 type=4 name="The Governor's Courtyard" cleared=0 selectable=1
log offering removed=0
node i=8 type=4 name="The Governor's Courtyard" cleared=1 selectable=0
```

Node remained pending after event choice and cleared only after final removal.

---

## R2. Event relic-offer continuation

Same event, choice B:

```text
event id=1 name="The Governor's Courtyard"
offer relic_a=3 relic_b=24
log event id=1 choice=2
node i=8 type=4 name="The Governor's Courtyard" cleared=0 selectable=1
log relic id=3
node i=8 type=4 name="The Governor's Courtyard" cleared=1 selectable=0
```

Node remained pending through relic selection and cleared after pick.

---

## R3. Normal elite reward continuation

Fixture:

- Longwei Town nodes 0-4 marked cleared;
- elite node 5 selectable;
- all piece unlock bits enabled;
- Ma bought at `(5,2)` for deterministic first-turn win.

Observed:

```text
node i=5 type=1 name="The Prefecture Garrison" cleared=0 selectable=1
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=20 actions=3
log damage side=black amount=30
log flip piece=Bing x=8 y=5
log flip piece=King x=6 y=0
result won=1
screen map
node i=5 type=1 name="The Prefecture Garrison" cleared=1 selectable=0
offer relic_a=6 relic_b=1
log relic id=6
node i=5 type=1 name="The Prefecture Garrison" cleared=1 selectable=0
```

Elite node cleared on win, then relic offer remained independently pending.

---

## R4. Event elite battle and reward continuation

Fixture:

- Zarqan Town nodes 0-8 marked cleared;
- Warlord's Challenge event node 9 selectable;
- all piece unlock bits enabled.

Command core:

```text
select i=9
choose a
hand
sell i=0
buy piece=3 x=5 y=2
end
relic id=25
nodes
```

Observed:

```text
node i=9 type=4 name="The Desert Arena" cleared=0 selectable=1
event id=10 name="The Desert Arena"
log event id=10 choice=1
screen battle
state side=white turn=1 cp=30 meter=10 enemy_meter=10 actions=3
log damage side=black amount=30
log flip piece=King x=6 y=0
result won=1
screen map
node i=9 type=4 name="The Desert Arena" cleared=1 selectable=0
offer relic_a=25 relic_b=9
log relic id=25
node i=9 type=4 name="The Desert Arena" cleared=1 selectable=0
```

Event choice suspended map flow, started battle, returned to map on win,
prepared reward, and accepted relic choice.

---

# GDD behavior matrix

| Behavior | GDD | Runtime baseline | Result |
|---|---|---|---|
| Three actions; move/buy/combine cost one | `GDD.md:88-96` | Move and buy each consumed one; illegal move consumed zero | Matches |
| Cards cost no actions | `GDD.md:96` | Double Time and Crusade play left action count unchanged | Matches |
| Temporary extra move | Double Time card behavior | First granted move cost zero; next cost one | Matches current authored behavior |
| Meter equals piece values | `GDD.md:48-58` | Buying Bing/Ma/Wazir increased meter by runtime value | Matches |
| Non-king flips when meter empties | `GDD.md:60` | Bound trace flipped Bing and battle continued | Matches |
| King flips after army exhausted | `GDD.md:62` | Elite trace flipped Bing, carried deficit, then King | Matches |
| Meter overflow max 200% | `GDD.md:66` | Vorath pressure on max 10 clamped meter to 20 | Matches cap |
| End-turn threats deal damage | `GDD.md:75-76` | Ma threat dealt 30 during half-turn resolve | Matches |
| Resolve carries remaining damage | `GDD.md:98-103` | 30 damage against 20 meter flipped Bing then King | Matches |
| Bound starts with +2 pieces | `GDD.md:1005` | Enemy meter 30 from King plus two basic pieces | Matches |
| Traitor starts 3 pieces in human half | `GDD.md:1017` | Setup effect produced enemy meter 40 before battle state | Matches |
| Every 2 losses gives +20 enemy capacity | `GDD.md:659-664` | `vorath=2` left maximum 10 and transient meter 20 | Mismatch: no capacity increase; gain also clamps to net +10 |
| Crusade makes 3 L-moves, attacks on resolution | `GDD.md:345` | Automatic path; immediate per-hop damage/cascade; no action cost | Mismatch; approved redesign target |
| Timur triggers below 20 meter | `GDD.md:972` | Meter crossed 25→10 and later cascaded; no substitution | Mismatch |
| Counter Coup echoes damage this turn | `GDD.md:281` | Effect expired before opponent half; no echo | Mismatch |
| Dragon Court removal continuation | `GDD.md:841` | Node stayed pending until one card removed | Continuation matches; +5 value not verified here |
| Dragon Court relic continuation | `GDD.md:841` | Node stayed pending until relic selected | Continuation matches; granted relic state not reloaded here |
| Elite win offers one of two relics | Campaign/event tables | Normal and event elite both emitted two-relic offer | Matches continuation flow |
| Warlord's Challenge reward | `GDD.md:860` | Event battle win emitted a two-relic offer | Continuation matches; exact pair comes from source, not GDD |

---

# Phase 0 findings

## Confirmed behavior to preserve

- base move and buy action costs;
- failed actions do not consume actions;
- free-move query result and one-shot consumption;
- setup pieces contributing to initial meter;
- pre-meter setup before post-meter start;
- half-turn damage timing;
- non-king flip, deficit carry, repeated cascade, and King loss;
- event suspension through removal/relic/battle continuations;
- normal and event elite reward flows.

## Confirmed behavior intentionally replaced

- Crusade automatic greedy path;
- Crusade immediate per-hop damage/cascade;
- Crusade bypass of action cost and ordinary move control.

## Confirmed baseline bugs or GDD mismatches

1. **Timur's Conquest:** no automatic substitution when meter crosses below 20,
   including after later cascade.
2. **Counter Coup:** `TURNS_1` expires before opponent half-turn damage, so
   normal reflected damage never fires in observed case.
3. **Vorath pressure:** implementation raises transient meter, not meter
   capacity. Lone-King maximum stays 10; attempted +20 then clamps at 20,
   producing only +10 net. GDD says +20 capacity baseline.
4. **Crusade:** runtime is automatic/immediate rather than player-controlled
   three-move path resolved at half-turn.

## Runtime-observation gaps

- Exact nested subject/frame restoration has no protocol projection. Crusade
  proves nested execution returns correctly, but not every hidden register.
- Fully boxed Crusade Knight was not reached without test-only setup hooks.
- Allocation failure and same-dispatch attachment require dedicated effect/list
  runtime fixtures in later implementation work.
- Deep run branches not listed above remain outside Phase 0 baseline.

These gaps are recorded, not silently treated as passes.

---

# Phase 0.5 fixture-reachability audit

## Scope and verdict meaning

This audit does not claim that deferred mechanics work. Most F1-F9 mechanics do
not exist yet. It establishes one deterministic public-protocol route for every
future trace, or records the exact missing public surface that its owning phase
must add before its gate can pass.

**Fixture-route audit: COMPLETE.** Every F1-F9 requirement below has either an
existing route contract or an explicit owning-phase gap. No F1 source change was
made.

A future feature is not verified by loading a patched result state. A fixture may
patch only persistent starting inputs, then must drive the transition and capture
normal protocol output.

## Observed fixture transport

Fresh isolated runtime proof:

```text
new seed=2 difficulty=0 challenge=7
save
title
load
kingdoms
quit
```

Observed:

```text
screen campaign
ok
screen title
ok
screen campaign
kingdom id=0 name="Longwei Empire" mastery=0 chain=0 tier=0
kingdom id=1 name="Kewarani Negusate" mastery=0 chain=0 tier=0
kingdom id=2 name="Zarqan Sultanate" mastery=0 chain=0 tier=0
kingdom id=3 name="Harushima Shogunate" mastery=0 chain=0 tier=0
kingdom id=4 name="Caelan Kingdom" mastery=0 chain=0 tier=0
ok
bye
```

This was a separate temporary working directory. Its generated save was deleted
when the trace ended. It proves that a profile can be created, persisted, loaded,
and projected through the public surface without touching repository state.

Deferred-surface probe from another isolated profile:

```text
new seed=2 difficulty=0 challenge=7
enter id=0
nodes
select i=0
combine ax=0 ay=0 bx=1 by=0
activate i=0
next_hand
quit
```

Observed decisive lines:

```text
node i=1 type=0 name="The Western Market" revealed=1 cleared=0 selectable=0 modifier=6
node i=8 type=4 name="The Governor's Courtyard" revealed=1 cleared=0 selectable=0 modifier=-1
screen battle
error msg="unknown command"
error msg="unknown command"
error msg="unknown command"
bye
```

Current map projection exposes unavailable node names and modifiers. Battle
protocol has no combine, generic active-item, or next-hand projection command.
These are real surface gaps, not assumptions from source reading.

Existing Phase 0 traces already prove battle, map, event, removal, relic, elite,
win, loss, and result surfaces. Reuse those routes rather than creating test-only
entry points:

| Existing route | Evidence | Reuse |
|---|---|---|
| battle commands and action state | B0-B2 | card, innate, action-cost, and item traces |
| setup/start ordering | B3 | F1 maximum-capacity and F9 setup order |
| ordinary damage/cascade | B4-B5 | F1 Timur and F9 boss/Vorath damage |
| nested card effect | B6 | F4 preview/active-item interaction boundaries |
| threshold and owner-turn defects | B7-B8 | F1 regression fixtures |
| event continuations | R1-R2 | F2 target and resume transitions |
| ordinary/event elite result paths | R3-R4 | F8 bespoke armies and reward transitions |

## Fixture rules

1. Start each fixture with normal `new` then `save` in an isolated temporary
   directory. Patch only generated text-save fields that describe legal
   persistent starting state: masteries, seed, difficulty, challenge, Vorath
   counter, battle parity, unlock bits, synergies, chains, liberation counters,
   map cleared/revealed bits, and recorded event choices.
2. Load the patched profile through `load`. All behavior evidence afterward uses
   public commands only: `enter`, `nodes`, `select`, `choose`, `remove`,
   `relic`, `hand`, `moves`, `attacks`, `buy`, `move`, `play`, `target`, `end`,
   `concede`, `save`, and `load`.
3. Never patch battle-local state. Save/load deliberately excludes the hand,
   card target state, actions, CP, meters, board pieces, pending rewards,
   current node, AI plan, and boss-local counters. A fixture must reach those
   states through gameplay commands after load.
4. Persisted state setup is not evidence. Each route records a visible state,
   board, hand, map, effect log, damage log, offer, result, or save/load result
   produced after the loaded profile is driven.
5. Every completed F phase adds its concrete seed, save patch, command script,
   expected capture, and reload assertion to this document. A route contract
   below is not a substitute for that evidence.
6. Do not add test-only engine entry points or a second fixture binary. Missing
   surface belongs in existing protocol/representation files and must expose
   normal game state or a normal player action.

## Shared setup vocabulary

| Need | Legal profile setup | Real transition and capture |
|---|---|---|
| specific card | enable only that card's saved unlock bit, then use deterministic `seed` and `hand` | `play`/`target`, then state/effect log/board |
| specific relic | enable its saved relic bit | enter a battle or select its map node; capture effect result |
| specific piece/recipe | enable saved piece bits | `buy`, then normal move/attack/combine route |
| mastery | set profile `masteries` before load; `run_new` copies it into kingdom state | enter matching kingdom and exercise innate/mastery behavior |
| deep map node | mark earlier tiers complete and a predecessor of target node cleared | `enter id=K`, `nodes`, `select i=N` |
| chain/Vorath state | patch saved `chains`, `ever`, `liberation`, `vorath`, and `battles` | select normal node, win/loss through battle, inspect map/log/save |
| five defeated overseers | patch saved `synergies=11111` only to unlock final-entry fixture | `vorath`, then real finale actions and result |

The map graph always accepts node zero, otherwise requires a cleared predecessor
(`run.c:589-623`). For a deep-node fixture, clear every prior tier and all
same-tier nodes except the target; this establishes a reachable state but does
not prove target behavior.

## Route inventory: F1 baseline defects and mastery gaps

| Requirement | Deterministic route | Capture required | Status |
|---|---|---|---|
| Counter Coup owner-turn lifetime | Existing B8 profile: human black, Counter Coup available, Wazir creates normal threat; play then end through enemy damage | Counter Coup application, reflected damage, expiry before next owner turn; run for both human colors | Existing route; F1 reuses |
| Vorath capacity | Existing B3 profile with `vorath=2`; start ordinary battle | initial maximum/meter, refill, and 200% clamp after maximum query | Existing route; F1 reuses |
| Timur below-20 crossing | Existing B7 profile: human black, Timur card, Wazir, meter 25; play and resolve | crossing, cascade completion, automatic substitution, once/twice mastery limit | Existing route; F1 reuses |
| Bulk Discount | Set relic bit 3 (`RELIC_BULK_DISCOUNT`); first Longwei battle has three actions and three cheap Bing purchases at distinct legal squares | three committed purchase costs, end-turn cheapest refund, turn reset, failed/evaluated buy exclusion | Existing route; F1 must add refund projection/log if CP delta alone is ambiguous |
| Double Time base and Selassie price | Set Kewarani mastery 1, then 3 in separate profiles; enter Kewarani, buy one home and one foreign piece in isolated runs | pre/post CP deltas and applied cost effect for no-home-discount, +40% foreign markup, then M3 +20% foreign markup | Existing setup route; F1 adds missing price behavior |
| Royal Substitution uses | Set Zarqan mastery 0/1/3; buy Zarqan piece adjacent to King, query `moves`, move onto King square, end and repeat next owner turn | ordinary action cost, first/second allowed or rejected by mastery, exact swap squares | Existing setup route; F1 adds usage state |

## Route inventory: F2 event targeting and run dispatch

All 30 authored events have one static map location. Each row below creates two
fixture scripts: choose A and choose B. Setup is the shared deep-node profile,
then `enter id=K`, `nodes`, `select i=N`, `choose a|b`. The final column states
the required post-choice transition; it is the evidence, not node selection.

| Kingdom/map/index | Event | A route | B route |
|---|---|---|---|
| Longwei Town/8 | Dragon Court Tribute | select held card target, then buy Longwei piece | select offered relic |
| Longwei Town/9 | Janggi Elder | select eligible Longwei piece type, then buy it | Vorath counter reduction then save/load |
| Longwei Province/9 | Cannon Salute | buy Pao after choice | Vorath counter reduction then save/load |
| Longwei Province/10 | Defector | select valid hidden modifier/node, then `nodes` | Vorath counter reduction then save/load |
| Longwei Country/6 | Scholar's Offer | buy Longwei piece after choice | select offered relic |
| Longwei Country/7 | Vorath's Decree | select held card, observe reduction, then save/load | select offered relic |
| Kewarani Town/8 | Salt Road Merchant | select offered relic | buy Medeq after choice |
| Kewarani Town/9 | Camel Caravan | buy Kewarani piece after choice | Vorath counter reduction then save/load |
| Kewarani Province/7 | Mansa's Court | select held card | start Kewarani battle and observe reinforcement |
| Kewarani Province/8 | Feast of Yod Abeba | select actual eligible battle node to skip | select offered relic |
| Kewarani Country/7 | Stolen Guard | event elite: real loss and real win/reward scripts | Vorath counter reduction then save/load |
| Kewarani Country/8 | Wandering Piece | Vorath counter reduction then save/load | select offered relic |
| Zarqan Town/9 | Warlord's Challenge | event elite: real loss and real win/relic scripts | Vorath counter reduction then save/load |
| Zarqan Town/10 | Bazaar of Samarkand | select two held cards, then buy Zarqan piece | Vorath counter reduction then save/load |
| Zarqan Province/9 | Mirage | event elite setup, loss, win, relic | start next battle and observe one-battle income |
| Zarqan Province/10 | Spy's Report | Vorath reduction 2 then save/load | Vorath reduction 1 then save/load |
| Zarqan Country/6 | Desert Crossing | select three valid hidden nodes, then `nodes` | select eligible chained figurehead |
| Zarqan Country/7 | Archive | select offered relic | select offered relic |
| Harushima Town/9 | Ronin | select offered relic | buy Harushima piece after choice |
| Harushima Town/10 | Spy Network | select three valid hidden nodes, then `nodes` | select offered relic |
| Harushima Province/9 | Burning Port | select two held cards, then buy Harushima piece | select offered relic |
| Harushima Province/10 | Forge Master | select eligible piece type, then buy it | select offered relic |
| Harushima Country/6 | Veteran Lance | buy Kyosha after choice | Vorath counter reduction then save/load |
| Harushima Country/7 | Deserter | select held card, then buy pawn-grade piece | select offered relic |
| Caelan Town/8 | Tournament | event elite setup, loss, win, relic | start next battle and observe one-battle income |
| Caelan Town/9 | Church's Blessing | buy Caelan piece after choice | select offered relic |
| Caelan Province/8 | Siege Engineer | select offered relic | buy Pao after choice |
| Caelan Province/9 | Pretender | event elite setup, loss, win, chain reward | select offered relic |
| Caelan Country/6 | Royal Decree | select held card, then buy Caelan piece | select offered relic |
| Caelan Country/7 | Queen's Favor | start two remaining Caelan battles and observe income | select offered relic |

Current `choose`, `remove`, and `relic` prove only fixed continuations (R1-R4).
They cannot emit typed candidate lists for cards, piece types, nodes, chain
figureheads, or relic offers. Current reveal effects only log because every node
starts revealed (`events.c:111-131`, `run.c:494-517`). Therefore every dynamic
cell above is **F2-blocked** until typed pending interaction, candidate emission,
`target i=N`, real node visibility, and one completion/resume path exist.

## Route inventory: F3-F6 composition surfaces

| Phase | Route contract | Required capture | Gap owner |
|---|---|---|---|
| F3 archive / Master's Notes | Profile enables zero or Master's Notes only; make an archive node reachable; `select i=N`; save/load; repeat with exhausted recipe pool | one versus two revealed recipe IDs, no identity branch, persisted recipe unlocks | F3 owns generic reveal-count query; current archive route exists but Master's Notes is hardcoded |
| F4 next hand / Counsel | Isolated card pool contains Counsel; `hand`/future projection exposes exactly next three; play Counsel; choose projected card; end into next draw | projected cards, selected discard, approved two-card-or-refill rule, realized next hand, modifier/bonus-draw cases | F4 blocked: no next-hand projection or generic active interaction surface |
| F4 Librarian's Notes / Deep Hand | Enable each relic in separate profile; enter battle; preview/keep-or-skip or activate held item on selected turn | projected top card and realized draw; activatable list, once-per-battle rejection, draw result | F4 blocked: no public preview, keep/skip, activatable-item list, or `activate` command |
| F5 map visibility / Surveyor's Map | Enable Surveyor's Map; enter map; inspect hidden map, clear/select successor, trigger event reveal, save/load, re-enter map | masked hidden identity/content, node reveal, modifier-only reveal, one deterministic map application | F2 first owns node visibility; F5 adds modifier visibility. Current `nodes` leaks all names and modifiers |
| F6 Divination | Isolated Longwei card pool contains Divination; play it before enemy turn; capture announced plan; end without changing board; compare actual AI execution | ordered buy/move/card plan, exact matching action log, invalidation/replan reason | F6 blocked: `ai_plan` is empty (`ai.c:199-209`); card play has no plan output |

Counsel's refill-versus-two-card rule remains an explicit user decision before F4
implementation. Fixture scripts must name the approved rule; this audit does not
choose it.

## Route inventory: F7 chain and recipe forbid

| Requirement | Route contract | Required capture | Current gap |
|---|---|---|---|
| Gold lock and Liberation Trial | Set one saved chain to Gold and make its lore-adjacent track active; `nodes` must show injected trial; select it; lose through `concede`; complete three ordinary battles; observe respawn; win trial | track lock, injected node, +2 army, restricted hand, delayed respawn, Gold cleared | F7 blocked: current Gold state has no injection/lock/trial behavior |
| recipe forbid | Set `vorath=3`; lose an ordinary battle to cross four; observe selected forbidden recipe; create matching parent pieces through `buy`; attempt public combine; save/load; retry | forbidden identity, failed matching combine, unaffected allowed combine, persisted ban | F7 blocked twice: current code only logs `forbid=1`; no forbidden state/query, and screen protocol exposes no `combine` command although `battle_combine` exists |

F7 must expose normal combination through the battle screen, for example
`combine ax=N ay=N bx=N by=N`, and return a normal success/error result. It
must not use a direct internal call for recipe evidence.

## Route inventory: F8 event elites

| Event | Deterministic entry | Required real-transition captures |
|---|---|---|
| Mirage | Zarqan Province node 9, `choose a` | Citadel enemy setup board, loss via `concede`, real win, relic continuation |
| Tournament | Caelan Town node 8, `choose a` | fixed mid-tier Caelan board, loss via `concede`, real win, relic continuation |
| Pretender | Caelan Province node 9, `choose a` | mirrored player board, loss via `concede`, real win, chain-removal continuation |

Current R4 proves event-battle suspension and relic continuation only. All three
currently begin standard regional battles, so their F8 board/rule captures are
not available until F8 implements event-owned setup data.

## Route inventory: F9 bosses and finale

| Fight | Deterministic entry | Required real-transition capture |
|---|---|---|
| Iron Strategist | complete Longwei Town/Province; make Longwei Country node 8 reachable | board, split damage, three-turn General condition, Pao salvo, win/loss/reward |
| Caravan of Conquest | complete Kewarani Town/Province; make Kewarani Country node 9 reachable | board, Salt Road movement, Double Time, two-turn reinforcement, win/loss/reward |
| Many-Faced King | complete Zarqan Town/Province; make Zarqan Country node 8 reachable | three Princes, flip redirect, overflow share, final win/loss/reward |
| Eternal Recursion | complete Harushima Town/Province; make Harushima Country node 8 reachable | four Kinsho, enemy flip, next-turn return/occupancy rule, King loss, win/loss/reward |
| Crowned Heretic | complete Caelan Town/Province; make Caelan Country node 8 reachable | full army, two or more lost pieces, later 10-per-Ghost human-turn damage, win/loss/reward |
| Vorath | enable saved `synergies=11111`; campaign `vorath` | 20x20 setup, quadrants, overshoot reset, exact-zero, next-turn liberation, title/result/mastery persistence |

All five overseer functions and `vorath_setup` are empty stubs. Current entry can
start only a standard battle, not the required boss behavior. F9 must publish
fixed seed/profile/action scripts that use real sequential turns. In particular,
Caravan cannot be accepted after one turn, and Crowned Heretic cannot be accepted
after one Ghost; both need state after the second required transition.

## Fixture-seam gap register

| Missing surface or state | Consequence | Owning phase and required resolution |
|---|---|---|
| typed run target list and `target i=N` | dynamic event choices cannot select card, piece type, node, figurehead, or offered relic | F2 adds typed pending run interaction and target emission |
| hidden node state plus masked map projection | reveal choices have no behavioral result | F2 adds node visibility; F5 adds separate modifier visibility |
| next-hand projection and active-item command | Counsel, Librarian's Notes, and Deep Hand lack observable player decisions | F4 adds projection plus generic active effect-item protocol |
| authoritative AI plan projection | Divination cannot show future execution | F6 stores/executes/emits one plan |
| Gold trial state | no route from Gold chain to lock, trial, delay, respawn, or release | F7 implements chain/map effects and persistence |
| public combine action | recipe forbid cannot be exercised by a player | F7 exposes normal combine command through existing battle protocol |
| forbidden-recipe persistence/projection | `forbid=1` does not identify or veto a recipe | F7 stores identity, emits it, queries before commit, and saves it |
| bespoke elite/boss/finale setup | board and recurring boss mechanics are indistinguishable from standard battle | F8/F9 add data/effect setup and observable logs/state |
| future interaction state persistence | next hand, visibility dimensions, recipe bans, Gold trial, and pending follow-ups cannot round-trip | F2-F7 extend existing text save/load format at their own gates |
| battle-local shortcut or forced result | would replace actual multi-turn behavior with a patched end state | prohibited; F8/F9 fixtures use real command sequences and `concede` only for loss branch |

---

# Phase 0 verdict

**Phase 0 baseline: COMPLETE. F0.5 fixture-route audit: COMPLETE.**

Required current runtime surfaces were driven through real protocol. Every
F1-F9 future trace now has a route contract or an owning-phase surface gap.
Source/header file count remains unchanged. F1 implementation remains
unstarted; no baseline or F0.5 audit result is an F1 behavior claim.

# F1 evidence — baseline defects and mastery gaps

Each fixture starts from a normal `new`/`save` in an isolated temporary
directory, patches only legal persistent fields, loads through `load`, then
drives public commands. Captures are decisive protocol output; probes are
adjacent negative checks.

## F1.1 Counter Coup owner-turn lifetime (reuses B8)

Fix: `effect_tick` moved from the end of `battle_half_turn` to the top of
`turn_start`, so a `TURNS_1` effect attached during the owner's half now
survives the opponent's half. Card keeps `TURNS_1`.

Fixture: `new seed=7 difficulty=3 challenge=7`, patch `battles=1` (human
black), `cards` bit 8 only (Counter Coup fills every draw), `pieces` all set.
Drive: `enter id=0` / `select i=0` / repeat `play i=0` then `end`.

Human black capture:

```text
log damage side=white amount=15
log effect name="Counter Coup" trigger=QUERY_METER_DAMAGE_TAKEN
log damage side=black amount=30
```

The 30 meter damage the human takes echoes 15 (50%) to the enemy before the
deduction. Human white (`battles=0`, `seed=89`):

```text
log damage side=black amount=5
log effect name="Counter Coup" trigger=QUERY_METER_DAMAGE_TAKEN
log damage side=white amount=10
```

Probe: the Phase 0 B8 baseline shows this echo absent pre-fix (the `TURNS_1`
copy expired at the playing side's half-turn end).

## F1.2 Vorath capacity (reuses B3)

Fix: `battle_meter_max` folds the piece sum through `QUERY_METER_AMOUNT`;
the capacity effect adds 20 per two losses before the maximum is used.

Fixture: `new seed=2 difficulty=0 challenge=7`, patch `vorath=2`.
Drive: `enter id=0` / `select i=0`.

```text
log effect name="Vorath Pressure" trigger=QUERY_METER_AMOUNT
state side=white turn=1 cp=30 meter=10 enemy_meter=30 actions=3
```

Probes: `vorath=0` -> `enemy_meter=10` (no fire); `vorath=4` ->
`enemy_meter=50`. Pre-fix the 200% clamp cut the lone-King case to 20.

## F1.3 Timur below-20 crossing (reuses B7)

Fix: one `ON_CASCADE_END` per damage/cascade transaction carries the lowest
meter; the watcher swaps when `battle_cascade_origin() >= 20` and lowest
`< 20`.

Fixture: `new seed=1 difficulty=1 challenge=7`, patch `battles=1` (human
black), `pieces` all set, `cards` bit 52 only (Timur's Conquest). Drive:
enter, `sell`/`buy piece=25` Wazirs beside the black king to a meter of 40,
`play i=0` (Timur), then `end` through AI attacks.

```text
log damage side=black amount=30
log effect name="Timur's Conquest" trigger=ON_CASCADE_END
row y=0 cells=····N·k·····   pre:  king at 6,0
row y=1 cells=···wwww·····
row y=0 cells=····N·w·····   post: Wazir at 6,0
row y=1 cells=···kwww·····   post: king swapped to 3,1
```

Probes: no fire when a hit lands at exactly meter 20; later crossings in the
same battle do not fire again at mastery 0 (limit 1).

## F1.4 Bulk Discount cheapest-of-three settlement

Fix: single shared mark; total per-turn discount equals the cheapest
committed cost; only `ON_PIECE_BUY` commits count.

Fixture: `new seed=2 difficulty=0 challenge=7`, patch `relics` bit 3,
`pieces` all set. Longwei battle. Bing (id 1) costs 3, piece 20 costs 12.

```text
three Bings:    cp 30 -> 27 -> 24 -> 24    3rd free (cheapest 3)
12,12,3 order:  cp 30 ------> 6 ---> 6      cheapest 3 free
3,12,12 order:  cp 30 -> 27 -> 15 -> 6      3rd pays 9 = 12 - 3
log effect name="Bulk Discount" trigger=QUERY_PIECE_CP_COST_BUY
```

Probe: two buys yield no discount.

## F1.5 Double Time pricing + Selassie mastery 3

Fix: two `QUERY_PIECE_CP_COST_BUY` effects on the Kewarani innate.

Fixture: `new seed=2 difficulty=0 challenge=7`, patch `masteries=0,1,0,0,0`
then `0,3,0,0,0`, `pieces` all set. Medeq (id 18) value 10.

```text
Medeq home (Kewarani):            paid 10   no 40% home discount
Medeq foreign (Longwei) mastery1: paid 7    140%, node modifier halves
Medeq foreign (Longwei) mastery3: paid 6    120%; DT foreign declines
```

The 7:6 ratio equals 140:120, isolating the Double Time markup.

## F1.6 Royal Substitution usage limit

Fix: shared `eff_noop` use pool; `QUERY_PIECE_MOVES` only gates,
`ON_PIECE_MOVE` counts an executed king swap.

Fixture: `new seed=2 difficulty=0 challenge=7`, patch `masteries=0,0,1,0,0`
then `0,0,3,0,0`, `pieces` all set. Zarqan battle, white king at (6,11).

```text
mastery 1: Wazir(6,10) moves offer king square 6,11; after the swap the
           next Wazir's moves omit it (no Royal Substitution fire).
mastery 3: two swaps counted (ON_PIECE_MOVE x2); the third piece's moves
           omit the king square.
```

Probe: repeated `moves` queries before a swap never spend a use.

## F1 verdict

Six baseline defects corrected and verified through the public protocol.
Source/header file count unchanged (20 .c / 5 .h). No F2+ work began.
