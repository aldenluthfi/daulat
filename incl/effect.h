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
    TRIGGER_PIECE_REMOVED,
    TRIGGER_QUERY_PIECE_COST,
    TRIGGER_QUERY_SELL_VALUE,
    TRIGGER_QUERY_DRAW_COUNT,
    TRIGGER_QUERY_TURN_INCOME,
    TRIGGER_QUERY_MOVE_COUNT,
    TRIGGER_QUERY_DAMAGE_MULT,
    TRIGGER_QUERY_METER_CAP,
    TRIGGER_QUERY_COMBINE_COST,
    TRIGGER_QUERY_COMBO_VALUE_BONUS,
    TRIGGER_QUERY_ADJ_KING_BONUS,
    TRIGGER_QUERY_RECLAIM_COST,
    TRIGGER_QUERY_ROYAL_SUB_COUNT,
    TRIGGER_QUERY_VISION_FLAGS,
    TRIGGER_MAP_ENTERED,
    TRIGGER_RUN_START,
    TRIGGER_COUNT
} EffectTrigger;

/*--------------------------------------------------------------------------*\
                              CAUSE DISCRIMINATORS
\*--------------------------------------------------------------------------*/

/// PlacementCause
///
/// Why a piece appeared on the board. Read by `TRIGGER_PIECE_PLACED`
/// handlers that filter on purchase, spawn, combine result, or
/// splitter spawn. Reclaim is not in this list; reclaimed pieces are
/// already on the board and fire `TRIGGER_PIECE_FLIPPED` instead.
///
typedef enum {
    PLACED_SPAWN,
    PLACED_BOUGHT,
    PLACED_COMBINE_RESULT,
    PLACED_SPLIT,
} PlacementCause;

/// FlipCause
///
/// Why a piece changed owner. Read by `TRIGGER_PIECE_FLIPPED`
/// handlers that filter on cascade, reclaim, forced flip, or Mercy.
///
typedef enum {
    FLIPPED_METER_CASCADE,
    FLIPPED_RECLAIM,
    FLIPPED_FORCED,
    FLIPPED_MERCY,
} FlipCause;

/// RemovalCause
///
/// Why a piece left the board. Combine ingredients do not emit
/// `TRIGGER_PIECE_REMOVED`; they go through `TRIGGER_PIECE_COMBINED`
/// so "you lost a piece" cards do not mistake voluntary combines for
/// losses.
///
typedef enum {
    REMOVED_SACRIFICE,
    REMOVED_MANDATE,
    REMOVED_SPLITTER_SUBSTITUTION,
} RemovalCause;

/*--------------------------------------------------------------------------*\
                              EFFECT FUNC IDS
\*--------------------------------------------------------------------------*/

/// EffectFuncId
///
/// Stable identifier for every concrete `EffectFunc` body. Used on
/// disk in place of the live pointer; reseated to `apply` on load
/// via `eff_lookup`. Each meta-layer effect with a real body
/// owns one enumerator here.
///
typedef enum {
    EFFECT_FUNC_TODO = 0,

    /* Relics (26) */
    EFFECT_FUNC_MERCHANTS_LEDGER,
    EFFECT_FUNC_MINTED_COIN,
    EFFECT_FUNC_TAX_STAMP,
    EFFECT_FUNC_BULK_DISCOUNT,
    EFFECT_FUNC_WAR_CHEST,
    EFFECT_FUNC_TRADE_ROUTES,
    EFFECT_FUNC_SOUL_SHARD,
    EFFECT_FUNC_VETERANS_BOND,
    EFFECT_FUNC_DEAD_MANS_PACT,
    EFFECT_FUNC_IRON_KING,
    EFFECT_FUNC_BLOODTHIRST,
    EFFECT_FUNC_LAST_BREATH,
    EFFECT_FUNC_TACTICIANS_SCROLL,
    EFFECT_FUNC_LIBRARIANS_NOTES,
    EFFECT_FUNC_COUNTRY_SEAL,
    EFFECT_FUNC_DEEP_HAND,
    EFFECT_FUNC_GILDED_ARCHIVE,
    EFFECT_FUNC_ALCHEMISTS_KIT,
    EFFECT_FUNC_MASTERS_NOTES,
    EFFECT_FUNC_PHILOSOPHERS_STONE,
    EFFECT_FUNC_INHERITED_POWER,
    EFFECT_FUNC_EAGLE_EYE,
    EFFECT_FUNC_SURVEYORS_MAP,
    EFFECT_FUNC_FORWARD_COMMAND,
    EFFECT_FUNC_FORTIFIED_LINE,
    EFFECT_FUNC_WARLORDS_BANNER,

    /* Chains (3) */
    EFFECT_FUNC_CHAIN_BRONZE,
    EFFECT_FUNC_CHAIN_SILVER,
    EFFECT_FUNC_CHAIN_GOLD,

    /* Innates (5) */
    EFFECT_FUNC_INNATE_BULWARK,
    EFFECT_FUNC_INNATE_RECLAIM,
    EFFECT_FUNC_INNATE_DOUBLE_TIME,
    EFFECT_FUNC_INNATE_ROYAL_SUB,
    EFFECT_FUNC_INNATE_CONQUERORS_REWARD,

    /* Figureheads (5) */
    EFFECT_FUNC_FH_MINGZHU,
    EFFECT_FUNC_FH_TOMOHITO,
    EFFECT_FUNC_FH_SELASSIE,
    EFFECT_FUNC_FH_TIMUR,
    EFFECT_FUNC_FH_ISABELLA,

    /* Synergies (5) */
    EFFECT_FUNC_SYN_LONGWEI,
    EFFECT_FUNC_SYN_HARUSHIMA,
    EFFECT_FUNC_SYN_KEWARANI,
    EFFECT_FUNC_SYN_ZARQAN,
    EFFECT_FUNC_SYN_CAELAN,

    /* Mastery hooks (5 × 3 levels = 15) */
    EFFECT_FUNC_MASTERY_L1_LONGWEI,
    EFFECT_FUNC_MASTERY_L1_HARUSHIMA,
    EFFECT_FUNC_MASTERY_L1_KEWARANI,
    EFFECT_FUNC_MASTERY_L1_ZARQAN,
    EFFECT_FUNC_MASTERY_L1_CAELAN,
    EFFECT_FUNC_MASTERY_L2_LONGWEI,
    EFFECT_FUNC_MASTERY_L2_HARUSHIMA,
    EFFECT_FUNC_MASTERY_L2_KEWARANI,
    EFFECT_FUNC_MASTERY_L2_ZARQAN,
    EFFECT_FUNC_MASTERY_L2_CAELAN,
    EFFECT_FUNC_MASTERY_L3_LONGWEI,
    EFFECT_FUNC_MASTERY_L3_HARUSHIMA,
    EFFECT_FUNC_MASTERY_L3_KEWARANI,
    EFFECT_FUNC_MASTERY_L3_ZARQAN,
    EFFECT_FUNC_MASTERY_L3_CAELAN,

    /* Overseers (5 non-Vorath) */
    EFFECT_FUNC_OVERSEER_IRON_STRATEGIST,
    EFFECT_FUNC_OVERSEER_ETERNAL_RECURSION,
    EFFECT_FUNC_OVERSEER_CARAVAN_OF_CONQUEST,
    EFFECT_FUNC_OVERSEER_MANY_FACED_KING,
    EFFECT_FUNC_OVERSEER_CROWNED_HERETIC,

    /* Vorath Memory */
    EFFECT_FUNC_VORATH_MEMORY_TALLY,
    EFFECT_FUNC_VORATH_MEMORY_APPLY,

    EFFECT_FUNC_COUNT
} EffectFuncId;

/*--------------------------------------------------------------------------*\
                              EFFECT
\*--------------------------------------------------------------------------*/

/// EffectFunc
///
/// Function pointer signature for every effect handler. The `context`
/// argument carries per-trigger evidence; `args` is the typed
/// argument list the data file recorded for this effect entry.
///
typedef void (*EffectFunc)(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
);

/// Effect
///
/// Behaviour entry registered with the bus. Combines a trigger key,
/// a stable `func_id` for serialization, the handler function,
/// typed read-only arguments, a per-instance writable scratch slot,
/// a duration in turns, the owner side, and an opaque source id used
/// for selective eviction (piece removal, card discard, relic loss).
///
/// `args[]` are the literal arguments from the data table; `scratch[]`
/// holds per-instance mutable state (counters, latches) that handlers
/// can update on each emit. Save/load serializes both arrays so an
/// in-progress battle round-trips faithfully.
///
typedef struct Effect {
    EffectTrigger trigger;
    EffectFuncId  func_id;
    EffectFunc    apply;
    EffectArg     args[MAX_EFFECT_ARGS];
    uint8_t       arg_count;
    EffectArg     scratch[MAX_EFFECT_SCRATCH];
    int16_t       duration_turns;
    Side          owner;
    uint32_t      source_id;
} Effect;

/// EFF
///
/// Compact macro for the func_id + apply pair inside an Effect
/// literal. Expands to two designated initializers; intended use:
/// `{.trigger = TRIGGER_X, EFF(NAME, name)}`. NAME is the upper-
/// case EffectFuncId suffix; name is the lower-case handler suffix
/// (the C preprocessor cannot case-convert tokens, so both forms
/// are passed explicitly).
///
#define EFF(NAME, name) .func_id = EFFECT_FUNC_##NAME, .apply = eff_##name

/// EffectSlot
///
/// Bus-internal entry owning an Effect by value plus its remaining
/// turns and active flag. Owning by value lets handlers update each
/// effect's `scratch[]` slots through `EffectCtx.self`. Slots are
/// never freed; they are deactivated when their effect expires.
///
typedef struct {
    Effect  effect;
    int16_t remaining_turns;
    bool    active;
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
    EffectTrigger       trigger;
    struct BattleState* battle;
    struct Effect*      self;
    union {
        struct {
            struct PieceState* attacker;
            struct PieceState* target;
            int*               damage_out;
            int*               damage_mult_out;
            int*               reduction_out;
        } resolve;
        struct {
            struct PieceState* piece;
            Position*          pos;
            PlacementCause     cause;
        } placed;
        struct {
            struct PieceState* piece;
            Side               old_owner;
            Side               new_owner;
            FlipCause          cause;
        } flipped;
        struct {
            struct PieceState* piece;
            RemovalCause       cause;
        } removed;
        struct {
            struct PieceState* piece;
            Position*          pos;
        } piece;
        struct {
            Side          side;
            CardInstance* card;
            int*          count_out;
            int*          value_out;
        } card;
        struct {
            int*     income_out;
            int*     cost_out;
            int*     moves_out;
            int*     meter_cap_out;
            int*     reclaim_cost_out;
            int*     royal_sub_count_out;
            int*     vision_flags_out;
            uint16_t template_id;
            Side     side;
            Tier     tier;
        } query;
        struct {
            struct PieceState* piece;
            MoveGen*           move_out;
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
    uint16_t   count;
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
/// in registration order; the caller fills `context->as` with the union
/// arm matching the trigger before calling.
///
/// Params:
/// - EffectBus         *bus     -> bus to scan
/// - struct BattleState *battle     -> battle state for handler use
/// - EffectTrigger      trigger -> trigger key to dispatch
/// - struct EffectCtx  *context     -> evidence (mutated with trigger)
///
void bus_emit(
    EffectBus*          bus,
    struct BattleState* battle,
    EffectTrigger       trigger,
    struct EffectCtx*   context
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
size_t bus_query_count(const EffectBus* bus, EffectTrigger trigger);

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

/// bus_scratch_for
///
/// Return a writable pointer to the scratch[] array of the first
/// active slot whose source_id matches. Used when a source (relic,
/// chain, innate) registers multiple effects that need shared
/// per-source state. The first registration owns the storage;
/// subsequent registrations look it up via this helper.
///
/// Params:
/// - EffectBus* bus       -> bus to scan
/// - uint32_t   source_id -> source identifier to match
///
/// Return:
/// EffectArg* -> pointer to scratch[0] of the matching slot, or NULL
///               if no active slot matches
///
EffectArg* bus_scratch_for(EffectBus* bus, uint32_t source_id);

/*--------------------------------------------------------------------------*\
                              FUNC LOOKUP
\*--------------------------------------------------------------------------*/

/// eff_lookup
///
/// Map an `EffectFuncId` back to its live `EffectFunc` pointer via
/// the registry table in `src/effects/eff_registry.c`. Returns NULL
/// (and logs a warning) for out-of-range ids. Used by the save/load
/// codec to rehydrate `Effect.apply` after deserialization.
///
/// Params:
/// - EffectFuncId id -> id to look up
///
/// Return:
/// EffectFunc -> live function pointer, or NULL on bad id
///
EffectFunc eff_lookup(EffectFuncId id);

/*--------------------------------------------------------------------------*\
                              SHARED PLACEHOLDER
\*--------------------------------------------------------------------------*/

/// eff_todo
///
/// Universal stand-in used as the `.apply` field for any effect
/// whose behaviour is not yet implemented. Discards every argument
/// and returns without mutating state.
///
/// Params:
/// - struct EffectCtx *context -> ignored
/// - const EffectArg  *args -> ignored
/// - size_t            count   -> ignored
///
/// Notes:
/// Defined exactly once in src/effects/eff_run.c. Never reintroduce
/// per-file static duplicates — the stand-in must remain a single
/// symbol so counting unimplemented effects stays trivial.
///
void eff_todo(struct EffectCtx* context, const EffectArg* args, size_t count);

#endif /* EFFECT_H */
