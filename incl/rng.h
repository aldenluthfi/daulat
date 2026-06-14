//!
//! Deterministic xorshift64 PRNG for reproducible battle traces.
//! All randomness flows through this so re-runs are byte-identical.
//! SplitMix64-style rejection sampling for rng_bound to avoid modulo bias.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef RNG_H
#define RNG_H

#include <stdint.h>

/*--------------------------------------------------------------------------*\
                              RNG STATE
\*--------------------------------------------------------------------------*/

/// Xorshift64 state. Seed with cfg->rng_seed in battle_init().
typedef struct {
    uint64_t state;
} Rng;

/*--------------------------------------------------------------------------*\
                              RNG API
\*--------------------------------------------------------------------------*/

/// Initialize RNG with a seed.
void rng_init(Rng* rng, uint64_t seed);

/// Advance state and return a uint64_t.
uint64_t rng_next(Rng* rng);

/// Return a uint64_t in [0, max). Never returns max itself.
uint64_t rng_range(Rng* rng, uint64_t max);

/// Return a float in [0.0f, 1.0f).
float rng_float(Rng* rng);

/// Return true with probability p (0.0 to 1.0).
bool rng_bool(Rng* rng, float p);

#endif /* RNG_H */
