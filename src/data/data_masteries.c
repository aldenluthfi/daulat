//! data_masteries.c
//!
//! Static templates for figurehead mastery rewards. Mastery progress
//! is tracked per kingdom across runs.
//!
//! Level 2: the figurehead card is added to the matching kingdom's
//! card set. Each card is a normal CardTemplate in the CARD_* enum;
//! the level-2 hook injects it into the player's cardset.
//!
//! Level 1 / Level 3 hooks shift where the kingdom innate activates
//! (level 1) and upgrade the kingdom's starting power (level 3).
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              MASTERY CARDS
\*--------------------------------------------------------------------------*/

/// MASTERY_CARDS
///
/// The five figurehead cards added to the kingdom card set once
/// Mastery-2 is reached. Indexed by CardId: CARD_MINGZHUS_SEAL ..
/// CARD_ISABELLAS_CORONATION. Effects default to eff_todo until the
/// per-card behaviour is implemented.
///
const CardTemplate MASTERY_CARDS[] = {
    [CARD_MINGZHUS_SEAL] =
        {
            .id = CARD_MINGZHUS_SEAL,
            .name = "Mingzhu's Seal",
            .kingdom = KINGDOM_LONGWEI,
            .tier = TIER_PROVINCE,
            .play_cost = 0,
            .sell_value = 0,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_TOMOHITOS_PATIENCE] =
        {
            .id = CARD_TOMOHITOS_PATIENCE,
            .name = "Tomohito's Patience",
            .kingdom = KINGDOM_HARUSHIMA,
            .tier = TIER_PROVINCE,
            .play_cost = 0,
            .sell_value = 0,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_SELASSIES_MARCH] =
        {
            .id = CARD_SELASSIES_MARCH,
            .name = "Selassie's March",
            .kingdom = KINGDOM_KEWARANI,
            .tier = TIER_PROVINCE,
            .play_cost = 0,
            .sell_value = 0,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_TIMURS_CONQUEST] =
        {
            .id = CARD_TIMURS_CONQUEST,
            .name = "Timur's Conquest",
            .kingdom = KINGDOM_ZARQAN,
            .tier = TIER_PROVINCE,
            .play_cost = 0,
            .sell_value = 0,
            .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
            .play_effect_count = 1,
        },
    [CARD_ISABELLAS_CORONATION] = {
        .id = CARD_ISABELLAS_CORONATION,
        .name = "Isabella's Coronation",
        .kingdom = KINGDOM_CAELAN,
        .tier = TIER_PROVINCE,
        .play_cost = 0,
        .sell_value = 0,
        .on_play = {{.trigger = TRIGGER_TURN_START, .apply = eff_todo}},
        .play_effect_count = 1,
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
