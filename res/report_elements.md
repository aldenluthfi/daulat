# Regnum — Element Catalog

**Source**: `res/GDD.md`, `incl/effect.h`, `incl/movegen.h`, `incl/screen.h`,
`incl/profile.h`, `incl/run.h`, `src/effects/*.c`, `src/movegens/*.c`
**Generated**: 2026-06-15
**Scope**: Every piece, card, relic, mastery card, innate, penalty chain,
battle modifier, board trait, figurehead power, overseer, synergy, event,
AI archetype, screen hook, and trigger — mapped to the function pointer(s)
and argument lists that implement them. Each entry includes a
**Descriptive Behaviour** row showing the GDD's design intent.
Unimplemented behaviors are marked `STUB` with a brief description.

---

## Conventions

Each entry is organized by element type. Within each type, entries are
grouped by kingdom (Universal first, then Longwei, Harushima, Kewarani,
Zarqan, Caelan, Mastery). Within a kingdom, entries are ordered by tier
(District → Town → Town → Town → Province → Province → Country →
Capstone).

### Piece table columns

| Column                    | Meaning                                           |
| ------------------------- | ------------------------------------------------- |
| **Descriptive Behaviour** | GDD vision statement of movement and abilities    |
| **MoveGen**               | `MoveGenFunc` + `EffectArg[]` params              |
| **Threat**                | `MoveGenFunc` + params (or `= move` if identical) |
| **Passives**              | `EffectFunc` + trigger + args                     |

### Card table columns

| Column                    | Meaning                                         |
| ------------------------- | ----------------------------------------------- |
| **Descriptive Behaviour** | GDD vision statement of card effect             |
| **Tier**                  | District / Town / Province / Country            |
| **Cost**                  | Play cost in centipawns (`-` if no play action) |
| **Sell**                  | Sell value in centipawns                        |
| **On Play**               | `EffectFunc` + trigger + args                   |
| **On Sell**               | `EffectFunc` + trigger + args (usually empty)   |

### Meta table columns

| Column                    | Meaning                                          |
| ------------------------- | ------------------------------------------------ |
| **Descriptive Behaviour** | GDD vision statement of effect                   |
| **Trigger**               | When the effect activates                        |
| **Apply**                 | `EffectFunc` + args                              |
| **Scope**                 | `local` (this turn) / `battle` / `run` / `piece` |

---

## 1. Pieces

### 1.1 Universal

#### King

| Field                 | Value                                                                                                                                         |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                                                     |
| Tier                  | King                                                                                                                                          |
| Value                 | 10                                                                                                                                            |
| Descriptive Behaviour | The sovereign piece. Moves one square in any direction. Cannot be bought; only one exists per side. Capturing the enemy king wins the battle. |
| MoveGen               | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)`                                                                  |
| Threat                | `= move`                                                                                                                                      |
| Passives              | None                                                                                                                                          |

---

### 1.2 Longwei

#### Bing

| Field                 | Value                                                                                                                            |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                          |
| Tier                  | District                                                                                                                         |
| Value                 | 10                                                                                                                               |
| Descriptive Behaviour | Forward 1. Gains a sideways step after first damaging an enemy piece. The cornerstone of Longwei's defensive formation strategy. |
| MoveGen               | `mg_step`, args: `(dx=0, dy=1)`                                                                                                  |
| Threat                | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)`                                                               |
| Passives              | **Sideways step after damage**: `eff_sideways_step`, trigger: `TRIGGER_PIECE_DEALT_DAMAGE`, args: `(self)`                       |

#### Xiang

| Field                 | Value                                                                                                             |
| --------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                           |
| Tier                  | District                                                                                                          |
| Value                 | 20                                                                                                                |
| Descriptive Behaviour | 2 diagonal. Cannot attack in enemy territory. The advisor who sees far but cannot strike deep into hostile lands. |
| MoveGen               | `mg_lw_xiang`, args: none                                                                                         |
| Threat                | partial (cannot attack into enemy territory)                                                                      |
| Passives              | None                                                                                                              |

#### Ma

| Field                 | Value                                                                                                                             |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                           |
| Tier                  | Town                                                                                                                              |
| Value                 | 30                                                                                                                                |
| Descriptive Behaviour | Knight L. Blocked if any piece sits in the elbow square. The horse that leaps over obstructions, but only when the path is clear. |
| MoveGen               | `mg_lw_ma`, args: none                                                                                                            |
| Threat                | `= move`                                                                                                                          |
| Passives              | None                                                                                                                              |

#### Pao

| Field                 | Value                                                                                                                                                                                                   |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                                                                                                 |
| Tier                  | Province                                                                                                                                                                                                |
| Value                 | 50                                                                                                                                                                                                      |
| Descriptive Behaviour | Rook-line movement. Can only attack if exactly one piece (the screen) sits between Pao and target on that line. The cannon that needs a screen to fire — mastery lies in creating and breaking screens. |
| MoveGen               | `mg_lw_pao`, args: none                                                                                                                                                                                 |
| Threat                | `= move`                                                                                                                                                                                                |
| Passives              | None                                                                                                                                                                                                    |

#### Liubo Diviner

| Field                 | Value                                                                                                                                     |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                                   |
| Tier                  | Overseer Reward                                                                                                                           |
| Value                 | 100                                                                                                                                       |
| Descriptive Behaviour | Teleports to any square currently threatened by any enemy piece. The diviner who reads the enemy's intent and appears where danger looms. |
| MoveGen               | `mg_lw_liubo`, args: none                                                                                                                 |
| Threat                | `mg_todo` (teleport — no threat squares)                                                                                                  |
| Passives              | **Teleport to threatened square**: `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(self)`                                              |

#### Sang

| Field                 | Value                                                                                                                                                                        |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                                                                      |
| Tier                  | Town                                                                                                                                                                         |
| Value                 | 30                                                                                                                                                                           |
| Descriptive Behaviour | 1 orthogonal then 2 diagonal (3 total). Blockable at either intermediate square. May enter enemy territory. The elephant combining forward momentum with sweeping diagonals. |
| MoveGen               | `mg_lw_sang`, args: none                                                                                                                                                     |
| Threat                | `= move`                                                                                                                                                                     |
| Passives              | None                                                                                                                                                                         |

#### Northern Cavalry

| Field                 | Value                                                                                                    |
| --------------------- | -------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                  |
| Tier                  | Province                                                                                                 |
| Value                 | 40                                                                                                       |
| Descriptive Behaviour | Knight L. Grants Bulwark to adjacent allies. The guardian knight whose presence fortifies the formation. |
| MoveGen               | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)`                           |
| Threat                | `= move`                                                                                                 |
| Passives              | **Bulwark**: `eff_bulwark`, trigger: `TRIGGER_RESOLVE_DEFENSE`, args: `(self, multiplier=2)`             |

#### Hwacha

| Field                 | Value                                                                                                                       |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                     |
| Tier                  | Country                                                                                                                     |
| Value                 | 60                                                                                                                          |
| Descriptive Behaviour | Pao mechanic applied to diagonals as well as rows and columns. The multi-directional cannon that threatens across all axes. |
| MoveGen               | `mg_lw_hwacha`, args: none                                                                                                  |
| Threat                | `= move`                                                                                                                    |
| Passives              | None                                                                                                                        |

---

### 1.3 Harushima

#### Fuhyo

| Field                 | Value                                                                        |
| --------------------- | ---------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                    |
| Tier                  | District                                                                     |
| Value                 | 10                                                                           |
| Descriptive Behaviour | Forward 1. The basic soldier of the Shogunate, advancing inexorably forward. |
| MoveGen               | `mg_step`, args: `(dx=0, dy=1)`                                              |
| Threat                | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)`           |
| Passives              | None                                                                         |

#### Kyosha

| Field                 | Value                                                                                                         |
| --------------------- | ------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                     |
| Tier                  | District                                                                                                      |
| Value                 | 30                                                                                                            |
| Descriptive Behaviour | Forward any distance (slider). The vertical rook that controls files and pressures deep into enemy territory. |
| MoveGen               | `mg_slide`, args: `(dx=0, dy=1, min=1, max=UNBOUNDED)`                                                        |
| Threat                | `= move`                                                                                                      |
| Passives              | None                                                                                                          |

#### Ginsho

| Field                 | Value                                                                                           |
| --------------------- | ----------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                       |
| Tier                  | Town                                                                                            |
| Value                 | 30                                                                                              |
| Descriptive Behaviour | 1 square diagonal or forward. The silver general with limited retreat, pressing forward always. |
| MoveGen               | `mg_hs_ginsho`, args: none                                                                      |
| Threat                | `= move`                                                                                        |
| Passives              | None                                                                                            |

#### Kinsho

| Field                 | Value                                                                                    |
| --------------------- | ---------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                |
| Tier                  | Province                                                                                 |
| Value                 | 35                                                                                       |
| Descriptive Behaviour | 1 square any direction except diagonal-backward. The gold general, versatile and secure. |
| MoveGen               | `mg_hs_kinsho`, args: none                                                               |
| Threat                | `= move`                                                                                 |
| Passives              | None                                                                                     |

#### Honorable Horse

| Field                 | Value                                                                                                                                         |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                                                     |
| Tier                  | Town                                                                                                                                          |
| Value                 | 40                                                                                                                                            |
| Descriptive Behaviour | Shogi knight (2 forward + 1 sideways). Becomes Kinsho movement after first entering enemy territory. The horse that transforms upon conquest. |
| MoveGen               | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)`                                                                |
| Threat                | `= move`                                                                                                                                      |
| Passives              | **Swap to Kinsho on territory entry**: `eff_swap_movegen`, trigger: `TRIGGER_PIECE_ENTERED_ENEMY_TERR`, args: `(self, mg_hs_kinsho)`          |

#### Promoted Bishop

| Field                 | Value                                                                                                   |
| --------------------- | ------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                               |
| Tier                  | Province                                                                                                |
| Value                 | 50                                                                                                      |
| Descriptive Behaviour | Bishop movement + 1 square orthogonal. The promoted bishop, soaring diagonally but also stepping aside. |
| MoveGen               | `mg_compound`, args: `(mg_slide_dirs diag, mg_step_set ferz)`                                           |
| Threat                | `= move`                                                                                                |
| Passives              | None                                                                                                    |

#### Shishi

| Field                 | Value                                                                                                                           |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                                       |
| Tier                  | Overseer Reward                                                                                                                 |
| Value                 | 100                                                                                                                             |
| Descriptive Behaviour | Acts twice per move action: move-then-move, or move-then-attack-without-moving. The lion king with overwhelming action economy. |
| MoveGen               | `mg_hs_shishi`, args: none                                                                                                      |
| Threat                | `= move`                                                                                                                        |
| Passives              | None                                                                                                                            |

#### Daimyo

| Field                 | Value                                                                                                                    |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Harushima                                                                                                                |
| Tier                  | Country                                                                                                                  |
| Value                 | 50                                                                                                                       |
| Descriptive Behaviour | King movement (1 any direction). Immune to flipping once, then normal. The sovereign who defies capture once per battle. |
| MoveGen               | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)`                                             |
| Threat                | `= move`                                                                                                                 |
| Passives              | **Immune once per battle**: `eff_immune_once`, trigger: `TRIGGER_RESOLVE_FLIP`, args: `(self)`                           |

#### Dragon

| Field                 | Value                                                                                                       |
| --------------------- | ----------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                   |
| Tier                  | Capstone                                                                                                    |
| Value                 | 70                                                                                                          |
| Descriptive Behaviour | Rook movement + 1 square diagonal. The dragon combining horizontal dominance with a single diagonal thrust. |
| MoveGen               | `mg_compound`, args: `(mg_slide_dirs ortho, mg_step_set ferz)`                                              |
| Threat                | `= move`                                                                                                    |
| Passives              | None                                                                                                        |

---

### 1.4 Kewarani

#### Medeq

| Field                 | Value                                                                                                                          |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Kewarani                                                                                                                       |
| Tier                  | District                                                                                                                       |
| Value                 | 10                                                                                                                             |
| Descriptive Behaviour | Diagonally 1, attacks forward 1 (Berolina pawn). The pawn that moves sideways but strikes forward, unpredictable to opponents. |
| MoveGen               | `mg_kw_berolina`, args: none                                                                                                   |
| Threat                | `mg_attack_only_subset`, args: `(mg_step diag forward, mg_step_set diag)`                                                      |
| Passives              | None                                                                                                                           |

#### Makwanam

| Field                 | Value                                                            |
| --------------------- | ---------------------------------------------------------------- |
| Kingdom               | Kewarani                                                         |
| Tier                  | District                                                         |
| Value                 | 15                                                               |
| Descriptive Behaviour | 1 diagonal (ferz). The swift diagonal mover, nimble but limited. |
| MoveGen               | `mg_step_set`, args: `(1,1) (-1,1) (1,-1) (-1,-1)`               |
| Threat                | `= move`                                                         |
| Passives              | None                                                             |

#### Saba

| Field                 | Value                                                                                                    |
| --------------------- | -------------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                                 |
| Tier                  | Town                                                                                                     |
| Value                 | 20                                                                                                       |
| Descriptive Behaviour | 2 diagonal. Unrestricted by territory. The far-ranging bishop equivalent that crosses boundaries freely. |
| MoveGen               | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)`                           |
| Threat                | `= move`                                                                                                 |
| Passives              | None                                                                                                     |

#### Faras

| Field                 | Value                                                                                                    |
| --------------------- | -------------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                                 |
| Tier                  | Province                                                                                                 |
| Value                 | 30                                                                                                       |
| Descriptive Behaviour | Second square orthogonally (Dababbah). The leaper covering distant orthogonal squares in a single bound. |
| MoveGen               | `mg_leap_set`, args: `(1,3) (3,1) (3,-1) (1,-3) (-1,-3) (-3,-1) (-3,1) (-1,3)`                           |
| Threat                | `= move`                                                                                                 |
| Passives              | None                                                                                                     |

#### Medeq Squad

| Field                 | Value                                                                                                                                      |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Kewarani                                                                                                                                   |
| Tier                  | Town                                                                                                                                       |
| Value                 | 20                                                                                                                                         |
| Descriptive Behaviour | Pawn movement. When flipped: removed, 2 friendly Medeq spawn adjacent to its square. The squad that scatters into skirmishers upon defeat. |
| MoveGen               | `mg_kw_berolina`, args: none                                                                                                               |
| Threat                | `= move`                                                                                                                                   |
| Passives              | **Splitter**: `eff_splitter_medeq`, trigger: `TRIGGER_PIECE_FLIPPED`, args: `(self, spawn_count=2)`                                        |

#### Sultan's Levy

| Field                 | Value                                                                                                                                                                                |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Kewarani                                                                                                                                                                             |
| Tier                  | Province                                                                                                                                                                             |
| Value                 | 110                                                                                                                                                                                  |
| Descriptive Behaviour | Negus Guard movement. When flipped: removed, 3 friendly Medeq spawn adjacent to your king. Consuming the capstone is irreversible. The army that dissolves into many upon sacrifice. |
| MoveGen               | `mg_kw_negus_guard`, args: none                                                                                                                                                      |
| Threat                | `= move`                                                                                                                                                                             |
| Passives              | **Splitter**: `eff_splitter_medeq`, trigger: `TRIGGER_PIECE_FLIPPED`, args: `(self, spawn_count=3)`                                                                                  |

#### Negus Guard

| Field                 | Value                                                                                                                                                                                        |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                                                                                                                     |
| Tier                  | Overseer Reward                                                                                                                                                                              |
| Value                 | 100                                                                                                                                                                                          |
| Descriptive Behaviour | Moves twice per action. When it would flip: removed from board instead, 2 friendly Medeq spawn adjacent to your king. The elite guard who refuses capture, instead summoning reinforcements. |
| MoveGen               | `mg_kw_negus_guard`, args: none                                                                                                                                                              |
| Threat                | `= move`                                                                                                                                                                                     |
| Passives              | **Splitter**: `eff_splitter_medeq`, trigger: `TRIGGER_PIECE_FLIPPED`, args: `(self, spawn_count=2)`                                                                                          |

---

### 1.5 Zarqan

#### Wazir

| Field                 | Value                                                            |
| --------------------- | ---------------------------------------------------------------- |
| Kingdom               | Zarqan                                                           |
| Tier                  | District                                                         |
| Value                 | 15                                                               |
| Descriptive Behaviour | 1 orthogonal. The basic orthogonal stepper, steady and reliable. |
| MoveGen               | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0)`                 |
| Threat                | `= move`                                                         |
| Passives              | None                                                             |

#### Jamal

| Field                 | Value                                                                          |
| --------------------- | ------------------------------------------------------------------------------ |
| Kingdom               | Zarqan                                                                         |
| Tier                  | District                                                                       |
| Value                 | 20                                                                             |
| Descriptive Behaviour | (1,3) leaper. The camel equivalent covering distant files.                     |
| MoveGen               | `mg_leap_set`, args: `(1,3) (3,1) (3,-1) (1,-3) (-1,-3) (-3,-1) (-3,1) (-1,3)` |
| Threat                | `= move`                                                                       |
| Passives              | None                                                                           |

#### Talliya

| Field                 | Value                                                                              |
| --------------------- | ---------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                             |
| Tier                  | Town                                                                               |
| Value                 | 30                                                                                 |
| Descriptive Behaviour | Diagonal slider. Must move at least 2. The vizier who plots long diagonal courses. |
| MoveGen               | `mg_slide_dirs`, args: `(diag_mask, min=2, max=UNBOUNDED)`                         |
| Threat                | `= move`                                                                           |
| Passives              | None                                                                               |

#### Ziraafa

| Field                 | Value                                                                                                             |
| --------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                                            |
| Tier                  | Province                                                                                                          |
| Value                 | 35                                                                                                                |
| Descriptive Behaviour | 1 diagonal step, then 3+ squares straight. The elephant combining a single diagonal with a long orthogonal slide. |
| MoveGen               | `mg_zq_ziraafa`, args: none                                                                                       |
| Threat                | `mg_zq_war_elephant`, args: none                                                                                  |
| Passives              | None                                                                                                              |

#### Old King

| Field                 | Value                                                                                                         |
| --------------------- | ------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                                        |
| Tier                  | Town                                                                                                          |
| Value                 | 30                                                                                                            |
| Descriptive Behaviour | King movement. Counts as a normal piece. The aged monarch who moves like a king but can be sacrificed freely. |
| MoveGen               | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)`                                  |
| Threat                | `= move`                                                                                                      |
| Passives              | None                                                                                                          |

#### Cataphract

| Field                 | Value                                                                                          |
| --------------------- | ---------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                         |
| Tier                  | Province                                                                                       |
| Value                 | 40                                                                                             |
| Descriptive Behaviour | May move as Knight or Jamal each action. The versatile warrior choosing between leap patterns. |
| MoveGen               | `mg_choice`, args: `(mg_leap_set knight, mg_leap_set camel)`                                   |
| Threat                | `= move`                                                                                       |
| Passives              | None                                                                                           |

#### Shahzadeh

| Field                 | Value                                                                                                                                                          |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                                                                                         |
| Tier                  | Overseer Reward                                                                                                                                                |
| Value                 | 100                                                                                                                                                            |
| Descriptive Behaviour | Second king-equivalent. Once per battle, swaps positions with the real king as a free action. The prince who protects the sovereign through position exchange. |
| MoveGen               | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0)`                                                                                                               |
| Threat                | `= move`                                                                                                                                                       |
| Passives              | **Free king swap**: `eff_free_king_swap`, trigger: `TRIGGER_TURN_START`, args: `(self)`                                                                        |

#### Rook

| Field                 | `mg_slide_dirs`, args: `(ortho_mask, min=1, max=UNBOUNDED)`                                            |
| --------------------- | ------------------------------------------------------------------------------------------------------ |
| Kingdom               | Zarqan                                                                                                 |
| Tier                  | Capstone                                                                                               |
| Value                 | 50                                                                                                     |
| Descriptive Behaviour | Orthogonal slider. The only way to obtain a Rook. Unlocks from run start, gates Dragon and Chancellor. |
| MoveGen               | `mg_slide_dirs`, args: `(ortho_mask, min=1, max=UNBOUNDED)`                                            |
| Threat                | `= move`                                                                                               |
| Passives              | None                                                                                                   |

#### War Elephant

| Field                 | Value                                                                                                                          |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Zarqan                                                                                                                         |
| Tier                  | Country                                                                                                                        |
| Value                 | 50                                                                                                                             |
| Descriptive Behaviour | Ziraafa movement. Deals damage to two adjacent enemies simultaneously. The elephant whose presence threatens multiple targets. |
| MoveGen               | `mg_zq_ziraafa`, args: none                                                                                                    |
| Threat                | `mg_zq_war_elephant`, args: none                                                                                               |
| Passives              | None                                                                                                                           |

---

### 1.6 Caelan

#### Pawn

| Field                 | Value                                                                                                                     |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                                    |
| Tier                  | District                                                                                                                  |
| Value                 | 10                                                                                                                        |
| Descriptive Behaviour | Forward 1 (or 2 if haven't moved before). Attacks diagonally. The basic soldier with double-step potential on first move. |
| MoveGen               | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)`                                                        |
| Threat                | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)`                                                        |
| Passives              | **First move 2 steps**: `eff_first_move_two`, trigger: `TRIGGER_PIECE_PLACED`, args: `(self)`                             |

#### Knight

| Field                 | Value                                                                          |
| --------------------- | ------------------------------------------------------------------------------ |
| Kingdom               | Caelan                                                                         |
| Tier                  | District                                                                       |
| Value                 | 30                                                                             |
| Descriptive Behaviour | Knight L. The classic leaping horse, unblockable and unpredictable.            |
| MoveGen               | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)` |
| Threat                | `= move`                                                                       |
| Passives              | None                                                                           |

#### Bishop

| Field                 | Value                                                                         |
| --------------------- | ----------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                        |
| Tier                  | Town                                                                          |
| Value                 | 30                                                                            |
| Descriptive Behaviour | Diagonal slider. The prelate controlling the diagonals, limited to one color. |
| MoveGen               | `mg_slide_dirs`, args: `(diag_mask, min=1, max=UNBOUNDED)`                    |
| Threat                | `= move`                                                                      |
| Passives              | None                                                                          |

#### Queen

| Field                 | Value                                                                                  |
| --------------------- | -------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                 |
| Tier                  | Province                                                                               |
| Value                 | 90                                                                                     |
| Descriptive Behaviour | Rook and bishop combined. The most powerful piece, combining all directional movement. |
| MoveGen               | `mg_slide_dirs`, args: `(full_mask, min=1, max=UNBOUNDED)`                             |
| Threat                | `= move`                                                                               |
| Passives              | None                                                                                   |

#### Gryphon

| Field                 | Value                                                                                                             |
| --------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                            |
| Tier                  | Overseer Reward                                                                                                   |
| Value                 | 100                                                                                                               |
| Descriptive Behaviour | 1 diagonal step then any distance orthogonal. The dragon horse combining diagonal approach with orthogonal sweep. |
| MoveGen               | `mg_ca_gryphon`, args: none                                                                                       |
| Threat                | `= move`                                                                                                          |
| Passives              | None                                                                                                              |

#### Chancellor

| Field                 | Value                                                                                                               |
| --------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                              |
| Tier                  | Province                                                                                                            |
| Value                 | 70                                                                                                                  |
| Descriptive Behaviour | Moves as Rook or leaps as Knight each action. The versatile commander choosing between ranged and leaping movement. |
| MoveGen               | `mg_choice`, args: `(mg_slide_dirs ortho, mg_leap_set knight)`                                                      |
| Threat                | `= move`                                                                                                            |
| Passives              | None                                                                                                                |

#### Sovereign Banner

| Field                 | Value                                                                                                                                                               |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                                                                              |
| Tier                  | Province                                                                                                                                                            |
| Value                 | 110                                                                                                                                                                 |
| Descriptive Behaviour | Queen movement. Adjacent allies: step-movers gain 1 extra square, leapers extend longest leg by 1, sliders unaffected. The banner whose presence empowers the army. |
| MoveGen               | `mg_slide_dirs`, args: `(full_mask, min=1, max=UNBOUNDED)`                                                                                                          |
| Threat                | `= move`                                                                                                                                                            |
| Passives              | **Adjacency buff**: `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(self, buff_type=meter_bonus_adjacent)`                                                       |

#### King

| Field                 | Value                                                                                            |
| --------------------- | ------------------------------------------------------------------------------------------------ |
| Kingdom               | Caelan                                                                                           |
| Tier                  | Country                                                                                          |
| Value                 | 60                                                                                               |
| Descriptive Behaviour | King movement. The sovereign piece, moving one square in any direction but essential to victory. |
| MoveGen               | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)`                     |
| Threat                | `= move`                                                                                         |
| Passives              | None                                                                                             |

---

## 2. Cards

### 2.1 Universal (12)

#### Pawn Storm

| Field                 | Value                                                                                 |
| --------------------- | ------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                             |
| Tier                  | District                                                                              |
| Descriptive Behaviour | Buy up to 3 pawns this turn; the third is free. An overwhelming wave of cheap pieces. |
| Cost                  | -                                                                                     |
| Sell                  | 15                                                                                    |
| On Play               | `eff_pawn_storm`, trigger: `TRIGGER_CARD_PLAYED`, args: `(count=3, free_3rd=true)`    |
| On Sell               | —                                                                                     |

#### Revitalize

| Field                 | Value                                                                                      |
| --------------------- | ------------------------------------------------------------------------------------------ |
| Kingdom               | Universal                                                                                  |
| Tier                  | District                                                                                   |
| Descriptive Behaviour | Restore 50 to your meter. A healing tide that refills vitality.                            |
| Cost                  | 80                                                                                         |
| Sell                  | 15                                                                                         |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(heal_target=ally_piece, amount=meter)` |
| On Sell               | —                                                                                          |

#### Hostage

| Field                 | Value                                                                                                      |
| --------------------- | ---------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                  |
| Tier                  | District                                                                                                   |
| Descriptive Behaviour | Passive. Next flip to your side: your meter gains 20 bonus. A political gambit that profits from captures. |
| Cost                  | -                                                                                                          |
| Sell                  | 20                                                                                                         |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(next_flip_to_side=passive)`                            |
| On Sell               | —                                                                                                          |

#### Last Stand

| Field                 | Value                                                                                                            |
| --------------------- | ---------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                        |
| Tier                  | Town                                                                                                             |
| Descriptive Behaviour | This turn, your meter cannot trigger a flip. Damage still applies. A desperate stand that delays the inevitable. |
| Cost                  | -                                                                                                                |
| Sell                  | 30                                                                                                               |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=defense_up, duration=1)`                          |
| On Sell               | —                                                                                                                |

#### Sacrifice

| Field                 | Value                                                                                           |
| --------------------- | ----------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                       |
| Tier                  | Town                                                                                            |
| Descriptive Behaviour | Remove one of your pieces. Gain its value x2 as meter. Trading a piece for overwhelming energy. |
| Cost                  | -                                                                                               |
| Sell                  | 25                                                                                              |
| On Play               | `eff_sacrifice`, trigger: `TRIGGER_CARD_PLAYED`, args: `(meter_bonus=2x_value)`                 |
| On Sell               | —                                                                                               |

#### Reforge

| Field                 | Value                                                                                                                                  |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                                              |
| Tier                  | Town                                                                                                                                   |
| Descriptive Behaviour | Passive. Next time one of your pieces flips, the cost for the first piece of that type is discounted 30% next turn. A strategic reset. |
| Cost                  | -                                                                                                                                      |
| Sell                  | 30                                                                                                                                     |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(next_flip_to_side=passive)`                                                        |
| On Sell               | —                                                                                                                                      |

#### Mercy

| Field                 | Value                                                                                     |
| --------------------- | ----------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                 |
| Tier                  | Province                                                                                  |
| Descriptive Behaviour | Target which piece flips next, overrides any previous uses. Controlling fate itself.      |
| Cost                  | -                                                                                         |
| Sell                  | 45                                                                                        |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(heal_target=ally_meter, amount=flat)` |
| On Sell               | —                                                                                         |

#### Bloodletting

| Field                 | Value                                                                                                      |
| --------------------- | ---------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                  |
| Tier                  | Province                                                                                                   |
| Descriptive Behaviour | This turn, all your attacks deal extra damage equal to 50% of your missing meter. Desperation fuels power. |
| Cost                  | -                                                                                                          |
| Sell                  | 45                                                                                                         |
| On Play               | `eff_bloodletting`, trigger: `TRIGGER_RESOLVE_ATTACK`, args: `(damage_mult=2)`                             |
| On Sell               | —                                                                                                          |

#### Counter Coup

| Field                 | Value                                                                                                      |
| --------------------- | ---------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                  |
| Tier                  | Province                                                                                                   |
| Descriptive Behaviour | This turn, all damage you take echoes back at 50% to enemy meter. Turning the enemy's strike against them. |
| Cost                  | -                                                                                                          |
| Sell                  | 50                                                                                                         |
| On Play               | `eff_counter_coup`, trigger: `TRIGGER_RESOLVE_DEFENSE`, args: `(counter_damage=1)`                         |
| On Sell               | —                                                                                                          |

#### Spite

| Field                 | Value                                                                                                                           |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                                       |
| Tier                  | Country                                                                                                                         |
| Descriptive Behaviour | Passive. Next time you lose a piece, deal its value x3 to enemy meter. A final revenge even in defeat.                          |
| Cost                  | 15                                                                                                                              |
| Sell                  | 70                                                                                                                              |
| On Play               | `eff_spite`, trigger: `TRIGGER_CARD_PLAYED`, args: `(passive1=TRIGGER_PIECE_FLIPPED+side, passive2=TRIGGER_PIECE_REMOVED+side)` |
| On Sell               | —                                                                                                                               |

#### Chain Break

| Field                 | Value                                                                                    |
| --------------------- | ---------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                |
| Tier                  | Country                                                                                  |
| Descriptive Behaviour | Force-flip one enemy piece of your choice. Rewriting the board state through sheer will. |
| Cost                  | 100                                                                                      |
| Sell                  | 70                                                                                       |
| On Play               | `eff_chain_break`, trigger: `TRIGGER_CARD_PLAYED`, args: `(flip_target=enemy_piece)`     |
| On Sell               | —                                                                                        |

#### Hydra

| Field                 | Value                                                                                                                              |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Universal                                                                                                                          |
| Tier                  | Country                                                                                                                            |
| Descriptive Behaviour | Passive. Next time one of your pieces flips, 2 friendly pawns spawn adjacent to your king. The hydra that regenerates from defeat. |
| Cost                  | -                                                                                                                                  |
| Sell                  | 80                                                                                                                                 |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(spawn_piece=PIECE_MEDEQ, count=3)`                                             |
| On Sell               | —                                                                                                                                  |

---

### 2.2 Longwei (7)

#### River Wade

| Field                 | Value                                                                                            |
| --------------------- | ------------------------------------------------------------------------------------------------ |
| Kingdom               | Longwei                                                                                          |
| Tier                  | District                                                                                         |
| Descriptive Behaviour | Target pawn permanently gains a sideways step this battle. Training a soldier to move laterally. |
| Cost                  | -                                                                                                |
| Sell                  | 15                                                                                               |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(meter_gain=flat)`                            |
| On Sell               | —                                                                                                |

#### Charge

| Field                 | Value                                                                                                          |
| --------------------- | -------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                        |
| Tier                  | District                                                                                                       |
| Descriptive Behaviour | Target slider may pass through one occupied square on its next move. Granting penetration through enemy lines. |
| Cost                  | -                                                                                                              |
| Sell                  | 15                                                                                                             |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=attack_up, duration=1)`                         |
| On Sell               | —                                                                                                              |

#### Formation

| Field                 | Value                                                                                                                |
| --------------------- | -------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                              |
| Tier                  | Town                                                                                                                 |
| Descriptive Behaviour | 3 of your pieces in a straight line each deal +50% damage this turn. The coordinated formation that amplifies force. |
| Cost                  | -                                                                                                                    |
| Sell                  | 30                                                                                                                   |
| On Play               | `eff_formation`, trigger: `TRIGGER_RESOLVE_ATTACK`, args: `(damage_bonus=longwei_pieces)`                            |
| On Sell               | —                                                                                                                    |

#### Divination

| Field                 | Value                                                                                                |
| --------------------- | ---------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                              |
| Tier                  | Town                                                                                                 |
| Descriptive Behaviour | Reveal the enemy's intended moves and cards for next turn. The diviner's foresight into enemy plans. |
| Cost                  | -                                                                                                    |
| Sell                  | 25                                                                                                   |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(peek_cards=3)`                                   |
| On Sell               | —                                                                                                    |

#### Cannon Volley

| Field                 | Value                                                                                                                              |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                            |
| Tier                  | Province                                                                                                                           |
| Descriptive Behaviour | Target Pao attacks every enemy on its row and column this turn, ignoring screens. The devastating broadside that sweeps the board. |
| Cost                  | 30                                                                                                                                 |
| Sell                  | 45                                                                                                                                 |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(damage=enemy_meter, amount=pao_piece_count)`                                   |
| On Sell               | —                                                                                                                                  |

#### Palace Decree

| Field                 | Value                                                                                                  |
| --------------------- | ------------------------------------------------------------------------------------------------------ |
| Kingdom               | Longwei                                                                                                |
| Tier                  | Province                                                                                               |
| Descriptive Behaviour | Enemy king restricted to a 3x3 zone for 2 turns. The sovereign's decree limiting the enemy's movement. |
| Cost                  | -                                                                                                      |
| Sell                  | 50                                                                                                     |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=fortified, duration=battle)`            |
| On Sell               | —                                                                                                      |

#### Mandate

| Field                 | Value                                                                                                                               |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                             |
| Tier                  | Country                                                                                                                             |
| Descriptive Behaviour | Remove one of your pieces. Deal damage to enemy meter by its value x3. The imperial mandate that sacrifices for overwhelming force. |
| Cost                  | -                                                                                                                                   |
| Sell                  | 75                                                                                                                                  |
| On Play               | `eff_mandate`, trigger: `TRIGGER_CARD_PLAYED`, args: `(damage_mult=3)`                                                              |
| On Sell               | —                                                                                                                                   |

---

### 2.3 Harushima (7)

#### Ronin

| Field                 | Value                                                                                                              |
| --------------------- | ------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Harushima                                                                                                          |
| Tier                  | District                                                                                                           |
| Descriptive Behaviour | Passive. Next time the targeted piece flips, refund its full value to you. The mercenary's insurance against loss. |
| Cost                  | -                                                                                                                  |
| Sell                  | 15                                                                                                                 |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=move_up, duration=1)`                               |
| On Sell               | —                                                                                                                  |

#### Resurrection

| Field                 | Value                                                                                                          |
| --------------------- | -------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                      |
| Tier                  | Town                                                                                                           |
| Descriptive Behaviour | Reclaim any flipped piece on the board to your control for free. Bringing fallen soldiers back into the fight. |
| Cost                  | -                                                                                                              |
| Sell                  | 25                                                                                                             |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(revive_piece=ally_graveyard)`                              |
| On Sell               | —                                                                                                              |

#### Gold Standard

| Field                 | Value                                                                                      |
| --------------------- | ------------------------------------------------------------------------------------------ |
| Kingdom               | Harushima                                                                                  |
| Tier                  | Town                                                                                       |
| Descriptive Behaviour | Target piece moves like a Kinsho this turn only. Granting temporary gold general movement. |
| Cost                  | -                                                                                          |
| Sell                  | 25                                                                                         |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(upgrade_piece=promote)`                |
| On Sell               | —                                                                                          |

#### Promotion

| Field                 | Value                                                                                                           |
| --------------------- | --------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                       |
| Tier                  | Town                                                                                                            |
| Descriptive Behaviour | Target piece permanently gains Ginsho movement for this battle. A lasting promotion to silver general movement. |
| Cost                  | -                                                                                                               |
| Sell                  | 30                                                                                                              |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_upgrade=harushima)`                                   |
| On Sell               | —                                                                                                               |

#### Dual Drop

| Field                 | Value                                                                                                                                      |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Harushima                                                                                                                                  |
| Tier                  | Province                                                                                                                                   |
| Descriptive Behaviour | Reclaim up to 2 flipped pieces at 30 cp total. If fewer than 2 exist, place free Fuhyo pieces for each missing reclaim. The double rescue. |
| Cost                  | 30                                                                                                                                         |
| Sell                  | 45                                                                                                                                         |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(draw_count=2, no_discard=true)`                                                        |
| On Sell               | —                                                                                                                                          |

#### Force Drop

| Field                 | Value                                                                                                               |
| --------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                           |
| Tier                  | Province                                                                                                            |
| Descriptive Behaviour | Place any unlocked piece of value up to 50 on any unoccupied square for free. Forcing immediate placement of power. |
| Cost                  | -                                                                                                                   |
| Sell                  | 50                                                                                                                  |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(enemy_discard=2)`                                               |
| On Sell               | —                                                                                                                   |

#### Bushido

| Field                 | Value                                                                                                                                  |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                                              |
| Tier                  | Country                                                                                                                                |
| Descriptive Behaviour | Passive. When the targeted piece flips, deal its value x2 to enemy meter. The warrior's honorable death dealing damage even in defeat. |
| Cost                  | -                                                                                                                                      |
| Sell                  | 60                                                                                                                                     |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(passive_buff=honorable_death)`                                                     |
| On Sell               | —                                                                                                                                      |

---

### 2.4 Kewarani (7)

#### Sultan's Gold

| Field                 | Value                                                            |
| --------------------- | ---------------------------------------------------------------- |
| Kingdom               | Kewarani                                                         |
| Tier                  | District                                                         |
| Descriptive Behaviour | Gain 60 cp. The sultan's treasury paying out.                    |
| Cost                  | -                                                                |
| Sell                  | 20                                                               |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(cp_bonus=2)` |
| On Sell               | —                                                                |

#### March

| Field                 | Value                                                                                                 |
| --------------------- | ----------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                              |
| Tier                  | District                                                                                              |
| Descriptive Behaviour | Every friendly pawn moves that can move forward 1 square, no action cost. The mass march of infantry. |
| Cost                  | -                                                                                                     |
| Sell                  | 15                                                                                                    |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(move_piece=free)`                                 |
| On Sell               | —                                                                                                     |

#### Double Time

| Field                 | Value                                                                                                                                                       |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                                                                                    |
| Tier                  | Town                                                                                                                                                        |
| Descriptive Behaviour | Target piece (any kingdom) makes one additional move this turn. Kewarani pieces with the innate active gain a third move instead. The doubled tempo of war. |
| Cost                  | -                                                                                                                                                           |
| Sell                  | 30                                                                                                                                                          |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(actions_bonus=1)`                                                                                       |
| On Sell               | —                                                                                                                                                           |

#### Salt Road

| Field                 | Value                                                                                           |
| --------------------- | ----------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                        |
| Tier                  | Town                                                                                            |
| Descriptive Behaviour | Gain 10 cp at the start of every remaining turn this battle. The trade route's ongoing tribute. |
| Cost                  | -                                                                                               |
| Sell                  | 35                                                                                              |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(sell_bonus=2)`                              |
| On Sell               | —                                                                                               |

#### Caravan

| Field                 | Value                                                                                                             |
| --------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                                          |
| Tier                  | Province                                                                                                          |
| Descriptive Behaviour | All pieces in a straight line move 1 square forward together, no individual action cost. The coordinated advance. |
| Cost                  | -                                                                                                                 |
| Sell                  | 45                                                                                                                |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(income_bonus=2)`                                              |
| On Sell               | —                                                                                                                 |

#### Doublestrike

| Field                 | Value                                                                          |
| --------------------- | ------------------------------------------------------------------------------ |
| Kingdom               | Kewarani                                                                       |
| Tier                  | Province                                                                       |
| Descriptive Behaviour | Move two of your pieces with 1 action cost. Doubling your movement efficiency. |
| Cost                  | -                                                                              |
| Sell                  | 50                                                                             |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=attack_twice)`  |
| On Sell               | —                                                                              |

#### Hajj

| Field                 | Value                                                                                                                      |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                                                                   |
| Tier                  | Country                                                                                                                    |
| Descriptive Behaviour | Target piece teleports to any unoccupied square on the board. Does not count as an attack. The sacred journey to anywhere. |
| Cost                  | -                                                                                                                          |
| Sell                  | 70                                                                                                                         |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=splitter_blessing)`                                         |
| On Sell               | —                                                                                                                          |

---

### 2.5 Zarqan (8)

#### Counsel

| Field                 | Value                                                                                            |
| --------------------- | ------------------------------------------------------------------------------------------------ |
| Kingdom               | Zarqan                                                                                           |
| Tier                  | District                                                                                         |
| Descriptive Behaviour | Peek at next turn's 3 cards. Discard 1 from the upcoming hand. The vizier's strategic foresight. |
| Cost                  | -                                                                                                |
| Sell                  | 15                                                                                               |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(peek_top=3)`                                 |
| On Sell               | —                                                                                                |

#### Pillage

| Field                 | Value                                                                                                        |
| --------------------- | ------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Zarqan                                                                                                       |
| Tier                  | District                                                                                                     |
| Descriptive Behaviour | Gain 5 cp for each friendly piece currently on the board, including the king. Plundering based on army size. |
| Cost                  | -                                                                                                            |
| Sell                  | 20                                                                                                           |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(cp_steal=3)`                                             |
| On Sell               | —                                                                                                            |

#### Royal Decoy

| Field                 | Value                                                                        |
| --------------------- | ---------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                       |
| Tier                  | Town                                                                         |
| Descriptive Behaviour | Swap positions of any 2 of your pieces. Repositioning the army without cost. |
| Cost                  | -                                                                            |
| Sell                  | 30                                                                           |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(swap_piece=enemy)`       |
| On Sell               | —                                                                            |

#### Bazaar

| Field                 | Value                                                                            |
| --------------------- | -------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                           |
| Tier                  | Town                                                                             |
| Descriptive Behaviour | Sell one of your pieces for 150% of its value. The bazaar's premium buying rate. |
| Cost                  | -                                                                                |
| Sell                  | 35                                                                               |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(discount=foreign_kingdom)`   |
| On Sell               | —                                                                                |

#### Steppe Riders

| Field                 | Value                                                                                                   |
| --------------------- | ------------------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                                  |
| Tier                  | Town                                                                                                    |
| Descriptive Behaviour | All your Knights, Jamals, and Cataphracts move twice this turn. The mounted warriors' doubled mobility. |
| Cost                  | -                                                                                                       |
| Sell                  | 30                                                                                                      |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(move_all=forward)`                                  |
| On Sell               | —                                                                                                       |

#### Ambition

| Field                 | Value                                                                                           |
| --------------------- | ----------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                          |
| Tier                  | Province                                                                                        |
| Descriptive Behaviour | Target piece copies any other unlocked piece's movement pattern this turn. The ambitious mimic. |
| Cost                  | -                                                                                               |
| Sell                  | 45                                                                                              |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(movegen_override=upgrade)`                  |
| On Sell               | —                                                                                               |

#### Citadel

| Field                 | Value                                                                                                                       |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                                                      |
| Tier                  | Province                                                                                                                    |
| Descriptive Behaviour | Target piece becomes immobile and immune (cannot move, be attacked, or deal damage) for 2 turns. The invulnerable fortress. |
| Cost                  | -                                                                                                                           |
| Sell                  | 50                                                                                                                          |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(territory_buff=defensive)`                                              |
| On Sell               | —                                                                                                                           |

#### Conquest

| Field                 | Value                                                                                                                            |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                                                                           |
| Tier                  | Country                                                                                                                          |
| Descriptive Behaviour | Target piece permanently adopts any other unlocked piece's movement pattern for this battle. The permanent conquest of movement. |
| Cost                  | 50                                                                                                                               |
| Sell                  | 80                                                                                                                               |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(upgrade_all=zarqan)`                                                         |
| On Sell               | —                                                                                                                                |

---

### 2.6 Caelan (8)

#### Castling

| Field                 | Value                                                                           |
| --------------------- | ------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                          |
| Tier                  | District                                                                        |
| Descriptive Behaviour | Your king and one of your Rooks swap positions. The royal castle rearrangement. |
| Cost                  | -                                                                               |
| Sell                  | 15                                                                              |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(swap_with_king=true)`       |
| On Sell               | —                                                                               |

#### Queen's Gambit

| Field                 | Value                                                                                                                      |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                                     |
| Tier                  | District                                                                                                                   |
| Descriptive Behaviour | Sacrifice a pawn. Draw 3 additional cards immediately; all must be played or sold this turn. Trading material for options. |
| Cost                  | -                                                                                                                          |
| Sell                  | 20                                                                                                                         |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(card_discount=queen)`                                                  |
| On Sell               | —                                                                                                                          |

#### Vengeance

| Field                 | Value                                                                                                               |
| --------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                              |
| Tier                  | Town                                                                                                                |
| Descriptive Behaviour | Deal 2x damage to an enemy piece that moved adjacent to one of your pieces last turn. Punishing those who approach. |
| Cost                  | -                                                                                                                   |
| Sell                  | 30                                                                                                                  |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(damage=enemy_meter, bonus=ally_lost)`                           |
| On Sell               | —                                                                                                                   |

#### Queen's Decree

| Field                 | Value                                                                                         |
| --------------------- | --------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                        |
| Tier                  | Town                                                                                          |
| Descriptive Behaviour | Your next attack this turn deals double damage. The queen's command of force.                 |
| Cost                  | -                                                                                             |
| Sell                  | 50                                                                                            |
| On Play               | `eff_queens_decree`, trigger: `TRIGGER_RESOLVE_ATTACK`, args: `(damage_boost=adjacent_queen)` |
| On Sell               | —                                                                                             |

#### Cathedral

| Field                 | Value                                                                                                                            |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                                           |
| Tier                  | Province                                                                                                                         |
| Descriptive Behaviour | Passive. Friendly pieces that are defended by a friendly bishop gets 40% damage reduction. The cathedral's spiritual protection. |
| Cost                  | -                                                                                                                                |
| Sell                  | 45                                                                                                                               |
| On Play               | `eff_cathedral`, trigger: `TRIGGER_RESOLVE_DEFENSE`, args: `(damage_reduction=1)`                                                |
| On Sell               | —                                                                                                                                |

#### Coronation

| Field                 | Value                                                                    |
| --------------------- | ------------------------------------------------------------------------ |
| Kingdom               | Caelan                                                                   |
| Tier                  | Province                                                                 |
| Descriptive Behaviour | Promote a pawn to Queen in place. The moment of coronation.              |
| Cost                  | -                                                                        |
| Sell                  | 55                                                                       |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(upgrade_piece=king)` |
| On Sell               | —                                                                        |

#### Crusade

| Field                 | Value                                                                                                         |
| --------------------- | ------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                        |
| Tier                  | Country                                                                                                       |
| Descriptive Behaviour | Target Knight makes 3 consecutive L-moves this turn, each attacking on resolution. The holy crusade of leaps. |
| Cost                  | -                                                                                                             |
| Sell                  | 70                                                                                                            |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(movegen_override=upgrade_all)`                            |
| On Sell               | —                                                                                                             |

#### Divine Right

| Field                 | Value                                                                        |
| --------------------- | ---------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                       |
| Tier                  | Country                                                                      |
| Descriptive Behaviour | Your king attacks as a Queen this turn. The divine right of sovereign power. |
| Cost                  | -                                                                            |
| Sell                  | 75                                                                           |
| On Play               | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(king_buff=battle)`       |
| On Sell               | —                                                                            |

---

### 2.7 Mastery Cards (5)

#### Mingzhu's Seal

| Field                 | Value                                                                             |
| --------------------- | --------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                           |
| Tier                  | Province                                                                          |
| Descriptive Behaviour | Mastery Level 2 reward. A powerful Longwei card granted upon mastery advancement. |
| Cost                  | -                                                                                 |
| Sell                  | 0                                                                                 |
| On Play               | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)`            |
| On Sell               | —                                                                                 |

#### Tomohito's Patience

| Field                 | Value                                                                               |
| --------------------- | ----------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                           |
| Tier                  | Province                                                                            |
| Descriptive Behaviour | Mastery Level 2 reward. A powerful Harushima card granted upon mastery advancement. |
| Cost                  | -                                                                                   |
| Sell                  | 0                                                                                   |
| On Play               | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)`              |
| On Sell               | —                                                                                   |

#### Selassie's March

| Field                 | Value                                                                              |
| --------------------- | ---------------------------------------------------------------------------------- |
| Kingdom               | Kewarani                                                                           |
| Tier                  | Province                                                                           |
| Descriptive Behaviour | Mastery Level 2 reward. A powerful Kewarani card granted upon mastery advancement. |
| Cost                  | -                                                                                  |
| Sell                  | 0                                                                                  |
| On Play               | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)`             |
| On Sell               | —                                                                                  |

#### Timur's Conquest

| Field                 | Value                                                                            |
| --------------------- | -------------------------------------------------------------------------------- |
| Kingdom               | Zarqan                                                                           |
| Tier                  | Province                                                                         |
| Descriptive Behaviour | Mastery Level 2 reward. A powerful Zarqan card granted upon mastery advancement. |
| Cost                  | -                                                                                |
| Sell                  | 0                                                                                |
| On Play               | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)`           |
| On Sell               | —                                                                                |

#### Isabella's Coronation

| Field                 | Value                                                                            |
| --------------------- | -------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                           |
| Tier                  | Province                                                                         |
| Descriptive Behaviour | Mastery Level 2 reward. A powerful Caelan card granted upon mastery advancement. |
| Cost                  | -                                                                                |
| Sell                  | 0                                                                                |
| On Play               | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)`           |
| On Sell               | —                                                                                |

---

## 3. Relics (26)

### 3.1 Economy (6)

#### Merchant's Ledger

| Field                 | Value                                                                      |
| --------------------- | -------------------------------------------------------------------------- |
| Descriptive Behaviour | Cards you sell are worth 5 cp more. The merchant's book of premium trades. |
| Trigger               | `TRIGGER_QUERY_SELL_VALUE`                                                 |
| Apply                 | `eff_merchants_ledger`, args: none                                         |
| Scope                 | run                                                                        |

#### Minted Coin

| Field                 | Value                                                         |
| --------------------- | ------------------------------------------------------------- |
| Descriptive Behaviour | Gain 5 extra income per turn. The treasury's golden standard. |
| Trigger               | `TRIGGER_TURN_START`                                          |
| Apply                 | `eff_minted_coin`, args: none                                 |
| Scope                 | run                                                           |

#### Tax Stamp

| Field                 | Value                                                                                             |
| --------------------- | ------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | When you play a card with cost greater than 0, gain 10 cp. The tax collector's stamp on commerce. |
| Trigger               | `TRIGGER_CARD_PLAYED`                                                                             |
| Apply                 | `eff_tax_stamp`, args: none                                                                       |
| Scope                 | run                                                                                               |

#### Bulk Discount

| Field                 | Value                                                                                       |
| --------------------- | ------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | If you bought 3 or more pieces this turn, your next piece is free. The bulk buyer rewarded. |
| Trigger               | `TRIGGER_QUERY_PIECE_COST` (apply) + `TRIGGER_PIECE_PLACED` (count) |
| Apply                 | `eff_bulk_discount`, args: none                                                             |
| Scope                 | battle                                                                                      |

#### War Chest

| Field                 | Value                                                                                                    |
| --------------------- | -------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | Convert your current cp to meter at a 5:1 ratio at battle start. The warlord's treasury of stored power. |
| Trigger               | `TRIGGER_TURN_END`                                                                                       |
| Apply                 | `eff_war_chest`, args: none                                                                              |
| Scope                 | battle                                                                                                   |

#### Trade Routes

| Field                 | Value                                                                                             |
| --------------------- | ------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | Foreign kingdom pieces cost the same as home kingdom pieces. The trade routes' equalizing effect. |
| Trigger               | `TRIGGER_RUN_START`                                                                               |
| Apply                 | `eff_trade_routes`, args: none                                                                    |
| Scope                 | run                                                                                               |

---

### 3.2 Meter (6)

#### Soul Shard

| Field                 | Value                                                                           |
| --------------------- | ------------------------------------------------------------------------------- |
| Descriptive Behaviour | When any piece flips, gain 30 meter. The soul shard absorbing captured essence. |
| Trigger               | `TRIGGER_PIECE_FLIPPED`                                                         |
| Apply                 | `eff_soul_shard`, args: none                                                    |
| Scope                 | run                                                                             |

#### Veteran's Bond

| Field                 | Value                                                                                                |
| --------------------- | ---------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | Gain +20 meter cap for each piece with value 50 or greater. The veteran's bond with powerful allies. |
| Trigger               | `TRIGGER_BATTLE_START`                                                                               |
| Apply                 | `eff_veterans_bond`, args: none                                                                      |
| Scope                 | battle                                                                                               |

#### Dead Man's Pact

| Field                 | Value                                                                                   |
| --------------------- | --------------------------------------------------------------------------------------- |
| Descriptive Behaviour | If your meter ever reaches 0, gain 20 meter (once per battle). The grim pact with fate. |
| Trigger               | `TRIGGER_PIECE_REMOVED`                                                                 |
| Apply                 | `eff_dead_mans_pact`, args: none                                                        |
| Scope                 | battle                                                                                  |

#### Iron King

| Field                 | Value                                                  |
| --------------------- | ------------------------------------------------------ |
| Descriptive Behaviour | Gain +10 meter cap. The iron king's expanded capacity. |
| Trigger               | `TRIGGER_BATTLE_START`                                 |
| Apply                 | `eff_iron_king`, args: none                            |
| Scope                 | battle                                                 |

#### Bloodthirst

| Field                 | Value                                                                                                       |
| --------------------- | ----------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | If your meter is higher than the enemy's, gain 5 meter each turn. The bloodthirst's advantage exploitation. |
| Trigger               | `TRIGGER_TURN_START`                                                                                        |
| Apply                 | `eff_bloodthirst`, args: none                                                                               |
| Scope                 | battle                                                                                                      |

#### Last Breath

| Field                 | Value                                                                                                   |
| --------------------- | ------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | When a piece is removed, deal its value as damage to the enemy meter. The last breath's parting strike. |
| Trigger               | `TRIGGER_PIECE_REMOVED`                                                                                 |
| Apply                 | `eff_last_breath`, args: none                                                                           |
| Scope                 | run                                                                                                     |

---

### 3.3 Cards (5)

#### Tactician's Scroll

| Field                 | Value                                                         |
| --------------------- | ------------------------------------------------------------- |
| Descriptive Behaviour | Draw at least 4 cards each turn. The tactician's preparation. |
| Trigger               | `TRIGGER_BATTLE_START`                                        |
| Apply                 | `eff_tacticians_scroll`, args: none                           |
| Scope                 | battle                                                        |

#### Librarian's Notes

| Field                 | Value                                                                                   |
| --------------------- | --------------------------------------------------------------------------------------- |
| Descriptive Behaviour | One additional card tier is available in Archives. The librarian's expanded collection. |
| Trigger               | `TRIGGER_MAP_ENTERED`                                                                   |
| Apply                 | `eff_librarians_notes`, args: none                                                      |
| Scope                 | run                                                                                     |

#### Country Seal

| Field                 | Value                                                                          |
| --------------------- | ------------------------------------------------------------------------------ |
| Descriptive Behaviour | Country-tier cards are worth 20 cp more when sold. The country seal's premium. |
| Trigger               | `TRIGGER_BATTLE_START`                                                         |
| Apply                 | `eff_country_seal`, args: none                                                 |
| Scope                 | battle                                                                         |

#### Deep Hand

| Field                 | Value                                                         |
| --------------------- | ------------------------------------------------------------- |
| Descriptive Behaviour | Draw 2 extra cards each turn. The deep hand's card advantage. |
| Trigger               | `TRIGGER_BATTLE_START`                                        |
| Apply                 | `eff_deep_hand`, args: none                                   |
| Scope                 | battle                                                        |

#### Gilded Archive

| Field                 | Value                                                                             |
| --------------------- | --------------------------------------------------------------------------------- |
| Descriptive Behaviour | District-tier cards are worth 10 cp more when sold. The gilded archive's premium. |
| Trigger               | `TRIGGER_RUN_START`                                                               |
| Apply                 | `eff_gilded_archive`, args: none                                                  |
| Scope                 | run                                                                               |

---

### 3.4 Combinations (4)

#### Alchemist's Kit

| Field                 | Value                                                            |
| --------------------- | ---------------------------------------------------------------- |
| Descriptive Behaviour | Combining pieces costs 0 cp. The alchemist's free transmutation. |
| Trigger               | `TRIGGER_PIECE_COMBINED`                                         |
| Apply                 | `eff_alchemists_kit`, args: none                                 |
| Scope                 | run                                                              |

#### Master's Notes

| Field                 | Value                                                             |
| --------------------- | ----------------------------------------------------------------- |
| Descriptive Behaviour | Archives offer double the selection. The master's expanded notes. |
| Trigger               | `TRIGGER_PIECE_COMBINED`                                          |
| Apply                 | `eff_masters_notes`, args: none                                   |
| Scope                 | run                                                               |

#### Philosopher's Stone

| Field                 | Value                                                                                           |
| --------------------- | ----------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | The first piece you combine each battle gains +20 value. The philosopher's stone's empowerment. |
| Trigger               | `TRIGGER_PIECE_COMBINED`                                                                        |
| Apply                 | `eff_philosophers_stone`, args: none                                                            |
| Scope                 | battle                                                                                          |

#### Inherited Power

| Field                 | Value                                                              |
| --------------------- | ------------------------------------------------------------------ |
| Descriptive Behaviour | Combined pieces gain +5 value. The inherited power of combination. |
| Trigger               | `TRIGGER_PIECE_COMBINED`                                           |
| Apply                 | `eff_inherited_power`, args: none                                  |
| Scope                 | run                                                                |

---

### 3.5 Board (5)

#### Eagle Eye

| Field                 | Value                                                   |
| --------------------- | ------------------------------------------------------- |
| Descriptive Behaviour | See enemy piece values. The eagle eye's perfect vision. |
| Trigger               | `TRIGGER_QUERY_VISION_FLAGS`                            |
| Apply                 | `eff_eagle_eye`, args: none                             |
| Scope                 | battle                                                  |

#### Surveyor's Map

| Field                 | Value                                                                        |
| --------------------- | ---------------------------------------------------------------------------- |
| Descriptive Behaviour | Battle modifiers are revealed at battle start. The surveyor's foreknowledge. |
| Trigger               | `TRIGGER_BATTLE_START`                                                       |
| Apply                 | `eff_surveyors_map`, args: none                                              |
| Scope                 | battle                                                                       |

#### Forward Command

| Field                 | Value                                                                                                                  |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | Your pieces deal +5 damage when placed in the forward half of the board. The forward command's aggressive positioning. |
| Trigger               | `TRIGGER_RESOLVE_ATTACK`                                                                                               |
| Apply                 | `eff_forward_command`, args: none                                                                                      |
| Scope                 | battle                                                                                                                 |

#### Fortified Line

| Field                 | Value                                                                                                          |
| --------------------- | -------------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | Your pieces deal +5 damage when placed in the back half of the board. The fortified line's defensive strength. |
| Trigger               | `TRIGGER_RESOLVE_ATTACK`                                                                                       |
| Apply                 | `eff_fortified_line`, args: none                                                                               |
| Scope                 | battle                                                                                                         |

#### Warlord's Banner

| Field                 | Value                                                                                                          |
| --------------------- | -------------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | Your pieces deal +5 damage for each square of territory you control. The warlord's banner's territorial might. |
| Trigger               | `TRIGGER_TURN_START`                                                                                           |
| Apply                 | `eff_warlords_banner`, args: none                                                                              |
| Scope                 | battle                                                                                                         |

---

## 4. Innate Powers (5)

### 4.1 Longwei — Bulwark

| Field                 | Value                                                                                                                                                          |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Longwei                                                                                                                                                        |
| Descriptive Behaviour | Any Longwei piece with an orthogonally adjacent friendly piece takes 50% reduced damage from all attacks that turn. The bulwark formation's shared protection. |
| Trigger               | `TRIGGER_RESOLVE_DEFENSE`                                                                                                                                      |
| Apply                 | `eff_innate_bulwark`, args: `(multiplier=2)`                                                                                                                   |
| Scope                 | innate (map)                                                                                                                                                   |

### 4.2 Harushima — Reclaim

| Field                 | Value                                                                                                                                                                     |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Harushima                                                                                                                                                                 |
| Descriptive Behaviour | A flipped piece anywhere on the board can be immediately converted to your control on its current square for 30 cp and 1 action. The Shogunate's honor-bound reclamation. |
| Trigger               | `TRIGGER_PIECE_REMOVED`                                                                                                                                                   |
| Apply                 | `eff_innate_reclaim`, args: none                                                                                                                                          |
| Scope                 | innate (map)                                                                                                                                                              |

### 4.3 Kewarani — Double Time

| Field                 | Value                                                                                                                                                                          |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Kewarani                                                                                                                                                                       |
| Descriptive Behaviour | Every Kewarani piece may move twice per move action. Kewarani pieces have no home kingdom discount and foreign penalty is now 40%. The doubled tempo of the Negusate's forces. |
| Trigger               | `TRIGGER_TURN_START`                                                                                                                                                           |
| Apply                 | `eff_innate_double_time`, args: none                                                                                                                                           |
| Scope                 | innate (map)                                                                                                                                                                   |

### 4.4 Zarqan — Royal Substitution

| Field                 | Value                                                                                                                          |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Kingdom               | Zarqan                                                                                                                         |
| Descriptive Behaviour | Any Zarqan piece swaps positions with the king as a free action once per battle. The royal substitution's protective maneuver. |
| Trigger               | `TRIGGER_TURN_START`                                                                                                           |
| Apply                 | `eff_innate_royal_sub`, args: none                                                                                             |
| Scope                 | innate (map)                                                                                                                   |

### 4.5 Caelan — Conqueror's Reward

| Field                 | Value                                                                                                                                                    |
| --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Kingdom               | Caelan                                                                                                                                                   |
| Descriptive Behaviour | When a Caelan piece contributes to a flip, its value permanently gains 50% (rounded up) for the rest of the battle. The conqueror's accumulating reward. |
| Trigger               | `TRIGGER_PIECE_FLIPPED`                                                                                                                                  |
| Apply                 | `eff_innate_conquerors_reward`, args: none                                                                                                               |
| Scope                 | innate (map)                                                                                                                                             |

---

## 5. Penalty Chains (3)

### 5.1 Bronze Chain

| Field                 | Value                                                                  |
| --------------------- | ---------------------------------------------------------------------- |
| Level                 | 1                                                                      |
| Descriptive Behaviour | Start each battle with 15 cp less. The bronze chain's economic burden. |
| Penalty               | `eff_chain_bronze`, args: none                                         |
| Scope                 | run                                                                    |

### 5.2 Silver Chain

| Field                 | Value                                                                                                      |
| --------------------- | ---------------------------------------------------------------------------------------------------------- |
| Level                 | 2                                                                                                          |
| Descriptive Behaviour | Start each battle with 5 cp less and draw 1 fewer card. The silver chain's economic and card disadvantage. |
| Penalty               | `eff_chain_silver`, args: none                                                                             |
| Scope                 | run                                                                                                        |

### 5.3 Gold Chain

| Field                 | Value                                                                                                               |
| --------------------- | ------------------------------------------------------------------------------------------------------------------- |
| Level                 | 3                                                                                                                   |
| Descriptive Behaviour | Start each battle with 8 cp less, draw 2 fewer cards, and start with 10 less meter. The gold chain's severe burden. |
| Penalty               | `eff_chain_gold`, args: none                                                                                        |
| Scope                 | run                                                                                                                 |

---

## 6. Battle Modifiers (12)

### 6.1 Economy (3)

#### Lean Times

| Field                 | Value                                                        |
| --------------------- | ------------------------------------------------------------ |
| Descriptive Behaviour | Start with 5 cp less. Austerity measures reducing resources. |
| Trigger               | `TRIGGER_BATTLE_START`                                       |
| Apply                 | `eff_todo`, args: `(cp_start=-5)`                            |
| Scope                 | battle                                                       |

#### Windfall

| Field                 | Value                                                    |
| --------------------- | -------------------------------------------------------- |
| Descriptive Behaviour | Start with 5 cp more. A fortunate windfall of resources. |
| Trigger               | `TRIGGER_BATTLE_START`                                   |
| Apply                 | `eff_todo`, args: `(cp_start=+5)`                        |
| Scope                 | battle                                                   |

#### Open Market

| Field                 | Value                                                                       |
| --------------------- | --------------------------------------------------------------------------- |
| Descriptive Behaviour | Foreign kingdom pieces cost 2 cp less. The open market's equalized pricing. |
| Trigger               | `TRIGGER_BATTLE_START`                                                      |
| Apply                 | `eff_todo`, args: `(foreign_discount=2)`                                    |
| Scope                 | battle                                                                      |

---

### 6.2 Meter (3)

#### Glass Cannon

| Field                 | Value                                       |
| --------------------- | ------------------------------------------- |
| Descriptive Behaviour | Start at 50% meter. High risk, high reward. |
| Trigger               | `TRIGGER_BATTLE_START`                      |
| Apply                 | `eff_todo`, args: `(meter_start=50pct)`     |
| Scope                 | battle                                      |

#### Bloodbath

| Field                 | Value                                                                  |
| --------------------- | ---------------------------------------------------------------------- |
| Descriptive Behaviour | All attacks deal 50% more damage. The bloodbath's increased lethality. |
| Trigger               | `TRIGGER_RESOLVE_ATTACK`                                               |
| Apply                 | `eff_todo`, args: `(damage_mult=1.5)`                                  |
| Scope                 | battle                                                                 |

#### Iron Will

| Field                 | Value                                                             |
| --------------------- | ----------------------------------------------------------------- |
| Descriptive Behaviour | All damage taken is halved. The iron will's defensive resilience. |
| Trigger               | `TRIGGER_RESOLVE_DEFENSE`                                         |
| Apply                 | `eff_todo`, args: `(damage_reduction=0.5)`                        |
| Scope                 | battle                                                            |

---

### 6.3 Cards (3)

#### Rich Hand

| Field                 | Value                                          |
| --------------------- | ---------------------------------------------- |
| Descriptive Behaviour | Draw 2 more cards each turn. Abundant options. |
| Trigger               | `TRIGGER_BATTLE_START`                         |
| Apply                 | `eff_todo`, args: `(draw_count=+2)`            |
| Scope                 | battle                                         |

#### Sparse Hand

| Field                 | Value                                         |
| --------------------- | --------------------------------------------- |
| Descriptive Behaviour | Draw 1 fewer card each turn. Limited options. |
| Trigger               | `TRIGGER_BATTLE_START`                        |
| Apply                 | `eff_todo`, args: `(draw_count=-1)`           |
| Scope                 | battle                                        |

#### Kingdom Purity

| Field                 | Value                                                                     |
| --------------------- | ------------------------------------------------------------------------- |
| Descriptive Behaviour | Only pieces from your current kingdom are available. Purity of tradition. |
| Trigger               | `TRIGGER_BATTLE_START`                                                    |
| Apply                 | `eff_todo`, args: `(same_kingdom_only=true)`                              |
| Scope                 | battle                                                                    |

---

### 6.4 Board (3)

#### Fog of War

| Field                 | Value                                                        |
| --------------------- | ------------------------------------------------------------ |
| Descriptive Behaviour | Enemy piece values are hidden. The fog of war's uncertainty. |
| Trigger               | `TRIGGER_BATTLE_START`                                       |
| Apply                 | `eff_todo`, args: `(hide_enemy_values=true)`                 |
| Scope                 | battle                                                       |

#### Dense Terrain

| Field                 | Value                                                      |
| --------------------- | ---------------------------------------------------------- |
| Descriptive Behaviour | Random squares are blocked. The dense terrain's obstacles. |
| Trigger               | `TRIGGER_BATTLE_START`                                     |
| Apply                 | `eff_todo`, args: `(block_squares=random)`                 |
| Scope                 | battle                                                     |

#### Extended Front

| Field                 | Value                                                             |
| --------------------- | ----------------------------------------------------------------- |
| Descriptive Behaviour | Board is 4 squares wider. The extended front's wider battlefield. |
| Trigger               | `TRIGGER_BATTLE_START`                                            |
| Apply                 | `eff_todo`, args: `(board_width=+4)`                              |
| Scope                 | battle                                                            |

---

## 7. Board Traits (10)

| Trait            | Descriptive Behaviour                                                                                       | Trigger                | Apply                                   | Scope  |
| ---------------- | ----------------------------------------------------------------------------------------------------------- | ---------------------- | --------------------------------------- | ------ |
| River Crossing   | Certain squares form a river that pieces cannot cross. The river crossing's territorial division.           | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=river)`     | battle |
| Palace           | A 3x3 zone in each back rank is marked as palace territory. The palace's protected zone.                    | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=palace)`    | battle |
| Fog Coast        | Random squares are obscured; their contents are hidden. The fog coast's limited visibility.                 | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=fog)`       | battle |
| Island Chain     | Scattered islands provide unique movement properties. The island chain's scattered terrain.                 | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=island)`    | battle |
| Trade Route      | Certain squares are trade routes that generate income when controlled. The trade route's income generation. | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=trade)`     | battle |
| Contested Market | Both players can place pieces on the same square, resolving conflict. The contested market's shared space.  | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=contested)` | battle |
| Sandstorm        | Movement is reduced by 1 square per move. The sandstorm's slowing effect.                                   | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=sand)`      | battle |
| Mirage           | Random squares change each turn, blocking or unblocking. The mirage's shifting terrain.                     | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=mirage)`    | battle |
| Castle Corners   | Corner squares provide defensive bonuses. The castle corners' fortified positions.                          | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=castle)`    | battle |
| Siege Trench     | Trench squares provide offensive bonuses. The siege trench's attacking positions.                           | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=trench)`    | battle |

---

## 8. Figurehead Powers (5)

| Kingdom   | Power           | Descriptive Behaviour                                                                                              | Trigger                 | Apply                         | Scope |
| --------- | --------------- | ------------------------------------------------------------------------------------------------------------------ | ----------------------- | ----------------------------- | ----- |
| Longwei   | Iron Strategist | If you win a battle with 3+ pieces remaining, deal 20 damage to enemy meter. The iron strategist's closing strike. | `TRIGGER_TURN_START`    | `eff_fh_mingzhu`, args: none  | run   |
| Harushima | Honorable Death | When a piece is removed, spawn a free Fuhyo adjacent to your king. The honorable death's posthumous gift.          | `TRIGGER_PIECE_REMOVED` | `eff_fh_tomohito`, args: none | run   |
| Kewarani  | Merchant Prince | Cards you sell are worth 1 cp more. The merchant prince's premium on trade.                                        | `TRIGGER_CARD_SOLD`     | `eff_fh_selassie`, args: none | run   |
| Zarqan    | Grand Vizier    | Once per battle, get a 1 cp discount on your first card purchase. The grand vizier's strategic discount.           | `TRIGGER_TURN_START`    | `eff_fh_timur`, args: none    | run   |
| Caelan    | Divine Right    | When a piece flips to your side, gain cp equal to its value. The divine right's wealth from conquest.              | `TRIGGER_PIECE_FLIPPED` | `eff_fh_isabella`, args: none | run   |

---

## 9. Overseers (5 + Vorath)

| Overseer            | Kingdom   | Descriptive Behaviour                                                                                | Trigger                | Apply                                          | Scope  |
| ------------------- | --------- | ---------------------------------------------------------------------------------------------------- | ---------------------- | ---------------------------------------------- | ------ |
| Iron Strategist     | Longwei   | Enemy spawns 3 Pao and 2 Wazir at the top rank. The iron strategist's disciplined opening.           | `TRIGGER_BATTLE_START` | `eff_overseer_iron_strategist`, args: none     | battle |
| Eternal Recursion   | Harushima | When you flip a piece, it is marked to flip back at turn end. The eternal recursion's endless cycle. | `TRIGGER_BATTLE_START` | `eff_overseer_eternal_recursion`, args: none   | battle |
| Caravan of Conquest | Kewarani  | On even turns, a Faras spawns at the top center. The caravan of conquest's reinforcements.           | `TRIGGER_BATTLE_START` | `eff_overseer_caravan_of_conquest`, args: none | battle |
| Many-Faced King     | Zarqan    | Enemy spawns 3 Shahzadeh and 1 King at the top. The many-faced king's royal guard.                   | `TRIGGER_BATTLE_START` | `eff_overseer_many_faced_king`, args: none     | battle |
| Crowned Heretic     | Caelan    | Enemy spawns a full Caelan back rank at the top. The crowned heretic's full royal court.             | `TRIGGER_BATTLE_START` | `eff_overseer_crowned_heretic`, args: none     | battle |

### Vorath (Grand King + 5 Minor Kings)

| Field                 | Value                                                                                                                                                                                                                                                                                                                                                           |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Descriptive Behaviour | A 20x20 board with Grand King and 5 Minor Kings across quadrants. The Grand King's meter resets to full at turn start unless the player attacked at least one Minor King in each of the 5 quadrants on the preceding turn. Win by reducing the Grand King's meter to zero in a turn where all 5 quadrants were struck. The final confrontation with the tyrant. |
| Trigger               | `TRIGGER_BATTLE_START`                                                                                                                                                                                                                                                                                                                                          |
| Apply                 | `eff_todo`, args: `(board=20x20, quadrants=5, grand_king_meter_reset)`                                                                                                                                                                                                                                                                                          |
| Scope                 | battle                                                                                                                                                                                                                                                                                                                                                          |

---

## 10. Kingdom Synergies (5)

| Cleared Kingdom | Synergy        | Descriptive Behaviour                                                                               | Trigger               | Apply                           | Scope |
| --------------- | -------------- | --------------------------------------------------------------------------------------------------- | --------------------- | ------------------------------- | ----- |
| Longwei         | Siege Engineer | In Harushima, your Pao pieces deal +10 damage. The siege engineer's cannon expertise.               | `TRIGGER_MAP_ENTERED` | `eff_syn_longwei`, args: none   | run   |
| Harushima       | Reclaimer      | In Caelan, draw a card when you play a Caelan card. The reclaimer's card advantage.                 | `TRIGGER_MAP_ENTERED` | `eff_syn_harushima`, args: none | run   |
| Kewarani        | The Tide       | In Zarqan, Kewarani pieces cost 10 cp less. The tide's flowing commerce.                            | `TRIGGER_MAP_ENTERED` | `eff_syn_kewarani`, args: none  | run   |
| Zarqan          | Trickster      | In Longwei, your Ziraafa and War Elephant pieces gain +5 value. The trickster's enhanced elephants. | `TRIGGER_MAP_ENTERED` | `eff_syn_zarqan`, args: none    | run   |
| Caelan          | The Hammer     | In Kewarani, gain 5 cp when you play Sultan's Gold. The hammer's gold tribute.                      | `TRIGGER_MAP_ENTERED` | `eff_syn_caelan`, args: none    | run   |

---

## 11. Events (per-kingdom + universal)

Events are narrative choices presented to the player on the map.
Each event has multiple options, each with an `EffectFunc` that applies
run-wide changes.

| Event             | Kingdom   | Descriptive Behaviour                                                | Options                                                                        |
| ----------------- | --------- | -------------------------------------------------------------------- | ------------------------------------------------------------------------------ |
| Iron Strategist   | Longwei   | A choice between defensive fortification or aggressive expansion.    | Option A: `eff_todo` (defensive path) / Option B: `eff_todo` (aggressive path) |
| Eternal Recursion | Harushima | A choice between the path of honor or pragmatic survival.            | Option A: `eff_todo` (honor path) / Option B: `eff_todo` (pragmatic path)      |
| Merchant Prince   | Kewarani  | A choice between the trade route or conquest.                        | Option A: `eff_todo` (trade path) / Option B: `eff_todo` (conquest path)       |
| Grand Vizier      | Zarqan    | A choice between loyalty to the sultan or betrayal.                  | Option A: `eff_todo` (loyalty path) / Option B: `eff_todo` (betrayal path)     |
| Divine Right      | Caelan    | A choice between faith in divine providence or pursuit of raw power. | Option A: `eff_todo` (faith path) / Option B: `eff_todo` (power path)          |

---

## 12. Combo Chain Climaxes (5)

| Kingdom   | Climax   | Descriptive Behaviour                                                          | Trigger                 | Apply                                  | Scope  |
| --------- | -------- | ------------------------------------------------------------------------------ | ----------------------- | -------------------------------------- | ------ |
| Longwei   | Siege    | All your pieces gain Bulwark this turn. The siege's coordinated defense.       | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=longwei)`   | battle |
| Harushima | Flood    | Reclaim one flipped piece for free. The flood's overwhelming reclamation.      | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=harushima)` | battle |
| Kewarani  | Stampede | All pieces gain 1 extra move this turn. The stampede's doubled mobility.       | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=kewarani)`  | battle |
| Zarqan    | Conquest | Free position-swap of any 4 of your pieces. The conquest's mass repositioning. | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=zarqan)`    | battle |
| Caelan    | Crusade  | All your pieces deal +50% damage this turn. The crusade's holy empowerment.    | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=caelan)`    | battle |

---

## 13. AI Archetypes (5)

| Archetype      | Kingdom   | Descriptive Behaviour                                                                        | Pick Function           | Primary Weights                                                                          | Fallback Weights                                                                         |
| -------------- | --------- | -------------------------------------------------------------------------------------------- | ----------------------- | ---------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------- |
| Siege Engineer | Longwei   | Prioritizes value trades and territory control. Builds formations around Pao screens.        | `NULL` (default scorer) | value_diff=12, territory=6, aggression=4, sell=4, save=6, max_cost=8, combo=5, reclaim=0 | value_diff=6, territory=9, aggression=2, sell=3, save=4, max_cost=6, combo=3, reclaim=0  |
| Reclaimer      | Harushima | Prioritizes piece recovery and combo chains. Will sacrifice tempo for reclaim opportunities. | `NULL` (default scorer) | value_diff=8, territory=4, aggression=2, sell=2, save=8, max_cost=5, combo=7, reclaim=10 | value_diff=4, territory=6, aggression=1, sell=2, save=5, max_cost=4, combo=4, reclaim=5  |
| The Tide       | Kewarani  | Prioritizes aggressive play and piece flooding. Plays for tempo over value.                  | `NULL` (default scorer) | value_diff=6, territory=3, aggression=8, sell=5, save=5, max_cost=7, combo=4, reclaim=0  | value_diff=3, territory=5, aggression=4, sell=3, save=3, max_cost=5, combo=2, reclaim=0  |
| Trickster      | Zarqan    | Prioritizes aggressive attacks and high-value targets. Plays for decisive strikes.           | `NULL` (default scorer) | value_diff=5, territory=2, aggression=12, sell=6, save=3, max_cost=9, combo=3, reclaim=0 | value_diff=2, territory=4, aggression=6, sell=4, save=2, max_cost=7, combo=2, reclaim=0  |
| The Hammer     | Caelan    | Prioritizes territory control and defensive setups. Builds toward powerful combinations.     | `NULL` (default scorer) | value_diff=9, territory=8, aggression=2, sell=3, save=7, max_cost=7, combo=6, reclaim=0  | value_diff=5, territory=10, aggression=1, sell=2, save=5, max_cost=5, combo=4, reclaim=0 |

---

## 14. Effect Functions Index

All `EffectFunc` implementations live in `src/effects/`:

| Function                           | File                  | Trigger(s)                         | Purpose                               | Status      |
| ---------------------------------- | --------------------- | ---------------------------------- | ------------------------------------- | ----------- |
| `eff_todo`                         | `eff_run.c`           | any                                | Placeholder; logs and no-ops          | IMPLEMENTED |
| `eff_bulwark`                      | `eff_damage.c`        | `TRIGGER_RESOLVE_DEFENSE`          | 2× damage reduction                   | STUB        |
| `eff_bloodletting`                 | `eff_damage.c`        | `TRIGGER_RESOLVE_ATTACK`           | 2× damage dealt                       | STUB        |
| `eff_counter_coup`                 | `eff_damage.c`        | `TRIGGER_RESOLVE_DEFENSE`          | Counter-attack on defended hit        | STUB        |
| `eff_cathedral`                    | `eff_damage.c`        | `TRIGGER_RESOLVE_DEFENSE`          | −1 damage taken                       | IMPLEMENTED |
| `eff_queens_decree`                | `eff_damage.c`        | `TRIGGER_RESOLVE_ATTACK`           | Damage boost for adjacent Queen       | IMPLEMENTED |
| `eff_formation`                    | `eff_damage.c`        | `TRIGGER_RESOLVE_ATTACK`           | Damage scales with Longwei pieces     | IMPLEMENTED |
| `eff_damage_mult`                  | `eff_damage.c`        | any                                | Multiply damage                       | STUB        |
| `eff_damage_reduce`                | `eff_damage.c`        | any                                | Reduce damage                         | STUB        |
| `eff_grant_immunity`               | `eff_damage.c`        | any                                | Grant damage immunity                 | STUB        |
| `eff_deal_damage`                  | `eff_damage.c`        | any                                | Deal damage to meter                  | STUB        |
| `eff_splitter_medeq`               | `eff_flip.c`          | `TRIGGER_PIECE_FLIPPED`            | Spawn Medeq + remove self             | STUB        |
| `eff_grant_flip_immunity`          | `eff_flip.c`          | any                                | Grant flip immunity                   | STUB        |
| `eff_flip_destroy_adjacent`        | `eff_flip.c`          | any                                | Destroy adjacent on flip              | STUB        |
| `eff_sideways_step`                | `eff_movement.c`      | `TRIGGER_PIECE_DEALT_DAMAGE`       | Grant sideways step after damage      | STUB        |
| `eff_first_move_two`               | `eff_movement.c`      | `TRIGGER_PIECE_PLACED`             | Allow first move of 2 squares         | STUB        |
| `eff_swap_movegen`                 | `eff_movement.c`      | `TRIGGER_PIECE_ENTERED_ENEMY_TERR` | Swap move override                    | STUB        |
| `eff_free_king_swap`               | `eff_movement.c`      | `TRIGGER_TURN_START`               | Free king swap action                 | STUB        |
| `eff_grant_extra_step`             | `eff_movement.c`      | any                                | Grant extra movement step             | STUB        |
| `eff_grant_friendly_pass`          | `eff_movement.c`      | any                                | Move through friendly pieces          | STUB        |
| `eff_pawn_storm`                   | `eff_card.c`          | `TRIGGER_CARD_PLAYED`              | Up to 3 pawns, 3rd free               | IMPLEMENTED |
| `eff_sacrifice`                    | `eff_card.c`          | `TRIGGER_CARD_PLAYED`              | Remove piece, gain 2× value as meter  | IMPLEMENTED |
| `eff_mandate`                      | `eff_card.c`          | `TRIGGER_CARD_PLAYED`              | Remove piece, deal 3× value as damage | IMPLEMENTED |
| `eff_chain_break`                  | `eff_piece.c`         | `TRIGGER_CARD_PLAYED`              | Force flip an enemy piece             | IMPLEMENTED |
| `eff_spite`                        | `eff_piece.c`         | `TRIGGER_CARD_PLAYED`              | Install two passives on piece loss    | IMPLEMENTED |
| `eff_immune_once`                  | `eff_piece.c`         | `TRIGGER_RESOLVE_FLIP`             | Block one flip this battle            | IMPLEMENTED |
| `eff_spawn_piece`                  | `eff_piece.c`         | any                                | Spawn a piece                         | STUB        |
| `eff_remove_piece`                 | `eff_piece.c`         | any                                | Remove a piece                        | STUB        |
| `eff_swap_pieces`                  | `eff_piece.c`         | any                                | Swap two pieces                       | STUB        |
| `eff_force_flip`                   | `eff_piece.c`         | any                                | Force flip a piece                    | STUB        |
| `eff_draw_extra`                   | `eff_card.c`          | any                                | Draw extra cards                      | STUB        |
| `eff_discard_card`                 | `eff_card.c`          | any                                | Discard a card                        | STUB        |
| `eff_peek_cards`                   | `eff_card.c`          | any                                | Peek at cards                         | STUB        |
| `eff_meter_add`                    | `eff_meter.c`         | any                                | Add to meter                          | IMPLEMENTED |
| `eff_meter_set`                    | `eff_meter.c`         | any                                | Set meter                             | IMPLEMENTED |
| `eff_meter_cap_up`                 | `eff_meter.c`         | any                                | Increase meter cap                    | IMPLEMENTED |
| `eff_meter_overflow_up`            | `eff_meter.c`         | any                                | Increase overflow cap                 | IMPLEMENTED |
| `eff_meter_overflow_add`           | `eff_meter.c`         | any                                | Add to overflow                       | STUB        |
| `eff_meter_refill`                 | `eff_meter.c`         | any                                | Refill meter to cap                   | IMPLEMENTED |
| `eff_cp_add`                       | `eff_economy.c`       | any                                | Add cp                                | IMPLEMENTED |
| `eff_cp_set`                       | `eff_economy.c`       | any                                | Set cp                                | IMPLEMENTED |
| `eff_cost_mod`                     | `eff_economy.c`       | `TRIGGER_QUERY_PIECE_COST`         | Modify buy cost                       | STUB        |
| `eff_sell_mod`                     | `eff_economy.c`       | `TRIGGER_QUERY_SELL_VALUE`         | Modify sell value                     | STUB        |
| `eff_income_add`                   | `eff_economy.c`       | `TRIGGER_QUERY_TURN_INCOME`        | Add to income                         | STUB        |
| `eff_chain_bronze`                 | `eff_chain.c`         | `TRIGGER_BATTLE_START`             | Bronze chain penalty                  | IMPLEMENTED |
| `eff_chain_silver`                 | `eff_chain.c`         | `TRIGGER_BATTLE_START`             | Silver chain penalty                  | IMPLEMENTED |
| `eff_chain_gold`                   | `eff_chain.c`         | `TRIGGER_BATTLE_START`             | Gold chain penalty                    | STUB        |
| `eff_fh_mingzhu`                   | `eff_figurehead.c`    | `TRIGGER_TURN_START`               | Longwei figurehead power              | IMPLEMENTED |
| `eff_fh_tomohito`                  | `eff_figurehead.c`    | `TRIGGER_QUERY_RECLAIM_COST`       | Harushima figurehead power            | IMPLEMENTED |
| `eff_fh_selassie`                  | `eff_figurehead.c`    | `TRIGGER_CARD_SOLD`                | Kewarani figurehead power             | IMPLEMENTED |
| `eff_fh_timur`                     | `eff_figurehead.c`    | `TRIGGER_QUERY_ROYAL_SUB_COUNT`    | Zarqan figurehead power               | IMPLEMENTED |
| `eff_fh_isabella`                  | `eff_figurehead.c`    | `TRIGGER_PIECE_FLIPPED`            | Caelan figurehead power               | IMPLEMENTED |
| `eff_innate_bulwark`               | `eff_innate.c`        | `TRIGGER_RESOLVE_DEFENSE`          | Longwei innate                        | IMPLEMENTED |
| `eff_innate_reclaim`               | `eff_innate.c`        | `TRIGGER_PIECE_REMOVED`            | Harushima innate                      | STUB        |
| `eff_innate_double_time`           | `eff_innate.c`        | `TRIGGER_TURN_START`               | Kewarani innate                       | IMPLEMENTED |
| `eff_innate_royal_sub`             | `eff_innate.c`        | `TRIGGER_TURN_START`               | Zarqan innate                         | STUB        |
| `eff_innate_conquerors_reward`     | `eff_innate.c`        | `TRIGGER_PIECE_FLIPPED`            | Caelan innate                         | IMPLEMENTED |
| `eff_mastery_l1_*`                 | `eff_mastery.c`       | `TRIGGER_MAP_ENTERED`              | Mastery Level 1 hooks                 | IMPLEMENTED |
| `eff_mastery_l2_*`                 | `eff_mastery.c`       | `TRIGGER_MAP_ENTERED`              | Mastery Level 2 hooks                 | IMPLEMENTED |
| `eff_mastery_l3_*`                 | `eff_mastery.c`       | `TRIGGER_MAP_ENTERED`              | Mastery Level 3 hooks                 | IMPLEMENTED |
| `eff_overseer_iron_strategist`     | `eff_overseer.c`      | `TRIGGER_BATTLE_START`             | Longwei overseer                      | IMPLEMENTED |
| `eff_overseer_eternal_recursion`   | `eff_overseer.c`      | `TRIGGER_BATTLE_START`             | Harushima overseer                    | IMPLEMENTED |
| `eff_overseer_caravan_of_conquest` | `eff_overseer.c`      | `TRIGGER_BATTLE_START`             | Kewarani overseer                     | IMPLEMENTED |
| `eff_overseer_many_faced_king`     | `eff_overseer.c`      | `TRIGGER_BATTLE_START`             | Zarqan overseer                       | IMPLEMENTED |
| `eff_overseer_crowned_heretic`     | `eff_overseer.c`      | `TRIGGER_BATTLE_START`             | Caelan overseer                       | IMPLEMENTED |
| `eff_merchants_ledger`             | `eff_relic.c`         | `TRIGGER_RUN_START`                | Merchant's Ledger relic               | IMPLEMENTED |
| `eff_minted_coin`                  | `eff_relic.c`         | `TRIGGER_RUN_START`                | Minted Coin relic                     | IMPLEMENTED |
| `eff_tax_stamp`                    | `eff_relic.c`         | `TRIGGER_CARD_SOLD`                | Tax Stamp relic                       | IMPLEMENTED |
| `eff_bulk_discount`                | `eff_relic.c`         | `TRIGGER_QUERY_PIECE_COST`         | Bulk Discount relic                   | IMPLEMENTED |
| `eff_war_chest`                    | `eff_relic.c`         | `TRIGGER_TURN_END`                 | War Chest relic                       | IMPLEMENTED |
| `eff_trade_routes`                 | `eff_relic.c`         | `TRIGGER_RUN_START`                | Trade Routes relic                    | IMPLEMENTED |
| `eff_soul_shard`                   | `eff_relic.c`         | `TRIGGER_PIECE_FLIPPED` (new_owner=PLAYER) | Soul Shard relic               | IMPLEMENTED |
| `eff_veterans_bond`                | `eff_relic.c`         | `TRIGGER_BATTLE_START`             | Veteran's Bond relic                  | IMPLEMENTED |
| `eff_dead_mans_pact`               | `eff_relic.c`         | `TRIGGER_PIECE_REMOVED`            | Dead Man's Pact relic                 | IMPLEMENTED |
| `eff_iron_king`                    | `eff_relic.c`         | `TRIGGER_BATTLE_START`             | Iron King relic                       | IMPLEMENTED |
| `eff_bloodthirst`                  | `eff_relic.c`         | `TRIGGER_TURN_START`               | Bloodthirst relic                     | IMPLEMENTED |
| `eff_last_breath`                  | `eff_relic.c`         | `TRIGGER_PIECE_REMOVED`            | Last Breath relic                     | IMPLEMENTED |
| `eff_tacticians_scroll`            | `eff_relic.c`         | `TRIGGER_BATTLE_START`             | Tactician's Scroll relic              | IMPLEMENTED |
| `eff_librarians_notes`             | `eff_relic.c`         | `TRIGGER_MAP_ENTERED`              | Librarian's Notes relic               | STUB        |
| `eff_country_seal`                 | `eff_relic.c`         | `TRIGGER_BATTLE_START`             | Country Seal relic                    | IMPLEMENTED |
| `eff_deep_hand`                    | `eff_relic.c`         | `TRIGGER_BATTLE_START`             | Deep Hand relic                       | STUB        |
| `eff_gilded_archive`               | `eff_relic.c`         | `TRIGGER_RUN_START`                | Gilded Archive relic                  | IMPLEMENTED |
| `eff_alchemists_kit`               | `eff_relic.c`         | `TRIGGER_PIECE_COMBINED`           | Alchemist's Kit relic                 | IMPLEMENTED |
| `eff_masters_notes`                | `eff_relic.c`         | `TRIGGER_PIECE_COMBINED`           | Master's Notes relic                  | IMPLEMENTED |
| `eff_philosophers_stone`           | `eff_relic.c`         | `TRIGGER_PIECE_COMBINED`           | Philosopher's Stone relic             | IMPLEMENTED |
| `eff_inherited_power`              | `eff_relic.c`         | `TRIGGER_PIECE_COMBINED`           | Inherited Power relic                 | IMPLEMENTED |
| `eff_eagle_eye`                    | `eff_relic.c`         | `TRIGGER_QUERY_VISION_FLAGS`       | Eagle Eye relic                       | IMPLEMENTED |
| `eff_surveyors_map`                | `eff_relic.c`         | `TRIGGER_BATTLE_START`             | Surveyor's Map relic                  | IMPLEMENTED |
| `eff_forward_command`              | `eff_relic.c`         | `TRIGGER_RESOLVE_ATTACK`           | Forward Command relic                 | IMPLEMENTED |
| `eff_fortified_line`               | `eff_relic.c`         | `TRIGGER_RESOLVE_ATTACK`           | Fortified Line relic                  | IMPLEMENTED |
| `eff_warlords_banner`              | `eff_relic.c`         | `TRIGGER_TURN_START`               | Warlord's Banner relic                | IMPLEMENTED |
| `eff_syn_longwei`                  | `eff_synergy.c`       | `TRIGGER_MAP_ENTERED`              | Longwei synergy                       | IMPLEMENTED |
| `eff_syn_harushima`                | `eff_synergy.c`       | `TRIGGER_MAP_ENTERED`              | Harushima synergy                     | IMPLEMENTED |
| `eff_syn_kewarani`                 | `eff_synergy.c`       | `TRIGGER_MAP_ENTERED`              | Kewarani synergy                      | IMPLEMENTED |
| `eff_syn_zarqan`                   | `eff_synergy.c`       | `TRIGGER_MAP_ENTERED`              | Zarqan synergy                        | IMPLEMENTED |
| `eff_syn_caelan`                   | `eff_synergy.c`       | `TRIGGER_MAP_ENTERED`              | Caelan synergy                        | IMPLEMENTED |
| `eff_vorath_memory_tally`          | `eff_vorath_memory.c` | `TRIGGER_PIECE_PLACED`             | Track most-played piece               | IMPLEMENTED |
| `eff_vorath_memory_apply`          | `eff_vorath_memory.c` | `TRIGGER_BATTLE_START`             | Spawn most-played piece for enemy     | IMPLEMENTED |

---

## 15. MoveGen Functions Index

All `MoveGenFunc` implementations live in `src/movegens/`:

### Basics (`mg_basics.c`)

| Function                  | Params                  | Used by                                                                       | Status      |
| ------------------------- | ----------------------- | ----------------------------------------------------------------------------- | ----------- |
| `ml_push`                 | pos                     | (helper)                                                                      | IMPLEMENTED |
| `is_friendly`             | piece, at               | (helper)                                                                      | IMPLEMENTED |
| `is_enemy`                | piece, at               | (helper)                                                                      | IMPLEMENTED |
| `can_move_to`             | piece, bs, pos          | (helper)                                                                      | IMPLEMENTED |
| `can_capture`             | piece, bs, pos          | (helper)                                                                      | IMPLEMENTED |
| `can_capture_or_empty`    | piece, bs, pos          | (helper)                                                                      | IMPLEMENTED |
| `mg_step`                 | dx, dy                  | Bing, Fuhyo, Medeq, Pawn                                                      | IMPLEMENTED |
| `mg_step_set`             | n × (dx,dy)             | Makwanam, Wazir, Old King, King, Daimyo, Pawn attack                          | IMPLEMENTED |
| `mg_slide`                | dx, dy, min, max        | Kyosha                                                                        | IMPLEMENTED |
| `mg_slide_dirs`           | dir_mask, min, max      | Bishop, Rook, Queen, Talliya, Chancellor, Gryphon                             | IMPLEMENTED |
| `mg_leap_set`             | n × (dx,dy)             | Ma, Knight, Northern Cavalry, Honorable Horse, Saba, Faras, Jamal, Cataphract | IMPLEMENTED |
| `mg_blockable_leap`       | dx, dy, intermediates[] | (reserved)                                                                    | IMPLEMENTED |
| `mg_compound`             | n × MoveGen             | Promoted Bishop, Dragon                                                       | STUB        |
| `mg_choice`               | n × MoveGen             | Cataphract, Chancellor                                                        | STUB        |
| `mg_double_act`           | MoveGen                 | (reserved)                                                                    | STUB        |
| `mg_territory_restricted` | MoveGen, mask           | (reserved)                                                                    | STUB        |
| `mg_attack_only_subset`   | move, attack            | Pawn, Medeq                                                                   | STUB        |
| `mg_generate`             | —                       | (dispatcher)                                                                  | IMPLEMENTED |
| `mg_generate_threat`      | —                       | (dispatcher)                                                                  | IMPLEMENTED |
| `mg_todo`                 | —                       | All unimplemented movegens                                                    | STUB        |

### Longwei (`mg_longwei.c`)

| Function       | Used by       | Status      |
| -------------- | ------------- | ----------- |
| `mg_lw_ma`     | Ma            | IMPLEMENTED |
| `mg_lw_xiang`  | Xiang         | IMPLEMENTED |
| `mg_lw_pao`    | Pao           | IMPLEMENTED |
| `mg_lw_hwacha` | Hwacha        | IMPLEMENTED |
| `mg_lw_sang`   | Sang          | IMPLEMENTED |
| `mg_lw_liubo`  | Liubo Diviner | IMPLEMENTED |

### Harushima (`mg_harushima.c`)

| Function                | Used by                               | Status      |
| ----------------------- | ------------------------------------- | ----------- |
| `mg_hs_kinsho`          | Kinsho, Honorable Horse (swap target) | IMPLEMENTED |
| `mg_hs_ginsho`          | Ginsho                                | IMPLEMENTED |
| `mg_hs_honorable_horse` | Honorable Horse (initial)             | IMPLEMENTED |
| `mg_hs_shishi`          | Shishi                                | IMPLEMENTED |

### Kewarani (`mg_kewarani.c`)

| Function            | Used by                    | Status      |
| ------------------- | -------------------------- | ----------- |
| `mg_kw_berolina`    | Medeq, Medeq Squad         | IMPLEMENTED |
| `mg_kw_negus_guard` | Sultan's Levy, Negus Guard | IMPLEMENTED |

### Zarqan (`mg_zarqan.c`)

| Function               | Used by                        | Status      |
| ---------------------- | ------------------------------ | ----------- |
| `mg_zq_ziraafa`        | Ziraafa, War Elephant          | IMPLEMENTED |
| `mg_zq_swap_with_king` | Shahzadeh                      | IMPLEMENTED |
| `mg_zq_war_elephant`   | Ziraafa, War Elephant (threat) | IMPLEMENTED |

### Caelan (`mg_caelan.c`)

| Function        | Used by | Status      |
| --------------- | ------- | ----------- |
| `mg_ca_gryphon` | Gryphon | IMPLEMENTED |

---

## 16. Trigger Index

| Trigger                            | Fires when                             | Key ctx fields                  |
| ---------------------------------- | -------------------------------------- | ------------------------------- |
| `TRIGGER_BATTLE_START`             | Battle initialized                     | —                               |
| `TRIGGER_BATTLE_END`               | Battle concluded                       | result                          |
| `TRIGGER_TURN_START`               | Turn begins                            | active_side                     |
| `TRIGGER_TURN_END`                 | Turn ends                              | active_side                     |
| `TRIGGER_RESOLVE_DEFENSE`          | Resolve phase, defender side           | attacker, defender, damage_mult |
| `TRIGGER_RESOLVE_ATTACK`           | Resolve phase, attacker side           | attacker, defender, damage_mult |
| `TRIGGER_RESOLVE_DAMAGE`           | Damage applied to meter                | attacker, victim_side, dmg      |
| `TRIGGER_RESOLVE_FLIP`             | Piece selected for flip                | piece, attacker                 |
| `TRIGGER_CARD_DRAWN`               | Card drawn                             | card, side                      |
| `TRIGGER_CARD_PLAYED`              | Card played                            | card, side                      |
| `TRIGGER_CARD_SOLD`                | Card sold                              | card, side                      |
| `TRIGGER_COMBO_CHAIN_2`            | 2nd same-kingdom card played this turn | card, side                      |
| `TRIGGER_COMBO_CHAIN_3`            | 3rd same-kingdom card played this turn | card, side                      |
| `TRIGGER_PIECE_PLACED`             | Piece placed on board                  | piece, pos, owner               |
| `TRIGGER_PIECE_MOVED`              | Piece moved                            | piece, from, to                 |
| `TRIGGER_PIECE_COMBINED`           | Piece combined                         | result_piece, ingredient        |
| `TRIGGER_PIECE_DEALT_DAMAGE`       | Piece contributed damage to meter      | piece, dmg                      |
| `TRIGGER_PIECE_ENTERED_ENEMY_TERR` | Piece entered enemy territory          | piece                           |
| `TRIGGER_PIECE_FLIPPED`            | Piece ownership changed                | piece, old_owner, new_owner     |
| `TRIGGER_PIECE_REMOVED`            | Piece left board                       | piece, cause                    |
| `TRIGGER_QUERY_PIECE_COST`         | Query: piece buy cost                  | template_id, cost_out           |
| `TRIGGER_QUERY_SELL_VALUE`         | Query: card sell value                 | card, value_out                 |
| `TRIGGER_QUERY_DRAW_COUNT`         | Query: cards to draw                   | side, count_out                 |
| `TRIGGER_QUERY_TURN_INCOME`        | Query: turn income                     | side, income_out                |
| `TRIGGER_QUERY_MOVE_COUNT`         | Query: extra moves                     | piece, moves_out                |
| `TRIGGER_QUERY_DAMAGE_MULT`        | Query: damage multiplier               | piece, damage_mult_out          |
| `TRIGGER_QUERY_METER_CAP`          | Query: meter cap                       | side, meter_cap_out             |
| `TRIGGER_QUERY_COMBINE_COST`       | Query: combination cost                | cost_out                        |
| `TRIGGER_QUERY_COMBO_VALUE_BONUS`  | Query: combo value bonus               | ingredients, value_out          |
| `TRIGGER_QUERY_ADJ_KING_BONUS`     | Query: adjacent-king bonus             | piece, value_out                |
| `TRIGGER_QUERY_RECLAIM_COST`       | Query: reclaim cost (Harushima innate) | reclaim_cost_out                |
| `TRIGGER_QUERY_ROYAL_SUB_COUNT`    | Query: royal-sub uses per battle       | royal_sub_count_out             |
| `TRIGGER_QUERY_VISION_FLAGS`       | Query: vision bitmap (Eagle Eye, fog)  | vision_flags_out                |
| `TRIGGER_MAP_ENTERED`              | Map entered                            | kingdom                         |
| `TRIGGER_RUN_START`                | Run initialized                        | —                               |

Causes that share a trigger:

| Trigger              | Cause enum         | Values                                                       |
| -------------------- | ------------------ | ------------------------------------------------------------ |
| `TRIGGER_PIECE_PLACED`  | `PlacementCause` | `SPAWN`, `BOUGHT`, `COMBINE_RESULT`, `SPLIT`                 |
| `TRIGGER_PIECE_FLIPPED` | `FlipCause`      | `METER_CASCADE`, `RECLAIM`, `FORCED`, `MERCY`                |
| `TRIGGER_PIECE_REMOVED` | `RemovalCause`   | `SACRIFICE`, `MANDATE`, `SPLITTER_SUBSTITUTION`              |

"Gained" and "lost" are not separate triggers — handlers filter
`TRIGGER_PIECE_FLIPPED` by `new_owner` or `old_owner`. "Bought"
filters `TRIGGER_PIECE_PLACED` by `cause == PLACED_BOUGHT`. War
Chest listens to `TRIGGER_TURN_END` and reads remaining cp directly.

---

## 17. Screen Lifecycle Hooks

### Screen V-Table (`incl/screen.h`)

Every visible mode implements a `Screen` v-table:

```c
typedef struct Screen {
    void (*enter)(struct App* app);      // Called on screen entry
    void (*leave)(struct App* app);      // Called on screen exit
    void (*event)(struct App* app, const SDL_Event* event);  // Input handler
    void (*tick)(struct App* app, float dt);  // Per-frame update
    void (*render)(struct App* app, SDL_Renderer* renderer);  // Draw
} Screen;
```

### Screen Registry

| Screen ID         | File                | Purpose                  | enter | leave | event | tick | render |
| ----------------- | ------------------- | ------------------------ | ----- | ----- | ----- | ---- | ------ |
| `SCREEN_TITLE`    | `screen_title.c`    | Main menu                | yes   | NULL  | yes   | yes  | yes    |
| `SCREEN_MAP`      | `screen_map.c`      | Overworld navigation     | yes   | NULL  | stub  | stub | yes    |
| `SCREEN_BATTLE`   | `screen_battle.c`   | Battle gameplay          | yes   | NULL  | stub  | stub | yes    |
| `SCREEN_EVENT`    | `screen_event.c`    | Event encounters         | yes   | NULL  | stub  | stub | yes    |
| `SCREEN_RESULTS`  | `screen_results.c`  | End-of-run summary       | yes   | NULL  | yes   | stub | yes    |
| `SCREEN_CODEX`    | `screen_codex.c`    | Card/piece encyclopedia  | yes   | NULL  | stub  | stub | yes    |
| `SCREEN_MASTERY`  | `screen_mastery.c`  | Mastery progress display | yes   | NULL  | stub  | stub | yes    |
| `SCREEN_SETTINGS` | `screen_settings.c` | Game settings            | yes   | NULL  | stub  | stub | yes    |

### Screen Transition Mechanism

Transitions are **deferred** to frame boundaries:

1. `screen_goto(app, id)` sets `app->next` and `app->transition_pending = true`
2. `screen_apply_transition(app)` called once per frame before `tick`:
   - Calls outgoing screen's `leave` hook (if non-NULL)
   - Swaps `app->current = app->next`
   - Clears `transition_pending`
   - Calls incoming screen's `enter` hook (if non-NULL)

### Title Screen Key Bindings

| Key | Action                                                                                |
| --- | ------------------------------------------------------------------------------------- |
| 1   | `start_new_run` — Creates new run, seeds Harushima-Town, saves and transitions to MAP |
| 2   | `continue_run` — Loads `run.regsav`, attaches profile, transitions to MAP             |
| 3   | Navigate to CODEX screen                                                              |
| 4   | Navigate to MASTERY screen                                                            |
| 5   | Navigate to SETTINGS screen                                                           |

### Global Key Bindings (`src/app.c`)

| Key | Action                             |
| --- | ---------------------------------- |
| ESC | Sets `quit_requested = true`       |
| N   | Cycles forward through all screens |

---

## 18. Profile Lifecycle

### Profile Persistence (`incl/profile.h`, `src/profile.c`)

The `Profile` struct persists across sessions:

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

**Lifecycle:**

- `profile_load()` — Loads from `profile.regsav` at app startup (`app_init`)
- `profile_save()` — Persists to disk on app exit (`app_quit`) and end of run

### Run Persistence (`incl/run.h`, `src/run.c`)

The `RunState` struct represents an in-progress run:

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
    uint8_t  chain_silver_pending[KINGDOM_COUNT];
    struct Profile* profile;
} RunState;
```

Reclaim cost and Royal Substitution count are not fields. They are
queries: `TRIGGER_QUERY_RECLAIM_COST` and
`TRIGGER_QUERY_ROYAL_SUB_COUNT`. Tomohito's and Timur's figureheads
listen and override on each call.

**Run Flags:**

- `RUN_FOREIGN_MARKUP_OFF` — Trade Routes relic active
- `RUN_DOUBLE_ARCHIVE` — Master's Notes relic active
- `RUN_VISION_ENEMY_VALUES` — Eagle Eye relic active
- `RUN_PREREVEAL_MODIFIER` — Surveyor's Map relic active

### End-of-Run Flow (`run_finalize`)

On `RUN_END_VORATH_WIN`:

- Increment `vorath_defeat_count` and `total_wins`
- Set `prestige_tier` to at least 1
- Advance `mastery_levels[k]` for all non-disqualified kingdoms (max level 3)

On `RUN_END_LOSS`:

- Increment `total_losses` only

In both cases: `profile_save()` is called and `run.regsav` is deleted.

### Battle Loss Handling (`map_on_battle_lost`)

- Increments `chain_levels[k]` for the current kingdom
- Increments `vorath_counter`
- Sets `mastery_disqualified[k] = true` (disqualifies that kingdom from mastery advancement)
- At chain level 3+, marks kingdom as `subjugated` and seeds a liberation trial

### Map Advancement (`map_advance`)

From `map_advance` in `src/map.c`:

- `NODE_BATTLE`, `NODE_ELITE`, `NODE_OVERSEER`, `NODE_LIBERATION_TRIAL` → `SCREEN_BATTLE`
- `NODE_EVENT` → `SCREEN_EVENT`
- `NODE_ARCHIVE`/`NODE_OFFERING` → Resolves immediately, stays on map

---

## 19. Once-Per-Battle Latches

Once-per-battle latches live on the registered `Effect.scratch[]`
of the relic itself, not on `BattleState`. Each latch handler
checks `context->self->scratch[0].v.i` for its triggered flag and
writes 1 after firing.

| Relic               | Slot          | Semantics                                |
| ------------------- | ------------- | ---------------------------------------- |
| Dead Man's Pact     | `scratch[0]`  | 0 = unfired, 1 = used                    |
| Philosopher's Stone | `scratch[0]`  | 0 = unfired, 1 = used                    |
| Deep Hand           | `scratch[0]`  | 0 = unfired, 1 = used                    |
| Bulk Discount       | `scratch[0]`  | buys this turn (PIECE_PLACED listener)   |
| Bulk Discount       | `scratch[1]`  | min buy cost this turn                   |

The `scratch[]` is wiped at battle end alongside the bus.

---

## 20. Event Log Entries

Located in `incl/battle.h`:

| Event                    | Data Captured                 |
| ------------------------ | ----------------------------- |
| `EVT_TURN_STARTED`       | turn_no                       |
| `EVT_TURN_ENDED`         | turn_no                       |
| `EVT_RESOLVE_BEGAN`      | —                             |
| `EVT_RESOLVE_ENDED`      | —                             |
| `EVT_PIECE_PLACED`       | piece_id, pos, tmpl_id, owner |
| `EVT_PIECE_MOVED`        | piece_id, from, to            |
| `EVT_PIECE_COMBINED`     | piece_id (x2)                 |
| `EVT_PIECE_REMOVED`      | piece_id                      |
| `EVT_PIECE_FLIPPED`      | piece_id, new_owner           |
| `EVT_PIECE_DEALT_DAMAGE` | attacker, victim_side, dmg    |
| `EVT_METER_CHANGED`      | side, old_val, new_val        |
| `EVT_CP_CHANGED`         | side, old_val, new_val        |
| `EVT_CARD_DRAWN`         | side, card_tmpl_id            |
| `EVT_CARD_PLAYED`        | side, card_tmpl_id            |
| `EVT_CARD_SOLD`          | side, card_tmpl_id            |
| `EVT_EFFECT_APPLIED`     | effect_source_id, trigger     |
| `EVT_BATTLE_ENDED`       | —                             |

