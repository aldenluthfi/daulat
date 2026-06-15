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
        {.trigger = TRIGGER_QUERY_SELL_VALUE,
         EFF(MERCHANTS_LEDGER, merchants_ledger)}
    ),
    RELIC(
        RELIC_MINTED_COIN, "Minted Coin",
        {.trigger = TRIGGER_QUERY_TURN_INCOME,
         EFF(MINTED_COIN, minted_coin)}
    ),
    RELIC(
        RELIC_TAX_STAMP, "Tax Stamp",
        {.trigger = TRIGGER_CARD_PLAYED, EFF(TAX_STAMP, tax_stamp)}
    ),
    RELIC(
        RELIC_BULK_DISCOUNT, "Bulk Discount",
        {.trigger = TRIGGER_QUERY_PIECE_COST,
         EFF(BULK_DISCOUNT, bulk_discount)}
    ),
    RELIC(
        RELIC_WAR_CHEST, "War Chest",
        {.trigger = TRIGGER_TURN_END, EFF(WAR_CHEST, war_chest)}
    ),
    RELIC(
        RELIC_TRADE_ROUTES, "Trade Routes",
        {.trigger = TRIGGER_RUN_START, EFF(TRADE_ROUTES, trade_routes)}
    ),
    /* Meter (6) */
    RELIC(
        RELIC_SOUL_SHARD, "Soul Shard",
        {.trigger = TRIGGER_PIECE_FLIPPED, EFF(SOUL_SHARD, soul_shard)}
    ),
    RELIC(
        RELIC_VETERANS_BOND, "Veteran's Bond",
        {.trigger = TRIGGER_QUERY_METER_CAP,
         EFF(VETERANS_BOND, veterans_bond)}
    ),
    RELIC(
        RELIC_DEAD_MANS_PACT, "Dead Man's Pact",
        {.trigger = TRIGGER_RESOLVE_FLIP,
         EFF(DEAD_MANS_PACT, dead_mans_pact)}
    ),
    RELIC(
        RELIC_IRON_KING, "Iron King",
        {.trigger = TRIGGER_QUERY_METER_CAP, EFF(IRON_KING, iron_king)}
    ),
    RELIC(
        RELIC_BLOODTHIRST, "Bloodthirst",
        {.trigger = TRIGGER_TURN_START, EFF(BLOODTHIRST, bloodthirst)}
    ),
    RELIC(
        RELIC_LAST_BREATH, "Last Breath",
        {.trigger = TRIGGER_PIECE_FLIPPED, EFF(LAST_BREATH, last_breath)}
    ),
    /* Cards (5) */
    RELIC(
        RELIC_TACTICIANS_SCROLL, "Tactician's Scroll",
        {.trigger = TRIGGER_QUERY_DRAW_COUNT,
         EFF(TACTICIANS_SCROLL, tacticians_scroll)}
    ),
    RELIC(
        RELIC_LIBRARIANS_NOTES, "Librarian's Notes",
        {.trigger = TRIGGER_TURN_START,
         EFF(LIBRARIANS_NOTES, librarians_notes)}
    ),
    RELIC(
        RELIC_COUNTRY_SEAL, "Country Seal",
        {.trigger = TRIGGER_QUERY_SELL_VALUE,
         EFF(COUNTRY_SEAL, country_seal)}
    ),
    RELIC(
        RELIC_DEEP_HAND, "Deep Hand",
        {.trigger = TRIGGER_BATTLE_START, EFF(DEEP_HAND, deep_hand)}
    ),
    RELIC(
        RELIC_GILDED_ARCHIVE, "Gilded Archive",
        {.trigger = TRIGGER_QUERY_SELL_VALUE,
         EFF(GILDED_ARCHIVE, gilded_archive)}
    ),
    /* Combinations (4) */
    RELIC(
        RELIC_ALCHEMISTS_KIT, "Alchemist's Kit",
        {.trigger = TRIGGER_QUERY_COMBINE_COST,
         EFF(ALCHEMISTS_KIT, alchemists_kit)}
    ),
    RELIC(
        RELIC_MASTERS_NOTES, "Master's Notes",
        {.trigger = TRIGGER_RUN_START,
         EFF(MASTERS_NOTES, masters_notes)}
    ),
    RELIC(
        RELIC_PHILOSOPHERS_STONE, "Philosopher's Stone",
        {.trigger = TRIGGER_PIECE_COMBINED,
         EFF(PHILOSOPHERS_STONE, philosophers_stone)}
    ),
    RELIC(
        RELIC_INHERITED_POWER, "Inherited Power",
        {.trigger = TRIGGER_PIECE_COMBINED,
         EFF(INHERITED_POWER, inherited_power)}
    ),
    /* Board (5) */
    RELIC(
        RELIC_EAGLE_EYE, "Eagle Eye",
        {.trigger = TRIGGER_QUERY_VISION_FLAGS, EFF(EAGLE_EYE, eagle_eye)}
    ),
    RELIC(
        RELIC_SURVEYORS_MAP, "Surveyor's Map",
        {.trigger = TRIGGER_RUN_START,
         EFF(SURVEYORS_MAP, surveyors_map)}
    ),
    RELIC(
        RELIC_FORWARD_COMMAND, "Forward Command",
        {.trigger = TRIGGER_RESOLVE_ATTACK,
         EFF(FORWARD_COMMAND, forward_command)}
    ),
    RELIC(
        RELIC_FORTIFIED_LINE, "Fortified Line",
        {.trigger = TRIGGER_RESOLVE_ATTACK,
         EFF(FORTIFIED_LINE, fortified_line)}
    ),
    RELIC(
        RELIC_WARLORDS_BANNER, "Warlord's Banner",
        {.trigger = TRIGGER_QUERY_ADJ_KING_BONUS,
         EFF(WARLORDS_BANNER, warlords_banner)}
    ),
};

const size_t RELICS_COUNT = sizeof(RELICS) / sizeof(RELICS[0]);
