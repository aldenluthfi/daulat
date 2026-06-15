//! recipe.c
//!
//! Combination recipe lookup.
//! Recipes are order-agnostic (A+B == B+A). Dense lookup table.
//! recipe_find scans the RECIPES array for a matching (a, b) pair.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              LOOKUP
\*--------------------------------------------------------------------------*/

/// recipe_find
///
/// Look up a combination recipe by ingredient ids. Order-agnostic.
///
/// Params:
/// - uint16_t ingredient_a -> first ingredient piece id
/// - uint16_t ingredient_b -> second ingredient piece id
///
/// Return:
/// const Recipe* -> matching recipe or NULL if no combination exists
///
const Recipe* recipe_find(uint16_t ingredient_a, uint16_t ingredient_b) {
    for (size_t i = 0; i < RECIPES_COUNT; i++) {
        const Recipe* r = &RECIPES[i];
        if ((r->ingredient_a == ingredient_a && r->ingredient_b == ingredient_b) ||
            (r->ingredient_a == ingredient_b && r->ingredient_b == ingredient_a)) {
            return r;
        }
    }
    return NULL;
}
