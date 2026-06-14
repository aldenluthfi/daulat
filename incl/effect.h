//!
//! effect.h
//!
//! Effect-bus public interface: the data records, triggers, context
//! union and dispatch API that every game behavior funnels through.
//!
//! Effects are uniform `(EffectFunc, EffectArg[])` tuples keyed by
//! an EffectTrigger; the EffectBus stores active effects with their
//! remaining duration and dispatches them on bus_emit(). Adding a
//! behavior is a data change, never an engine edit.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef EFFECT_H
#define EFFECT_H

#include "defs.h"
#include "movegen.h"
#include "types.h"

struct EffectCtx;
struct BattleState;

/*--------------------------------------------------------------------------*\
                              TRIGGERS
\*--------------------------------------------------------------------------*/

/// EffectTrigger
///
/// Discriminated lifecycle / query event that the bus uses as a
/// dispatch key. Adding a new trigger requires extending only this
/// enum plus any data row that wants to listen to it.
///
typedef enum {
    TRIGGER_BATTLE_START,
    TRIGGER_BATTLE_END,
    TRIGGER_TURN_START,
    TRIGGER_TURN_END,
    TRIGGER_RESOLVE_DEFENSE,
    TRIGGER_RESOLVE_ATTACK,
    TRIGGER_RESOLVE_DAMAGE,
    TRIGGER_RESOLVE_FLIP,
    TRIGGER_CARD_DRAWN,
    TRIGGER_CARD_PLAYED,
    TRIGGER_CARD_SOLD,
    TRIGGER_COMBO_CHAIN_2,
    TRIGGER_COMBO_CHAIN_3,
    TRIGGER_PIECE_PLACED,
    TRIGGER_PIECE_MOVED,
    TRIGGER_PIECE_COMBINED,
    TRIGGER_PIECE_DEALT_DAMAGE,
    TRIGGER_PIECE_ENTERED_ENEMY_TERR,
    TRIGGER_PIECE_FLIPPED,
    TRIGGER_PIECE_LOST,
    TRIGGER_PIECE_GAINED,
    TRIGGER_QUERY_PIECE_COST,
    TRIGGER_QUERY_SELL_VALUE,
    TRIGGER_QUERY_DRAW_COUNT,
    TRIGGER_QUERY_TURN_INCOME,
    TRIGGER_QUERY_MOVE_COUNT,
    TRIGGER_QUERY_DAMAGE_MULT,
    TRIGGER_QUERY_METER_CAP,
    TRIGGER_MAP_ENTERED,
    TRIGGER_RUN_START,
    TRIGGER_COUNT
} EffectTrigger;

/*--------------------------------------------------------------------------*\
                              EFFECT
\*--------------------------------------------------------------------------*/

/// EffectFunc
///
/// Function pointer signature for every effect handler. The `ctx`
/// argument carries per-trigger evidence; `args` is the typed
/// argument list the data file recorded for this effect entry.
///
typedef void (*EffectFunc)(
    struct EffectCtx* ctx, const EffectArg* args, size_t n
);

/// Effect
///
/// Behaviour entry registered with the bus. Combines a trigger key,
/// the handler function, typed arguments, a duration in turns, the
/// owner side, and an opaque source id used for selective eviction
/// (piece removal, card discard, relic loss).
///
typedef struct {
    EffectTrigger trigger;
    EffectFunc apply;
    EffectArg args[MAX_EFFECT_ARGS];
    uint8_t arg_count;
    int16_t duration_turns;
    Side owner;
    uint32_t source_id;
} Effect;

/// EffectSlot
///
/// Bus-internal entry pairing an Effect pointer with its current
/// remaining-turns counter and active flag. Slots are never freed —
/// they are merely deactivated when their effect expires.
///
typedef struct {
    const Effect* effect;
    int16_t remaining_turns;
    bool active;
} EffectSlot;

/*--------------------------------------------------------------------------*\
                              EFFECT CONTEXT
\*--------------------------------------------------------------------------*/

/// EffectCtx
///
/// Per-trigger evidence handed to every EffectFunc invocation. The
/// union arms cover the cases the bus dispatches: resolve, piece
/// lifecycle, card lifecycle, query out-parameters, movement
/// override, meta hooks, and a generic source-id arm.
///
struct EffectCtx {
    EffectTrigger trigger;
    union {
        struct {
            struct PieceState* attacker;
            struct PieceState* target;
            int* damage_out;
            int* damage_mult_out;
            int* reduction_out;
        } resolve;
        struct {
            struct PieceState* piece;
            Position* pos;
        } piece;
        struct {
            Side side;
            CardInstance* card;
            int* count_out;
            int* value_out;
        } card;
        struct {
            int* income_out;
            int* cost_out;
            int* moves_out;
            int* meter_cap_out;
        } query;
        struct {
            struct PieceState* piece;
            MoveGen* move_out;
        } movement;
        struct {
            Kingdom kingdom;
        } meta;
        struct {
            uint32_t source_id;
        } generic;
    } as;
};

/*--------------------------------------------------------------------------*\
                              EFFECT BUS
\*--------------------------------------------------------------------------*/

/// EffectBus
///
/// Dispatch container holding every active effect for the current
/// battle. Slots are scanned linearly on emit; the slot count never
/// exceeds MAX_EFFECTS.
///
typedef struct {
    EffectSlot slots[MAX_EFFECTS];
    uint16_t count;
} EffectBus;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// bus_init
///
/// Zero a bus and mark every slot empty. Must be called once per
/// battle before any other bus operation.
///
/// Params:
/// - EffectBus *bus -> bus to initialise
///
void bus_init(EffectBus* bus);

/// bus_register
///
/// Append a copy of `*e` to the bus as a fresh active slot. Drops
/// the registration with a warning if the bus is already full.
///
/// Params:
/// - EffectBus    *bus -> destination bus
/// - const Effect *e   -> effect to register; copied by reference
///
void bus_register(EffectBus* bus, const Effect* e);

/// bus_emit
///
/// Invoke every active handler whose trigger matches. Handlers run
/// in registration order; the caller fills `ctx->as` with the union
/// arm matching the trigger before calling.
///
/// Params:
/// - EffectBus         *bus     -> bus to scan
/// - struct BattleState *bs     -> battle state for handler use
/// - EffectTrigger      trigger -> trigger key to dispatch
/// - struct EffectCtx  *ctx     -> evidence (mutated with trigger)
///
void bus_emit(
    EffectBus* bus, struct BattleState* bs, EffectTrigger trigger,
    struct EffectCtx* ctx
);

/// bus_tick_turn_end
///
/// Expire every duration-0 effect at end of turn so this-turn-only
/// effects fall off the bus.
///
/// Params:
/// - EffectBus *bus -> bus to tick
///
void bus_tick_turn_end(EffectBus* bus);

/// bus_tick_turn_start
///
/// Decrement remaining-turn counters at start of turn. Duration -1
/// (run-wide) and duration 0 (this-turn) are left alone; only
/// positive durations are touched.
///
/// Params:
/// - EffectBus *bus -> bus to tick
///
void bus_tick_turn_start(EffectBus* bus);

/// bus_query_count
///
/// Count how many active slots match a given trigger. Used by query
/// triggers that need to know if any handler is listening.
///
/// Params:
/// - const EffectBus *bus -> bus to scan
/// - EffectTrigger    t   -> trigger key to count
///
/// Return:
/// size_t -> number of active matching slots
///
size_t bus_query_count(const EffectBus* bus, EffectTrigger t);

/// bus_evict_by_source
///
/// Deactivate every slot whose effect was registered with a matching
/// source_id. Used when a piece is removed, a card discarded, or a
/// relic lost so its passives stop firing.
///
/// Params:
/// - EffectBus *bus       -> bus to scan
/// - uint32_t   source_id -> source identifier to match
///
void bus_evict_by_source(EffectBus* bus, uint32_t source_id);

/*--------------------------------------------------------------------------*\
                              SHARED PLACEHOLDER
\*--------------------------------------------------------------------------*/

/// eff_todo
///
/// Universal placeholder used as the `.apply` field for any effect
/// whose behaviour is not yet implemented. Discards every argument
/// and returns without mutating state.
///
/// Params:
/// - struct EffectCtx *ctx -> ignored
/// - const EffectArg  *args -> ignored
/// - size_t            n   -> ignored
///
/// Notes:
/// Defined exactly once in src/effects/eff_run.c. Never reintroduce
/// per-file static duplicates — the placeholder must remain a single
/// symbol so counting unimplemented effects stays trivial.
///
void eff_todo(struct EffectCtx* ctx, const EffectArg* args, size_t n);

#endif /* EFFECT_H */
