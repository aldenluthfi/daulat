//! data_figureheads.c
//!
//! Static templates for the five kingdom figureheads' starting
//! powers. All five powers are simultaneously active for every run
//! because the figureheads are allied against Vorath.
//!
//! The .effects field is intentionally zero for now; once the
//! figurehead power handlers are written, each kingdom's effect
//! lands here and registers at run start.
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
        .kingdom = KINGDOM_LONGWEI,
        .name = "Longwei Figurehead",
        .description = "Longwei starting power.",
        .effect_count = 0,
    },
    {
        .kingdom = KINGDOM_HARUSHIMA,
        .name = "Harushima Figurehead",
        .description = "Harushima starting power.",
        .effect_count = 0,
    },
    {
        .kingdom = KINGDOM_KEWARANI,
        .name = "Kewarani Figurehead",
        .description = "Kewarani starting power.",
        .effect_count = 0,
    },
    {
        .kingdom = KINGDOM_ZARQAN,
        .name = "Zarqan Figurehead",
        .description = "Zarqan starting power.",
        .effect_count = 0,
    },
    {
        .kingdom = KINGDOM_CAELAN,
        .name = "Caelan Figurehead",
        .description = "Caelan starting power.",
        .effect_count = 0,
    },
};

const size_t FIGUREHEADS_COUNT = sizeof(FIGUREHEADS) / sizeof(FIGUREHEADS[0]);
