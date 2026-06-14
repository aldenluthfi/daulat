//! data_relics.c
//!
//! Static templates for the 26 relics that can be acquired across a
//! run. Relics are organised into Economy, Meter, Cards, Combinations
//! and Board categories per the GDD; each entry holds the effect that
//! activates when the relic is added to RunState.
//!
//! The RELIC() helper macro keeps each entry one-line per effect and
//! computes effect_count from the variadic argument list at compile
//! time, so adding effects never requires touching a separate count.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              RELICS
\*--------------------------------------------------------------------------*/

/// RELIC
///
/// Compact macro for declaring a RelicTemplate. Wraps the variadic
/// effect list into both the .effects initializer and an
/// auto-computed .effect_count.
///
/// Params:
/// - id_   -> RelicId enumerator
/// - name_ -> printable string literal
/// - ...   -> one or more Effect initializers
///
#define RELIC(id_, name_, ...)                                                 \
    {                                                                          \
        .id = id_,                                                             \
        .name = name_,                                                         \
        .effects = {__VA_ARGS__},                                              \
        .effect_count = sizeof((Effect[]){__VA_ARGS__}) / sizeof(Effect),      \
    }

/// RELICS
///
/// Dense relic table. Iteration order matches the GDD category
/// breakdown (Economy, Meter, Cards, Combinations, Board) so future
/// rebalancing edits stay easy to read.
///
const RelicTemplate RELICS[] = {
    RELIC(
        RELIC_MERCHANTS_LEDGER, "Merchant's Ledger",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TREASURY_KEY, "Treasury Key",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_GOLDEN_LAMP, "Golden Lamp",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TRIBUTE_CHEST, "Tribute Chest",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_WANDERING_COIN, "Wandering Coin",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_STORM_BELL, "Storm Bell",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_OVERFLOW_ORB, "Overflow Orb",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_METER_MASON, "Meter Mason",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TIDE_CALLER, "Tide Caller",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_RESERVOIR_STONE, "Reservoir Stone",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_CARDSMITH_HAMMER, "Cardsmith Hammer",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_DECK_TRAY, "Deck Tray",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_WILD_JOKER, "Wild Joker",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_SCRIBBLERS_QUILL, "Scribbler's Quill",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_CARDINAL_ROBE, "Cardinal Robe",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_ALCHEMISTS_STONE, "Alchemist's Stone",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_FUSION_FURNACE, "Fusion Furnace",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_COMBINE_CATALYST, "Combine Catalyst",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TRANSMUTE_CRYSTAL, "Transmute Crystal",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_MASTERWORK_ANVIL, "Masterwork Anvil",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_FORTRESS_WALL, "Fortress Wall",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_BATTLE_STANDARD, "Battle Standard",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_MIRROR_SHIELD, "Mirror Shield",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_PATHFINDER_BOOTS, "Pathfinder Boots",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TERRITORY_MAP, "Territory Map",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
};

const size_t RELICS_COUNT = sizeof(RELICS) / sizeof(RELICS[0]);
