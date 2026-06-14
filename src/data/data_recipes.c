//! data_recipes.c
//!
//! Static combination-recipe table. Each entry pairs two ingredient
//! piece ids with a result piece id and the recipe depth (1 for a
//! direct combination, 2 for a recipe whose ingredient is itself a
//! combo result).
//!
//! All recipes are order-agnostic — recipe_find() in recipe.c checks
//! both (a,b) and (b,a). The result piece occupies the second
//! selected piece's square and inherits its result's kingdom.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              RECIPES
\*--------------------------------------------------------------------------*/

/// RECIPES
///
/// Linear recipe table. Iteration follows the GDD kingdom order
/// (Longwei, Harushima, Kewarani, Zarqan, Caelan) so balance edits
/// stay locality-friendly. recipe_find() does a linear scan; the
/// table is small enough that no indexing structure is warranted.
///
const Recipe RECIPES[] = {
    {PIECE_XIANG, PIECE_MA, PIECE_SANG, 1},
    {PIECE_MA, PIECE_PAO, PIECE_NORTHERN_CAVALRY, 1},
    {PIECE_BING, PIECE_PAO, PIECE_HWACHA, 1},
    {PIECE_GINSHO, PIECE_KINSHO, PIECE_SHISHI, 1},
    {PIECE_KYOSHA, PIECE_FUHYO, PIECE_HONORABLE_HORSE, 1},
    {PIECE_BISHOP, PIECE_ROOK, PIECE_PROMOTED_BISHOP, 1},
    {PIECE_KNIGHT, PIECE_ROOK, PIECE_DRAGON, 1},
    {PIECE_MAKWANAM, PIECE_SABA, PIECE_FARAS, 1},
    {PIECE_FARAS, PIECE_NEGUS_GUARD, PIECE_MEDEQ_SQUAD, 1},
    {PIECE_WAZIR, PIECE_JAMAL, PIECE_CATAPHRACT, 1},
    {PIECE_TALLIYA, PIECE_WAZIR, PIECE_ZIRAAFA, 1},
    {PIECE_CATAPHRACT, PIECE_ZIRAAFA, PIECE_SHAHZADEH, 1},
    {PIECE_WAZIR, PIECE_OLD_KING, PIECE_WAR_ELEPHANT, 1},
    {PIECE_KNIGHT, PIECE_BISHOP, PIECE_CHANCELLOR, 1},
    {PIECE_BISHOP, PIECE_KNIGHT, PIECE_GRYPHON, 1},
};

const size_t RECIPES_COUNT = sizeof(RECIPES) / sizeof(RECIPES[0]);
