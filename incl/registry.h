//!
//! Registry: accessors for all static template arrays by id.
//! Every lookup is O(1) via dense array indexing with id-range ladders.
//! Declarations live in incl/data.h; implementations in src/registry.c.
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

const PieceTemplate* piece_template(uint16_t id);
size_t piece_count(void);

/*--------------------------------------------------------------------------*\
                              CARD REGISTRY
\*--------------------------------------------------------------------------*/

const CardTemplate* card_template(uint16_t id);
size_t card_count(void);

/*--------------------------------------------------------------------------*\
                              RECIPE REGISTRY
\*--------------------------------------------------------------------------*/

const Recipe* recipe_find(uint16_t a, uint16_t b);
size_t recipe_count(void);

/*--------------------------------------------------------------------------*\
                              RELIC REGISTRY
\*--------------------------------------------------------------------------*/

const RelicTemplate* relic_template(uint16_t id);
size_t relic_count(void);

/*--------------------------------------------------------------------------*\
                              INNATE REGISTRY
\*--------------------------------------------------------------------------*/

const InnateTemplate* innate_template(Kingdom k);

/*--------------------------------------------------------------------------*\
                              CHAIN REGISTRY
\*--------------------------------------------------------------------------*/

const Chain* chain_template(uint8_t level);

/*--------------------------------------------------------------------------*\
                              MODIFIER REGISTRY
\*--------------------------------------------------------------------------*/

const Modifier* modifier_template(uint16_t id);
size_t modifier_count(void);

/*--------------------------------------------------------------------------*\
                              TRAIT REGISTRY
\*--------------------------------------------------------------------------*/

const BoardTrait* trait_template(uint16_t id);
size_t trait_count(void);

/*--------------------------------------------------------------------------*\
                              FIGUREHEAD REGISTRY
\*--------------------------------------------------------------------------*/

const FigureheadPower* figurehead_power(Kingdom k);

/*--------------------------------------------------------------------------*\
                              SYNERGY REGISTRY
\*--------------------------------------------------------------------------*/

const Synergy* synergy_template(Kingdom cleared);

/*--------------------------------------------------------------------------*\
                              AI ARCHETYPE REGISTRY
\*--------------------------------------------------------------------------*/

const AIArchetype* ai_archetype(Kingdom k);
size_t ai_archetype_count(void);

#endif /* REGISTRY_H */