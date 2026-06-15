//! eff_run.c
//!
//! Run-scope effect handlers: relic activation, penalty-chain
//! application, and inter-kingdom synergy bonuses. These handlers
//! migrate effects into the battle's EffectBus when the player
//! enters a new battle or clears an Overseer.
//!
//! Also hosts the universal stand-in eff_todo used by data files
//! for unimplemented effects.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              RUN MODIFIERS
\*--------------------------------------------------------------------------*/

/// eff_apply_relic
///
/// Copies every effect declared by a RelicTemplate onto the active
/// battle's EffectBus so its passive bonuses participate in dispatch
/// alongside everything else.
///
/// Params:
/// - struct EffectCtx *context -> ignored; bus access goes through battle
/// - const EffectArg *args -> args[0] carries the relic template id
/// - size_t count              -> argument count from the caller
///
void eff_apply_relic(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// eff_apply_chain
///
/// Applies a penalty-chain template's effects when the player loses
/// a battle in a chained kingdom. Penalties register as run-scope
/// effects that stay live until the chain is cleared.
///
/// Params:
/// - struct EffectCtx *context -> ignored
/// - const EffectArg *args -> args[0] carries the chain level (1-3)
/// - size_t count              -> argument count from the caller
///
void eff_apply_chain(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// eff_apply_synergy
///
/// Activates the synergy bonus that an Overseer victory grants when
/// the player next battles in the matching kingdom. Registers a
/// run-scope effect keyed to the relevant trigger.
///
/// Params:
/// - struct EffectCtx *context -> ignored
/// - const EffectArg *args -> args[0] carries the cleared kingdom id
/// - size_t count              -> argument count from the caller
///
void eff_apply_synergy(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/*--------------------------------------------------------------------------*\
                              SHARED PLACEHOLDER
\*--------------------------------------------------------------------------*/

/// eff_todo
///
/// Universal stand-in for any effect whose behaviour has not yet
/// been implemented. Discards every argument and returns without
/// mutating state so the data file can keep listing the effect even
/// before its real handler exists.
///
/// Params:
/// - struct EffectCtx *context -> ignored
/// - const EffectArg *args -> ignored
/// - size_t count              -> ignored
///
/// Notes:
/// Must remain the only definition in the codebase. Reintroducing
/// per-file static duplicates would make it impossible to count
/// unimplemented effects with a single grep.
///
void eff_todo(struct EffectCtx* context, const EffectArg* args, size_t count) {
    (void)context;
    (void)args;
    (void)count;
}
