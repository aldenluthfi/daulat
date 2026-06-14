//!
//! Combination recipes: what two pieces combine into, if anything.
//! Order-agnostic lookup — (a, b) == (b, a).
//! recipe_find scans the RECIPES array for a matching (a, b) pair.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef RECIPE_H
#define RECIPE_H

#include "defs.h"

/*--------------------------------------------------------------------------*\
                              RECIPE
\*--------------------------------------------------------------------------*/

/// A combination of two piece templates into a result template.
typedef struct {
    uint16_t ingredient_a;
    uint16_t ingredient_b;
    uint16_t result;
    uint8_t  steps; /* 1 direct, 2 multi-step */
} Recipe;

extern const Recipe RECIPES[];
extern const size_t RECIPES_COUNT;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// Look up a recipe. Returns NULL if no combination exists.
const Recipe* recipe_find(uint16_t a, uint16_t b);

#endif /* RECIPE_H */