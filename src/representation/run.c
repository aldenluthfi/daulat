//! run.c
//!
//! Run-scale campaign logic. Owns the static map layout tables, map
//! generation, node selection, unlock schedules, chain and liberation
//! bookkeeping, the Vorath counter, events, offerings, and relic picks.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// rng_mix
///
/// Mixes a seed with a salt into a new deterministic seed using a
/// splitmix-style hash, so every campaign subsystem draws from its own
/// independent stream.
///
/// Params:
/// - seed -> base seed to mix
/// - salt -> stream identifier to mix in
///
/// Return: mixed seed
///
size_t rng_mix(size_t seed, size_t salt) {
    size_t mixed = seed + salt * 0x9E3779B97F4A7C15;

    mixed = (mixed ^ (mixed >> 30)) * 0xBF58476D1CE4E5B9;
    mixed = (mixed ^ (mixed >> 27)) * 0x94D049BB133111EB;

    return mixed ^ (mixed >> 31);
}

/// run_new
///
/// Allocates and initializes a fresh run on the engine: seeds, kingdom
/// states, all fifteen campaign maps, starting unlocks, difficulty, and
/// the optional challenge.
///
/// Params:
/// - engine     -> engine to attach the run to
/// - seed       -> run seed for all deterministic draws
/// - difficulty -> difficulty of the run
/// - challenge  -> challenge modifier, CHALLENGE_SENTINEL for none
///
void run_new(EngineState* engine, size_t seed, Difficulty difficulty,
             ChallengeRunID challenge) {
    if (engine->run) {
        run_free(engine->run);
    }

    RunState* run = calloc(1, sizeof(RunState));

    run->seed       = seed ? seed : RNG_SEED;
    run->difficulty = difficulty;
    run->challenge  = challenge;

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        run->kingdoms[kingdom].id      = (KingdomID) kingdom;
        run->kingdoms[kingdom].mastery = engine->masteries[kingdom];
    }

    run->pieces[PIECE_PAWN]   = true;
    run->pieces[PIECE_KNIGHT] = true;
    run->pieces[PIECE_BISHOP] = true;

    engine->run = run;
}

/// run_free
///
/// Frees everything the run owns including all campaign maps and their
/// nodes, then the run itself.
///
/// Params:
/// - run -> run to deallocate
///
void run_free(RunState* run) {
    free(run);
}

/// run_enter_map
///
/// Enters the given kingdom's active map, applying that tier's
/// idempotent unlock schedule on first entry.
///
/// Params:
/// - engine  -> engine owning the run
/// - kingdom -> kingdom whose active map is entered
///
void run_enter_map(EngineState* engine, KingdomID kingdom) {
    (void) engine;
    (void) kingdom;
}

/// run_select_node
///
/// Selects a node on the current map when it is reachable from the
/// cleared frontier, dispatching per node type into a battle, event,
/// offering, archive, or relic offer.
///
/// Params:
/// - engine -> engine owning the run
/// - index  -> node index on the current map
///
/// Return: true when the node was selectable
///
bool run_select_node(EngineState* engine, size_t index) {
    (void) engine;
    (void) index;

    return false;
}

/// run_battle_result
///
/// Applies a finished battle to the run: node clearing, chain and
/// liberation bookkeeping, the Vorath counter with its thresholds,
/// overseer rewards, and run completion on a Vorath win.
///
/// Params:
/// - engine -> engine owning the run
/// - won    -> whether the player won the battle
///
void run_battle_result(EngineState* engine, bool won) {
    (void) engine;
    (void) won;
}

/// run_event_choose
///
/// Resolves the pending narrative event with the player's choice,
/// dispatching to the owning kingdom's event handler.
///
/// Params:
/// - engine -> engine owning the run
/// - choice -> choice taken by the player
///
void run_event_choose(EngineState* engine, EventChoice choice) {
    (void) engine;
    (void) choice;
}

/// run_offering
///
/// Removes the chosen card from the run's draw pool at an offering
/// node.
///
/// Params:
/// - engine -> engine owning the run
/// - card   -> card to remove from the pool
///
void run_offering(EngineState* engine, CardID card) {
    (void) engine;
    (void) card;
}

/// run_relic_pick
///
/// Grants the chosen relic from a pending elite or overseer offer.
///
/// Params:
/// - engine -> engine owning the run
/// - relic  -> relic chosen by the player
///
void run_relic_pick(EngineState* engine, RelicID relic) {
    (void) engine;
    (void) relic;
}

/// run_pressure
///
/// Computes the enemy pressure for battles in the kingdom, scaling the
/// free pieces the enemy starts with as the run progresses.
///
/// Params:
/// - run     -> run to measure
/// - kingdom -> kingdom the battle takes place in
///
/// Return: pressure level for the kingdom
///
size_t run_pressure(RunState* run, KingdomID kingdom) {
    (void) run;
    (void) kingdom;

    return 0;
}
