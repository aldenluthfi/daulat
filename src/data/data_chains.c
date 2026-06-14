//! data_chains.c
//!
//! Static templates for the three-tier penalty chain (Bronze, Silver,
//! Gold) that builds up as the player loses battles in a kingdom.
//!
//! Bronze deducts starting cp, Silver gifts the enemy a free piece,
//! and Gold locks the kingdom's track until a Liberation Trial is
//! cleared. The actual handlers are stubs until the chain UI lands.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              CHAINS
\*--------------------------------------------------------------------------*/

/// CHAINS
///
/// Ordered array of penalty chains. Index zero is the Bronze chain
/// (1 penalty), index one is Silver (2 penalties), index two is
/// Gold (3 penalties). chain_template() in registry.c does a linear
/// match on .level.
///
const Chain CHAINS[] = {
    {
        .level = 1,
        .penalties =
            {
                {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
            },
        .penalty_count = 1,
    },
    {
        .level = 2,
        .penalties =
            {
                {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
                {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
            },
        .penalty_count = 2,
    },
    {
        .level = 3,
        .penalties =
            {
                {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
                {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
                {.trigger = TRIGGER_RUN_START, .apply = eff_todo},
            },
        .penalty_count = 3,
    },
};

const size_t CHAINS_COUNT = sizeof(CHAINS) / sizeof(CHAINS[0]);
