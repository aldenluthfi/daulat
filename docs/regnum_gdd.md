# REGNUM

_Five kingdoms. One tyrant. No mercy._

---

## 1. THE WORLD

| Figurehead       | Kingdom             | Tradition                 |
| ---------------- | ------------------- | ------------------------- |
| Empress Mingzhu  | Longwei Empire      | Xiangqi, Janggi           |
| Shogun Tomohito  | Harushima Shogunate | Shogi, Chu Shogi          |
| Negesta Selassie | Kewarani Negusate   | Senterej                  |
| Sultan Timur     | Zarqan Sultanate    | Tamerlane Chess, Shatranj |
| Queen Isabella   | Caelan Kingdom      | Chess, Grant Acredex      |

Vorath, the Unbroken, rules all five figureheads as vassals. To dethrone him, the player climbs the hierarchy of each kingdom through three maps (Town, Province, Country) then faces Vorath on a grand arena board.

---

## 2. THE BATTLE

### Board

Irregularly shaped, checkered. Cells may be missing to create unusual shapes.

| Map Level | Board Size  |
| --------- | ----------- |
| Town      | 12×12       |
| Province  | 14×14-15x15 |
| Country   | 16×16-18x18 |
| Vorath    | 20×20       |

Both players start with only their king. The player always goes first. Battles last 10 turns.

### Piece Value

Every piece has one number: its **value**. Value is simultaneously:

- The centipawn cost to buy it
- Its contribution to its owner's meter
- The damage it deals when attacking

King has value 10 and cannot be bought.

### The Meter

Each player's meter is the sum of all their pieces' values.

When the meter empties: one random non-king piece flips to the opponent's side. Meter recalculates from remaining pieces.

Win by flipping the enemy king. This occurs when only the enemy king remains and its meter hits zero.

If 10 turns pass without a king flip: the player controlling more territory wins.

### Territory

A square belongs to whichever player has a piece closer to it (Chebyshev distance). Equidistant squares are neutral. Territory is the tiebreaker only; it does not restrict placement.

### Pieces and Damage

Pieces cannot be captured by movement. They exist on the board until they flip. Pieces cannot move onto occupied squares.

At end of each turn: every enemy square your pieces threaten contributes your piece's value as damage to the enemy meter. Multiple attackers on the same square stack.

### Economy

|                       | Amount                            |
| --------------------- | --------------------------------- |
| Starting centipawns   | 20                                |
| Per-turn income       | 10                                |
| Home region discount  | 40% off own kingdom's pieces      |
| Foreign region markup | 20% more foreign kingdom's pieces |
| Combining cost        | 0 cp, 1 action                    |

### Turn Structure

1. Draw 3 cards. All 3 must be played or sold by end of turn. Unsold cards auto-sell at printed sell value.
2. Spend up to 3 actions (in any combination):
   - Move a piece
   - Buy and place a piece anywhere on the board
   - Combine two pieces

Cards do not cost actions.

### Resolve Order

1. Opponent's defensive buffs apply to their pieces (damage reductions, immunities, Bulwark)
2. Your offensive buffs apply to your pieces (damage amplifications)
3. Each attacking piece deals its modified value as damage to the enemy meter
4. If either meter hit zero, a random non-king piece flips; meter recalculates
5. End-of-turn card triggers resolve

---

## 3. PIECES

### Combination Rules

- Select two friendly pieces anywhere on the board. Cost: 1 action, 0 cp.
- Result occupies the **second selected piece's square**. The first square becomes vacant.
- Result adopts the kingdom of its result piece, not its parents.
- No tier locks. Gate is ingredient availability.
- Multi-step combos require an ingredient that is itself a combo result.

---

### Longwei Empire

#### Base Pieces

| Piece                      | Tier            | Value | Movement                                                                                                        |
| -------------------------- | --------------- | ----- | --------------------------------------------------------------------------------------------------------------- |
| Bing                       | District        | 10    | Forward 1. Gains a sideways step after first damaging an enemy piece.                                           |
| Xiang                      | District        | 20    | 2 diagonal. Cannot land in enemy territory.                                                                     |
| Ma                         | Town            | 30    | Knight L. Blocked if any piece sits in the elbow square.                                                        |
| Pao                        | Province        | 50    | Rook-line movement. Can only attack if exactly one piece (the screen) sits between Pao and target on that line. |
| Liubo Diviner _(capstone)_ | Overseer reward | 100   | Teleports to any square currently threatened by any enemy piece.                                                |

#### Combinations (Longwei)

| Recipe       | Result           | Value | Notes                                                                                                                        |
| ------------ | ---------------- | ----- | ---------------------------------------------------------------------------------------------------------------------------- |
| Bing + Saba  | Sang             | 30    | Janggi elephant: 1 orthogonal then 2 diagonal (3 total). Blockable at either intermediate square. May enter enemy territory. |
| Färäs + Bing | Northern Cavalry | 40    | Knight L. Grants Bulwark to adjacent allies.                                                                                 |
| Pao + Bishop | Hwacha           | 60    | Pao mechanic applied to diagonals as well as rows and columns.                                                               |

---

### Harushima Shogunate

#### Base Pieces

| Piece               | Tier            | Value | Movement                                                                        |
| ------------------- | --------------- | ----- | ------------------------------------------------------------------------------- |
| Fuhyo               | District        | 10    | Forward 1.                                                                      |
| Kyosha              | District        | 30    | Forward any distance (slider).                                                  |
| Ginsho              | Town            | 30    | 1 square diagonal or forward.                                                   |
| Kinsho              | Province        | 35    | 1 square any direction except diagonal-backward.                                |
| Shishi _(capstone)_ | Overseer reward | 100   | Acts twice per move action: move-then-move, or move-then-attack-without-moving. |

#### Combinations (Harushima)

| Recipe                      | Result          | Value | Notes                                                                                                |
| --------------------------- | --------------- | ----- | ---------------------------------------------------------------------------------------------------- |
| Kyosha + Knight             | Honorable Horse | 40    | Shogi knight (2 forward + 1 sideways). Becomes Kinsho movement after first entering enemy territory. |
| Bishop + Wazir              | Promoted Bishop | 50    | Bishop movement + 1 square orthogonal.                                                               |
| Kinsho + Ginsho             | Daimyo          | 50    | King movement (1 any direction). Immune to flipping once, then normal.                               |
| Rook + Wazir _(multi-step)_ | Dragon          | 70    | Rook movement + 1 square diagonal.                                                                   |

---

### Kewarani Negusate

#### Base Pieces

| Piece                    | Tier            | Value | Movement                                                                                                              |
| ------------------------ | --------------- | ----- | --------------------------------------------------------------------------------------------------------------------- |
| Medeq                    | District        | 10    | Forward 1. Attacks diagonally. Paired-move with opposing Medeq in opening phase.                                      |
| Makwanam                 | District        | 15    | 1 diagonal (ferz).                                                                                                    |
| Saba                     | Town            | 20    | 2 diagonal. Unrestricted by territory.                                                                                |
| Faras                    | Province        | 30    | Knight L.                                                                                                             |
| Negus Guard _(capstone)_ | Overseer reward | 100   | Moves twice per action. When it would flip: removed from board instead, 2 friendly Medeq spawn adjacent to your king. |

#### Combinations (Kewarani)

| Recipe                            | Result        | Value | Notes                                                                                                                              |
| --------------------------------- | ------------- | ----- | ---------------------------------------------------------------------------------------------------------------------------------- |
| Pawn + Pawn (any kingdoms)        | Medeq Squad   | 20    | Pawn movement. When flipped: removed, 2 friendly Medeq spawn adjacent to its square.                                               |
| Negus Guard + Pawn _(multi-step)_ | Sultan's Levy | 110   | Negus Guard movement. When flipped: removed, 3 friendly Medeq spawn adjacent to your king. Consuming the capstone is irreversible. |

All Kewarani splitter pieces (Medeq Squad, Sultan's Levy, Negus Guard) resist flipping. When any would flip, they are removed instead and spawn friendly Medeq pawns.

---

### Zarqan Sultanate

#### Base Pieces

| Piece                  | Tier            | Value | Movement                                                                                      |
| ---------------------- | --------------- | ----- | --------------------------------------------------------------------------------------------- |
| Wazir                  | District        | 15    | 1 orthogonal.                                                                                 |
| Jamal                  | District        | 20    | (1,3) leaper.                                                                                 |
| Talliya                | Town            | 30    | Diagonal slider. Must move at least 2.                                                        |
| Ziraafa                | Province        | 35    | 1 diagonal step, then 3+ squares straight.                                                    |
| Shahzadeh _(capstone)_ | Overseer reward | 100   | Second king-equivalent. Once per battle, swaps positions with the real king as a free action. |

#### Combinations (Zarqan)

| Recipe           | Result       | Value | Notes                                                                  |
| ---------------- | ------------ | ----- | ---------------------------------------------------------------------- |
| Makwanam + Wazir | Old King     | 30    | King movement. Counts as a normal piece.                               |
| Knight + Jamal   | Cataphract   | 40    | May move as Knight or Jamal each action.                               |
| Kyosha + Wazir   | Rook         | 50    | Orthogonal slider. The only way to obtain a Rook.                      |
| Ziraafa + Jamal  | War Elephant | 50    | Ziraafa movement. Deals damage to two adjacent enemies simultaneously. |

---

### Caelan Kingdom

#### Base Pieces

| Piece                | Tier            | Value | Movement                                                |
| -------------------- | --------------- | ----- | ------------------------------------------------------- |
| Pawn                 | District        | 10    | Forward 1 (or 2 from starting row). Attacks diagonally. |
| Knight               | District        | 30    | Knight L.                                               |
| Bishop               | Town            | 30    | Diagonal slider.                                        |
| Queen                | Province        | 90    | Rook and bishop combined.                               |
| Gryphon _(capstone)_ | Overseer reward | 100   | 1 diagonal step then any distance orthogonal.           |

#### Combinations (Caelan)

| Recipe                        | Result           | Value | Notes                                                                                                                  |
| ----------------------------- | ---------------- | ----- | ---------------------------------------------------------------------------------------------------------------------- |
| Rook + Bishop _(multi-step)_  | Chancellor       | 70    | Moves as Rook or leaps as Knight each action.                                                                          |
| Queen + Kyosha _(multi-step)_ | Sovereign Banner | 110   | Queen movement. Adjacent allies: step-movers gain 1 extra square, leapers extend longest leg by 1, sliders unaffected. |

---

### Strategic Notes

- Combined pieces adopt the result's kingdom. A Bishop (Caelan) + Wazir (Zarqan) produces a Harushima Promoted Bishop with Shogi's Reclaim innate.
- The Rook (Kyosha + Wazir) unlocks from run start. It gates the Dragon (Rook + Wazir) and the Chancellor (Rook + Bishop).
- Mono-kingdom builds reliably trigger 3-card combo climaxes. Diverse builds access multiple innates.

---

## 4. INNATE POWERS

Innates activate when a kingdom's Province map is entered. Before that, pieces from that kingdom have no innate.

| Kingdom   | Innate                                                                                                                                     | Cost / Limit                                                 |
| --------- | ------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------ |
| Longwei   | **Bulwark**: any Longwei piece with an orthogonally adjacent friendly piece takes 50% reduced damage from all attacks that turn            | Passive                                                      |
| Harushima | **Reclaim**: a flipped piece anywhere on the board can be immediately converted to your control on its current square                      | 30 cp and 1 action per reclaim                               |
| Kewarani  | **Double Time**: every Kewarani piece may move twice per move action                                                                       | Kewarani pieces cost 20% more (Medeq costs 12 instead of 10) |
| Zarqan    | **Royal Substitution**: any Zarqan piece swaps positions with the king as a free action                                                    | Once per battle                                              |
| Caelan    | **Conqueror's Reward**: when a Caelan piece contributes to a flip, its value permanently gains 50% (rounded up) for the rest of the battle | Slow snowball                                                |

Bulwark is distinct from the Pao screen mechanic. A Pao screen is a piece between the Pao and its target that enables the attack. Bulwark is damage reduction for adjacent Longwei pieces.

---

## 5. CARDS

### Format

- 3 cards drawn per turn. All 3 must be played or sold by end of turn.
- Cards never carry over between turns or battles.
- Cards do not cost actions.
- Unsold cards auto-sell at their sell value.

### Card Set

The **card set** is the collection of distinct cards that can be drawn each turn. Each turn, 3 cards are chosen at random from this set.

The set contains every card from tiers you have unlocked via map progress. It grows as you enter new maps. It cannot be added to by events or relics. It only grows through progression and can only shrink through the Offering node.

At run start: 3 Universal District cards + each kingdom's District cards (12 cards total).

### Tiers

| Tier     | Sell Range |
| -------- | ---------- |
| District | 15-20 cp   |
| Town     | 25-40 cp   |
| Province | 45-60 cp   |
| Country  | 70-100 cp  |

### Universal Cards

| Card         | Tier     | Play / Sell | Effect                                                                                       |
| ------------ | -------- | ----------- | -------------------------------------------------------------------------------------------- |
| Pawn Storm   | District | - / 15      | Buy up to 3 pawns this turn; the third is free.                                              |
| Revitalize   | District | 80 / 15     | Restore 50 to your meter.                                                                    |
| Hostage      | District | - / 20      | Passive. Next flip to your side: your meter gains 20 bonus.                                  |
| Last Stand   | Town     | - / 30      | This turn, your meter cannot trigger a flip. Damage still applies; excess is discarded.      |
| Sacrifice    | Town     | - / 25      | Remove one of your pieces. Gain its value x2 as meter.                                       |
| Reforge      | Town     | - / 30      | Passive. Next time one of your pieces flips, it returns to your side next turn at 80% value. |
| Mercy        | Province | - / 45      | Passive. Next flip on your side: you choose which piece flips.                               |
| Bloodletting | Province | - / 45      | This turn, all your attacks deal extra damage equal to 50% of your missing meter.            |
| Counter Coup | Province | - / 50      | This turn, all damage you take echoes back at 50% to enemy meter.                            |
| Spite        | Country  | - / 70      | Passive. Next time you lose a piece, deal its value x3 to enemy meter.                       |
| Chain Break  | Country  | 100 / 70    | Force-flip one enemy piece of your choice.                                                   |
| Hydra        | Country  | - / 80      | Passive. Next time one of your pieces flips, 2 friendly pawns spawn adjacent to your king.   |

### Longwei Cards

| Card          | Tier     | Play / Sell | Effect                                                                            |
| ------------- | -------- | ----------- | --------------------------------------------------------------------------------- |
| River Wade    | District | - / 15      | Target pawn permanently gains a sideways step this battle.                        |
| Charge        | District | - / 15      | Target slider may pass through one occupied square on its next move.              |
| Formation     | Town     | - / 30      | 3 of your pieces in a straight line each deal +50% damage this turn.              |
| Divination    | Town     | - / 25      | Reveal the enemy's intended moves and cards for next turn.                        |
| Cannon Volley | Province | 30 / 45     | Target Pao attacks every enemy on its row and column this turn, ignoring screens. |
| Palace Decree | Province | - / 50      | Enemy king restricted to a 3x3 zone for 2 turns.                                  |
| Mandate       | Country  | - / 75      | Remove one of your pieces. Reduce enemy meter by its value x3.                    |

### Harushima Cards

| Card          | Tier     | Play / Sell | Effect                                                                                                                  |
| ------------- | -------- | ----------- | ----------------------------------------------------------------------------------------------------------------------- |
| Ronin         | District | - / 15      | Passive. Next time the targeted piece flips, refund its full value to you.                                              |
| Resurrection  | Town     | - / 25      | Reclaim any flipped piece on the board to your control for free.                                                        |
| Gold Standard | Town     | - / 25      | Target piece moves like a Kinsho this turn only.                                                                        |
| Promotion     | Town     | - / 30      | Target piece permanently gains Ginsho movement for this battle.                                                         |
| Dual Drop     | Province | 30 / 45     | Reclaim up to 2 flipped pieces at 30 cp total. If fewer than 2 exist, place free Fuhyo pieces for each missing reclaim. |
| Force Drop    | Province | - / 50      | Place any unlocked piece of value up to 50 on any unoccupied square for free.                                           |
| Bushido       | Country  | - / 70      | Passive. When the targeted piece flips, deal its value x2 to enemy meter.                                               |

### Kewarani Cards

| Card          | Tier     | Play / Sell | Effect                                                                                                                            |
| ------------- | -------- | ----------- | --------------------------------------------------------------------------------------------------------------------------------- |
| Sultan's Gold | District | - / 20      | Gain 60 cp.                                                                                                                       |
| March         | District | - / 15      | Every friendly pawn moves forward 1 square, no action cost.                                                                       |
| Double Time   | Town     | - / 30      | Target piece (any kingdom) makes one additional move this turn. Kewarani pieces with the innate active gain a third move instead. |
| Salt Road     | Town     | - / 35      | Gain 10 cp at the start of every remaining turn this battle.                                                                      |
| Caravan       | Province | - / 45      | All pieces in a straight line move 1 square together, no individual action cost.                                                  |
| Doublestrike  | Province | - / 50      | Two of your pieces each take a full move action, counting as only 1 action total.                                                 |
| Hajj          | Country  | - / 70      | Target piece teleports to any unoccupied square on the board. Does not count as an attack.                                        |

### Zarqan Cards

| Card          | Tier     | Play / Sell | Effect                                                                                           |
| ------------- | -------- | ----------- | ------------------------------------------------------------------------------------------------ |
| Counsel       | District | - / 15      | Peek at next turn's 3 cards. Discard 1 from the upcoming hand.                                   |
| Pillage       | District | - / 20      | Gain 5 cp for each friendly piece currently on the board, including the king.                    |
| Royal Decoy   | Town     | - / 30      | Swap positions of any 2 of your pieces.                                                          |
| Bazaar        | Town     | - / 35      | Sell one of your pieces for 150% of its value.                                                   |
| Steppe Riders | Town     | - / 30      | All your Knights, Jamals, and Cataphracts move twice this turn.                                  |
| Ambition      | Province | - / 45      | Target piece copies any other unlocked piece's movement pattern this turn.                       |
| Citadel       | Province | - / 50      | Target piece becomes immobile and immune (cannot move, be attacked, or deal damage) for 2 turns. |
| Conquest      | Country  | 50 / 80     | Target piece permanently adopts any other unlocked piece's movement pattern for this battle.     |

### Caelan Cards

| Card           | Tier     | Play / Sell | Effect                                                                                       |
| -------------- | -------- | ----------- | -------------------------------------------------------------------------------------------- |
| Castling       | District | - / 15      | Your king and one of your Rooks swap positions.                                              |
| Queen's Gambit | District | - / 20      | Sacrifice a pawn. Draw 3 additional cards immediately; all must be played or sold this turn. |
| Vengeance      | Town     | - / 30      | Deal 2x damage to an enemy piece that moved adjacent to one of your pieces last turn.        |
| Queen's Decree | Town     | - / 50      | Your next attack this turn deals double damage.                                              |
| Cathedral      | Province | - / 45      | Passive. Enemy attacks crossing a friendly Bishop's diagonal deal half damage this turn.     |
| Coronation     | Province | - / 55      | Promote a pawn to Queen in place.                                                            |
| Crusade        | Country  | - / 70      | Target Knight makes 3 consecutive L-moves this turn, each attacking on resolution.           |
| Divine Right   | Country  | - / 75      | Your king attacks as a Queen this turn.                                                      |

### Combo Chains

Playing cards from the same kingdom in one turn:

- **2 cards**: +15 cp refund when the second same-kingdom card is played.
- **3 cards**: kingdom climax triggers.

| Kingdom   | 3-Card Climax                              |
| --------- | ------------------------------------------ |
| Longwei   | All your pieces gain Bulwark this turn     |
| Harushima | Reclaim one flipped piece for free         |
| Kewarani  | All pieces gain 1 extra move this turn     |
| Zarqan    | Free position-swap of any 4 of your pieces |
| Caelan    | All your pieces deal +50% damage this turn |

---

## 6. THE CAMPAIGN

### Structure

5 kingdoms, each with 3 maps:

| Map      | Board Scale | Notes                                               |
| -------- | ----------- | --------------------------------------------------- |
| Town     | 12x12       | Entry point. Street-level scale.                    |
| Province | 14x15       | Shows the town as one node among several.           |
| Country  | 16x18       | Shows provinces as regions. Ends with the Overseer. |

Each map has 5-7 battle nodes plus event nodes. Total run: 25-35 battles, 5 Overseers, Vorath.

### Unlocks

| Moment                | Unlock                                                                                          |
| --------------------- | ----------------------------------------------------------------------------------------------- |
| Run start             | All District-tier pieces (2 per kingdom, 10 total) and all District-tier cards for all kingdoms |
| Entering Town map     | That kingdom's Town piece + Town cards                                                          |
| Entering Province map | That kingdom's Province piece + Province cards + innate activates                               |
| Entering Country map  | That kingdom's Country-tier cards                                                               |
| Defeating Overseer    | That kingdom's capstone piece + figurehead unchained                                            |

Universal cards of a given tier unlock the first time any kingdom enters that tier.

### Battle Independence

Every battle starts identically: king only, 20 cp. The only things that change across the run are which pieces and cards are available, determined by map progress.

### Switching Tracks

Tracks can be freely switched. A map must be fully cleared before advancing to the next in that kingdom.

### Vorath's Pressure

Every cleared map adds +1 free piece to enemy starting armies in all other kingdoms.

### Map Nodes

| Node     | Effect                                                                                     |
| -------- | ------------------------------------------------------------------------------------------ |
| Battle   | Standard combat                                                                            |
| Elite    | Harder combat (enemy +1 piece). Reward: choose 1 relic from 2.                             |
| Archive  | Reveal 1 combination recipe.                                                               |
| Offering | Permanently remove 1 card from your card set. It cannot be drawn for the rest of this run. |
| Event    | Text event with a trade-off choice. Resolved immediately on the map screen.                |
| Overseer | Country map final node.                                                                    |

### Node Layouts

Fixed nodes per map type:

| Map      | Battles | Elite | Offering | Events | Overseer |
| -------- | ------- | ----- | -------- | ------ | -------- |
| Town     | 5       | 1     | 1        | 2      | —        |
| Province | 5       | 1     | 1        | 2      | —        |
| Country  | 4       | 1     | 1        | 2      | 1        |

Archive nodes are not fixed and may be absent from any given map. Each kingdom's total Archives across all three maps equals its number of combination recipes, placed at the tier where each recipe first becomes craftable. See individual kingdom layouts for exact placement.

---

**LONGWEI EMPIRE**

Town Map:

- The Copper Gate (Battle)
- The Western Market (Battle)
- The River Quarter (Battle)
- The Silk Exchange (Battle)
- The Second Wall (Battle)
- The Prefecture Garrison (Elite)
- The Hall of Records (Archive: _Sang_ (Bing + Saba))
- The River Shrine (Offering)
- The Governor's Courtyard (Event: Dragon Court Tribute)
- The Janggi Elder's School (Event: The Janggi Elder)

Province Map:

- The Dragon Bridge (Battle)
- The Eastern Checkpoint (Battle)
- The Iron Mines (Battle)
- The Great Canal (Battle)
- The Silk Road Station (Battle)
- The Border Fortress (Elite)
- The Imperial Library (Archive: _Hwacha_ (Pao + Bishop))
- The Battle Codex (Archive: _Northern Cavalry_ (Färäs + Bing))
- The War Memorial (Offering)
- The Cannon Works (Event: Cannon Salute)
- The River Crossing Post (Event: The Defector)

Country Map:

- The Palace Approach (Battle)
- The Outer City (Battle)
- The Dragon Gate (Battle)
- The Throne Antechamber (Battle)
- The Palace Guard (Elite)
- The Dragon Altar (Offering)
- The Scholar's Garden (Event: The Scholar's Offer)
- The Imperial Court (Event: Vorath's Decree)
- The Hall of Ten Thousand Victories (Overseer: Iron Strategist)

---

**HARUSHIMA SHOGUNATE**

Town Map:

- The Harbor District (Battle)
- The Fish Market (Battle)
- The Merchant's Ward (Battle)
- The Eastern Docks (Battle)
- The Bridge Quarter (Battle)
- The Coastal Garrison (Elite)
- The Dojo of Records (Archive: _Honorable Horse_ (Kyosha + Knight))
- The Sailor's Almanac (Archive: _Promoted Bishop_ (Bishop + Wazir))
- The Sea Shrine (Offering)
- The Wandering Blade Inn (Event: The Ronin)
- The Dockmaster's Office (Event: The Spy Network)

Province Map:

- The Rice Fields (Battle)
- The Mountain Pass (Battle)
- The Castle Outskirts (Battle)
- The Inland Road (Battle)
- The Valley Fort (Battle)
- The Mountain Garrison (Elite)
- The Castle Archive (Archive: _Daimyo_ (Kinsho + Ginsho))
- The Field Commander's Notes (Archive: _Dragon_ (Rook + Wazir))
- The Ancestor Shrine (Offering)
- The Merchant Ward (Event: The Burning Port)
- The Smithy Quarter (Event: The Forge Master)

Country Map:

- The Castle Town (Battle)
- The Inner Gate (Battle)
- The Shogun's Garden (Battle)
- The Keep Approach (Battle)
- The Castle Guard (Elite)
- The Offering Stone (Offering)
- The Lance Regiment's Post (Event: The Veteran Lance)
- The Emperor's Courier (Event: The Deserter)
- The Hall of Eternal Recursion (Overseer: Eternal Recursion)

---

**KEWARANI NEGUSATE**

Town Map:

- The Market of Addis (Battle)
- The Well Road (Battle)
- The Spice Quarter (Battle)
- The Caravan Gate (Battle)
- The Outpost Road (Battle)
- The Market Garrison (Elite)
- The Salt Road Archive (Archive: _Medeq Squad_ (Pawn + Pawn))
- The Road Shrine (Offering)
- The Salt Road (Event: Salt Road Merchant)
- The Camel Traders' Camp (Event: The Camel Caravan)

Province Map:

- The Highland Road (Battle)
- The Ancient Ford (Battle)
- The Thorn Pass (Battle)
- The Lowland Camp (Battle)
- The Plateau Fort (Battle)
- The Highland Garrison (Elite)
- The Rain Shrine (Offering)
- The Court House (Event: Mansa's Court)
- The Festival Grounds (Event: Feast of Yod Abeba)

Country Map:

- The Palace Road (Battle)
- The City Gates (Battle)
- The Inner Court (Battle)
- The Throne Hall Approach (Battle)
- The Royal Guard (Elite)
- The Negus Guard Archive (Archive: _Sultan's Levy_ (Negus Guard + Pawn))
- The Sacred Spring (Offering)
- The Prison Tower (Event: The Stolen Guard)
- The Road's End (Event: The Wandering Piece)
- The Throne of the Negesta (Overseer: Caravan of Conquest)

---

**ZARQAN SULTANATE**

Town Map:

- The Oasis Market (Battle)
- The Caravanserai (Battle)
- The Date Merchants (Battle)
- The Copper Bazaar (Battle)
- The Southern Gate (Battle)
- The Garrison of the Dunes (Elite)
- The Wazir's Library (Archive: _Cataphract_ (Knight + Jamal))
- The Rukh Records (Archive: _Rook_ (Kyosha + Wazir))
- The Desert Shrine (Offering)
- The Desert Arena (Event: The Warlord's Challenge)
- The Grand Bazaar (Event: Bazaar of Samarkand)

Province Map:

- The Steppe Road (Battle)
- The Dune Fortress (Battle)
- The Salt Flats (Battle)
- The Oasis Outpost (Battle)
- The Cliffside Fort (Battle)
- The Steppe Garrison (Elite)
- The Traveller's Archive (Archive: _Old King_ (Makwanam + Wazir))
- The War Records (Archive: _War Elephant_ (Ziraafa + Jamal))
- The Wind Shrine (Offering)
- The Mirage Outpost (Event: The Mirage)
- The Spy's Tent (Event: The Spy's Report)

Country Map:

- The Sultan's Road (Battle)
- The Citadel Gate (Battle)
- The Inner Fortress (Battle)
- The Throne Hall (Battle)
- The Sultan's Guard (Elite)
- The Flame Altar (Offering)
- The Crossing Point (Event: The Desert Crossing)
- The Ancient Ruin (Event: The Archive)
- The Hall of Many Faces (Overseer: The Many-Faced King)

---

**CAELAN KINGDOM**

Town Map:

- The Market Square (Battle)
- The Guild District (Battle)
- The East Road (Battle)
- The Mill Quarter (Battle)
- The Town Gate (Battle)
- The Town Garrison (Elite)
- The Scribe's Hall (Archive: _Chancellor_ (Rook + Bishop))
- The Town Chapel (Offering)
- The Tournament Grounds (Event: The Tournament)
- The Cathedral (Event: The Church's Blessing)

Province Map:

- The King's Road (Battle)
- The Manor Grounds (Battle)
- The Border Fort (Battle)
- The Old Bridge (Battle)
- The Forest Road (Battle)
- The Border Garrison (Elite)
- The Royal Archive (Archive: _Sovereign Banner_ (Queen + Kyosha))
- The Wayside Shrine (Offering)
- The Engineer's Workshop (Event: The Siege Engineer)
- The Pretender's Manor (Event: The Pretender)

Country Map:

- The Capital Gates (Battle)
- The King's Quarter (Battle)
- The Palace Ward (Battle)
- The Throne Approach (Battle)
- The Royal Guard (Elite)
- The Royal Altar (Offering)
- The Royal Chamber (Event: The Royal Decree)
- The Old Road (Event: Rest)
- The Crowned Throne (Overseer: The Crowned Heretic)

### Chain Penalty

Each figurehead has 3 chain slots. Losing battles in that kingdom fills them.

| Chain  | Effect                                           |
| ------ | ------------------------------------------------ |
| Bronze | Battles in this region start with -15 cp         |
| Silver | Enemy gets +1 free starting piece in this region |
| Gold   | Figurehead Subjugated: track locks               |

Winning a battle in a chained region removes the most recent chain.

### Liberation Trial

When a figurehead becomes Subjugated, a Liberation Trial node appears in a random other kingdom's track. Regular battles in other kingdoms are unaffected. The Liberation Trial: enemy starts with +2 pieces; only the subjugated kingdom's cards are drawable. Winning breaks the Gold chain. Losing makes the node respawn after 3 battles.

### Global Vorath Counter

| Threshold      | Effect                                               |
| -------------- | ---------------------------------------------------- |
| Every 2 losses | Enemy meters gain +50 capacity everywhere            |
| Every 4 losses | A random combination recipe is forbidden for the run |

Run ends when all 5 figureheads are simultaneously Subjugated, or Vorath is defeated.

---

## 7. THE OVERSEERS AND VORATH

Overseers are bespoke fights at the end of each Country map. Defeating one unlocks the capstone piece and unchains the figurehead.

### Iron Strategist (Longwei Overseer)

- **Mechanic**: Damage is split equally across all enemy pieces rather than hitting the meter directly. Sustained multi-piece pressure works; burst attacks are diluted.
- **Army**: 3 Pao in triangular palace formation. Each fires once per battle as a free action: attacks every enemy piece in a chosen 3x3 zone on its row or column, dealing 50 damage per piece hit.
- **Twist**: To collapse the king's meter, the player must first flip the boss's 2 Generals (value 10 each). Generals only flip if directly attacked by an adjacent piece on 3 consecutive turns.
- **Reward**: Liubo Diviner + 2 Longwei cards permanently added to the card set.

### Eternal Recursion (Harushima Overseer)

- **Mechanic**: Every flipped enemy piece returns to the boss's side at the end of the next turn. Pieces cannot be permanently removed.
- **Army**: 4 Kinsho in a fortress around the king.
- **Twist**: The only win is flipping the king. Chain Break becomes essential.
- **Reward**: Shishi + 2 Harushima cards permanently added to the card set.

### Caravan of Conquest (Kewarani Overseer)

- **Mechanic**: Boss gains +1 free piece every 2 turns, dropped at a random edge.
- **Board**: A Salt Road diagonal crosses the board. Movement is doubled on it for both sides.
- **Twist**: Boss pieces have Double Time active. The player must outpace the scaling.
- **Reward**: Negus Guard + 2 Kewarani cards permanently added to the card set.

### Many-Faced King (Zarqan Overseer)

- **Mechanic**: The boss has 3 Princes on the board. Any can be the "real king." When the king would flip, the boss reveals one Prince was it and that one flips. The others inherit king status until all 3 are flipped.
- **Special**: The boss can redirect meter overflow to a chosen Prince's meter share when the meter would empty.
- **Twist**: All 3 Princes must be flipped. Spread damage; do not focus one.
- **Reward**: Shahzadeh + 2 Zarqan cards permanently added to the card set.

### Crowned Heretic (Caelan Overseer)

- **Mechanic**: Boss starts with a full Caelan army: 8 Pawns, 2 Knights, 2 Bishops, 2 Rooks, 1 Queen, 1 King.
- **Special**: Every piece the boss loses becomes a Ghost, an off-board entity dealing 10 damage to your meter at the start of each of your turns. Ghosts accumulate.
- **Twist**: Boss meter starts at ~520. Sacrifice plays are essential.
- **Reward**: Gryphon + 2 Caelan cards permanently added to the card set.

---

### Vorath

**Board**: 20x20, 5 quadrants (one per kingdom's aesthetic).
**Army**: 5 Minor Kings (one per quadrant, value 30 each) + 1 Grand King (value 10) + mixed pieces from all 5 kingdoms.
**Player setup**: One free starting piece per unchained figurehead.

**Mechanic**: The Grand King's meter resets every turn unless the player attacked at least one Minor King in each of the 5 quadrants that turn. Sustained pressure across all 5 quadrants is required.

**Win condition**: Reduce the Grand King's meter to exactly 0 in a turn where all 5 quadrants were attacked. The Grand King flips to your side. On the following turn, the Grand King attacks all 5 Minor Kings, flipping them. The board is liberated.

Overshooting (going below 0) triggers a random Minor King flip and resets the Grand King's meter.

---

## 8. ENGAGEMENT SYSTEMS

### Battle Modifiers

One modifier is drawn per battle and revealed before entry.

#### Economy

| Modifier          | Effect                          |
| ----------------- | ------------------------------- |
| Lean Times        | Starting cp -20                 |
| Windfall          | Both sides start with +30 cp    |
| Open Market       | All pieces cost 50% this battle |
| Devalued Currency | All card sell values halved     |
| Tax Collector     | +5 cp on top of each card sold  |

#### Meter

| Modifier     | Effect                                                                            |
| ------------ | --------------------------------------------------------------------------------- |
| Glass Cannon | Both meters start at 50% of calculated value                                      |
| Bloodbath    | Each flip triggers 2 pieces instead of 1                                          |
| Iron Will    | Meters floor at 10 until a single attack would push below 0, then empty instantly |
| Overflow     | Gaining a flipped piece adds 30 bonus to your meter                               |
| Mirror       | Damage you take also hits your own meter at 25%                                   |

#### Cards

| Modifier       | Effect                                                          |
| -------------- | --------------------------------------------------------------- |
| Rich Hand      | Draw 4 cards per turn                                           |
| Sparse Hand    | Draw 2 cards per turn                                           |
| Kingdom Purity | Only cards matching the region being fought in are drawn        |
| Lucky Strike   | First card drawn each turn is always the highest-tier available |

#### Board

| Modifier       | Effect                                       |
| -------------- | -------------------------------------------- |
| Fog of War     | Enemy piece values hidden until they attack  |
| Dense Terrain  | 20% of squares impassable, revealed at start |
| Extended Front | Board +2 columns wide                        |
| Compressed     | Board -2 columns wide                        |

---

### Figurehead Starting Powers

All five starting powers are always active from run start. Since all five figureheads are simultaneously allied against Vorath, every power below applies for the entire run.

| Figurehead       | Power                                                                                                    |
| ---------------- | -------------------------------------------------------------------------------------------------------- |
| Empress Mingzhu  | One free Pao on the board at battle start in Longwei territory. Outside Longwei: +20 cp.                 |
| Shogun Tomohito  | Reclaim costs 10 cp instead of 30 for the entire run.                                                    |
| Negesta Selassie | Kewarani Double Time active from run start, before Province is cleared. (+20% piece cost still applies.) |
| Sultan Timur     | Royal Substitution usable twice per battle.                                                              |
| Queen Isabella   | First turn of every battle includes one guaranteed Province-tier Caelan card.                            |

---

### Relics

Found at **Elite nodes** and **certain narrative events** (choose 1 from 2 at each opportunity). Affects the entire run.

There are 26 unique relics. The offer pool never includes a relic already held — every offer is a new acquisition. A full run contains exactly 15 Elite node opportunities and up to 11 relic-eligible event choices, totalling 26 with perfect play — one for each relic.

#### Economy

| Relic             | Effect                                                   |
| ----------------- | -------------------------------------------------------- |
| Merchant's Ledger | Card sell values +5 cp                                   |
| Minted Coin       | +5 cp at start of every turn                             |
| Tax Stamp         | +10 cp whenever you play a card with a play cost         |
| Bulk Discount     | When buying 3+ pieces in one turn, the cheapest is free  |
| War Chest         | Unspent cp at end of turn carries over, up to 30         |
| Trade Routes      | The foreign kingdom markup is removed for the entire run |

#### Meter

| Relic           | Effect                                                                               |
| --------------- | ------------------------------------------------------------------------------------ |
| Soul Shard      | Gaining a flipped piece adds 30 bonus to your meter                                  |
| Veteran's Bond  | Pieces with value 50+ contribute 20 extra to your meter                              |
| Dead Man's Pact | First time meter would empty in a battle, reset to 20 instead (once per battle)      |
| Iron King       | Your king contributes 20 to your meter instead of 10                                 |
| Bloodthirst     | At the start of each turn where your meter exceeds the enemy's, gain 5 to your meter |
| Last Breath     | When a friendly piece flips, deal damage equal to its value to the enemy meter       |

#### Cards

| Relic              | Effect                                                                                             |
| ------------------ | -------------------------------------------------------------------------------------------------- |
| Tactician's Scroll | Draw 4 cards per turn                                                                              |
| Librarian's Notes  | Once per turn, before drawing, see the top card and choose to skip it                              |
| Country Seal       | Selling a Country-tier card gives +20 extra cp                                                     |
| Deep Hand          | Once per battle, draw 2 extra cards on a turn of your choice. All must be played or sold that turn |
| Gilded Archive     | District-tier cards sell for +10 cp                                                                |

#### Combinations

| Relic               | Effect                                                                     |
| ------------------- | -------------------------------------------------------------------------- |
| Alchemist's Kit     | Combinations cost 0 actions                                                |
| Master's Notes      | Archive nodes reveal 2 recipes instead of 1                                |
| Philosopher's Stone | Once per run, a combined piece permanently gains +20 value for that battle |
| Inherited Power     | All combined pieces gain +5 value above their normal result value          |

#### Board

| Relic            | Effect                                                    |
| ---------------- | --------------------------------------------------------- |
| Eagle Eye        | Enemy piece values always visible                         |
| Surveyor's Map   | One random battle node's modifier pre-revealed per map    |
| Forward Command  | Your pieces deal +5 damage when occupying enemy territory |
| Fortified Line   | Pieces that did not move this turn deal +5 damage         |
| Warlord's Banner | Pieces adjacent to your king deal +5 damage               |

---

### Narrative Events

Event nodes appear as full-screen panels on the campaign map. The situation is described in 2–3 sentences followed by two choices. Select one; the effect resolves immediately and the map resumes. No combat, no animations.

Valid rewards: relics, recipe reveals, piece bonuses (run-wide), card set removal, Vorath counter reduction, chain removal, map reveals.

#### Longwei

| Event                | A                                                                             | B                                                                                  |
| -------------------- | ----------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| The Scholar's Offer  | Reveal 1 recipe                                                               | Choose 1 relic from 2                                                              |
| Dragon Court Tribute | Remove 1 card from card set; all Longwei piece values permanently +5 this run | Refuse. Enemy pieces in all Longwei battles gain +5 value for the rest of the run. |
| The Defector         | Reveal the next battle's modifier before committing to the node               | Reveal 1 recipe                                                                    |
| The Janggi Elder     | One Longwei piece type gains +5 value this run                                | Reveal 1 recipe                                                                    |
| Cannon Salute        | All your Pao permanently gain +5 value this run                               | Reduce Global Vorath Counter by 2                                                  |

#### Harushima

| Event             | A                                                                                   | B                                 |
| ----------------- | ----------------------------------------------------------------------------------- | --------------------------------- |
| The Ronin         | Choose 1 relic from 2                                                               | Reveal 1 recipe                   |
| The Spy Network   | Reveal the next 3 map nodes                                                         | Reduce Global Vorath Counter by 2 |
| The Burning Port  | Remove 2 Harushima cards from card set; all Harushima pieces cost 15% less this run | Refuse                            |
| The Forge Master  | All of one piece type costs 20% less this run                                       | All Ginsho gain +5 value this run |
| The Veteran Lance | All Kyosha gain +5 value this run                                                   | Reveal 1 recipe                   |

#### Kewarani

| Event              | A                                                                  | B                                                                            |
| ------------------ | ------------------------------------------------------------------ | ---------------------------------------------------------------------------- |
| Mansa's Court      | Remove 1 card from card set; reveal 1 recipe                       | Refuse. Enemy armies in Kewarani gain +1 free piece for the rest of the run. |
| Salt Road Merchant | Choose 1 relic from 2                                              | All Medeq gain +3 value this run                                             |
| The Stolen Guard   | Elite battle. Win: remove 1 Bronze chain from Kewarani figurehead. | Reveal 1 recipe                                                              |
| The Camel Caravan  | Kewarani pieces cost 15% less this run                             | Reduce Global Vorath Counter by 2                                            |
| Feast of Yod Abeba | Skip the next battle node                                          | Choose 1 relic from 2                                                        |

#### Zarqan

| Event                   | A                                                                         | B                                         |
| ----------------------- | ------------------------------------------------------------------------- | ----------------------------------------- |
| The Warlord's Challenge | Elite battle. Win: choose 1 relic from 2.                                 | Reduce Global Vorath Counter by 2         |
| Bazaar of Samarkand     | Remove 2 cards from card set; all Zarqan pieces cost 15% less this run    | Reveal 1 recipe                           |
| The Mirage              | Elite (all enemy pieces have Citadel active). Win: choose 1 relic from 2. | Reveal 1 recipe                           |
| The Spy's Report        | Reduce Global Vorath Counter by 2                                         | Reveal 1 recipe                           |
| The Desert Crossing     | Reveal 3 map nodes ahead                                                  | Remove 1 Bronze chain from any figurehead |

#### Caelan

| Event                 | A                                                                                       | B                                  |
| --------------------- | --------------------------------------------------------------------------------------- | ---------------------------------- |
| The Tournament        | Elite (fixed mid-tier Caelan enemy). Win: choose 1 relic from 2.                        | Reveal 1 recipe                    |
| The Church's Blessing | Caelan pieces cost 10% less this run                                                    | All Bishops gain +5 value this run |
| The Siege Engineer    | Reveal all Longwei combination recipes                                                  | Choose 1 relic from 2              |
| The Pretender         | Elite (enemy mirrors your piece setup). Win: remove 1 Silver chain from any figurehead. | Reduce Global Vorath Counter by 3  |
| The Royal Decree      | Remove 1 card from card set; all Caelan pieces gain +3 value this run                   | Choose 1 relic from 2              |

#### Universal

| Event               | A                                                              | B                                 |
| ------------------- | -------------------------------------------------------------- | --------------------------------- |
| The Wandering Piece | Reveal all recipes for one kingdom of your choice              | Choose 1 relic from 2             |
| Vorath's Decree     | Remove 1 card from card set; reduce Global Vorath Counter by 3 | Refuse. Global Counter +2.        |
| The Deserter        | Remove 1 card from card set; all pawns gain +3 value this run  | Reduce Global Vorath Counter by 2 |
| The Archive         | Reveal 2 combination recipes                                   | Choose 1 relic from 2             |
| Rest                | No reward. Lore text only.                                     |                                   |

---

### Board Traits

Active on 50% of battles in their kingdom. Revealed before entering the node.

#### Longwei

| Trait          | Effect                                                                                                                                                     |
| -------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| River Crossing | A row bisects the board. Xiang cannot cross it. Bing gains sideways step immediately upon crossing. Pao attacks freely through any piece on the river row. |
| The Palace     | A 3x3 zone near the enemy king. Enemy king cannot voluntarily leave it.                                                                                    |

#### Harushima

| Trait        | Effect                                                               |
| ------------ | -------------------------------------------------------------------- |
| Fog Coast    | 3 farthest columns show enemy piece values as "?" until they attack. |
| Island Chain | 3-4 impassable column gaps force pieces to route around.             |

#### Kewarani

| Trait            | Effect                                                                                                               |
| ---------------- | -------------------------------------------------------------------------------------------------------------------- |
| Trade Route      | A diagonal path crosses the board. Any piece on it gains +1 movement per action.                                     |
| Contested Market | Start of each player turn: 1 random District piece appears at the board edge. The first piece to reach it claims it. |

#### Zarqan

| Trait     | Effect                                                                            |
| --------- | --------------------------------------------------------------------------------- |
| Sandstorm | On even turns, all sliders move at most 3 squares.                                |
| Mirage    | 5 random squares appear occupied but are not. Movement attempts waste the action. |

#### Caelan

| Trait          | Effect                                                                   |
| -------------- | ------------------------------------------------------------------------ |
| Castle Corners | The four 2x2 corner zones grant Bulwark to any piece standing in them.   |
| Siege Trench   | A row of impassable squares crosses near the center with a 2-square gap. |

---

### AI Archetypes

| Kingdom   | Primary                                                                                                                | Fallback (when behind)                      |
| --------- | ---------------------------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| Longwei   | Siege Engineer: sets up Pao screens; buys Ma early; sacrifices meter to establish 3-piece lines                        | Buys cheapest pieces to rebuild meter       |
| Harushima | Reclaimer: lets low-value pieces flip then reclaims them; hoards cp early; plays defensively                           | Switches to aggressive high-value purchases |
| Kewarani  | The Tide: floods Medeq and Makwanam; never buys above 30 cp; moves all 3 actions every turn; sells most cards          | Mass-attacks highest-value player pieces    |
| Zarqan    | Trickster: saves Royal Substitution for committed attacks; repositions constantly; plays cards over buying             | Switches to high-value purchases            |
| Caelan    | The Hammer: hoards cp for 1 large turn with 2 high-value buys; focuses all attacks on the player's most valuable piece | Floods pawns as defensive stall             |

---

### Meta-Progression

#### Combination Codex

Recipes discovered in any run are permanently unlocked in all future runs. A Codex screen tracks discovered and undiscovered recipes between runs. When all recipes are known, Archive nodes reveal a recipe from a kingdom whose Country map has not yet been entered.

#### Figurehead Mastery

Mastery is tracked independently per kingdom. It advances when **both** conditions are met in the same run:

- **That kingdom's figurehead was never chained** — not a single chain penalty applied across Town, Province, and Country. A single loss anywhere in that track disqualifies it for the run with no recovery.
- **Vorath was defeated** — mastery is not awarded for partial runs. The full run must be won.

A single loss in a kingdom locks out its mastery for that run regardless of what follows. Players naturally master their most comfortable kingdoms first while unfamiliar ones lag behind. Rewards apply to all future runs from the run after they are earned.

| Level | Reward                                                                     |
| ----- | -------------------------------------------------------------------------- |
| 1     | That kingdom's innate activates when entering Town map instead of Province |
| 2     | A unique figurehead card is permanently added to that kingdom's card set   |
| 3     | That kingdom's starting power is upgraded. Cosmetic title change.          |

Mastery-2 unique cards:

- **Mingzhu's Seal** (Longwei): Target 1 enemy piece. It cannot move for 3 turns and permanently loses 10 value this battle. No play cost.
- **Tomohito's Patience** (Harushima): Reclaim any number of flipped pieces this turn at 15 cp each.
- **Selassie's March** (Kewarani): Kewarani pieces move 3 times per action this turn.
- **Timur's Conquest** (Zarqan): Royal Substitution triggers automatically when meter drops below 20.
- **Isabella's Coronation** (Caelan): All friendly Pawns promote to Queens simultaneously.

Mastery-3 starting power upgrades:

- **Mingzhu** (Longwei): The free starting Pao applies in all kingdoms' territory, not just Longwei. The out-of-kingdom +20 cp fallback is replaced by the universal Pao.
- **Tomohito** (Harushima): Reclaim costs 5 cp instead of 10 for the entire run.
- **Selassie** (Kewarani): The Double Time +20% piece cost penalty is waived.
- **Timur** (Zarqan): Royal Substitution is usable three times per battle instead of twice.
- **Isabella** (Caelan): The guaranteed first-turn card upgrades from Province-tier to Country-tier.

#### Vorath's Memory

After defeating Vorath, future runs track which piece type you used most. Vorath adds 2 pieces to his starting army specifically designed to counter it.

---

### Kingdom Synergies

Defeating a kingdom's Overseer adds a passive bonus when fighting in its lore-adjacent kingdom. Active for the rest of the run.

| Cleared   | Bonus in                                            |
| --------- | --------------------------------------------------- |
| Longwei   | Harushima: Pao attacks deal +10 damage              |
| Harushima | Caelan: Caelan pieces draw 1 bonus card when played |
| Kewarani  | Zarqan: Kewarani pieces cost 10 cp less             |
| Zarqan    | Longwei: Ziraafa and Talliya gain +5 value          |
| Caelan    | Kewarani: Sultan's Gold yields +10 extra cp         |

### Prestige (New Game+)

Unlocked after defeating Vorath once.

| Prestige | Change                                                                      |
| -------- | --------------------------------------------------------------------------- |
| 1        | Vorath's Pressure starts at +2 pieces instead of 0                          |
| 2        | Bronze penalty becomes -25 cp. All Overseers start with an active modifier. |
| 3        | All enemy kingdoms' innates are active from battle 1 of each map            |

### Challenge Runs

| Challenge            | Constraint                                                                                                                                               |
| -------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Daily Conquest       | A new seed generates each day. Every player that day gets the same map layout, modifiers, and event outcomes.                                            |
| Solo Vanguard        | Only 1 piece allowed on the board at any time                                                                                                            |
| Pacifist Doctrine    | Cannot buy any piece above 20 cp                                                                                                                         |
| Blind Draft          | Card names and effects are hidden until played or sold. You see kingdom and tier only. Commit to play or sell without knowing which specific card it is. |
| The Traitor's Gambit | The enemy starts with 3 pieces already placed in your half of the board                                                                                  |
| Clockwork            | 30 seconds per turn                                                                                                                                      |

---

## 9. DEVELOPMENT ROADMAP

### Phase 1: Core Loop

Validate the meter mechanic and card-consumption feel before adding content.

- 1 kingdom: Harushima (Reclaim innate is most mechanically unique)
- 1 Town map, 10-turn battles
- Full core mechanics: meter, end-of-turn damage, flips, placement, combination, card consumption
- 8 pieces, 10 cards, 1 board layout
- 1 AI archetype
- **Goal**: 10 playtests. Does the meter feel dramatic? Do card decisions feel real?

### Phase 2: Single Kingdom Vertical Slice

- 1 kingdom, all 3 maps + Overseer
- Full piece roster and combination tree for that kingdom + all Universal cards
- 3 AI archetypes
- Full campaign map with all node types
- **Goal**: One complete figurehead arc, start to Overseer defeat.

### Phase 3: Full Content

- All 5 kingdoms and tracks
- All Overseers + Vorath
- Full piece roster, all combinations, all cards
- Chain penalty system, Liberation Trial nodes
- **Goal**: Playable start to finish.

### Phase 4: Engagement Systems

All systems in Section 8.

### Technical Targets

- Meter UI must show projected damage before resolve. Players should never be surprised by the numbers.
- AI stores intended moves each turn. Divination card reads this store.
- Build an irregular-board generator early. Hardcoded boards create a recognizable loop after 10 hours.
- All movement patterns are data-driven, not hardcoded. The combination system mixes patterns at runtime.

### Known Failure Modes

| Problem              | Mitigation                                                                              |
| -------------------- | --------------------------------------------------------------------------------------- |
| Board overcrowding   | Combinations reduce piece count. Hajj, Royal Decoy, and Citadel act as pressure valves. |
| Meter math fatigue   | Show projected resolve values in the UI before confirming the turn.                     |
| Card RNG frustration | The Offering node lets players remove cards they never want to draw.                    |
| AI sameness          | Each kingdom's AI needs a distinct decision tree, not one generic script.               |
