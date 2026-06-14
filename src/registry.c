//! registry.c
//!
//! O(1) accessor implementations for every static template table.
//! Each function in this file pairs with a declaration in
//! registry.h and a backing array declared in incl/data.h that lives
//! in the matching src/data/data_*.c translation unit.
//!
//! Lookups that walk multiple arrays (piece_template, card_template)
//! gate on the id range each kingdom owns so the function returns in
//! constant time after a small ladder of integer comparisons.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              PIECE
\*--------------------------------------------------------------------------*/

/// piece_template
///
/// Return the immutable PieceTemplate for the given piece id, or
/// NULL when the id falls outside every known range. The King is
/// served from the universal table; each kingdom owns a contiguous
/// id range.
///
/// Params:
/// - uint16_t id -> piece id to look up
///
/// Return:
/// const PieceTemplate * -> template pointer or NULL
///
const PieceTemplate* piece_template(uint16_t id) {
    if (id == PIECE_KING)
        return &PIECES_UNIVERSAL[0];
    if (id >= PIECE_BING && id <= PIECE_HWACHA) {
        return &PIECES_LONGWEI[id - PIECE_BING];
    }
    if (id >= PIECE_FUHYO && id <= PIECE_DRAGON) {
        return &PIECES_HARUSHIMA[id - PIECE_FUHYO];
    }
    if (id >= PIECE_MEDEQ && id <= PIECE_SULTANS_LEVY) {
        return &PIECES_KEWARANI[id - PIECE_MEDEQ];
    }
    if (id >= PIECE_WAZIR && id <= PIECE_WAR_ELEPHANT) {
        return &PIECES_ZARQAN[id - PIECE_WAZIR];
    }
    if (id >= PIECE_PAWN && id <= PIECE_SOVEREIGN_BANNER) {
        return &PIECES_CAELAN[id - PIECE_PAWN];
    }
    return NULL;
}

/// piece_count
///
/// Return the total number of distinct PieceId enumerators. Used by
/// callers that need to size piece-id-keyed containers.
///
/// Return:
/// size_t -> piece id cardinality
///
size_t piece_count(void) { return PIECE_ID_COUNT; }

/*--------------------------------------------------------------------------*\
                              CARD
\*--------------------------------------------------------------------------*/

/// card_template
///
/// Return the immutable CardTemplate for the given card id, or NULL
/// when the id falls outside every known range. Universal cards
/// sit first; each kingdom owns a contiguous id range thereafter.
///
/// Params:
/// - uint16_t id -> card id to look up
///
/// Return:
/// const CardTemplate * -> template pointer or NULL
///
const CardTemplate* card_template(uint16_t id) {
    if (id >= CARD_PAWN_STORM && id <= CARD_HYDRA) {
        return &CARDS_UNIVERSAL[id - CARD_PAWN_STORM];
    }
    if (id >= CARD_RIVER_WADE && id <= CARD_WISDOM) {
        return &CARDS_LONGWEI[id - CARD_RIVER_WADE];
    }
    if (id >= CARD_RONIN && id <= CARD_REVENGE) {
        return &CARDS_HARUSHIMA[id - CARD_RONIN];
    }
    if (id >= CARD_SULTANS_GOLD && id <= CARD_HAJJ) {
        return &CARDS_KEWARANI[id - CARD_SULTANS_GOLD];
    }
    if (id >= CARD_COUNSEL && id <= CARD_TREACHERY) {
        return &CARDS_ZARQAN[id - CARD_COUNSEL];
    }
    if (id >= CARD_CASTLING && id <= CARD_DIVINE_INTERVENTION) {
        return &CARDS_CAELAN[id - CARD_CASTLING];
    }
    return NULL;
}

/// card_count
///
/// Return the total number of distinct CardId enumerators.
///
/// Return:
/// size_t -> card id cardinality
///
size_t card_count(void) { return CARD_ID_COUNT; }

/*--------------------------------------------------------------------------*\
                              RECIPE
\*--------------------------------------------------------------------------*/

/// recipe_count
///
/// Return the number of combination recipes the engine knows about.
/// recipe_find() in recipe.c handles the actual matching.
///
/// Return:
/// size_t -> recipe array length
///
size_t recipe_count(void) { return RECIPES_COUNT; }

/*--------------------------------------------------------------------------*\
                              RELIC
\*--------------------------------------------------------------------------*/

/// relic_template
///
/// Return the immutable RelicTemplate for the given relic id, or
/// NULL when the id is out of range.
///
/// Params:
/// - uint16_t id -> relic id to look up
///
/// Return:
/// const RelicTemplate * -> template pointer or NULL
///
const RelicTemplate* relic_template(uint16_t id) {
    if (id >= RELICS_COUNT)
        return NULL;
    return &RELICS[id];
}

/// relic_count
///
/// Return the number of relics defined in the relic table.
///
/// Return:
/// size_t -> relic array length
///
size_t relic_count(void) { return RELICS_COUNT; }

/*--------------------------------------------------------------------------*\
                              INNATE
\*--------------------------------------------------------------------------*/

/// innate_template
///
/// Return the innate template for the given kingdom, or NULL if no
/// innate is registered. Lookup is a linear scan over the five
/// kingdom entries.
///
/// Params:
/// - Kingdom k -> kingdom to look up
///
/// Return:
/// const InnateTemplate * -> template pointer or NULL
///
const InnateTemplate* innate_template(Kingdom k) {
    for (size_t i = 0; i < INNATES_COUNT; i++) {
        if (INNATES[i].kingdom == k)
            return &INNATES[i];
    }
    return NULL;
}

/*--------------------------------------------------------------------------*\
                              CHAIN
\*--------------------------------------------------------------------------*/

/// chain_template
///
/// Return the penalty chain matching the given level (1=Bronze,
/// 2=Silver, 3=Gold), or NULL if no entry exists.
///
/// Params:
/// - uint8_t level -> chain level to look up
///
/// Return:
/// const Chain * -> chain pointer or NULL
///
const Chain* chain_template(uint8_t level) {
    for (size_t i = 0; i < CHAINS_COUNT; i++) {
        if (CHAINS[i].level == level)
            return &CHAINS[i];
    }
    return NULL;
}

/*--------------------------------------------------------------------------*\
                              MODIFIER
\*--------------------------------------------------------------------------*/

/// modifier_template
///
/// Return the modifier template for the given id, or NULL when the
/// id is out of range.
///
/// Params:
/// - uint16_t id -> modifier id to look up
///
/// Return:
/// const Modifier * -> template pointer or NULL
///
const Modifier* modifier_template(uint16_t id) {
    if (id >= MODIFIERS_COUNT)
        return NULL;
    return &MODIFIERS[id];
}

/// modifier_count
///
/// Return the number of modifiers defined in the modifier table.
///
/// Return:
/// size_t -> modifier array length
///
size_t modifier_count(void) { return MODIFIERS_COUNT; }

/*--------------------------------------------------------------------------*\
                              TRAIT
\*--------------------------------------------------------------------------*/

/// trait_template
///
/// Return the board-trait template for the given id, or NULL when
/// the id is out of range.
///
/// Params:
/// - uint16_t id -> trait id to look up
///
/// Return:
/// const BoardTrait * -> template pointer or NULL
///
const BoardTrait* trait_template(uint16_t id) {
    if (id >= TRAITS_COUNT)
        return NULL;
    return &TRAITS[id];
}

/// trait_count
///
/// Return the number of board traits defined in the trait table.
///
/// Return:
/// size_t -> trait array length
///
size_t trait_count(void) { return TRAITS_COUNT; }

/*--------------------------------------------------------------------------*\
                              SYNERGY
\*--------------------------------------------------------------------------*/

/// synergy_template
///
/// Return the synergy template for the given cleared kingdom, or
/// NULL when no synergy is registered for that kingdom.
///
/// Params:
/// - Kingdom cleared -> kingdom whose overseer was beaten
///
/// Return:
/// const Synergy * -> synergy pointer or NULL
///
const Synergy* synergy_template(Kingdom cleared) {
    for (size_t i = 0; i < SYNERGIES_COUNT; i++) {
        if (SYNERGIES[i].cleared == cleared)
            return &SYNERGIES[i];
    }
    return NULL;
}

/*--------------------------------------------------------------------------*\
                              FIGUREHEAD
\*--------------------------------------------------------------------------*/

/// figurehead_power
///
/// Return the figurehead power belonging to the given kingdom, or
/// NULL when no entry exists.
///
/// Params:
/// - Kingdom k -> kingdom to look up
///
/// Return:
/// const FigureheadPower * -> power pointer or NULL
///
const FigureheadPower* figurehead_power(Kingdom k) {
    for (size_t i = 0; i < FIGUREHEADS_COUNT; i++) {
        if (FIGUREHEADS[i].kingdom == k)
            return &FIGUREHEADS[i];
    }
    return NULL;
}

/*--------------------------------------------------------------------------*\
                              OVERSEER
\*--------------------------------------------------------------------------*/

/// overseer_template
///
/// Return the Overseer template for the given id, or NULL when the
/// id is out of range.
///
/// Params:
/// - uint16_t id -> overseer id to look up
///
/// Return:
/// const Overseer * -> overseer pointer or NULL
///
const Overseer* overseer_template(uint16_t id) {
    if (id >= OVERSEERS_COUNT)
        return NULL;
    return &OVERSEERS[id];
}

/*--------------------------------------------------------------------------*\
                              ARCHETYPE
\*--------------------------------------------------------------------------*/

/// ai_archetype
///
/// Return the AI archetype matching the given kingdom, or NULL when
/// no archetype is registered.
///
/// Params:
/// - Kingdom k -> kingdom to look up
///
/// Return:
/// const AIArchetype * -> archetype pointer or NULL
///
const AIArchetype* ai_archetype(Kingdom k) {
    for (size_t i = 0; i < ARCHETYPES_COUNT; i++) {
        if (ARCHETYPES[i].kingdom == k)
            return &ARCHETYPES[i];
    }
    return NULL;
}

/// ai_archetype_count
///
/// Return the number of archetypes defined in the archetype table.
///
/// Return:
/// size_t -> archetype array length
///
size_t ai_archetype_count(void) { return ARCHETYPES_COUNT; }
