//! data_events.c
//!
//! Static templates for narrative events that appear on the campaign
//! map between battles. Each event has a name, description, and id;
//! the actual two-option payoff is supplied by EventOption entries
//! attached when the map is generated.
//!
//! Universal events sit first in the array, followed by each
//! kingdom's events in canonical order. The .type field mirrors
//! .id today and is reserved for category-level UI grouping.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              EVENTS
\*--------------------------------------------------------------------------*/

/// EVENTS
///
/// Dense event-template table indexed by EventId. Entries reference
/// their string identity only; option payoffs are described at the
/// map-generator layer to avoid duplicating per-event Effect blocks
/// for every possible outcome here.
///
const EventTemplate EVENTS[] = {
    {
        .id          = EVENT_UNIVERSAL_WANDERER,
        .name        = "The Wanderer",
        .description = "A mysterious traveller offers a gift.",
        .type        = EVENT_UNIVERSAL_WANDERER,
    },
    {
        .id          = EVENT_UNIVERSAL_MARKET,
        .name        = "Travelling Market",
        .description = "A merchant caravan sets up nearby.",
        .type        = EVENT_UNIVERSAL_MARKET,
    },
    {
        .id          = EVENT_UNIVERSAL_AMBUSH,
        .name        = "Ambush!",
        .description = "Bandits attack the caravan!",
        .type        = EVENT_UNIVERSAL_AMBUSH,
    },
    {
        .id          = EVENT_LONGWEI_RIVER_FESTIVAL,
        .name        = "River Festival",
        .description = "A celebration by the river.",
        .type        = EVENT_LONGWEI_RIVER_FESTIVAL,
    },
    {
        .id          = EVENT_LONGWEI_SKY_LADDER,
        .name        = "Sky Ladder",
        .description = "A path to the heavens.",
        .type        = EVENT_LONGWEI_SKY_LADDER,
    },
    {
        .id          = EVENT_HARUSHIMA_HONOR_TRIAL,
        .name        = "Honor Trial",
        .description = "Prove your worth.",
        .type        = EVENT_HARUSHIMA_HONOR_TRIAL,
    },
    {
        .id          = EVENT_HARUSHIMA_TAKENOKO_SWARM,
        .name        = "Takenoko Swarm",
        .description = "Bamboo shoots everywhere!",
        .type        = EVENT_HARUSHIMA_TAKENOKO_SWARM,
    },
    {
        .id          = EVENT_KEWARANI_PILGRIMAGE_SEASON,
        .name        = "Pilgrimage Season",
        .description = "Pilgrims travel through.",
        .type        = EVENT_KEWARANI_PILGRIMAGE_SEASON,
    },
    {
        .id          = EVENT_KEWARANI_BAZAAR_RUMOR,
        .name        = "Bazaar Rumor",
        .description = "News from the market.",
        .type        = EVENT_KEWARANI_BAZAAR_RUMOR,
    },
    {
        .id          = EVENT_ZARQAN_SANDSTORM,
        .name        = "Sandstorm",
        .description = "A desert tempest approaches.",
        .type        = EVENT_ZARQAN_SANDSTORM,
    },
    {
        .id          = EVENT_ZARQAN_OASIS_DISCOVERY,
        .name        = "Oasis Discovery",
        .description = "Water in the wasteland.",
        .type        = EVENT_ZARQAN_OASIS_DISCOVERY,
    },
    {
        .id          = EVENT_CAELAN_TOURNAMENT,
        .name        = "Tournament",
        .description = "A grand competition.",
        .type        = EVENT_CAELAN_TOURNAMENT,
    },
    {
        .id          = EVENT_CAELAN_CASTLE_FESTIVAL,
        .name        = "Castle Festival",
        .description = "Celebration at the keep.",
        .type        = EVENT_CAELAN_CASTLE_FESTIVAL,
    },
};

const size_t EVENTS_COUNT = sizeof(EVENTS) / sizeof(EVENTS[0]);
