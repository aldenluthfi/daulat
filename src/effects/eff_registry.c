//! eff_registry.c
//!
//! Effect-function lookup table. Maps every `EffectFuncId` to its
//! live function pointer so the save/load codec can serialize an
//! id on disk and rehydrate the `Effect.apply` pointer on load.
//!
//! Phase 2 only populates EFFECT_FUNC_TODO. Phase 3 grows the table
//! alongside the `EffectFuncId` enum as meta-layer bodies land.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              REGISTRY TABLE
\*--------------------------------------------------------------------------*/

/// EFFECT_FUNCS
///
/// Dense lookup table indexed by `EffectFuncId`. Every cell that is
/// not yet populated falls back to `eff_todo` so missing entries
/// never crash — they just no-op.
///
static const EffectFunc EFFECT_FUNCS[EFFECT_FUNC_COUNT] = {
    [EFFECT_FUNC_TODO] = eff_todo,
};

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

EffectFunc eff_lookup(EffectFuncId id) {
    if (id < 0 || id >= EFFECT_FUNC_COUNT) {
        log_warn("eff_lookup: out-of-range id %d", id);
        return NULL;
    }
    EffectFunc func = EFFECT_FUNCS[id];
    return (func != NULL) ? func : eff_todo;
}
