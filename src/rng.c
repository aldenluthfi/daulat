//! rng.c
//!
//! Deterministic xorshift64 pseudo-RNG for the Regnum battle engine.
//! All randomness flows through this so re-runs are byte-identical.
//! SplitMix64-style rejection sampling for rng_bound to avoid modulo bias.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/// rng_init
///
/// Seed the RNG state. Pass 0 to get a deterministic fallback of 1.
///
/// Params:
/// - Rng* rng -> RNG state to seed
/// - uint64_t seed -> initial state value
///
void rng_init(Rng* rng, uint64_t seed) {
    rng->state = (seed != 0) ? seed : 1;
}

/// rng_next
///
/// Advance state and return the raw xorshift64 output.
///
/// Params:
/// - Rng* rng -> RNG state to advance
///
/// Return:
/// uint64_t -> raw random value
///
uint64_t rng_next(Rng* rng) {
    uint64_t value = rng->state;
    value ^= value >> 12;
    value ^= value << 25;
    value ^= value >> 27;
    rng->state = value;
    return value * 0x2545F4914F6CDD1DULL;
}

/// rng_bound
///
/// Return a value in [0, bound) using rejection sampling.
///
/// Params:
/// - Rng* rng -> RNG state to sample from
/// - uint64_t bound -> exclusive upper bound
///
/// Return:
/// uint64_t -> value in [0, bound)
///
uint64_t rng_bound(Rng* rng, uint64_t bound) {
    uint64_t mask = bound - 1;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;
    mask |= mask >> 32;
    uint64_t value;
    do {
        value = rng_next(rng) & mask;
    } while (value >= bound);
    return value;
}

/// rng_range
///
/// Alias for rng_bound for uniform naming with the rest of the API.
///
/// Params:
/// - Rng* rng -> RNG state to sample from
/// - uint64_t max -> exclusive upper bound
///
/// Return:
/// uint64_t -> value in [0, max)
///
uint64_t rng_range(Rng* rng, uint64_t max) {
    return rng_bound(rng, max);
}

/// rng_float
///
/// Return a float in [0.0f, 1.0f) using the top 24 bits of state.
///
/// Params:
/// - Rng* rng -> RNG state to sample from
///
/// Return:
/// float -> uniform random float in [0.0f, 1.0f)
///
float rng_float(Rng* rng) {
    return (float)(rng_next(rng) >> 40) / (float)(1 << 24);
}
