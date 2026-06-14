//! data.h
//!
//! Public extern declarations for every static data array compiled
//! out of src/data/. Keeping these here lets registry.c (and the
//! rest of the engine) reach the tables without redeclaring them
//! inline, which would otherwise read as forward declarations.
//!
//! Each pair is `extern const TYPE NAME[]` plus a matching
//! `extern const size_t NAME_COUNT` so callers can iterate the array
//! without baking sizes into the source.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef DATA_H
#define DATA_H

#include "piece.h"
#include "card.h"
#include "recipe.h"
#include "meta.h"
#include "ai.h"

/*--------------------------------------------------------------------------*\
                              PIECE TABLES
\*--------------------------------------------------------------------------*/

extern const PieceTemplate PIECES_LONGWEI[];
extern const size_t PIECES_LONGWEI_COUNT;

extern const PieceTemplate PIECES_HARUSHIMA[];
extern const size_t PIECES_HARUSHIMA_COUNT;

extern const PieceTemplate PIECES_KEWARANI[];
extern const size_t PIECES_KEWARANI_COUNT;

extern const PieceTemplate PIECES_ZARQAN[];
extern const size_t PIECES_ZARQAN_COUNT;

extern const PieceTemplate PIECES_CAELAN[];
extern const size_t PIECES_CAELAN_COUNT;

extern const PieceTemplate PIECES_UNIVERSAL[];
extern const size_t PIECES_UNIVERSAL_COUNT;

/*--------------------------------------------------------------------------*\
                              CARD TABLES
\*--------------------------------------------------------------------------*/

extern const CardTemplate CARDS_UNIVERSAL[];
extern const size_t CARDS_UNIVERSAL_COUNT;

extern const CardTemplate CARDS_LONGWEI[];
extern const size_t CARDS_LONGWEI_COUNT;

extern const CardTemplate CARDS_HARUSHIMA[];
extern const size_t CARDS_HARUSHIMA_COUNT;

extern const CardTemplate CARDS_KEWARANI[];
extern const size_t CARDS_KEWARANI_COUNT;

extern const CardTemplate CARDS_ZARQAN[];
extern const size_t CARDS_ZARQAN_COUNT;

extern const CardTemplate CARDS_CAELAN[];
extern const size_t CARDS_CAELAN_COUNT;

/*--------------------------------------------------------------------------*\
                              SUPPORTING TABLES
\*--------------------------------------------------------------------------*/

extern const Recipe RECIPES[];
extern const size_t RECIPES_COUNT;

extern const RelicTemplate RELICS[];
extern const size_t RELICS_COUNT;

extern const InnateTemplate INNATES[];
extern const size_t INNATES_COUNT;

extern const Chain CHAINS[];
extern const size_t CHAINS_COUNT;

extern const Modifier MODIFIERS[];
extern const size_t MODIFIERS_COUNT;

extern const BoardTrait TRAITS[];
extern const size_t TRAITS_COUNT;

extern const Synergy SYNERGIES[];
extern const size_t SYNERGIES_COUNT;

extern const FigureheadPower FIGUREHEADS[];
extern const size_t FIGUREHEADS_COUNT;

extern const Overseer OVERSEERS[];
extern const size_t OVERSEERS_COUNT;

extern const AIArchetype ARCHETYPES[];
extern const size_t ARCHETYPES_COUNT;

extern const EventTemplate EVENTS[];
extern const size_t EVENTS_COUNT;

extern const MasteryCard MASTERY_CARDS[];
extern const size_t MASTERY_CARDS_COUNT;

extern const MasteryHook MASTERY_HOOKS[];
extern const size_t MASTERY_HOOKS_COUNT;

#endif /* DATA_H */
