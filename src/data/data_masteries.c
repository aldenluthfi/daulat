//! data_masteries.c
//!
//! Static templates for figurehead mastery rewards. Mastery progress
//! is tracked per kingdom across runs; the level-2 mastery card and
//! level-1 / level-3 mastery hooks below activate based on how many
//! flawless runs the player has completed in each kingdom.
//!
//! Both arrays use TRIGGER_RUN_START for their stub effect so the
//! handler fires once at the beginning of each run, before any
//! battle state is constructed.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              MASTERY CARDS
\*--------------------------------------------------------------------------*/

/// MASTERY_CARDS
///
/// Unique level-2 mastery cards added to the kingdom's card set once
/// the player has cleared a flawless run of that kingdom. One entry
/// per kingdom, indexed by MasteryId.
///
const MasteryCard MASTERY_CARDS[] = {
    {
        .id = MASTERY_CARD_LONGWEI,
        .name = "Longwei Mastery",
        .description = "Longwei kingdom mastery.",
        .kingdom = KINGDOM_LONGWEI,
        .tier = TIER_KING,
        .effects[0] = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MASTERY_CARD_HARUSHIMA,
        .name = "Harushima Mastery",
        .description = "Harushima kingdom mastery.",
        .kingdom = KINGDOM_HARUSHIMA,
        .tier = TIER_KING,
        .effects[0] = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MASTERY_CARD_KEWARANI,
        .name = "Kewarani Mastery",
        .description = "Kewarani kingdom mastery.",
        .kingdom = KINGDOM_KEWARANI,
        .tier = TIER_KING,
        .effects[0] = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MASTERY_CARD_ZARQAN,
        .name = "Zarqan Mastery",
        .description = "Zarqan kingdom mastery.",
        .kingdom = KINGDOM_ZARQAN,
        .tier = TIER_KING,
        .effects[0] = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
        .effect_count = 1,
    },
    {
        .id = MASTERY_CARD_CAELAN,
        .name = "Caelan Mastery",
        .description = "Caelan kingdom mastery.",
        .kingdom = KINGDOM_CAELAN,
        .tier = TIER_KING,
        .effects[0] = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
        .effect_count = 1,
    },
};

const size_t MASTERY_CARDS_COUNT =
    sizeof(MASTERY_CARDS) / sizeof(MASTERY_CARDS[0]);

/*--------------------------------------------------------------------------*\
                              MASTERY HOOKS
\*--------------------------------------------------------------------------*/

/// MASTERY_HOOKS
///
/// Run-start hooks that activate when the player has reached the
/// given mastery level (1 = innate-shift, 3 = starting-power
/// upgrade). The first five entries handle level 1, the last five
/// handle level 3.
///
const MasteryHook MASTERY_HOOKS[] = {
    {
        .level = 1,
        .kingdom = KINGDOM_LONGWEI,
        .name = "Longwei Lv1 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 1,
        .kingdom = KINGDOM_HARUSHIMA,
        .name = "Harushima Lv1 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 1,
        .kingdom = KINGDOM_KEWARANI,
        .name = "Kewarani Lv1 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 1,
        .kingdom = KINGDOM_ZARQAN,
        .name = "Zarqan Lv1 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 1,
        .kingdom = KINGDOM_CAELAN,
        .name = "Caelan Lv1 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 3,
        .kingdom = KINGDOM_LONGWEI,
        .name = "Longwei Lv3 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 3,
        .kingdom = KINGDOM_HARUSHIMA,
        .name = "Harushima Lv3 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 3,
        .kingdom = KINGDOM_KEWARANI,
        .name = "Kewarani Lv3 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 3,
        .kingdom = KINGDOM_ZARQAN,
        .name = "Zarqan Lv3 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
    {
        .level = 3,
        .kingdom = KINGDOM_CAELAN,
        .name = "Caelan Lv3 Hook",
        .effect = {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
    },
};

const size_t MASTERY_HOOKS_COUNT =
    sizeof(MASTERY_HOOKS) / sizeof(MASTERY_HOOKS[0]);
