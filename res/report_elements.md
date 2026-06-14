# Regnum — Element Catalog

**Source**: `res/GDD.md`
**Generated**: 2026-06-14
**Scope**: Every piece, card, relic, mastery card, innate, penalty chain,
battle modifier, board trait, figurehead power, overseer, synergy, event,
and AI archetype — mapped to the function pointer(s) and argument lists
that implement them. Unimplemented behaviors are marked `eff_todo` with a
brief description of the intended behavior.

---

## Conventions

Each entry is organized by element type. Within each type, entries are
grouped by kingdom (Universal first, then Longwei, Harushima, Kewarani,
Zarqan, Caelan, Mastery). Within a kingdom, entries are ordered by tier
(District → Town → Town → Town → Province → Province → Country →
Capstone).

### Piece table columns

| Column | Meaning |
|--------|---------|
| **MoveGen** | `MoveGenFunc` + `EffectArg[]` params |
| **Threat** | `MoveGenFunc` + params (or `= move` if identical) |
| **Passives** | `EffectFunc` + trigger + args |

### Card table columns

| Column | Meaning |
|--------|---------|
| **Tier** | District / Town / Province / Country |
| **Cost** | Play cost in centipawns (`-` if no play action) |
| **Sell** | Sell value in centipawns |
| **On Play** | `EffectFunc` + trigger + args |
| **On Sell** | `EffectFunc` + trigger + args (usually empty) |

### Meta table columns

| Column | Meaning |
|--------|---------|
| **Trigger** | When the effect activates |
| **Apply** | `EffectFunc` + args |
| **Scope** | `local` (this turn) / `battle` / `run` / `piece` |

---

## 1. Pieces

### 1.1 Universal

#### King
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | King |
| Value | 10 |
| MoveGen | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)` |
| Threat | `= move` |
| Passives | None |

---

### 1.2 Longwei

#### Bing
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | District |
| Value | 1 |
| MoveGen | `mg_step`, args: `(dx=0, dy=1)` |
| Threat | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)` |
| Passives | **Sideways step after damage**: `eff_sideways_step`, trigger: `TRIGGER_PIECE_DEALT_DAMAGE`, args: `(self)` |

#### Xiang
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | District |
| Value | 2 |
| MoveGen | `mg_lw_xiang`, args: none |
| Threat | partial (cannot attack into enemy territory) |
| Passives | None |

#### Ma
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | District |
| Value | 2 |
| MoveGen | `mg_lw_ma`, args: none |
| Threat | `= move` |
| Passives | None |

#### Pao
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_lw_pao`, args: none |
| Threat | `= move` |
| Passives | None |

#### Liubo Diviner
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_lw_liubo`, args: none |
| Threat | `mg_todo` (teleport — no threat squares) |
| Passives | **Teleport to threatened square**: passive Effect, trigger: `TRIGGER_TURN_START`, apply: `eff_todo` |

#### Sang
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Town |
| Value | 4 |
| MoveGen | `mg_lw_sang`, args: none |
| Threat | `= move` |
| Passives | None |

#### Northern Cavalry
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)` |
| Threat | `= move` |
| Passives | **Bulwark**: `eff_bulwark`, trigger: `TRIGGER_RESOLVE_DEFENSE`, args: `(self, multiplier=2)` |

#### Hwacha
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Country |
| Value | 6 |
| MoveGen | `mg_lw_hwacha`, args: none |
| Threat | `= move` |
| Passives | None |

---

### 1.3 Harushima

#### Fuhyo
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | District |
| Value | 1 |
| MoveGen | `mg_step`, args: `(dx=0, dy=1)` |
| Threat | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)` |
| Passives | None |

#### Kyosha
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | District |
| Value | 2 |
| MoveGen | `mg_slide`, args: `(dx=0, dy=1, min=1, max=UNBOUNDED)` |
| Threat | `= move` |
| Passives | None |

#### Ginsho
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_hs_ginsho`, args: none |
| Threat | `= move` |
| Passives | None |

#### Kinsho
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_hs_kinsho`, args: none |
| Threat | `= move` |
| Passives | None |

#### Honorable Horse
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)` |
| Threat | `= move` |
| Passives | **Swap to Kinsho on territory entry**: `eff_swap_movegen`, trigger: `TRIGGER_PIECE_ENTERED_ENEMY_TERR`, args: `(self, mg_hs_kinsho)` |

#### Promoted Bishop
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_compound`, args: `(mg_slide_dirs diag, mg_step_set ferz)` |
| Threat | `= move` |
| Passives | None |

#### Shishi
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_hs_shishi`, args: none |
| Threat | `= move` |
| Passives | None |

#### Daimyo
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Country |
| Value | 6 |
| MoveGen | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)` |
| Threat | `= move` |
| Passives | **Immune once per battle**: `eff_immune_once`, trigger: `TRIGGER_RESOLVE_FLIP`, args: `(self)` |

#### Dragon
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Capstone |
| Value | 8 |
| MoveGen | `mg_compound`, args: `(mg_slide_dirs ortho, mg_step_set ferz)` |
| Threat | `= move` |
| Passives | None |

---

### 1.4 Kewarani

#### Medeq
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | District |
| Value | 1 |
| MoveGen | `mg_kw_berolina`, args: none |
| Threat | `mg_attack_only_subset`, args: `(mg_step diag forward, mg_step_set diag)` |
| Passives | None |

#### Makwanam
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | District |
| Value | 2 |
| MoveGen | `mg_step_set`, args: `(1,1) (-1,1) (1,-1) (-1,-1)` |
| Threat | `= move` |
| Passives | None |

#### Saba
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)` |
| Threat | `= move` |
| Passives | None |

#### Faras
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_leap_set`, args: `(1,3) (3,1) (3,-1) (1,-3) (-1,-3) (-3,-1) (-3,1) (-1,3)` |
| Threat | `= move` |
| Passives | None |

#### Medeq Squad
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Town |
| Value | 4 |
| MoveGen | `mg_kw_berolina`, args: none |
| Threat | `= move` |
| Passives | **Splitter**: `eff_splitter_medeq`, trigger: `TRIGGER_PIECE_FLIPPED`, args: `(self, spawn_count=2)` |

#### Sultan's Levy
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_kw_negus_guard`, args: none |
| Threat | `= move` |
| Passives | **Splitter**: `eff_splitter_medeq`, trigger: `TRIGGER_PIECE_FLIPPED`, args: `(self, spawn_count=3)` |

#### Negus Guard
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Capstone |
| Value | 7 |
| MoveGen | `mg_kw_negus_guard`, args: none |
| Threat | `= move` |
| Passives | **Splitter**: `eff_splitter_medeq`, trigger: `TRIGGER_PIECE_FLIPPED`, args: `(self, spawn_count=2)` |

---

### 1.5 Zarqan

#### Wazir
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | District |
| Value | 1 |
| MoveGen | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0)` |
| Threat | `= move` |
| Passives | None |

#### Jamal
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | District |
| Value | 2 |
| MoveGen | `mg_leap_set`, args: `(1,3) (3,1) (3,-1) (1,-3) (-1,-3) (-3,-1) (-3,1) (-1,3)` |
| Threat | `= move` |
| Passives | None |

#### Talliya
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_slide_dirs`, args: `(diag_mask, min=2, max=UNBOUNDED)` |
| Threat | `= move` |
| Passives | None |

#### Ziraafa
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_zq_ziraafa`, args: none |
| Threat | `mg_zq_war_elephant`, args: none |
| Passives | None |

#### Old King
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Town |
| Value | 4 |
| MoveGen | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)` |
| Threat | `= move` |
| Passives | None |

#### Cataphract
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_choice`, args: `(mg_leap_set knight, mg_leap_set camel)` |
| Threat | `= move` |
| Passives | None |

#### Shahzadeh
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0)` |
| Threat | `= move` |
| Passives | **Free king swap**: `eff_free_king_swap`, trigger: `TRIGGER_TURN_START`, args: `(self)` |

#### Rook
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Capstone |
| Value | 7 |
| MoveGen | `mg_slide_dirs`, args: `(ortho_mask, min=1, max=UNBOUNDED)` |
| Threat | `= move` |
| Passives | None |

#### War Elephant
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Country |
| Value | 6 |
| MoveGen | `mg_zq_ziraafa`, args: none |
| Threat | `mg_zq_war_elephant`, args: none |
| Passives | None |

---

### 1.6 Caelan

#### Pawn
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | District |
| Value | 1 |
| MoveGen | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)` |
| Threat | `mg_attack_only_subset`, args: `(mg_step (0,1), mg_step_set diag)` |
| Passives | **First move 2 steps**: `eff_first_move_two`, trigger: `TRIGGER_PIECE_PLACED`, args: `(self)` |

#### Knight
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | District |
| Value | 2 |
| MoveGen | `mg_leap_set`, args: `(1,2) (2,1) (2,-1) (1,-2) (-1,-2) (-2,-1) (-2,1) (-1,2)` |
| Threat | `= move` |
| Passives | None |

#### Bishop
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_slide_dirs`, args: `(diag_mask, min=1, max=UNBOUNDED)` |
| Threat | `= move` |
| Passives | None |

#### Queen
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Town |
| Value | 4 |
| MoveGen | `mg_slide_dirs`, args: `(full_mask, min=1, max=UNBOUNDED)` |
| Threat | `= move` |
| Passives | None |

#### Gryphon
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Town |
| Value | 3 |
| MoveGen | `mg_ca_gryphon`, args: none |
| Threat | `= move` |
| Passives | None |

#### Chancellor
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_choice`, args: `(mg_slide_dirs ortho, mg_leap_set knight)` |
| Threat | `= move` |
| Passives | None |

#### Sovereign Banner
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Province |
| Value | 5 |
| MoveGen | `mg_slide_dirs`, args: `(full_mask, min=1, max=UNBOUNDED)` |
| Threat | `= move` |
| Passives | **Adjacency buff**: `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(self, buff_type=meter_bonus_adjacent)` |

#### King
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Country |
| Value | 6 |
| MoveGen | `mg_step_set`, args: `(0,1) (0,-1) (1,0) (-1,0) (1,1) (1,-1) (-1,1) (-1,-1)` |
| Threat | `= move` |
| Passives | None |

---

## 2. Cards

### 2.1 Universal (12)

#### Pawn Storm
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_pawn_storm`, trigger: `TRIGGER_CARD_PLAYED`, args: `(count=3, free_3rd=true)` |
| On Sell | — |

#### Revitalize
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(heal_target=ally_piece, amount=meter)` |
| On Sell | — |

#### Hostage
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | District |
| Cost | 3 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(next_flip_to_side=passive)` |
| On Sell | — |

#### Last Stand
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Town |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=defense_up, duration=1)` |
| On Sell | — |

#### Sacrifice
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Town |
| Cost | 0 |
| Sell | 0 |
| On Play | `eff_sacrifice`, trigger: `TRIGGER_CARD_PLAYED`, args: `(meter_bonus=2x_value)` |
| On Sell | — |

#### Reforge
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Town |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(next_flip_to_side=passive)` |
| On Sell | — |

#### Mercy
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Province |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(heal_target=ally_meter, amount=flat)` |
| On Sell | — |

#### Bloodletting
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Province |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_bloodletting`, trigger: `TRIGGER_RESOLVE_ATTACK`, args: `(damage_mult=2)` |
| On Sell | — |

#### Counter Coup
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Province |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_counter_coup`, trigger: `TRIGGER_RESOLVE_DEFENSE`, args: `(counter_damage=1)` |
| On Sell | — |

#### Spite
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Country |
| Cost | 6 |
| Sell | 3 |
| On Play | `eff_spite`, trigger: `TRIGGER_CARD_PLAYED`, args: `(passive1=TRIGGER_PIECE_FLIPPED+side, passive2=TRIGGER_PIECE_REMOVED+side)` |
| On Sell | — |

#### Chain Break
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Country |
| Cost | 7 |
| Sell | 4 |
| On Play | `eff_chain_break`, trigger: `TRIGGER_CARD_PLAYED`, args: `(flip_target=enemy_piece)` |
| On Sell | — |

#### Hydra
| Field | Value |
|-------|-------|
| Kingdom | Universal |
| Tier | Country |
| Cost | 8 |
| Sell | 5 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(spawn_piece=PIECE_MEDEQ, count=3)` |
| On Sell | — |

---

### 2.2 Longwei (7)

#### River Wade
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(meter_gain=flat)` |
| On Sell | — |

#### Charge
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | District |
| Cost | 3 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=attack_up, duration=1)` |
| On Sell | — |

#### Formation
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Town |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_formation`, trigger: `TRIGGER_RESOLVE_ATTACK`, args: `(damage_bonus=longwei_pieces)` |
| On Sell | — |

#### Divination
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Town |
| Cost | 3 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(peek_cards=3)` |
| On Sell | — |

#### Cannon Volley
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Province |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(damage=enemy_meter, amount=pao_piece_count)` |
| On Sell | — |

#### Palace Decree
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Province |
| Cost | 6 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=fortified, duration=battle)` |
| On Sell | — |

#### Mandate
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Country |
| Cost | 7 |
| Sell | 4 |
| On Play | `eff_mandate`, trigger: `TRIGGER_CARD_PLAYED`, args: `(damage_mult=3)` |
| On Sell | — |

---

### 2.3 Harushima (7)

#### Ronin
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=move_up, duration=1)` |
| On Sell | — |

#### Resurrection
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Town |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(revive_piece=ally_graveyard)` |
| On Sell | — |

#### Gold Standard
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Town |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(upgrade_piece=promote)` |
| On Sell | — |

#### Promotion
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Town |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_upgrade=harushima)` |
| On Sell | — |

#### Dual Drop
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Province |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(draw_count=2, no_discard=true)` |
| On Sell | — |

#### Force Drop
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Province |
| Cost | 6 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(enemy_discard=2)` |
| On Sell | — |

#### Bushido
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Country |
| Cost | 8 |
| Sell | 5 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(passive_buff=honorable_death)` |
| On Sell | — |

---

### 2.4 Kewarani (7)

#### Sultan's Gold
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(cp_bonus=2)` |
| On Sell | — |

#### March
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(move_piece=free)` |
| On Sell | — |

#### Double Time
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Town |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(actions_bonus=1)` |
| On Sell | — |

#### Salt Road
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Town |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(sell_bonus=2)` |
| On Sell | — |

#### Caravan
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Province |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(income_bonus=2)` |
| On Sell | — |

#### Doublestrike
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Province |
| Cost | 6 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=attack_twice)` |
| On Sell | — |

#### Hajj
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Country |
| Cost | 7 |
| Sell | 4 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(piece_buff=splitter_blessing)` |
| On Sell | — |

---

### 2.5 Zarqan (8)

#### Counsel
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(peek_top=3)` |
| On Sell | — |

#### Pillage
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | District |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(cp_steal=3)` |
| On Sell | — |

#### Royal Decoy
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Town |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(swap_piece=enemy)` |
| On Sell | — |

#### Bazaar
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Town |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(discount=foreign_kingdom)` |
| On Sell | — |

#### Steppe Riders
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Town |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(move_all=forward)` |
| On Sell | — |

#### Ambition
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Province |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(movegen_override=upgrade)` |
| On Sell | — |

#### Citadel
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Province |
| Cost | 6 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(territory_buff=defensive)` |
| On Sell | — |

#### Conquest
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Country |
| Cost | 8 |
| Sell | 5 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(upgrade_all=zarqan)` |
| On Sell | — |

---

### 2.6 Caelan (8)

#### Castling
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | District |
| Cost | 3 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(swap_with_king=true)` |
| On Sell | — |

#### Queen's Gambit
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | District |
| Cost | 2 |
| Sell | 1 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(card_discount=queen)` |
| On Sell | — |

#### Vengeance
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Town |
| Cost | 4 |
| Sell | 2 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(damage=enemy_meter, bonus=ally_lost)` |
| On Sell | — |

#### Queen's Decree
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Town |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_queens_decree`, trigger: `TRIGGER_RESOLVE_ATTACK`, args: `(damage_boost=adjacent_queen)` |
| On Sell | — |

#### Cathedral
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Province |
| Cost | 5 |
| Sell | 3 |
| On Play | `eff_cathedral`, trigger: `TRIGGER_RESOLVE_DEFENSE`, args: `(damage_reduction=1)` |
| On Sell | — |

#### Coronation
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Province |
| Cost | 6 |
| Sell | 3 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(upgrade_piece=king)` |
| On Sell | — |

#### Crusade
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Country |
| Cost | 7 |
| Sell | 4 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(movegen_override=upgrade_all)` |
| On Sell | — |

#### Divine Right
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Country |
| Cost | 8 |
| Sell | 5 |
| On Play | `eff_todo`, trigger: `TRIGGER_CARD_PLAYED`, args: `(king_buff=battle)` |
| On Sell | — |

---

### 2.7 Mastery Cards (5)

#### Mingzhu's Seal
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Tier | Province |
| Cost | — |
| Sell | 0 |
| On Play | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)` |
| On Sell | — |

#### Tomohito's Patience
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Tier | Province |
| Cost | — |
| Sell | 0 |
| On Play | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)` |
| On Sell | — |

#### Selassie's March
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Tier | Province |
| Cost | — |
| Sell | 0 |
| On Play | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)` |
| On Sell | — |

#### Timur's Conquest
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Tier | Province |
| Cost | — |
| Sell | 0 |
| On Play | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)` |
| On Sell | — |

#### Isabella's Coronation
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Tier | Province |
| Cost | — |
| Sell | 0 |
| On Play | `eff_todo`, trigger: `TRIGGER_TURN_START`, args: `(mastery_card=true)` |
| On Sell | — |

---

## 3. Relics (26)

### 3.1 Economy (6)

#### Merchant's Ledger
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RUN_START` |
| Apply | `eff_todo`, args: `(income_bonus=1)` |
| Scope | run |

#### Minted Coin
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RUN_START` |
| Apply | `eff_todo`, args: `(cp_start_bonus=5)` |
| Scope | run |

#### Tax Stamp
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_CARD_SOLD` |
| Apply | `eff_todo`, args: `(sell_value_bonus=1)` |
| Scope | run |

#### Bulk Discount
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(discount=bulk)` |
| Scope | battle |

#### War Chest
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(cp_start_bonus=3)` |
| Scope | battle |

#### Trade Routes
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_TURN_START` |
| Apply | `eff_todo`, args: `(income_bonus=2)` |
| Scope | run |

---

### 3.2 Meter (6)

#### Soul Shard
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_FLIPPED` |
| Apply | `eff_todo`, args: `(meter_gain=piece_value)` |
| Scope | run |

#### Veteran's Bond
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(meter_start_bonus=10)` |
| Scope | battle |

#### Dead Man's Pact
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_REMOVED` |
| Apply | `eff_todo`, args: `(meter_bonus=on_remove)` |
| Scope | run |

#### Iron King
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RESOLVE_DEFENSE` |
| Apply | `eff_todo`, args: `(damage_reduction=flat)` |
| Scope | battle |

#### Bloodthirst
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RESOLVE_ATTACK` |
| Apply | `eff_todo`, args: `(damage_boost=pieces_lost)` |
| Scope | battle |

#### Last Breath
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_REMOVED` |
| Apply | `eff_todo`, args: `(damage=enemy_meter, amount=piece_value)` |
| Scope | run |

---

### 3.3 Cards (5)

#### Tactician's Scroll
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(draw_bonus=1)` |
| Scope | battle |

#### Librarian's Notes
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_MAP_ENTERED` |
| Apply | `eff_todo`, args: `(cardset_expansion=1)` |
| Scope | run |

#### Country Seal
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(same_kingdom_draw=2)` |
| Scope | battle |

#### Deep Hand
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(hand_size_bonus=2)` |
| Scope | battle |

#### Gilded Archive
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RUN_START` |
| Apply | `eff_todo`, args: `(cardset_bonus=3)` |
| Scope | run |

---

### 3.4 Combinations (4)

#### Alchemist's Kit
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_COMBINED` |
| Apply | `eff_todo`, args: `(meter_bonus=on_combine)` |
| Scope | run |

#### Master's Notes
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_COMBINED` |
| Apply | `eff_todo`, args: `(value_bonus=result_piece)` |
| Scope | run |

#### Philosopher's Stone
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_COMBINED` |
| Apply | `eff_todo`, args: `(upgrade_piece=on_combine)` |
| Scope | run |

#### Inherited Power
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_COMBINED` |
| Apply | `eff_todo`, args: `(keep_buffs=on_combine)` |
| Scope | run |

---

### 3.5 Board (5)

#### Eagle Eye
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(threat_reveal=true)` |
| Scope | battle |

#### Surveyor's Map
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(board_size_bonus=2)` |
| Scope | battle |

#### Forward Command
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_PIECE_PLACED` |
| Apply | `eff_todo`, args: `(forward_placement=1)` |
| Scope | battle |

#### Fortified Line
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(territory_defense=bonus)` |
| Scope | battle |

#### Warlord's Banner
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_TURN_START` |
| Apply | `eff_todo`, args: `(meter_gain=territory_held)` |
| Scope | battle |

---

## 4. Innate Powers (5)

### 4.1 Longwei — Bulwark
| Field | Value |
|-------|-------|
| Kingdom | Longwei |
| Trigger | `TRIGGER_RESOLVE_DEFENSE` |
| Apply | `eff_bulwark`, args: `(multiplier=2)` |
| Scope | innate (map) |

### 4.2 Harushima — Reclaim
| Field | Value |
|-------|-------|
| Kingdom | Harushima |
| Trigger | `TRIGGER_PIECE_REMOVED` |
| Apply | `eff_todo`, args: `(respawn_medeq=on_remove)` |
| Scope | innate (map) |

### 4.3 Kewarani — Double Time
| Field | Value |
|-------|-------|
| Kingdom | Kewarani |
| Trigger | `TRIGGER_TURN_START` |
| Apply | `eff_todo`, args: `(actions_bonus=1)` |
| Scope | innate (map) |

### 4.4 Zarqan — Royal Substitution
| Field | Value |
|-------|-------|
| Kingdom | Zarqan |
| Trigger | `TRIGGER_TURN_START` |
| Apply | `eff_todo`, args: `(king_swap=free)` |
| Scope | innate (map) |

### 4.5 Caelan — Conqueror's Reward
| Field | Value |
|-------|-------|
| Kingdom | Caelan |
| Trigger | `TRIGGER_PIECE_FLIPPED` |
| Apply | `eff_todo`, args: `(value_mod=+1_per_flip)` |
| Scope | innate (map) |

---

## 5. Penalty Chains (3)

### 5.1 Bronze Chain
| Field | Value |
|-------|-------|
| Level | 1 |
| Penalty | `eff_todo`, args: `(cp_start=-3)` |
| Scope | run |

### 5.2 Silver Chain
| Field | Value |
|-------|-------|
| Level | 2 |
| Penalty | `eff_todo`, args: `(cp_start=-5, hand_size=-1)` |
| Scope | run |

### 5.3 Gold Chain
| Field | Value |
|-------|-------|
| Level | 3 |
| Penalty | `eff_todo`, args: `(cp_start=-8, hand_size=-2, meter_start=-10)` |
| Scope | run |

---

## 6. Battle Modifiers (12)

### 6.1 Economy (3)

#### Lean Times
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(cp_start=-5)` |
| Scope | battle |

#### Windfall
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(cp_start=+5)` |
| Scope | battle |

#### Open Market
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(foreign_discount=2)` |
| Scope | battle |

---

### 6.2 Meter (3)

#### Glass Cannon
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(meter_start=50pct)` |
| Scope | battle |

#### Bloodbath
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RESOLVE_ATTACK` |
| Apply | `eff_todo`, args: `(damage_mult=1.5)` |
| Scope | battle |

#### Iron Will
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_RESOLVE_DEFENSE` |
| Apply | `eff_todo`, args: `(damage_reduction=0.5)` |
| Scope | battle |

---

### 6.3 Cards (3)

#### Rich Hand
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(draw_count=+2)` |
| Scope | battle |

#### Sparse Hand
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(draw_count=-1)` |
| Scope | battle |

#### Kingdom Purity
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(same_kingdom_only=true)` |
| Scope | battle |

---

### 6.4 Board (3)

#### Fog of War
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(hide_enemy_values=true)` |
| Scope | battle |

#### Dense Terrain
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(block_squares=random)` |
| Scope | battle |

#### Extended Front
| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(board_width=+4)` |
| Scope | battle |

---

## 7. Board Traits (10)

| Trait | Trigger | Apply | Scope |
|-------|----------|-------|--------|
| River Crossing | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=river)` | battle |
| Palace | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=palace)` | battle |
| Fog Coast | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=fog)` | battle |
| Island Chain | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=island)` | battle |
| Trade Route | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=trade)` | battle |
| Contested Market | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=contested)` | battle |
| Sandstorm | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=sand)` | battle |
| Mirage | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=mirage)` | battle |
| Castle Corners | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=castle)` | battle |
| Siege Trench | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(terrain=trench)` | battle |

---

## 8. Figurehead Powers (5)

| Kingdom | Power | Trigger | Apply | Scope |
|---------|-------|----------|-------|--------|
| Longwei | Iron Strategist | `TRIGGER_TURN_START` | `eff_todo`, args: `(streak_attack=true)` | run |
| Harushima | Honorable Death | `TRIGGER_PIECE_REMOVED` | `eff_todo`, args: `(respawn_medeq)` | run |
| Kewarani | Merchant Prince | `TRIGGER_CARD_SOLD` | `eff_todo`, args: `(sell_bonus=1)` | run |
| Zarqan | Grand Vizier | `TRIGGER_TURN_START` | `eff_todo`, args: `(free_card_discount=1)` | run |
| Caelan | Divine Right | `TRIGGER_PIECE_FLIPPED` | `eff_todo`, args: `(value_mod=+1)` | run |

---

## 9. Overseers (5 + Vorath)

| Overseer | Kingdom | Trigger | Apply | Scope |
|---------|---------|---------|-------|--------|
| Iron Strategist | Longwei | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(overseer=iron_strategist)` | battle |
| Eternal Recursion | Harushima | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(overseer=eternal_recursion)` | battle |
| Caravan of Conquest | Kewarani | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(overseer=caravan)` | battle |
| Many-Faced King | Zarqan | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(overseer=many_faced)` | battle |
| Crowned Heretic | Caelan | `TRIGGER_BATTLE_START` | `eff_todo`, args: `(overseer=crowned_heretic)` | battle |

### Vorath (Grand King + 5 Minor Kings)

| Field | Value |
|-------|-------|
| Trigger | `TRIGGER_BATTLE_START` |
| Apply | `eff_todo`, args: `(board=20x20, quadrants=5, grand_king_meter_reset)` |
| Scope | battle |

The Grand King's meter resets to full at the start of every turn unless
the player attacked at least one Minor King in each of the 5 quadrants
on the immediately preceding turn. The run-end win condition is reducing
the Grand King's meter to zero in a turn where all 5 quadrants were struck.

---

## 10. Kingdom Synergies (5)

| Cleared Kingdom | Synergy | Trigger | Apply | Scope |
|-----------------|---------|---------|-------|--------|
| Longwei | Siege Engineer | `TRIGGER_MAP_ENTERED` | `eff_todo`, args: `(synergy=longwei)` | run |
| Harushima | Reclaimer | `TRIGGER_MAP_ENTERED` | `eff_todo`, args: `(synergy=harushima)` | run |
| Kewarani | The Tide | `TRIGGER_MAP_ENTERED` | `eff_todo`, args: `(synergy=kewarani)` | run |
| Zarqan | Trickster | `TRIGGER_MAP_ENTERED` | `eff_todo`, args: `(synergy=zarqan)` | run |
| Caelan | The Hammer | `TRIGGER_MAP_ENTERED` | `eff_todo`, args: `(synergy=caelan)` | run |

---

## 11. Events (per-kingdom + universal)

Events are narrative choices presented to the player on the map.
Each event has multiple options, each with an `EffectFunc` that applies
run-wide changes.

| Event | Kingdom | Options |
|-------|---------|---------|
| Iron Strategist | Longwei | Option A: `eff_todo` (defensive path) / Option B: `eff_todo` (aggressive path) |
| Eternal Recursion | Harushima | Option A: `eff_todo` (honor path) / Option B: `eff_todo` (pragmatic path) |
| Merchant Prince | Kewarani | Option A: `eff_todo` (trade path) / Option B: `eff_todo` (conquest path) |
| Grand Vizier | Zarqan | Option A: `eff_todo` (loyalty path) / Option B: `eff_todo` (betrayal path) |
| Divine Right | Caelan | Option A: `eff_todo` (faith path) / Option B: `eff_todo` (power path) |

---

## 12. Combo Chain Climaxes (5)

| Kingdom | Climax | Trigger | Apply | Scope |
|---------|--------|---------|-------|--------|
| Longwei | Siege | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=longwei)` | battle |
| Harushima | Flood | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=harushima)` | battle |
| Kewarani | Stampede | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=kewarani)` | battle |
| Zarqan | Conquest | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=zarqan)` | battle |
| Caelan | Crusade | `TRIGGER_COMBO_CHAIN_3` | `eff_todo`, args: `(chain3=caelan)` | battle |

---

## 13. AI Archetypes (5)

| Archetype | Kingdom | Pick Function | Primary Weights | Fallback Weights |
|-----------|---------|---------------|-----------------|------------------|
| Siege Engineer | Longwei | `NULL` (default scorer) | value_diff=12, territory=6, aggression=4, sell=4, save=6, max_cost=8, combo=5, reclaim=0 | value_diff=6, territory=9, aggression=2, sell=3, save=4, max_cost=6, combo=3, reclaim=0 |
| Reclaimer | Harushima | `NULL` (default scorer) | value_diff=8, territory=4, aggression=2, sell=2, save=8, max_cost=5, combo=7, reclaim=10 | value_diff=4, territory=6, aggression=1, sell=2, save=5, max_cost=4, combo=4, reclaim=5 |
| The Tide | Kewarani | `NULL` (default scorer) | value_diff=6, territory=3, aggression=8, sell=5, save=5, max_cost=7, combo=4, reclaim=0 | value_diff=3, territory=5, aggression=4, sell=3, save=3, max_cost=5, combo=2, reclaim=0 |
| Trickster | Zarqan | `NULL` (default scorer) | value_diff=5, territory=2, aggression=12, sell=6, save=3, max_cost=9, combo=3, reclaim=0 | value_diff=2, territory=4, aggression=6, sell=4, save=2, max_cost=7, combo=2, reclaim=0 |
| The Hammer | Caelan | `NULL` (default scorer) | value_diff=9, territory=8, aggression=2, sell=3, save=7, max_cost=7, combo=6, reclaim=0 | value_diff=5, territory=10, aggression=1, sell=2, save=5, max_cost=5, combo=4, reclaim=0 |

---

## 14. Effect Functions Index

All `EffectFunc` implementations live in `src/effects/`:

| Function | File | Trigger(s) | Purpose |
|----------|------|------------|---------|
| `eff_todo` | `eff_*.c` | any | Placeholder; logs and no-ops |
| `eff_bulwark` | `eff_damage.c` | `TRIGGER_RESOLVE_DEFENSE` | 2× damage reduction |
| `eff_bloodletting` | `eff_damage.c` | `TRIGGER_RESOLVE_ATTACK` | 2× damage dealt |
| `eff_counter_coup` | `eff_damage.c` | `TRIGGER_RESOLVE_DEFENSE` | Counter-attack on defended hit |
| `eff_cathedral` | `eff_damage.c` | `TRIGGER_RESOLVE_DEFENSE` | −1 damage taken |
| `eff_queens_decree` | `eff_damage.c` | `TRIGGER_RESOLVE_ATTACK` | Damage boost for adjacent Queen |
| `eff_formation` | `eff_damage.c` | `TRIGGER_RESOLVE_ATTACK` | Damage scales with Longwei pieces |
| `eff_splitter_medeq` | `eff_flip.c` | `TRIGGER_PIECE_FLIPPED` | Spawn Medeq + remove self |
| `eff_sideways_step` | `eff_movement.c` | `TRIGGER_PIECE_DEALT_DAMAGE` | Grant sideways step after damage |
| `eff_first_move_two` | `eff_movement.c` | `TRIGGER_PIECE_PLACED` | Allow first move of 2 squares |
| `eff_swap_movegen` | `eff_movement.c` | `TRIGGER_PIECE_ENTERED_ENEMY_TERR` | Swap move override |
| `eff_free_king_swap` | `eff_movement.c` | `TRIGGER_TURN_START` | Free king swap action |
| `eff_pawn_storm` | `eff_card.c` | `TRIGGER_CARD_PLAYED` | Up to 3 pawns, 3rd free |
| `eff_sacrifice` | `eff_card.c` | `TRIGGER_CARD_PLAYED` | Remove piece, gain 2× value as meter |
| `eff_mandate` | `eff_card.c` | `TRIGGER_CARD_PLAYED` | Remove piece, deal 3× value as damage |
| `eff_chain_break` | `eff_piece.c` | `TRIGGER_CARD_PLAYED` | Force flip an enemy piece |
| `eff_spite` | `eff_piece.c` | `TRIGGER_CARD_PLAYED` | Install two passives on piece loss |
| `eff_immune_once` | `eff_piece.c` | `TRIGGER_RESOLVE_FLIP` | Block one flip this battle |
| `eff_buymeter` | `eff_meter.c` | any | Adjust meter |
| `eff_cp` | `eff_economy.c` | any | Adjust cp |
| `eff_draw` | `eff_card.c` | any | Draw extra cards |
| `eff_buy_cost` | `eff_economy.c` | `TRIGGER_QUERY_PIECE_COST` | Modify buy cost |
| `eff_sell_value` | `eff_economy.c` | `TRIGGER_QUERY_SELL_VALUE` | Modify sell value |

---

## 15. MoveGen Functions Index

All `MoveGenFunc` implementations live in `src/movegens/`:

### Basics (`mg_basics.c`)

| Function | Params | Used by |
|----------|--------|---------|
| `mg_step` | `dx, dy` | Bing, Fuhyo, Medeq, Pawn |
| `mg_step_set` | `n × (dx,dy)` | Makwanam, Wazir, Old King, King, Daimyo, Pawn attack |
| `mg_slide` | `dx, dy, min, max` | Kyosha |
| `mg_slide_dirs` | `dir_mask, min, max` | Bishop, Rook, Queen, Talliya, Chancellor, Gryphon |
| `mg_leap_set` | `n × (dx,dy)` | Ma, Knight, Northern Cavalry, Honorable Horse, Saba, Faras, Jamal, Cataphract |
| `mg_blockable_leap` | `dx, dy, intermediates[]` | (reserved for future pieces) |
| `mg_compound` | `n × MoveGen` | Promoted Bishop, Dragon |
| `mg_choice` | `n × MoveGen` | Cataphract, Chancellor |
| `mg_double_act` | `MoveGen` | (reserved) |
| `mg_territory_restricted` | `MoveGen, mask` | (reserved) |
| `mg_attack_only_subset` | `move, attack` | Pawn, Medeq |
| `mg_todo` | — | All unimplemented movegens |

### Longwei (`mg_longwei.c`)

| Function | Used by |
|----------|---------|
| `mg_lw_ma` | Ma |
| `mg_lw_xiang` | Xiang |
| `mg_lw_pao` | Pao |
| `mg_lw_hwacha` | Hwacha |
| `mg_lw_sang` | Sang |
| `mg_lw_liubo` | Liubo Diviner |

### Harushima (`mg_harushima.c`)

| Function | Used by |
|----------|---------|
| `mg_hs_kinsho` | Kinsho, Honorable Horse (swap target) |
| `mg_hs_ginsho` | Ginsho |
| `mg_hs_honorable_horse` | Honorable Horse (initial) |
| `mg_hs_shishi` | Shishi |

### Kewarani (`mg_kewarani.c`)

| Function | Used by |
|----------|---------|
| `mg_kw_berolina` | Medeq, Medeq Squad |
| `mg_kw_negus_guard` | Sultan's Levy, Negus Guard |

### Zarqan (`mg_zarqan.c`)

| Function | Used by |
|----------|---------|
| `mg_zq_ziraafa` | Ziraafa, War Elephant |
| `mg_zq_swap_with_king` | Shahzadeh |
| `mg_zq_war_elephant` | Ziraafa, War Elephant (threat) |

### Caelan (`mg_caelan.c`)

| Function | Used by |
|----------|---------|
| `mg_ca_gryphon` | Gryphon |

---

## 16. Trigger Index

| Trigger | Fires when | Key ctx fields |
|---------|------------|---------------|
| `TRIGGER_BATTLE_START` | Battle initialized | — |
| `TRIGGER_BATTLE_END` | Battle concluded | result |
| `TRIGGER_TURN_START` | Turn begins | active_side |
| `TRIGGER_TURN_END` | Turn ends | active_side |
| `TRIGGER_RESOLVE_DEFENSE` | Resolve phase, defender side | attacker, defender, damage_mult |
| `TRIGGER_RESOLVE_ATTACK` | Resolve phase, attacker side | attacker, defender, damage_mult |
| `TRIGGER_RESOLVE_DAMAGE` | Damage applied to meter | attacker, victim_side, dmg |
| `TRIGGER_RESOLVE_FLIP` | Piece selected for flip | piece, attacker |
| `TRIGGER_CARD_DRAWN` | Card drawn | card, side |
| `TRIGGER_CARD_PLAYED` | Card played | card, side |
| `TRIGGER_CARD_SOLD` | Card sold | card, side |
| `TRIGGER_COMBO_CHAIN_2` | 2nd same-kingdom card played this turn | card, side |
| `TRIGGER_COMBO_CHAIN_3` | 3rd same-kingdom card played this turn | card, side |
| `TRIGGER_PIECE_PLACED` | Piece placed on board | piece, pos, owner |
| `TRIGGER_PIECE_MOVED` | Piece moved | piece, from, to |
| `TRIGGER_PIECE_COMBINED` | Piece combined | result_piece, ingredient |
| `TRIGGER_PIECE_DEALT_DAMAGE` | Piece contributed damage to meter | piece, dmg |
| `TRIGGER_PIECE_ENTERED_ENEMY_TERR` | Piece entered enemy territory | piece |
| `TRIGGER_PIECE_FLIPPED` | Piece ownership changed | piece, old_owner, new_owner |
| `TRIGGER_PIECE_REMOVED` | Piece left board | piece, reason |
| `TRIGGER_QUERY_PIECE_COST` | Query: piece buy cost | tmpl_id, out_cost |
| `TRIGGER_QUERY_SELL_VALUE` | Query: card sell value | card, out_value |
| `TRIGGER_QUERY_DRAW_COUNT` | Query: cards to draw | side, out_count |
| `TRIGGER_QUERY_TURN_INCOME` | Query: turn income | side, out_income |
| `TRIGGER_QUERY_MOVE_COUNT` | Query: extra moves | piece, out_count |
| `TRIGGER_QUERY_DAMAGE_MULT` | Query: damage multiplier | piece, out_mult |
| `TRIGGER_QUERY_METER_CAP` | Query: meter cap | side, out_cap |
| `TRIGGER_MAP_ENTERED` | Province entered on map | province |
| `TRIGGER_RUN_START` | Run initialized | — |
