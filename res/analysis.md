# Daulat — EffectItem Philosophy Analysis

Per-instance audit of every EffectItem against the design invariant:

> A game element's outcome is modified **with and only with effects**.
> Effects are isolated, composable, self-filtering. Agnostic engine code
> never names a specific card/relic/modifier/kingdom/difficulty. The engine
> fires triggers; items supply effects.

## How to read

Each row: **Item · GDD behaviour · Decomposition (effect → trigger +
placement) · Current impl (`file:line`) · Status**.

Status legend:

- ✅ **compliant** — behaviour is an effect fired through a trigger.
- ⚠️ **wrong shape** — effect-driven but inconsistent with the item model
  (bare `Effect`, function-dispatched, or magnitude injected by the engine).
- ❌ **hardcoded** — an agnostic engine function names this item by id, or
  the item is a stub whose behaviour lives in imperative engine code.
- ⬜ **stub** — no behaviour implemented anywhere yet (deferred phase).

## Violation index (agnostic code naming specific items)

| Site                                                          | Item(s) named                                                                                | Fix phase    |
| ------------------------------------------------------------- | -------------------------------------------------------------------------------------------- | ------------ |
| ~~`battle_draw` battle.c:328~~ ✅ deleted                      | ~~`MODIFIER_LUCKY_STRIKE`~~ now `ON_CARDS_DRAWN` effect + `battle_draw_pool`                 | 2            |
| `screen.c`:175 (`emit_hand` blind mask)                       | `CHALLENGE_BLIND_DRAFT` — moved to `QUERY_HAND_STATE`                                        | 5            |
| ~~`battle_play` a/b codec + protocol~~ ✅ STRIPPED             | ~~every targeting card~~ now two-step QUERY_CARD_TARGETS / ON_CARD_TARGET_SELECTED           | 2.5          |
| ~~`battle_play` battle.c:1049~~ ✅ deleted                     | ~~`CARD_QUEENS_GAMBIT`~~ draws in its own effect                                             | 2.5          |
| ~~`battle_buy` mark reads~~ ✅ deleted                        | ~~`CARD_PAWN_STORM`, `CARD_REFORGE`~~ now self-contained ACTION/CP_COST_BUY + ON_PIECE_BUY effects | 4      |
| ~~`battle_begin` width if-ladder~~ ✅ QUERY_BOARD_DIMENSION    | ~~`MODIFIER_EXTENDED_FRONT`/`COMPRESSED`~~ done                                              | 3            |
| ~~`battle_terrain`~~ ✅ deleted; ON_BOARD_BUILD                | ~~`MODIFIER_DENSE_TERRAIN`, `BOARD_TRAIT_MIRAGE`, `BOARD_TRAIT_ISLAND_CHAIN`~~ done          | 3            |
| ~~`battle_walk_run` list placement~~ ✅ deleted               | ~~`RELIC_SOUL_SHARD`~~ now human-seat ON_PIECE_FLIP (gaining side fires)                     | 4            |
| ~~`battle_cascade` flip count~~ ✅ QUERY_FLIP_COUNT           | ~~`MODIFIER_BLOODBATH`~~ now +1 flip effect                                                  | 4            |
| `battle_price` battle.c:759                                   | `RELIC_TRADE_ROUTES` (+ `run_cost_bonus`) — defer w/ Phase 7 pricing cleanup                 | 7            |
| `battle_challenge_allows_buy` battle.c:702-706                | `CHALLENGE_PACIFIST_DOCTRINE`/`SOLO_VANGUARD`                                                | 5            |
| `battle_setup_armies` — ✅ `CHAIN_SILVER` → QUERY_ENEMY_ARMY_COUNT | still `DIFFICULTY_*`, `MAP_NODE_ELITE/LIBERATION`, `CHALLENGE_THE_TRAITORS_GAMBIT`      | 5            |
| `battle_walk_innates` battle.c:1768                           | `DIFFICULTY_ENSLAVED`                                                                        | 5            |
| `SYNERGY_REGISTRY` universal.c:1197                           | bare `Effect[]`, not an item                                                                 | 6            |
| `KINGDOM_INNATE/CLIMAX` universal.c:1374                      | function-pointer dispatch                                                                    | 6            |
| `run_value_bonus` run.c:1258-1309                             | events by kingdom/id ladder                                                                  | 7            |
| `run_cost_bonus` run.c:1322-1346                              | events by kingdom ladder                                                                     | 7            |
| `run_event_choose` run.c:1122-1162                            | `VORATH_REDUCTIONS` table + `EVENT_DESERT_CROSSING`                                          | 7            |
| `EVENT_NAME/TEXT/OPTION_*` universal.c:1365                   | empty parallel arrays                                                                        | 7            |
| `KINGDOM_EVENT` universal.c:1399                              | function-pointer dispatch (stub bodies)                                                      | 7            |
| `run_select_node` run.c:906                                   | `RELIC_MASTERS_NOTES`                                                                        | 7 (run-side) |
| `battle_begin` battle.c:1980 + `run_battle_result` run.c:1072 | Vorath counter thresholds                                                                    | 8            |

## Pieces (41) — `PIECE_REGISTRY`, kingdom data files

Reference category. Movement is data-driven `mv`/`at` generators (GDD §9);
conditional behaviour is an embedded `effects[]` slot self-filtering on
`battle_subject() == self` with `args[0] = self` set at spawn. All ✅.

| Piece            | GDD                                          | Decomposition (embedded effect → trigger)                                           | Impl        | Status |
| ---------------- | -------------------------------------------- | ----------------------------------------------------------------------------------- | ----------- | ------ |
| King             | 1 any dir; value 10; unbuyable; flip = lose  | pure movegen                                                                        | universal.c | ✅     |
| Bing             | fwd 1; sideways after first blood            | First Blood → QUERY_PIECE_DAMAGE_DEALT (mark); Sidestep → QUERY_PIECE_MOVES/ATTACKS | longwei.c   | ✅     |
| Xiang            | 2 diagonal; no attack in enemy territory     | River Ban → QUERY_PIECE_CAN_ATTACK                                                  | longwei.c   | ✅     |
| Ma               | knight L; elbow-blocked                      | pure movegen (elbow check)                                                          | longwei.c   | ✅     |
| Pao              | rook move; cannon-screen attack              | pure movegen (`cannon_cover`)                                                       | longwei.c   | ✅     |
| Liubo Diviner    | teleport to any enemy-threatened square      | pure movegen                                                                        | longwei.c   | ✅     |
| Sang             | 1 orthogonal then 2 diagonal                 | pure movegen                                                                        | longwei.c   | ✅     |
| Northern Cavalry | knight; grants Bulwark to adjacent allies    | Cavalry Bulwark → QUERY_PIECE_DAMAGE_TAKEN (adjacency)                              | longwei.c   | ✅     |
| Hwacha           | cannon screen on all 8 directions            | pure movegen                                                                        | longwei.c   | ✅     |
| Medeq            | berolina pawn                                | pure movegen                                                                        | kewarani.c  | ✅     |
| Makwanam         | ferz (1 diagonal)                            | pure movegen                                                                        | kewarani.c  | ✅     |
| Saba             | 2 diagonal, unrestricted                     | pure movegen                                                                        | kewarani.c  | ✅     |
| Faras            | dababbah (2 orthogonal)                      | pure movegen                                                                        | kewarani.c  | ✅     |
| Negus Guard      | moves twice; splitter on flip                | Double Time → QUERY_PIECE_ACTION_COST_MOVE; split → ON_PIECE_FLIP_PRE               | kewarani.c  | ✅     |
| Medeq Squad      | splitter on flip                             | eff_split_square → ON_PIECE_FLIP_PRE                                                | kewarani.c  | ✅     |
| Sultan's Levy    | moves twice; splitter (3 Medeq)              | Double Time + eff_split_king → ON_PIECE_FLIP_PRE                                    | kewarani.c  | ✅     |
| Wazir            | 1 orthogonal                                 | pure movegen                                                                        | zarqan.c    | ✅     |
| Jamal            | (1,3) leaper                                 | pure movegen                                                                        | zarqan.c    | ✅     |
| Talliya          | diagonal slider, min 2                       | pure movegen                                                                        | zarqan.c    | ✅     |
| Ziraafa          | 1 diagonal then 3+ straight                  | pure movegen                                                                        | zarqan.c    | ✅     |
| Shahzadeh        | king-equivalent; swaps with king             | eff_shahzadeh_swap → QUERY_PIECE_MOVES (relaxed: repeatable)                        | zarqan.c    | ✅     |
| Old King         | king movement                                | pure movegen                                                                        | zarqan.c    | ✅     |
| Cataphract       | knight or jamal                              | pure movegen (`mg_compound`)                                                        | zarqan.c    | ✅     |
| Rook             | orthogonal slider                            | pure movegen                                                                        | zarqan.c    | ✅     |
| War Elephant     | ziraafa move; hits 2 adjacent enemies        | pure movegen/coverage                                                               | zarqan.c    | ✅     |
| Fuhyo            | forward 1                                    | pure movegen                                                                        | harushima.c | ✅     |
| Kyosha           | forward slider                               | pure movegen                                                                        | harushima.c | ✅     |
| Ginsho           | 1 diagonal or forward                        | pure movegen                                                                        | harushima.c | ✅     |
| Kinsho           | 1 any except diagonal-back                   | pure movegen                                                                        | harushima.c | ✅     |
| Shishi           | acts twice per action                        | Double Time → QUERY_PIECE_ACTION_COST_MOVE                                          | harushima.c | ✅     |
| Honorable Horse  | shogi knight; promotes after enemy territory | eff_horse_cross + gold mv/at → QUERY_PIECE_MOVES/ATTACKS                            | harushima.c | ✅     |
| Promoted Bishop  | bishop + 1 orthogonal                        | pure movegen (`mg_compound`)                                                        | harushima.c | ✅     |
| Daimyo           | king move; immune to flip once               | eff_daimyo_immune → QUERY_PIECE_CAN_FLIP                                            | harushima.c | ✅     |
| Dragon           | rook + 1 diagonal                            | pure movegen (`mg_compound`)                                                        | harushima.c | ✅     |
| Pawn             | fwd 1/2; attacks diagonal                    | Double Step → QUERY_PIECE_MOVES                                                     | caelan.c    | ✅     |
| Knight           | knight L                                     | pure movegen                                                                        | caelan.c    | ✅     |
| Bishop           | diagonal slider                              | pure movegen                                                                        | caelan.c    | ✅     |
| Queen            | rook + bishop                                | pure movegen                                                                        | caelan.c    | ✅     |
| Gryphon          | 1 diagonal then any orthogonal               | pure movegen                                                                        | caelan.c    | ✅     |
| Chancellor       | rook or knight                               | pure movegen (`mg_compound`)                                                        | caelan.c    | ✅     |
| Sovereign Banner | queen; adjacency movement aura               | Banner Aura → QUERY_PIECE_MOVES                                                     | caelan.c    | ✅     |

## Cards (54) — `CARD_REGISTRY`

Play pipeline (`battle_play`): an `ON_CARD_PLAY` slot runs immediately
(x = `Card*`); any other trigger attaches to the actor's list (args[0] =
side, args[1..2] = encoded targets); `eff_noop` slots become marks. Nearly
all ✅. Exceptions flagged.

**Targeting** (sacrifice/flip/pick) **was** supplied blind via the `a`/`b`
codec at play time, forcing the protocol to know each card's encoding.
Phase 2.5 **replaced** that with the two-step `QUERY_CARD_TARGETS` /
`ON_CARD_TARGET_SELECTED` model and deleted the codec entirely; the
**Card Targeting** section below records it. Targeting-card rows stay ✅;
their decomposition now names those triggers.

| Card                       | GDD                                            | Decomposition                                                                                                            | Impl                         | Status                                                                                                                                                                                     |
| -------------------------- | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ | ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Pawn Storm                 | 3 free-action pawns, third free                | ON_CARD_PLAY attaches counter mark + ACTION_COST_BUY / CP_COST_BUY / ON_PIECE_BUY effects                                | universal.c                  | ✅ self-contained; engine reads gone (verified seed=18: 3 free-action, 3rd free cp, 4th full+action)                                                                                       |
| Revitalize                 | +50 meter                                      | ON_CARD_PLAY                                                                                                             | universal.c                  | ✅                                                                                                                                                                                         |
| Hostage                    | next flip to you: +20 meter                    | ON_PIECE_FLIP                                                                                                            | universal.c                  | ✅                                                                                                                                                                                         |
| Last Stand                 | no flip this turn                              | QUERY_PIECE_CAN_FLIP veto                                                                                                | universal.c                  | ✅                                                                                                                                                                                         |
| Sacrifice                  | remove piece, +2× value meter                  | QUERY_CARD_TARGETS + ON_CARD_TARGET_SELECTED (own non-king piece)                                                        | universal.c                  | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Reforge                    | next flip discounts that type                  | ON_PIECE_FLIP_PRE attaches mark + QUERY_PIECE_CP_COST_BUY (30% off) + ON_PIECE_BUY (spend) effects                       | universal.c                  | ✅ self-contained; engine reads gone (code-verified — needs a flip to exercise)                                                                                                            |
| Mercy                      | retarget next flip                             | QUERY_CARD_TARGETS (own piece) → ON_CARD_TARGET_SELECTED attaches ON_PIECE_FLIP_PRE observer                             | universal.c                  | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Bloodletting               | +50% missing-meter damage                      | QUERY_PIECE_DAMAGE_DEALT                                                                                                 | universal.c                  | ✅                                                                                                                                                                                         |
| Counter Coup               | echo 50% taken damage                          | QUERY_METER_DAMAGE_TAKEN                                                                                                 | universal.c                  | ✅                                                                                                                                                                                         |
| Spite                      | next loss: 3× value to enemy                   | ON_PIECE_FLIP_PRE                                                                                                        | universal.c                  | ✅                                                                                                                                                                                         |
| Chain Break                | force-flip 1 enemy                             | QUERY_CARD_TARGETS (enemy non-king) + ON_CARD_TARGET_SELECTED                                                            | universal.c                  | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Hydra                      | next flip: 2 pawns by king                     | ON_PIECE_FLIP_PRE                                                                                                        | universal.c                  | ✅                                                                                                                                                                                         |
| River Wade                 | pawn gains sideways                            | QUERY_CARD_TARGETS (pawn) + ON_CARD_TARGET_SELECTED → embeds QUERY_PIECE_MOVES                                           | longwei.c                    | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Charge                     | slider passes one blocker                      | QUERY_CARD_TARGETS (`class == MOVE_SLIDER` only, NOT king) + ON_CARD_TARGET_SELECTED → attaches QUERY_PIECE_MOVES        | longwei.c                    | ✅ (2.5b) — earlier hallucination offered any own piece incl. king; fixed to slider-class                                                                                                  |
| Formation                  | line of 3: +50% damage                         | QUERY_PIECE_DAMAGE_DEALT                                                                                                 | longwei.c                    | ✅                                                                                                                                                                                         |
| Divination                 | reveal enemy plan                              | ON_CARD_PLAY → `ai_plan`                                                                                                 | longwei.c                    | ✅ (preview relies on QUERY_NEXT_HAND / plan store — Phase 2)                                                                                                                              |
| Cannon Volley              | Pao hits row+column                            | QUERY_CARD_TARGETS (Pao) + ON_CARD_TARGET_SELECTED → `battle_damage` (flip cascade, not raw meter -=)                    | longwei.c                    | ✅ (2.5d)                                                                                                                                                                                  |
| Palace Decree              | enemy king in 3×3, 2 turns                     | ON_CARD_PLAY → attaches QUERY_PIECE_MOVES (auto: enemy king)                                                             | longwei.c                    | ✅                                                                                                                                                                                         |
| Mandate                    | remove piece, 3× value to enemy                | QUERY_CARD_TARGETS (own non-king) + ON_CARD_TARGET_SELECTED → remove + `battle_damage` (flip cascade)                    | longwei.c                    | ✅ (2.5d)                                                                                                                                                                                  |
| Mingzhu's Seal (M2)        | 1 enemy can't move 3 turns                     | QUERY_CARD_TARGETS (enemy) + ON_CARD_TARGET_SELECTED → attaches QUERY_PIECE_CAN_MOVE                                     | longwei.c                    | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Sultan's Gold              | +60 cp                                         | ON_CARD_PLAY                                                                                                             | kewarani.c                   | ✅                                                                                                                                                                                         |
| March                      | pawns move fwd 1 free                          | ON_CARD_PLAY                                                                                                             | kewarani.c                   | ✅                                                                                                                                                                                         |
| Double Time                | +1 move (3rd if innate active)                 | QUERY_CARD_TARGETS (own incl. king) + ON_CARD_TARGET_SELECTED → grant free move; 2nd free move if piece carries the innate's `eff_double_move`                                             | kewarani.c                   | ✅ (2.5d) — third-move clause added                                                                                                                                                        |
| Salt Road                  | +10 cp each turn                               | ON_TURN_START                                                                                                            | kewarani.c                   | ✅                                                                                                                                                                                         |
| Caravan                    | line moves fwd together                        | ON_CARD_PLAY                                                                                                             | kewarani.c                   | ✅                                                                                                                                                                                         |
| Doublestrike               | move 2 pieces, 1 action                        | ON_CARD_PLAY                                                                                                             | kewarani.c                   | ✅                                                                                                                                                                                         |
| Hajj                       | teleport piece anywhere                        | QUERY_CARD_TARGETS ×2 (piece, dest) + ON_CARD_TARGET_SELECTED                                                            | kewarani.c                   | ✅ (multi-target → 2.5)                                                                                                                                                                    |
| Selassie's March (M2)      | Kewarani move 3×                               | ON_CARD_PLAY                                                                                                             | kewarani.c                   | ✅                                                                                                                                                                                         |
| Ronin                      | passive: target's next flip refunds its value  | QUERY_CARD_TARGETS (own piece) + ON_CARD_TARGET_SELECTED → attaches ON_PIECE_FLIP observer                               | harushima.c                  | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Resurrection               | reclaim any flipped piece free                 | QUERY_CARD_TARGETS (`battle_piece_flips > 0`, cumulative counter not MARK) + ON_CARD_TARGET_SELECTED → `battle_flip`     | harushima.c                  | ✅ (2.5c)                                                                                                                                                                                  |
| Gold Standard              | target moves as Kinsho this turn               | QUERY_CARD_TARGETS (piece) + ON_CARD_TARGET_SELECTED → attaches QUERY_PIECE_MOVES (`mg_compound` Kinsho)                 | harushima.c                  | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Promotion                  | target gains Ginsho movement this battle       | QUERY_CARD_TARGETS (piece) + ON_CARD_TARGET_SELECTED → attaches QUERY_PIECE_MOVES                                        | harushima.c                  | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Dual Drop                  | reclaim ≤2 flipped @30cp; else free Fuhyo      | **dynamic** per-step: offers flipped enemies (`battle_piece_flips > 0`), or empty squares to drop a Fuhyo when none remain, so always playable + Fuhyo location chosen                     | harushima.c                  | ✅ (2.5c)                                                                                                                                                                                  |
| Force Drop                 | place unlocked piece ≤50 on empty square       | QUERY_CARD_TARGETS ×2 (**PIECE_TYPE** `battle_piece_unlocked` & ≤50, then **SQUARE** empty) + ON_CARD_TARGET_SELECTED; spawn "for free", NO meter gain                                     | harushima.c                  | ✅ (2.5d) — spurious `meter += value` removed                                                                                                                                              |
| Bushido                    | passive: target's flip deals 2× value to enemy | QUERY_CARD_TARGETS (piece) + ON_CARD_TARGET_SELECTED → attaches ON_PIECE_FLIP observer → `battle_damage` (flip cascade)  | harushima.c                  | ✅ (2.5d)                                                                                                                                                                                  |
| Tomohito's Patience (M2)   | reclaim ≤3 flipped @15cp each                  | QUERY_CARD_TARGETS ×≤3 (flipped pieces) + ON_CARD_TARGET_SELECTED                                                        | harushima.c                  | ✅ (multi-target → 2.5)                                                                                                                                                                    |
| Counsel                    | peek next 3, discard 1                         | QUERY_CARD_TARGETS advertises 3 **NEXT_CARD** targets (from QUERY_NEXT_HAND) + ON_CARD_TARGET_SELECTED discards the pick | zarqan.c                     | ✅ (targeting → 2.5; preview data Phase 2)                                                                                                                                                 |
| Pillage                    | +5 cp per friendly piece                       | ON_CARD_PLAY                                                                                                             | zarqan.c                     | ✅                                                                                                                                                                                         |
| Royal Decoy                | swap 2 friendly                                | QUERY_CARD_TARGETS ×2 (own pieces) + ON_CARD_TARGET_SELECTED                                                             | zarqan.c                     | ✅ (multi-target → 2.5)                                                                                                                                                                    |
| Bazaar                     | sell piece for 150%                            | QUERY_CARD_TARGETS (own piece) + ON_CARD_TARGET_SELECTED                                                                 | zarqan.c                     | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Steppe Riders              | knights/jamals/cataphracts move 2×             | ON_CARD_PLAY                                                                                                             | zarqan.c                     | ✅                                                                                                                                                                                         |
| Ambition                   | copy movement 1 turn                           | QUERY_CARD_TARGETS ×2 (piece, copied) + ON_CARD_TARGET_SELECTED (`mg_compound`)                                          | zarqan.c                     | ✅ (multi-target → 2.5)                                                                                                                                                                    |
| Citadel                    | piece immobile+immune 2 turns                  | QUERY_CARD_TARGETS (piece) + ON_CARD_TARGET_SELECTED → attaches CAN_MOVE/ATTACK/CAN_FLIP                                 | zarqan.c                     | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Conquest                   | permanently copy movement                      | QUERY_CARD_TARGETS ×2 (piece, copied) + ON_CARD_TARGET_SELECTED                                                          | zarqan.c                     | ✅ (multi-target → 2.5)                                                                                                                                                                    |
| Timur's Conquest (M2)      | auto Royal Sub < 20 meter                      | attaches observer                                                                                                        | zarqan.c                     | ✅                                                                                                                                                                                         |
| Castling                   | king ↔ rook swap                               | QUERY_CARD_TARGETS (own rook) + ON_CARD_TARGET_SELECTED                                                                  | caelan.c                     | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Queen's Gambit             | sacrifice pawn (any-kingdom); draw 3           | QUERY_CARD_TARGETS (`piece_is_pawn`) + ON_CARD_TARGET_SELECTED → remove pawn + `battle_draw(3)`                          | caelan.c                     | ✅ (2.5) — battle.c:1049 3-draw hardcode DELETED; draws in its own effect so `battle_draw` fires ON_CARDS_DRAWN and Lucky Strike composes free                                            |
| Vengeance                  | 2× to enemy that moved adjacent last turn      | **automatic** ON_CARD_PLAY scan (no target): sums 2× value of each enemy with `battle_piece_move_turn == turn-1` adjacent to a friendly, one `battle_damage`                                                                       | caelan.c                     | ✅ (2.5d) — was wrongly a targeting card ("way off"); GDD has no "Target", the condition is the selector                                                                                    |
| Queen's Decree             | next attack ×2                                 | QUERY_PIECE_DAMAGE_DEALT                                                                                                 | caelan.c                     | ✅                                                                                                                                                                                         |
| Cathedral                  | bishop-defended: −40%                          | QUERY_PIECE_DAMAGE_TAKEN                                                                                                 | caelan.c                     | ✅                                                                                                                                                                                         |
| Coronation                 | promote pawn → queen                           | QUERY_CARD_TARGETS (`piece_is_pawn`, any kingdom — not just PIECE_PAWN) + ON_CARD_TARGET_SELECTED                        | caelan.c                     | ✅ (2.5b)                                                                                                                                                                                  |
| Crusade                    | knight 3 lunges                                | QUERY_CARD_TARGETS (own knight) + ON_CARD_TARGET_SELECTED → `battle_lunge`                                               | caelan.c                     | ✅ (targeting → 2.5)                                                                                                                                                                       |
| Divine Right               | king attacks as queen                          | QUERY_PIECE_ATTACKS                                                                                                      | caelan.c                     | ✅                                                                                                                                                                                         |
| Isabella's Coronation (M2) | all pawns → queens                             | ON_CARD_PLAY                                                                                                             | caelan.c                     | ✅                                                                                                                                                                                         |

## Card Targeting — two-step play (`QUERY_CARD_TARGETS` / `ON_CARD_TARGET_SELECTED`)

**Current model — protocol-seam violation.** `battle_play(battle, hand, a,
b)` takes pre-encoded targets `a`/`b` (codec §4: square `y*20+x`, piece-type
`+1000`, card `+2000`); the card's `ON_CARD_PLAY` effect reads them from
`args[1..2]`. This forces the **protocol/UI/AI caller to know each card's
target encoding** — per-card knowledge living outside the card, the same
seam violation as display masking.

**Targets are typed, not just squares** — modularity: a new target kind
extends the enum, never the protocol or `battle_play`.

```c
enum TargetKind {
    TARGET_SQUARE,      // value = y*20 + x   (board piece / empty square)
    TARGET_NEXT_CARD,   // value = slot in the projected next hand
    TARGET_PIECE_TYPE,  // value = PieceID (from the unlocked set)
    TARGET_NONE,        // list terminator (like SQUARE_END)
};
struct CardTarget { TargetKind kind; int value; };
```

**New model — two-step, self-describing.**

1. `battle_play(battle, hand)` — no targets. Pay cost, run no-target
   `ON_CARD_PLAY` work, fire the play broadcast (Tax Stamp, synergy, combo /
   climax). If the card has a `QUERY_CARD_TARGETS` effect it needs a target:
   fire it to enumerate legal `CardTarget`s, emit them, park a
   `PENDING_CARD`, return.
2. `battle_card_target(battle, index)` — the protocol sends a plain **index
   into the advertised list**; battle re-derives the descriptor, fires
   `ON_CARD_TARGET_SELECTED` with it, the card resolves (immediately, or by
   attaching a target-bound observer), the play finalizes (combo count,
   clear the hand slot).

**Triggers (Phase 2.5):**

- `QUERY_CARD_TARGETS` — x = `CardTarget*` the card appends to,
  `TARGET_NONE`-terminated (same shape as a movegen list). The card
  advertises its own legal targets, of whatever kind.
- `ON_CARD_TARGET_SELECTED` — x = `CardTarget*` (the chosen descriptor).

**Protocol relays only an index.** It emits the advertised descriptors —
`kind` tells the UI how to render (highlight a square / show a next-hand
card / show a piece-type menu), `value` identifies the thing — and the
player returns an index. No square, no codec, nothing card-specific crosses
the protocol seam.

**Multi-target** cards fire the query once per slot, accumulate choices in
context, and resolve on the last; **slots may mix kinds**: Force Drop =
`TARGET_PIECE_TYPE` then `TARGET_SQUARE`; Ambition/Conquest = piece then
`TARGET_PIECE_TYPE` (copied); Hajj = piece then dest square; Royal Decoy /
Dual Drop / Tomohito's = repeated piece squares. **Counsel** advertises its
three `TARGET_NEXT_CARD` targets (sourced by firing `QUERY_NEXT_HAND`) and
discards the picked one — an ordinary targeting card, not a separate path.
An empty target list makes the play unplayable (reject / refund).

**Targeting cards** (now carry the two triggers; the blind `a`/`b` codec is
gone): Sacrifice, Mercy, Chain Break, River Wade, Charge, Cannon Volley,
Mandate, Mingzhu's Seal, Double Time, Hajj, Royal Decoy, Bazaar, Ambition,
Citadel, Conquest, Counsel, Castling, Queen's Gambit, Coronation, Crusade
— plus the Harushima piece pickers (Ronin, Resurrection, Gold Standard,
Promotion, Dual Drop, Force Drop, Bushido). Kinds beyond board squares:
Counsel (`NEXT_CARD`), Force Drop / Ambition / Conquest (`PIECE_TYPE`).
**Vengeance is NOT here** — the GDD condition ("enemy that moved adjacent
last turn") is the selector, so it is **automatic** (`ON_CARD_PLAY` scan),
not a manual target (2.5d fix). **No-target** cards (immediate / auto /
passive: Revitalize, Pillage, Sultan's Gold, Palace Decree auto-king,
Divine Right, Isabella's, Formation, Bloodletting, etc.) keep plain
`ON_CARD_PLAY` and are unaffected.

Phase 2.5 shipped in slices: **2.5** two-step + multi-target infra and all
card migrations, `a`/`b` fully stripped; **2.5b** consolidation + first GDD
pass; **2.5c** de-bloat — one `card_targets_*` per category taking an inline
Clang block, magic marks (`MARK_MOVED`/`MARK_FLIPPED`) replaced by
`QUERY_PIECE_HAS_MOVED`/`HAS_FLIPPED` counters + move-turn timestamps,
`QUERY_CARD_CAN_PLAY` dry-run playability, Dual Drop made dynamic; **2.5d**
full GDD fidelity audit (Vengeance→automatic, `battle_damage` for enemy
meter on Cannon Volley/Mandate/Bushido, Force Drop meter gain removed,
Double Time third-move clause). All targeting cards ✅ effect-driven; the
protocol holds zero per-card target knowledge.

## Relics (26) — `RELIC_REGISTRY`, relic.c

Attached to the human seat at `battle_begin` (`battle_walk_run`). Effect
relics ✅; six are stubs whose behaviour lives at a hardcoded call-site.

| Relic                 | GDD                           | Decomposition                                      | Impl                                                                   | Status                                                                                                                   |
| --------------------- | ----------------------------- | -------------------------------------------------- | ---------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| Merchant's Ledger     | +5 sell                       | QUERY_CARD_SELL_COST                               | relic.c                                                                | ✅                                                                                                                       |
| Minted Coin           | +5 income/turn                | QUERY_CP_INCOME                                    | relic.c                                                                | ✅                                                                                                                       |
| Tax Stamp             | +10 on paid card              | ON_CARD_PLAY                                       | relic.c                                                                | ✅                                                                                                                       |
| Bulk Discount         | cheapest of 3+ free           | QUERY_PIECE_CP_COST_BUY + ON_TURN_START reset      | relic.c                                                                | ✅ (impl frees the 3rd — flagged interpretation; reset targets wrong context, see note)                                  |
| War Chest             | unspent cp → 20% meter        | ON_TURN_END                                        | relic.c                                                                | ✅                                                                                                                       |
| **Trade Routes**      | remove foreign markup for run | should be QUERY_PIECE_CP_COST_BUY                  | **battle.c:759** (`run->relics[RELIC_TRADE_ROUTES]` in `battle_price`) | ❌ hardcoded id in agnostic pricing; item is a stub                                                                      |
| Soul Shard            | gaining flip: +30 meter       | eff_soul_shard → ON_PIECE_FLIP (human seat)        | relic.c                                                               | ✅ on human seat; ON_PIECE_FLIP fires the gaining side's list so it runs only on flips onto the human — no engine id check |
| Veteran's Bond        | 50+ value: +20 meter          | QUERY_PIECE_DAMAGE_DEALT (effective value)         | relic.c                                                                | ✅                                                                                                                       |
| Dead Man's Pact       | first lethal → reset 20       | QUERY_METER_DAMAGE_TAKEN                           | relic.c                                                                | ✅                                                                                                                       |
| Iron King             | king +10 meter                | QUERY_PIECE_DAMAGE_DEALT (effective value)         | relic.c                                                                | ✅                                                                                                                       |
| Bloodthirst           | leading: +5 meter             | ON_TURN_START                                      | relic.c                                                                | ✅                                                                                                                       |
| Last Breath           | friendly flip → enemy damage  | ON_PIECE_FLIP                                      | relic.c                                                                | ✅                                                                                                                       |
| Tactician's Scroll    | draw 4                        | QUERY_CARD_DRAW_COUNT                              | relic.c                                                                | ✅                                                                                                                       |
| **Librarian's Notes** | peek + skip top card          | QUERY_NEXT_HAND (needs deterministic next-hand pre-roll) | relic.c (stub)                                              | ⬜ Phase 2 preview slice — deferred with Counsel data + `QUERY_NEXT_HAND`; not yet built                                 |
| Country Seal          | +20 sell (country)            | QUERY_CARD_SELL_COST                               | relic.c                                                                | ✅                                                                                                                       |
| **Deep Hand**         | +2 draw once/battle           | should be QUERY_CARD_DRAW_COUNT (player-triggered) | relic.c (stub)                                                         | ⬜ Phase 2                                                                                                               |
| Gilded Archive        | +10 sell (district)           | QUERY_CARD_SELL_COST                               | relic.c                                                                | ✅                                                                                                                       |
| Alchemist's Kit       | combine 0 actions             | QUERY_PIECE_ACTION_COST_COMBINE                    | relic.c                                                                | ✅                                                                                                                       |
| **Master's Notes**    | archive reveals 2 recipes     | run-side reveal                                    | **run.c:906** (`run->relics[RELIC_MASTERS_NOTES]`)                     | ❌ hardcoded id in `run_select_node`; item is a stub                                                                     |
| Philosopher's Stone   | +20 once/battle               | ON_PIECE_COMBINE                                   | relic.c                                                                | ✅                                                                                                                       |
| Inherited Power       | +5 all combined               | ON_PIECE_COMBINE                                   | relic.c                                                                | ✅                                                                                                                       |
| Eagle Eye             | enemy pieces always shown     | QUERY_BOARD_STATE: resets view all-visible + noops every OTHER QUERY_BOARD_STATE effect (seat + pieces) except itself, each read → catches mid-battle blinders | relic.c            | ✅ Phase 3 (GDD "values" read as the piece — hiding a lookup-able value is no nerf; hiding the piece is)                 |
| **Surveyor's Map**    | pre-reveal 1 modifier/map     | run/map-side reveal                                | relic.c (stub)                                                         | ⬜ Phase 3 (run-side)                                                                                                    |
| Forward Command       | +5 damage in enemy territory  | QUERY_PIECE_DAMAGE_DEALT                           | relic.c                                                                | ✅                                                                                                                       |
| Fortified Line        | +5 damage if unmoved          | QUERY_PIECE_DAMAGE_DEALT; `battle_piece_move_turn != turn` (timestamp, no MARK) | relic.c                                                   | ✅ (2.5c)                                                                                                                |
| Warlord's Banner      | +5 damage adjacent to king    | QUERY_PIECE_DAMAGE_DEALT                           | relic.c                                                                | ✅                                                                                                                       |

Note — **Bulk Discount latent bug**: `eff_bulk_reset` clears its **own**
context (relic.c:94), not `eff_bulk_buy`'s separate context, so the per-turn
counter never resets. Correct pattern = a shared `eff_noop` mark both
effects find (like Pawn Storm's counter). Fold into Phase 4.

## Battle Modifiers (18) — `MODIFIER_REGISTRY`, universal.c

One copy per seat (`battle_walk_modifiers`), args[0] = that seat. 12 ✅,
6 stubs whose behaviour is hardcoded in the engine.

| Modifier           | GDD                            | Decomposition                   | Impl                                             | Status                                                      |
| ------------------ | ------------------------------ | ------------------------------- | ------------------------------------------------ | ----------------------------------------------------------- |
| Lean Times         | −20 cp start                   | ON_BATTLE_START                 | universal.c                                      | ✅                                                          |
| Windfall           | +30 cp start                   | ON_BATTLE_START                 | universal.c                                      | ✅                                                          |
| Open Market        | pieces 50%                     | QUERY_PIECE_CP_COST_BUY         | universal.c                                      | ✅                                                          |
| Devalued Currency  | sell halved                    | QUERY_CARD_SELL_COST            | universal.c                                      | ✅                                                          |
| Tax Collector      | +5 on sell                     | ON_CARD_SELL                    | universal.c                                      | ✅                                                          |
| Glass Cannon       | refill 50%                     | QUERY_METER_REFILL              | universal.c                                      | ✅                                                          |
| Bloodbath          | 2 flips per empty              | eff_bloodbath → QUERY_FLIP_COUNT +1 | universal.c                                  | ✅ cascade fires QUERY_FLIP_COUNT (base 1) for the emptied side; effect adds 1                                    |
| Iron Will          | 25% recoil                     | QUERY_METER_DAMAGE_TAKEN        | universal.c                                      | ✅                                                          |
| Overflow           | gained flip +30 value          | ON_PIECE_FLIP                   | universal.c                                      | ✅                                                          |
| Mirror             | 25% self-heal                  | QUERY_METER_DAMAGE_TAKEN        | universal.c                                      | ✅                                                          |
| Rich Hand          | draw 4                         | QUERY_CARD_DRAW_COUNT           | universal.c                                      | ✅                                                          |
| Sparse Hand        | draw 2                         | QUERY_CARD_DRAW_COUNT           | universal.c                                      | ✅                                                          |
| Kingdom Purity     | region cards only              | QUERY_CARD_CAN_DRAW             | universal.c                                      | ✅                                                          |
| Lucky Strike       | first card highest tier        | ON_CARDS_DRAWN (x = `Card**`)   | universal.c                                      | ✅ Phase 2 — `battle_draw` hardcode deleted; once-per-turn (stamps `turn` in ctx), replaces `hand[0]` with a random top-tier from the shared `battle_draw_pool` via `battle_rand` (GDD "any highest-tier available", not forced-first) |
| Fog of War         | hide enemy pieces off attacked | QUERY_BOARD_STATE (piece, not value) | universal.c                                 | ✅ Phase 3                                                 |
| Dense Terrain      | 20% voids                      | ON_BOARD_BUILD                  | universal.c                                      | ✅ Phase 3                                                 |
| Extended Front     | +2 columns                     | QUERY_BOARD_DIMENSION           | universal.c                                      | ✅ Phase 3                                                 |
| Compressed         | −2 columns                     | QUERY_BOARD_DIMENSION           | universal.c                                      | ✅ Phase 3                                                 |

## Board Traits (10) — `TRAIT_REGISTRY`, kingdom data files

One copy per seat (`battle_walk_traits`). 4 ✅, 6 stubs.

| Trait                | GDD                                       | Decomposition                             | Impl                                         | Status                     |
| -------------------- | ----------------------------------------- | ----------------------------------------- | -------------------------------------------- | -------------------------- |
| River Crossing       | Xiang can't cross; Bing sideways on cross | eff_river_crossing → QUERY_PIECE_MOVES (prune) + eff_river_bing → ON_PIECE_MOVE (grant) | longwei.c | ✅ verified seed=2: Xiang cross-river moves pruned, Bing gains sideways on crossing |
| The Palace           | king confined to 3×3                      | eff_palace → QUERY_PIECE_MOVES            | longwei.c                                    | ✅                         |
| Trade Route          | +1 movement on diagonal path              | eff_trade_route → QUERY_PIECE_MOVES (main-diagonal piece gains empty king-steps) | kewarani.c | ✅ verified seed=8: diagonal piece +1 in all dirs, non-diagonal unaffected (interp: "+1 movement" = +1 square all directions) |
| Contested Market     | spawn claimable neutral each turn         | eff_contested_market → ON_TURN_START; claim resolves via QUERY_SQUARE_OWNER | kewarani.c | ✅ resolve-then-spawn: neutral market pieces claimed by territory holder or removed if contested, then a random Kewarani Town piece spawns neutral on a contested square |
| Sandstorm            | even turns sliders ≤ 3                    | eff_sandstorm → QUERY_PIECE_MOVES         | zarqan.c                                     | ✅                         |
| Mirage               | 5% voids                                  | ON_BOARD_BUILD                            | zarqan.c                                     | ✅ Phase 3                 |
| Fog Coast            | hide far-3-row enemy pieces until moved   | QUERY_BOARD_STATE (piece, not value)      | harushima.c                                  | ✅ Phase 3                 |
| Island Chain         | 10% voids middle                          | ON_BOARD_BUILD                            | harushima.c                                  | ✅ Phase 3                 |
| Castle Corners       | 2×2 corners immune                        | eff_castle_corners → QUERY_PIECE_CAN_FLIP | caelan.c                                     | ✅                         |
| Siege Trench         | middle row immune                         | eff_siege_trench → QUERY_PIECE_CAN_FLIP   | caelan.c                                     | ✅                         |

## Chain Penalties (3) — `CHAIN_REGISTRY`, universal.c

Attached to the human seat at `battle_setup_armies` when the region is
chained.

| Chain      | GDD                                | Decomposition                           | Impl                                                     | Status                                                                       |
| ---------- | ---------------------------------- | --------------------------------------- | -------------------------------------------------------- | ---------------------------------------------------------------------------- |
| Bronze     | −10 cp (−25 Shackled)              | eff_bronze_chain → QUERY_CP_INCOME turn-1 −10 | universal.c; the opening penalty rides the shared cp seam | ✅ opening cp composes on QUERY_CP_INCOME; engine names no chain |
| Silver     | enemy +1 starting piece            | eff_silver_chain → QUERY_ENEMY_ARMY_COUNT +1 | universal.c (chains attached cumulatively Bronze..level) | ✅ setup fires QUERY_ENEMY_ARMY_COUNT for the human seat; effect adds 1  |
| Gold       | figurehead subjugated, track locks | run-state (chain cap + liberation node) | run.c chain logic                                        | ✅ run-state (no battle effect) — liberation node deferred                   |

## Kingdom Synergies (5) — `SYNERGY_REGISTRY`, universal.c

Earned by clearing an overseer; attached to the human seat when fighting
the lore-adjacent region (`battle_walk_synergies`). Effect-driven, but the
registry is a bare `const Effect[]` — no name/desc, inconsistent with every
other item. All ⚠️ (wrong shape → reshape to EffectItem in Phase 6).

| Synergy            | GDD                      | Decomposition                              | Impl             | Status         |
| ------------------ | ------------------------ | ------------------------------------------ | ---------------- | -------------- |
| Longwei → Kewarani | Pao +10 damage           | eff_syn_pao → QUERY_PIECE_DAMAGE_DEALT     | universal.c:1197 | ⚠️ bare Effect |
| Kewarani → Zarqan  | Kewarani pieces −10 cp   | eff_syn_kewarani → QUERY_PIECE_CP_COST_BUY | universal.c      | ⚠️ bare Effect |
| Zarqan → Harushima | Ziraafa/Talliya +5 value | eff_syn_zarqan → QUERY_PIECE_DAMAGE_DEALT  | universal.c      | ⚠️ bare Effect |
| Harushima → Caelan | Caelan card draws +1     | eff_syn_harushima → ON_CARD_PLAY           | universal.c      | ⚠️ bare Effect |
| Caelan → Longwei   | Sultan's Gold +10 cp     | eff_syn_caelan → ON_CARD_PLAY              | universal.c      | ⚠️ bare Effect |

## Kingdom Innates (5) — `KINGDOM_INNATE[]` fn dispatch

Each is a function that attaches an effect (effect-driven) but is
function-pointer-dispatched, not a data EffectItem; mastery scaling is
passed as a fn arg. All ⚠️ (→ data EffectItem registry in Phase 6).

| Innate                      | GDD                                    | Decomposition                                                   | Impl        | Status                                                                       |
| --------------------------- | -------------------------------------- | --------------------------------------------------------------- | ----------- | ---------------------------------------------------------------------------- |
| Bulwark (Longwei)           | adjacent Longwei: −50% (−60% M3)       | eff_bulwark → QUERY_PIECE_DAMAGE_TAKEN                          | longwei.c   | ⚠️ fn-dispatched                                                             |
| Double Time (Kewarani)      | Kewarani move twice                    | embeds eff_double_move + ON_PIECE_BUY hook                      | kewarani.c  | ⚠️ fn-dispatched; GDD cost (no home discount / +40% foreign) NOT implemented |
| Royal Substitution (Zarqan) | swap with king, once, free             | eff_royal_sub → QUERY_PIECE_MOVES                               | zarqan.c    | ⚠️ fn-dispatched; relaxed to repeatable + costs action; M3 (twice) not impl  |
| Reclaim (Harushima)         | convert flipped piece, 30 cp + action  | base reclaim always available; innate attaches only M3 cost cut | harushima.c | ⚠️ fn-dispatched; innate near-no-op                                          |
| Conqueror's Reward (Caelan) | Caelan flip-helper +50% value (60% M3) | eff_conqueror → ON_PIECE_FLIP (damager attribution)             | caelan.c    | ⚠️ fn-dispatched                                                             |

## Kingdom Climaxes (5) — `KINGDOM_CLIMAX[]` fn dispatch

Fired at the 3rd same-kingdom card play (`battle_play:1064`). Some attach a
turn-long effect; some act imperatively (no effect at all). All ⚠️.

| Climax    | GDD                          | Decomposition                                 | Impl        | Status                       |
| --------- | ---------------------------- | --------------------------------------------- | ----------- | ---------------------------- |
| Longwei   | all pieces Bulwark this turn | eff_longwei_climax → QUERY_PIECE_DAMAGE_TAKEN | longwei.c   | ⚠️ fn-dispatched             |
| Kewarani  | all pieces +1 move           | imperative loop (`piece_grant_free_move`)     | kewarani.c  | ⚠️ fn-dispatched, imperative |
| Zarqan    | swap any 2 pieces            | imperative (swaps top-2 value)                | zarqan.c    | ⚠️ fn-dispatched, imperative |
| Harushima | reclaim 1 flipped free       | imperative (`battle_flip` first enemy)        | harushima.c | ⚠️ fn-dispatched, imperative |
| Caelan    | all pieces +50% damage       | eff_caelan_climax → QUERY_PIECE_DAMAGE_DEALT  | caelan.c    | ⚠️ fn-dispatched             |

## Narrative Events (30 × 2 choices) — the biggest violation

`EVENT_NAME/TEXT/OPTION_*` are empty `= {}` (universal.c:1365).
`KINGDOM_EVENT[]` handler bodies are all empty stubs. Actual behaviour is
split across three hardcoded run.c ladders: `run_value_bonus` (1258),
`run_cost_bonus` (1322), `run_event_choose`→`VORATH_REDUCTIONS` (1122) +
Desert Crossing (1155). Many choices (relic offers, card removal, map
reveals, elite battles, skip battle, next-battle cp) are **unimplemented**.

All ❌ → become `Event` EffectItems (name/desc/option text) with
run-immediate `ON_EVENT_CHOOSE` effects + run-persistent `ENTIRE_RUN`
effects (Phase 7).

| Event                   | Choice A (GDD)                            | Choice B (GDD)                              | Current impl                                                  | Status                                         |
| ----------------------- | ----------------------------------------- | ------------------------------------------- | ------------------------------------------------------------- | ---------------------------------------------- |
| The Scholar's Offer     | Longwei +3 value/run                      | relic (Veteran's Bond / Tax Stamp)          | A: run_value_bonus:1264; B: **unimpl**                        | ❌                                             |
| Dragon Court Tribute    | remove card; Longwei +5 value             | relic (Bulk Discount / Fortified Line)      | A: run_value_bonus:1268 (card-removal unimpl); B: unimpl      | ❌                                             |
| The Defector            | reveal next modifier                      | −1 Vorath                                   | A: unimpl; B: VORATH_REDUCTIONS                               | ❌                                             |
| The Janggi Elder        | one Longwei type +5 value                 | −1 Vorath                                   | A: unimpl; B: VORATH_REDUCTIONS                               | ❌                                             |
| Cannon Salute           | all Pao +5 value                          | −2 Vorath                                   | A: run_value_bonus:1284; B: VORATH_REDUCTIONS                 | ❌                                             |
| Mansa's Court           | remove 1 card                             | refuse: Kewarani enemies +1 piece/run       | both **unimpl**                                               | ❌                                             |
| Salt Road Merchant      | relic (Soul Shard / Gilded Archive)       | all Medeq +3 value                          | A: unimpl; B: run_value_bonus:1293                            | ❌                                             |
| The Stolen Guard        | elite; win → remove Bronze chain          | −1 Vorath                                   | A: unimpl; B: VORATH_REDUCTIONS                               | ❌                                             |
| The Camel Caravan       | Kewarani −15% cost                        | −2 Vorath                                   | A: run_cost_bonus:1325; B: VORATH_REDUCTIONS                  | ❌                                             |
| Feast of Yod Abeba      | skip next battle                          | relic (Last Breath / Warlord's Banner)      | both **unimpl**                                               | ❌                                             |
| The Warlord's Challenge | elite; win → relic                        | −2 Vorath                                   | A: unimpl; B: VORATH_REDUCTIONS                               | ❌                                             |
| Bazaar of Samarkand     | remove 2 cards; Zarqan −15% cost          | −2 Vorath                                   | A: run_cost_bonus:1330 (removal unimpl); B: VORATH_REDUCTIONS | ❌                                             |
| The Mirage              | elite (Citadel enemies); win → relic      | +15 cp next battle                          | both **unimpl**                                               | ❌                                             |
| The Spy's Report        | −2 Vorath                                 | −1 Vorath                                   | both VORATH_REDUCTIONS                                        | ❌ (only run-immediate; still hardcoded table) |
| The Desert Crossing     | reveal 3 nodes ahead                      | remove 1 Bronze chain                       | A: unimpl; B: run_event_choose:1155 special-case              | ❌                                             |
| The Ronin               | relic (Eagle Eye / Forward Command)       | all Harushima +3 value                      | A: unimpl; B: run_value_bonus:1273                            | ❌                                             |
| The Spy Network         | reveal 3 nodes                            | relic (Minted Coin / Surveyor's Map)        | both **unimpl**                                               | ❌                                             |
| The Burning Port        | remove 2 Harushima cards; −15% cost       | relic (Country Seal / Fortified Line)       | A: run_cost_bonus:1335 (removal unimpl); B: unimpl            | ❌                                             |
| The Forge Master        | one piece type −20% cost                  | relic (Veteran's Bond / Tactician's Scroll) | both **unimpl**                                               | ❌                                             |
| The Veteran Lance       | all Kyosha +5 value                       | −1 Vorath                                   | A: run_value_bonus:1298; B: VORATH_REDUCTIONS                 | ❌                                             |
| The Tournament          | elite; win → relic                        | +20 cp next battle                          | both **unimpl**                                               | ❌                                             |
| The Church's Blessing   | Caelan −10% cost                          | relic (Gilded Archive / Bulk Discount)      | A: run_cost_bonus:1340; B: unimpl                             | ❌                                             |
| The Siege Engineer      | relic (Master's Notes / Alchemist's Kit)  | all Pao +5 value                            | A: unimpl; B: run_value_bonus:1288                            | ❌                                             |
| The Pretender           | elite (mirror); win → remove Silver chain | relic (Dead Man's Pact / Trade Routes)      | both **unimpl**                                               | ❌                                             |
| The Royal Decree        | remove card; Caelan +3 value              | relic (Iron King / Bloodthirst)             | A: run_value_bonus:1278 (removal unimpl); B: unimpl           | ❌                                             |
| The Queen's Favor       | +25 cp each remaining battle this map     | relic (Veteran's Bond / Minted Coin)        | both **unimpl**                                               | ❌                                             |
| The Wandering Piece     | −2 Vorath                                 | relic (Veteran's Bond / Deep Hand)          | A: VORATH_REDUCTIONS; B: unimpl                               | ❌                                             |
| Vorath's Decree         | remove card; −3 Vorath                    | relic (Soul Shard / Forward Command)        | A: VORATH_REDUCTIONS (removal unimpl); B: unimpl              | ❌                                             |
| The Deserter            | remove card; all pawns +3 value           | relic (Warlord's Banner / Dead Man's Pact)  | A: run_value_bonus:1303 (removal unimpl); B: unimpl           | ❌                                             |
| The Archive             | relic (Philosopher's / Inherited)         | relic (Master's Notes / Alchemist's Kit)    | both **unimpl**                                               | ❌                                             |

## Difficulties (4) — `DIFFICULTY_REGISTRY` EffectItems ✅

`DIFFICULTY_REGISTRY[DIFFICULTY_NONE]` (universal.c) attached cumulatively to
the human seat by `battle_walk_rules` at battle start; every inline `if` in
the engine is deleted. Difficulties stack (a higher level includes the
lower penalties), mirroring the cumulative chain attach.

| Difficulty | GDD                                | Decomposition                                   | Impl                          | Status       |
| ---------- | ---------------------------------- | ----------------------------------------------- | ----------------------------- | ------------ |
| Free       | baseline                           | —                                               | registry no-effect entry      | ✅ baseline  |
| Bound      | Vorath pressure starts +2          | eff_bound → QUERY_ENEMY_ARMY_COUNT +2           | universal.c                   | ✅           |
| Shackled   | Bronze becomes −25 cp              | eff_shackled → QUERY_CP_INCOME turn-1 −15 in a chained region | universal.c                   | ✅           |
| Enslaved   | enemy innates active from battle 1 | eff_enslaved → ON_BATTLE_SETUP attach enemy innate | universal.c                | ✅           |

## Challenge Runs (6) — `CHALLENGE_REGISTRY` EffectItems ✅

`CHALLENGE_REGISTRY[CHALLENGE_COUNT]` (universal.c) attached to the human
seat by `battle_walk_rules`; the engine names no challenge. Two remain
genuinely non-effect (category G) and carry no-effect registry entries.

| Challenge            | GDD                            | Decomposition                          | Impl        | Status                          |
| -------------------- | ------------------------------ | -------------------------------------- | ----------- | ------------------------------- |
| Daily Conquest       | same seed for all players/day  | seed choice                            | run_new     | ⬜ G — run setup, not an effect |
| Solo Vanguard        | max 1 piece on board           | eff_solo_vanguard → QUERY_PIECE_CAN_BUY veto | universal.c | ✅                         |
| Pacifist Doctrine    | no buy > 20 cp                 | eff_pacifist → QUERY_PIECE_CAN_BUY veto | universal.c | ✅                              |
| Blind Draft          | hide card id/name until played | eff_blind_draft → QUERY_HAND_STATE     | universal.c | ✅                              |
| The Traitor's Gambit | enemy 3 pieces in your half    | eff_traitors_gambit → ON_BATTLE_SETUP spawn | universal.c | ✅                         |
| Clockwork            | 30 s/turn                      | real-time timer                        | screen/UI   | ⬜ G — timer, not game state    |

## Overseers (5) — `KINGDOM_OVERSEER[]` fn dispatch, all stubs

Bespoke boss battles (army + special rules). All empty stubs; deferred to
master-plan Phase 7 (separate from this rectification's phases). When built,
their rules should compose from effects, not engine special-cases.

| Overseer                       | GDD mechanic                             | Impl                | Status                   |
| ------------------------------ | ---------------------------------------- | ------------------- | ------------------------ |
| Iron Strategist (Longwei)      | damage split; flip 2 Generals; 3 Pao     | longwei.c stub      | ⬜ deferred (MP Phase 7) |
| Caravan of Conquest (Kewarani) | +1 piece/2 turns; Salt Road; Double Time | kewarani.c stub     | ⬜ deferred (MP Phase 7) |
| Many-Faced King (Zarqan)       | 3 Princes; overflow redirect             | zarqan.c stub       | ⬜ deferred (MP Phase 7) |
| Eternal Recursion (Harushima)  | flipped pieces return; 4 Kinsho          | harushima.c stub    | ⬜ deferred (MP Phase 7) |
| Crowned Heretic (Caelan)       | full army; ghosts; ~520 meter            | caelan.c stub       | ⬜ deferred (MP Phase 7) |
| (Vorath)                       | Grand + 5 Minor Kings; quadrant pressure | `vorath_setup` stub | ⬜ deferred (MP Phase 7) |

## Global Vorath Counter (1) — hardcoded thresholds

| Threshold      | GDD                       | Decomposition          | Impl                            | Status                                                              |
| -------------- | ------------------------- | ---------------------- | ------------------------------- | ------------------------------------------------------------------- |
| Every 2 losses | enemy meters +20 baseline | ON_BATTLE_START effect | **battle.c:1980**               | ❌ hardcoded (master-plan §4:540 already specifies ON_BATTLE_START) |
| Every 4 losses | forbid 1 random recipe    | run effect             | **run.c:1072** (state/log only) | ❌ hardcoded                                                        |

---

## Summary

| Category        | ✅      | ⚠️     | ❌     | ⬜     | Total   |
| --------------- | ------- | ------ | ------ | ------ | ------- |
| Pieces          | 41      |        |        |        | 41      |
| Cards           | 51      | 2      | 1      |        | 54      |
| Relics          | 19      | 1      | 2      | 4      | 26      |
| Modifiers       | 12      |        | 5      | 1      | 18      |
| Board Traits    | 4       |        | 2      | 4      | 10      |
| Chain Penalties | 1       | 1      | 1      |        | 3       |
| Synergies       |         | 5      |        |        | 5       |
| Innates         |         | 5      |        |        | 5       |
| Climaxes        |         | 5      |        |        | 5       |
| Events          |         |        | 30     |        | 30      |
| Difficulties    | 1       |        | 3      |        | 4       |
| Challenges      |         |        | 4      | 2      | 6       |
| Overseers       |         |        |        | 6      | 6       |
| Vorath Counter  |         |        | 2      |        | 2       |
| **Total**       | **129** | **19** | **50** | **17** | **215** |

**129 ✅** already effect-driven (pieces, most cards/relics/modifiers, the
compliant traits) prove the pattern. **19 ⚠️** are effect-driven but the
wrong shape (bare-Effect synergies, fn-dispatched innates/climaxes, the
Soul Shard placement, the two mark-read-inline cards, the Bronze magnitude
injection). **50 ❌** are the real violations: agnostic code naming a
specific item, or a stub whose behaviour is hardcoded — dominated by the 30
events. **17 ⬜** are unimplemented stubs to build as effects (display
relics/modifiers/traits, unimplemented traits) plus category-G non-effects
(Daily Conquest, Clockwork) and the Phase-7 overseers.

Beyond value-mutation, the philosophy reaches two seams that the plan also
closes: the **protocol** queries board/hand/target state through triggers
(Phase 2.5/3) so `screen.c` and the card-play caller name no item, and the
**run** fires its own effect list (Phase 7) so events/difficulties never
sit in hardcoded ladders.

Phases 2–8 of the plan drive ⚠️/❌/⬜ → ✅. Category-G items (Daily
Conquest seed, Clockwork timer, AI archetypes) stay as-is by design.
