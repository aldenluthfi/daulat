//! effect.c
//!
//! EffectBus: single dispatch point for all game behaviors.
//! Effects are registered with scope (local / timed / battle / run).
//! Duration ticks expire local effects and decrement timed ones.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                                 BUS INIT
\*--------------------------------------------------------------------------*/

/// bus_init
///
/// Zero a bus and mark every slot empty.
///
/// Params:
/// - EffectBus* bus -> bus to initialize
///
void bus_init(EffectBus* bus) {
    bus->count = 0;
}

/*--------------------------------------------------------------------------*\
                                 BUS REGISTER
\*--------------------------------------------------------------------------*/

/// bus_register
///
/// Append a copy of `*effect` to the bus as a fresh active slot.
///
/// Params:
/// - EffectBus* bus -> destination bus
/// - const Effect* effect -> effect to register
///
void bus_register(EffectBus* bus, const Effect* effect) {
    if (bus->count >= MAX_EFFECTS) {
        log_warn(
            "EffectBus full, dropping effect (trigger=%d)\n",
            effect->trigger
        );
        return;
    }
    bus->slots[bus->count].effect          = effect;
    bus->slots[bus->count].remaining_turns = effect->duration_turns;
    bus->slots[bus->count].active          = true;
    bus->count++;
}

/*--------------------------------------------------------------------------*\
                                 BUS EMIT
\*--------------------------------------------------------------------------*/

/// bus_emit
///
/// Invoke every active handler whose trigger matches.
///
/// Params:
/// - EffectBus* bus -> bus to scan
/// - BattleState* bs -> battle state for handler use
/// - EffectTrigger trigger -> trigger key to dispatch
/// - struct EffectCtx* ctx -> evidence (mutated with trigger)
///
void bus_emit(
    EffectBus*        bus,
    BattleState*      bs,
    EffectTrigger     trigger,
    struct EffectCtx* ctx
) {
    (void)bs;
    ctx->trigger = trigger;
    for (uint16_t i = 0; i < bus->count; i++) {
        if (!bus->slots[i].active)
            continue;
        if (bus->slots[i].effect->trigger != trigger)
            continue;
        bus->slots[i].effect->apply(
            ctx,
            bus->slots[i].effect->args,
            bus->slots[i].effect->arg_count
        );
    }
}

/*--------------------------------------------------------------------------*\
                                 BUS TICK TURN END
\*--------------------------------------------------------------------------*/

/// bus_tick_turn_end
///
/// Expire every duration-0 effect at end of turn.
///
/// Params:
/// - EffectBus* bus -> bus to tick
///
void bus_tick_turn_end(EffectBus* bus) {
    for (uint16_t i = 0; i < bus->count; i++) {
        if (!bus->slots[i].active)
            continue;
        if (bus->slots[i].remaining_turns == 0) {
            bus->slots[i].active = false;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 BUS TICK TURN START
\*--------------------------------------------------------------------------*/

/// bus_tick_turn_start
///
/// Decrement remaining-turn counters at start of turn.
///
/// Params:
/// - EffectBus* bus -> bus to tick
///
void bus_tick_turn_start(EffectBus* bus) {
    for (uint16_t i = 0; i < bus->count; i++) {
        if (!bus->slots[i].active)
            continue;
        if (bus->slots[i].remaining_turns > 0) {
            bus->slots[i].remaining_turns--;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 BUS QUERY COUNT
\*--------------------------------------------------------------------------*/

/// bus_query_count
///
/// Count how many active slots match a given trigger.
///
/// Params:
/// - const EffectBus* bus -> bus to scan
/// - EffectTrigger t -> trigger key to count
///
/// Return:
/// size_t -> number of active matching slots
///
size_t bus_query_count(const EffectBus* bus, EffectTrigger trigger) {
    size_t count = 0;
    for (uint16_t i = 0; i < bus->count; i++) {
        if (bus->slots[i].active && bus->slots[i].effect->trigger == trigger) {
            count++;
        }
    }
    return count;
}

/*--------------------------------------------------------------------------*\
                                 BUS EVICT BY SOURCE
\*--------------------------------------------------------------------------*/

/// bus_evict_by_source
///
/// Deactivate every slot whose effect was registered with a matching
/// source_id.
///
/// Params:
/// - EffectBus* bus -> bus to scan
/// - uint32_t source_id -> source identifier to match
///
void bus_evict_by_source(EffectBus* bus, uint32_t source_id) {
    for (uint16_t i = 0; i < bus->count; i++) {
        if (bus->slots[i].effect->source_id == source_id) {
            bus->slots[i].active = false;
        }
    }
}
