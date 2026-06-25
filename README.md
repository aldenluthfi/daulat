<picture>
  <source media="(prefers-color-scheme: light)" srcset="/.github/meta/dark.png">
  <source media="(prefers-color-scheme: dark)" srcset="/.github/meta/light.png">
  <img alt="Daulat">
</picture>

A roguelike that fuses chess variants with deckbuilding. Climb three maps in each of five kingdoms — each rooted in a real chess tradition — to unchain its figurehead, then face Vorath the Unbroken on a grand arena board.

## Concept

| Figurehead       | Kingdom             | Tradition                 |
| ---------------- | ------------------- | ------------------------- |
| Empress Mingzhu  | Longwei Empire      | Xiangqi, Janggi           |
| Shogun Tomohito  | Harushima Shogunate | Shogi, Chu Shogi          |
| Negesta Selassie | Kewarani Negusate   | Senterej                  |
| Sultan Timur     | Zarqan Sultanate    | Tamerlane Chess, Shatranj |
| Queen Isabella   | Caelan Kingdom      | Chess, Grant Acredex      |

Every battle starts with just your king on an irregular checkered board. You buy pieces, combine them across kingdom lines, and play a fresh hand of 3 cards each turn. Pieces cannot capture by movement — instead they deal damage to an enemy **meter** equal to their value. When the meter empties, a random piece flips sides. Flip the enemy king to win.

## Key Systems

- **The Meter** — a single number tracks each side's army strength. Empty it to force flips. It can overflow up to 200%.
- **Piece Value** — one number is simultaneously its cost, its meter contribution, and the damage it deals.
- **Combinations** — fuse any two friendly pieces into a stronger one. Mixed-kingdom combos produce hybrid movement patterns.
- **Cards** — draw 3 per turn; all must be played or sold. Same-kingdom combos trigger climaxes.
- **Innates** — each kingdom has a signature ability (Bulwark, Reclaim, Double Time, Royal Substitution, Conqueror's Reward) that activates at the Province map.
- **Campaign** — 25–35 battles across 5 kingdoms, 5 Overseers, then Vorath on a 20×20 board. Losses chain figureheads; chained kingdoms suffer escalating penalties.

## Design Document

The full Game Design Document lives at [`res/GDD.md`](res/GDD.md). It covers all pieces, recipes, cards, overseers, relics, events, board traits, AI archetypes, meta-progression, and the development roadmap.

## Building

The project builds with `make`:

```sh
make debug      # symbols, no optimization, assertions on
make release    # -O3, stripped
make clean
```

Output binary: `bin/daulat`.

## Status

Early development. See [Development Roadmap](res/GDD.md#9-development-roadmap) in the GDD for the planned phases — currently targeting Phase 1 (single-kingdom vertical slice to validate the meter and card-consumption feel).
