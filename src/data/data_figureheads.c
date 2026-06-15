//! data_figureheads.c
//!
//! Static templates for the five kingdom figureheads' starting
//! powers. All five powers are simultaneously active for every run
//! because the figureheads are allied against Vorath.
//!
//! Each effect maps to its kingdom-specific body. Mingzhu
//! and Isabella fire at battle start, the other three configure
//! run-wide overrides on run start.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              FIGUREHEAD POWERS
\*--------------------------------------------------------------------------*/

/// FIGUREHEADS
///
/// Linear array of figurehead powers keyed by .kingdom. Only five
/// entries, so the registry walks the array on lookup.
///
const FigureheadPower FIGUREHEADS[] = {
    {
        .kingdom     = KINGDOM_LONGWEI,
        .name        = "Mingzhu's Cannon Salute",
        .description = "Spawn a free Pao in Longwei; +20 cp elsewhere.",
        .effects =
            {
                {.trigger = TRIGGER_BATTLE_START,
                 EFF(FH_MINGZHU, fh_mingzhu)},
            },
        .effect_count = 1,
    },
    {
        .kingdom     = KINGDOM_HARUSHIMA,
        .name        = "Tomohito's Patience",
        .description = "Reclaim cost drops to 10 cp (5 at Mastery 3).",
        .effects =
            {
                {.trigger = TRIGGER_RUN_START,
                 EFF(FH_TOMOHITO, fh_tomohito)},
            },
        .effect_count = 1,
    },
    {
        .kingdom     = KINGDOM_KEWARANI,
        .name        = "Selassie's March",
        .description = "Double Time active from run start.",
        .effects =
            {
                {.trigger = TRIGGER_BATTLE_START,
                 EFF(FH_SELASSIE, fh_selassie)},
            },
        .effect_count = 1,
    },
    {
        .kingdom     = KINGDOM_ZARQAN,
        .name        = "Timur's Royal Substitution",
        .description = "Royal Substitution usable twice per battle.",
        .effects =
            {
                {.trigger = TRIGGER_RUN_START,
                 EFF(FH_TIMUR, fh_timur)},
            },
        .effect_count = 1,
    },
    {
        .kingdom     = KINGDOM_CAELAN,
        .name        = "Isabella's Crusade",
        .description = "Guaranteed Province-tier Caelan card at turn 1.",
        .effects =
            {
                {.trigger = TRIGGER_BATTLE_START,
                 EFF(FH_ISABELLA, fh_isabella)},
            },
        .effect_count = 1,
    },
};

const size_t FIGUREHEADS_COUNT = sizeof(FIGUREHEADS) / sizeof(FIGUREHEADS[0]);
