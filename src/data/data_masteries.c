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
            .id         = CARD_MINGZHUS_SEAL,
            .name       = "Mingzhu's Seal",
            .kingdom    = KINGDOM_LONGWEI,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 0,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_TOMOHITOS_PATIENCE] =
        {
            .id         = CARD_TOMOHITOS_PATIENCE,
            .name       = "Tomohito's Patience",
            .kingdom    = KINGDOM_HARUSHIMA,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 0,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_SELASSIES_MARCH] =
        {
            .id         = CARD_SELASSIES_MARCH,
            .name       = "Selassie's March",
            .kingdom    = KINGDOM_KEWARANI,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 0,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_TIMURS_CONQUEST] =
        {
            .id         = CARD_TIMURS_CONQUEST,
            .name       = "Timur's Conquest",
            .kingdom    = KINGDOM_ZARQAN,
            .tier       = TIER_PROVINCE,
            .play_cost  = 0,
            .sell_value = 0,
            .on_play    = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
            .play_effect_count = 1,
        },
    [CARD_ISABELLAS_CORONATION] = {
        .id                = CARD_ISABELLAS_CORONATION,
        .name              = "Isabella's Coronation",
        .kingdom           = KINGDOM_CAELAN,
        .tier              = TIER_PROVINCE,
        .play_cost         = 0,
        .sell_value        = 0,
        .on_play           = {{.trigger = TRIGGER_TURN_START, EFF(TODO, todo)}},
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
/// Per-kingdom mastery hooks at levels 1, 2 and 3. Level 1 activates
/// the kingdom innate at Town entry; level 2 appends the figurehead
/// card to the cardset; level 3 flips a RunState flag that the
/// figurehead and innate bodies consult for the upgraded variant.
///
const MasteryHook MASTERY_HOOKS[] = {
    {.level   = 1,
     .kingdom = KINGDOM_LONGWEI,
     .name    = "Longwei Lv1 Hook",
     .effect =
         {.trigger = TRIGGER_MAP_ENTERED,
          EFF(MASTERY_L1_LONGWEI, mastery_l1_longwei)}},
    {.level   = 1,
     .kingdom = KINGDOM_HARUSHIMA,
     .name    = "Harushima Lv1 Hook",
     .effect =
         {.trigger = TRIGGER_MAP_ENTERED,
          EFF(MASTERY_L1_HARUSHIMA, mastery_l1_harushima)}},
    {.level   = 1,
     .kingdom = KINGDOM_KEWARANI,
     .name    = "Kewarani Lv1 Hook",
     .effect =
         {.trigger = TRIGGER_MAP_ENTERED,
          EFF(MASTERY_L1_KEWARANI, mastery_l1_kewarani)}},
    {.level   = 1,
     .kingdom = KINGDOM_ZARQAN,
     .name    = "Zarqan Lv1 Hook",
     .effect =
         {.trigger = TRIGGER_MAP_ENTERED,
          EFF(MASTERY_L1_ZARQAN, mastery_l1_zarqan)}},
    {.level   = 1,
     .kingdom = KINGDOM_CAELAN,
     .name    = "Caelan Lv1 Hook",
     .effect =
         {.trigger = TRIGGER_MAP_ENTERED,
          EFF(MASTERY_L1_CAELAN, mastery_l1_caelan)}},
    {.level   = 2,
     .kingdom = KINGDOM_LONGWEI,
     .name    = "Longwei Lv2 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L2_LONGWEI, mastery_l2_longwei)}},
    {.level   = 2,
     .kingdom = KINGDOM_HARUSHIMA,
     .name    = "Harushima Lv2 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L2_HARUSHIMA, mastery_l2_harushima)}},
    {.level   = 2,
     .kingdom = KINGDOM_KEWARANI,
     .name    = "Kewarani Lv2 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L2_KEWARANI, mastery_l2_kewarani)}},
    {.level   = 2,
     .kingdom = KINGDOM_ZARQAN,
     .name    = "Zarqan Lv2 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L2_ZARQAN, mastery_l2_zarqan)}},
    {.level   = 2,
     .kingdom = KINGDOM_CAELAN,
     .name    = "Caelan Lv2 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L2_CAELAN, mastery_l2_caelan)}},
    {.level   = 3,
     .kingdom = KINGDOM_LONGWEI,
     .name    = "Longwei Lv3 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L3_LONGWEI, mastery_l3_longwei)}},
    {.level   = 3,
     .kingdom = KINGDOM_HARUSHIMA,
     .name    = "Harushima Lv3 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L3_HARUSHIMA, mastery_l3_harushima)}},
    {.level   = 3,
     .kingdom = KINGDOM_KEWARANI,
     .name    = "Kewarani Lv3 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L3_KEWARANI, mastery_l3_kewarani)}},
    {.level   = 3,
     .kingdom = KINGDOM_ZARQAN,
     .name    = "Zarqan Lv3 Hook",
     .effect =
         {.trigger = TRIGGER_RUN_START,
          EFF(MASTERY_L3_ZARQAN, mastery_l3_zarqan)}},
    {.level   = 3,
     .kingdom = KINGDOM_CAELAN,
     .name    = "Caelan Lv3 Hook",
     .effect  = {
         .trigger = TRIGGER_RUN_START,
         EFF(MASTERY_L3_CAELAN, mastery_l3_caelan)
     }},
};

const size_t MASTERY_HOOKS_COUNT =
    sizeof(MASTERY_HOOKS) / sizeof(MASTERY_HOOKS[0]);
