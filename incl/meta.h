//!
//! Meta layer: relics, innates, chains, modifiers, traits, events.
//! All meta concepts are templates that own Effect[] arrays.
//! Activation is idempotent — re-entering a kingdom re-applies the effect.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef META_H
#define META_H

#include "defs.h"
#include "types.h"

struct Effect;

/*--------------------------------------------------------------------------*\
                              RELIC
\*--------------------------------------------------------------------------*/

typedef struct {
    uint16_t id;
    const char* name;
    const char* description;
    Effect effects[8];
    uint8_t effect_count;
} RelicTemplate;

/*--------------------------------------------------------------------------*\
                              INNATE
\*--------------------------------------------------------------------------*/

typedef struct {
    Kingdom kingdom;
    const char* name;
    const char* description;
    Effect effects[8];
    uint8_t effect_count;
} InnateTemplate;

/*--------------------------------------------------------------------------*\
                              CHAIN (penalty)
\*--------------------------------------------------------------------------*/

typedef struct {
    uint8_t level; /* 0=Bronze, 1=Silver, 2=Gold */
    const char* name;
    const char* description;
    Effect penalties[8];
    uint8_t penalty_count;
} Chain;

/*--------------------------------------------------------------------------*\
                              BATTLE MODIFIER
\*--------------------------------------------------------------------------*/

typedef struct {
    uint16_t id;
    const char* name;
    const char* description;
    ModifierId type;
    Effect effects[4];
    uint8_t effect_count;
} Modifier;

/*--------------------------------------------------------------------------*\
                              BOARD TRAIT
\*--------------------------------------------------------------------------*/

typedef struct {
    uint16_t id;
    const char* name;
    const char* description;
    TraitId type;
    Effect effects[4];
    uint8_t effect_count;
} BoardTrait;

/*--------------------------------------------------------------------------*\
                              FIGUREHEAD POWER
\*--------------------------------------------------------------------------*/

typedef struct {
    Kingdom kingdom;
    const char* name;
    const char* description;
    Effect effects[4];
    uint8_t effect_count;
} FigureheadPower;

/*--------------------------------------------------------------------------*\
                              SYNERGY
\*--------------------------------------------------------------------------*/

typedef struct {
    Kingdom cleared;
    const char* name;
    const char* description;
    Effect bonus;
} Synergy;

/*--------------------------------------------------------------------------*\
                              OVERSEER
\*--------------------------------------------------------------------------*/

typedef struct {
    uint16_t id;
    const char* name;
    const char* description;
    Effect effects[8];
    uint8_t effect_count;
} Overseer;

/*--------------------------------------------------------------------------*\
                              MASTERY
\*--------------------------------------------------------------------------*/

typedef struct {
    uint16_t id;
    const char* name;
    const char* description;
    Kingdom kingdom;
    Tier tier;
    Effect effects[8];
    uint8_t effect_count;
} MasteryCard;

typedef struct {
    Kingdom kingdom;
    int level; /* 1 or 3 */
    const char* name;
    Effect effect;
} MasteryHook;

/*--------------------------------------------------------------------------*\
                              EVENT
\*--------------------------------------------------------------------------*/

typedef struct {
    uint16_t id;
    const char* name;
    const char* description;
    EventId type;
} EventTemplate;

typedef struct {
    const char* option_text;
    Effect effect;
} EventOption;

#endif /* META_H */