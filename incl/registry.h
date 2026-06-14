//! registry.h
//!
//! Registry: accessors for all static template arrays by id.
//! Every lookup is O(1) via dense array indexing with id-range ladders.
//! Declarations live in incl/data.h; implementations in src/registry.c.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef REGISTRY_H
#define REGISTRY_H

#include "ai.h"
#include "card.h"
#include "meta.h"
#include "piece.h"
#include "recipe.h"

/*--------------------------------------------------------------------------*\
                              PIECE REGISTRY
\*--------------------------------------------------------------------------*/

/// piece_template
///
/// Look up a piece template by id.
///
/// Params:
/// - uint16_t id -> piece template id
///
/// Return:
/// const PieceTemplate* -> template or NULL
///
const PieceTemplate* piece_template(uint16_t id);

/// piece_count
///
/// Return the total number of distinct PieceId enumerators.
///
/// Return:
/// size_t -> piece id cardinality
///
size_t piece_count(void);

/*--------------------------------------------------------------------------*\
                              CARD REGISTRY
\*--------------------------------------------------------------------------*/

/// card_template
///
/// Look up a card template by id.
///
/// Params:
/// - uint16_t id -> card template id
///
/// Return:
/// const CardTemplate* -> template or NULL
///
const CardTemplate* card_template(uint16_t id);

/// card_count
///
/// Return the total number of distinct CardId enumerators.
///
/// Return:
/// size_t -> card id cardinality
///
size_t card_count(void);

/*--------------------------------------------------------------------------*\
                              RECIPE REGISTRY
\*--------------------------------------------------------------------------*/

/// recipe_find
///
/// Look up a combination recipe by ingredient ids.
///
/// Params:
/// - uint16_t a -> first ingredient piece id
/// - uint16_t b -> second ingredient piece id
///
/// Return:
/// const Recipe* -> matching recipe or NULL
///
const Recipe* recipe_find(uint16_t a, uint16_t b);

/// recipe_count
///
/// Return the number of combination recipes.
///
/// Return:
/// size_t -> recipe array length
///
size_t recipe_count(void);

/*--------------------------------------------------------------------------*\
                              RELIC REGISTRY
\*--------------------------------------------------------------------------*/

/// relic_template
///
/// Look up a relic template by id.
///
/// Params:
/// - uint16_t id -> relic template id
///
/// Return:
/// const RelicTemplate* -> template or NULL
///
const RelicTemplate* relic_template(uint16_t id);

/// relic_count
///
/// Return the number of relics defined.
///
/// Return:
/// size_t -> relic array length
///
size_t relic_count(void);

/*--------------------------------------------------------------------------*\
                              INNATE REGISTRY
\*--------------------------------------------------------------------------*/

/// innate_template
///
/// Look up the innate ability for a kingdom.
///
/// Params:
/// - Kingdom k -> kingdom to look up
///
/// Return:
/// const InnateTemplate* -> template or NULL
///
const InnateTemplate* innate_template(Kingdom k);

/*--------------------------------------------------------------------------*\
                              CHAIN REGISTRY
\*--------------------------------------------------------------------------*/

/// chain_template
///
/// Look up the penalty chain for a level.
///
/// Params:
/// - uint8_t level -> chain level (0=Bronze, 1=Silver, 2=Gold)
///
/// Return:
/// const Chain* -> chain or NULL
///
const Chain* chain_template(uint8_t level);

/*--------------------------------------------------------------------------*\
                              MODIFIER REGISTRY
\*--------------------------------------------------------------------------*/

/// modifier_template
///
/// Look up a modifier template by id.
///
/// Params:
/// - uint16_t id -> modifier template id
///
/// Return:
/// const Modifier* -> template or NULL
///
const Modifier* modifier_template(uint16_t id);

/// modifier_count
///
/// Return the number of modifiers defined.
///
/// Return:
/// size_t -> modifier array length
///
size_t modifier_count(void);

/*--------------------------------------------------------------------------*\
                              TRAIT REGISTRY
\*--------------------------------------------------------------------------*/

/// trait_template
///
/// Look up a board trait template by id.
///
/// Params:
/// - uint16_t id -> trait template id
///
/// Return:
/// const BoardTrait* -> template or NULL
///
const BoardTrait* trait_template(uint16_t id);

/// trait_count
///
/// Return the number of board traits defined.
///
/// Return:
/// size_t -> trait array length
///
size_t trait_count(void);

/*--------------------------------------------------------------------------*\
                              FIGUREHEAD REGISTRY
\*--------------------------------------------------------------------------*/

/// figurehead_power
///
/// Look up the figurehead power for a kingdom.
///
/// Params:
/// - Kingdom k -> kingdom to look up
///
/// Return:
/// const FigureheadPower* -> power or NULL
///
const FigureheadPower* figurehead_power(Kingdom k);

/*--------------------------------------------------------------------------*\
                              SYNERGY REGISTRY
\*--------------------------------------------------------------------------*/

/// synergy_template
///
/// Look up the synergy bonus for a cleared kingdom.
///
/// Params:
/// - Kingdom cleared -> cleared kingdom
///
/// Return:
/// const Synergy* -> synergy or NULL
///
const Synergy* synergy_template(Kingdom cleared);

/*--------------------------------------------------------------------------*\
                              AI ARCHETYPE REGISTRY
\*--------------------------------------------------------------------------*/

/// ai_archetype
///
/// Look up the AI archetype for a kingdom.
///
/// Params:
/// - Kingdom k -> kingdom to look up
///
/// Return:
/// const AIArchetype* -> archetype or NULL
///
const AIArchetype* ai_archetype(Kingdom k);

/// ai_archetype_count
///
/// Return the number of archetypes defined.
///
/// Return:
/// size_t -> archetype array length
///
size_t ai_archetype_count(void);

#endif /* REGISTRY_H */