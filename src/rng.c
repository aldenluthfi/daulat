//! rng.c
//!
//! Deterministic xorshift64 pseudo-RNG for the Regnum battle engine.
//! All randomness flows through this so re-runs are byte-identical.
//! SplitMix64-style rejection sampling for rng_bound to avoid modulo bias.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

void rng_init(Rng* rng, uint64_t seed) {
    rng->state = (seed != 0) ? seed : 1;
}

uint64_t rng_next(Rng* rng) {
    uint64_t x = rng->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng->state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

uint64_t rng_bound(Rng* rng, uint64_t bound) {
    uint64_t mask = bound - 1;
    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16;
    mask |= mask >> 32;
    uint64_t x;
    do {
        x = rng_next(rng) & mask;
    } while (x >= bound);
    return x;
}

uint64_t rng_range(Rng* rng, uint64_t max) {
    return rng_bound(rng, max);
}

float rng_float(Rng* rng) {
    return (float)(rng_next(rng) >> 40) / (float)(1 << 24);
}
