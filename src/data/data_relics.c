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
        .id           = id_,                                                   \
        .name         = name_,                                                 \
        .effects      = {__VA_ARGS__},                                         \
        .effect_count = sizeof((Effect[]){__VA_ARGS__}) / sizeof(Effect),      \
    }

/// RELICS
///
/// Dense relic table. Iteration order matches the GDD category
/// breakdown (Economy 6, Meter 6, Cards 5, Combinations 4, Board 5)
/// so future rebalancing edits stay easy to read.
///
const RelicTemplate RELICS[] = {
    /* Economy (6) */
    RELIC(
        RELIC_MERCHANTS_LEDGER, "Merchant's Ledger",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_MINTED_COIN, "Minted Coin",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TAX_STAMP, "Tax Stamp",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_BULK_DISCOUNT, "Bulk Discount",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_WAR_CHEST, "War Chest",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_TRADE_ROUTES, "Trade Routes",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    /* Meter (6) */
    RELIC(
        RELIC_SOUL_SHARD, "Soul Shard",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_VETERANS_BOND, "Veteran's Bond",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_DEAD_MANS_PACT, "Dead Man's Pact",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_IRON_KING, "Iron King",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_BLOODTHIRST, "Bloodthirst",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_LAST_BREATH, "Last Breath",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    /* Cards (5) */
    RELIC(
        RELIC_TACTICIANS_SCROLL, "Tactician's Scroll",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_LIBRARIANS_NOTES, "Librarian's Notes",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_COUNTRY_SEAL, "Country Seal",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_DEEP_HAND, "Deep Hand",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_GILDED_ARCHIVE, "Gilded Archive",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    /* Combinations (4) */
    RELIC(
        RELIC_ALCHEMISTS_KIT, "Alchemist's Kit",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_MASTERS_NOTES, "Master's Notes",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_PHILOSOPHERS_STONE, "Philosopher's Stone",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_INHERITED_POWER, "Inherited Power",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    /* Board (5) */
    RELIC(
        RELIC_EAGLE_EYE, "Eagle Eye",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_SURVEYORS_MAP, "Surveyor's Map",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_FORWARD_COMMAND, "Forward Command",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_FORTIFIED_LINE, "Fortified Line",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
    RELIC(
        RELIC_WARLORDS_BANNER, "Warlord's Banner",
        {.trigger = TRIGGER_TURN_START, .apply = eff_todo}
    ),
};

const size_t RELICS_COUNT = sizeof(RELICS) / sizeof(RELICS[0]);
