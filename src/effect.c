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

void bus_init(EffectBus* bus) { bus->count = 0; }

/*--------------------------------------------------------------------------*\
                                 BUS REGISTER
\*--------------------------------------------------------------------------*/

void bus_register(EffectBus* bus, const Effect* e) {
    if (bus->count >= MAX_EFFECTS) {
        log_warn("EffectBus full, dropping effect (trigger=%d)\n", e->trigger);
        return;
    }
    bus->slots[bus->count].effect = e;
    bus->slots[bus->count].remaining_turns = e->duration_turns;
    bus->slots[bus->count].active = true;
    bus->count++;
}

/*--------------------------------------------------------------------------*\
                                 BUS EMIT
\*--------------------------------------------------------------------------*/

void bus_emit(
    EffectBus* bus, BattleState* bs, EffectTrigger trigger,
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
            ctx, bus->slots[i].effect->args, bus->slots[i].effect->arg_count
        );
    }
}

/*--------------------------------------------------------------------------*\
                                 BUS TICK TURN END
\*--------------------------------------------------------------------------*/

void bus_tick_turn_end(EffectBus* bus) {
    for (uint16_t i = 0; i < bus->count; i++) {
        if (!bus->slots[i].active)
            continue;
        if (bus->slots[i].remaining_turns ==
            0) { 
            bus->slots[i].active = false;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 BUS TICK TURN START
\*--------------------------------------------------------------------------*/

void bus_tick_turn_start(EffectBus* bus) {
    for (uint16_t i = 0; i < bus->count; i++) {
        if (!bus->slots[i].active)
            continue;
        if (bus->slots[i].remaining_turns >
            0) { 
            bus->slots[i].remaining_turns--;
        }
    }
}

/*--------------------------------------------------------------------------*\
                                 BUS QUERY COUNT
\*--------------------------------------------------------------------------*/

size_t bus_query_count(const EffectBus* bus, EffectTrigger t) {
    size_t n = 0;
    for (uint16_t i = 0; i < bus->count; i++) {
        if (bus->slots[i].active && bus->slots[i].effect->trigger == t) {
            n++;
        }
    }
    return n;
}

/*--------------------------------------------------------------------------*\
                                 BUS EVICT BY SOURCE
\*--------------------------------------------------------------------------*/

void bus_evict_by_source(EffectBus* bus, uint32_t source_id) {
    for (uint16_t i = 0; i < bus->count; i++) {
        if (bus->slots[i].effect->source_id == source_id) {
            bus->slots[i].active = false;
        }
    }
}
