//! meta.h
//!
//! Meta layer: relics, innates, chains, modifiers, traits, events.
//! All meta concepts are templates that own Effect[] arrays.
//! Activation is idempotent — re-entering a kingdom re-applies the effect.
//!
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

/// RelicTemplate
///
/// Persistent item gained from battles. Applies passive effects while held.
///
/// Fields:
/// - uint16_t id -> unique identifier
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - Effect effects[8] -> passive effects
/// - uint8_t effect_count -> number of effects
///
typedef struct RelicTemplate {
    uint16_t    id;
    const char* name;
    const char* description;
    Effect      effects[8];
    uint8_t     effect_count;
} RelicTemplate;

/*--------------------------------------------------------------------------*\
                              INNATE
\*--------------------------------------------------------------------------*/

/// InnateTemplate
///
/// Kingdom-specific passive ability active for the entire run.
///
/// Fields:
/// - Kingdom kingdom -> owning kingdom
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - Effect effects[8] -> passive effects
/// - uint8_t effect_count -> number of effects
///
typedef struct {
    Kingdom     kingdom;
    const char* name;
    const char* description;
    Effect      effects[8];
    uint8_t     effect_count;
} InnateTemplate;

/*--------------------------------------------------------------------------*\
                              CHAIN (penalty)
\*--------------------------------------------------------------------------*/

/// Chain
///
/// Penalty applied to the run based on consecutive losses.
///
/// Fields:
/// - uint8_t level -> chain level (0=Bronze, 1=Silver, 2=Gold)
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - Effect penalties[8] -> negative effects
/// - uint8_t penalty_count -> number of penalties
///
typedef struct {
    uint8_t     level; /* 0=Bronze, 1=Silver, 2=Gold */
    const char* name;
    const char* description;
    Effect      penalties[8];
    uint8_t     penalty_count;
} Chain;

/*--------------------------------------------------------------------------*\
                              BATTLE MODIFIER
\*--------------------------------------------------------------------------*/

/// Modifier
///
/// Temporary or permanent modifier affecting battle rules.
///
/// Fields:
/// - uint16_t id -> unique identifier
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - ModifierId type -> modifier category
/// - Effect effects[4] -> applied effects
/// - uint8_t effect_count -> number of effects
///
typedef struct {
    uint16_t    id;
    const char* name;
    const char* description;
    ModifierId  type;
    Effect      effects[4];
    uint8_t     effect_count;
} Modifier;

/*--------------------------------------------------------------------------*\
                              BOARD TRAIT
\*--------------------------------------------------------------------------*/

/// BoardTrait
///
/// Persistent board condition affecting all battles.
///
/// Fields:
/// - uint16_t id -> unique identifier
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - TraitId type -> trait category
/// - Effect effects[4] -> applied effects
/// - uint8_t effect_count -> number of effects
///
typedef struct {
    uint16_t    id;
    const char* name;
    const char* description;
    TraitId     type;
    Effect      effects[4];
    uint8_t     effect_count;
} BoardTrait;

/*--------------------------------------------------------------------------*\
                              FIGUREHEAD POWER
\*--------------------------------------------------------------------------*/

/// FigureheadPower
///
/// Kingdom-specific power earned after clearing a kingdom.
///
/// Fields:
/// - Kingdom kingdom -> owning kingdom
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - Effect effects[4] -> applied effects
/// - uint8_t effect_count -> number of effects
///
typedef struct {
    Kingdom     kingdom;
    const char* name;
    const char* description;
    Effect      effects[4];
    uint8_t     effect_count;
} FigureheadPower;

/*--------------------------------------------------------------------------*\
                              SYNERGY
\*--------------------------------------------------------------------------*/

/// Synergy
///
/// Bonus granted when clearing a kingdom's overseer.
///
/// Fields:
/// - Kingdom cleared -> kingdom that was cleared
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - Effect bonus -> synergy bonus effect
///
typedef struct {
    Kingdom     cleared;
    const char* name;
    const char* description;
    Effect      bonus;
} Synergy;

/*--------------------------------------------------------------------------*\
                              OVERSEER
\*--------------------------------------------------------------------------*/

/// Overseer
///
/// Boss piece for a kingdom with unique mechanics.
///
/// Fields:
/// - uint16_t id -> unique identifier
/// - const char* name -> display name
/// - const char* description -> flavor text
/// - Effect effects[8] -> overseer abilities
/// - uint8_t effect_count -> number of effects
///
typedef struct {
    uint16_t    id;
    const char* name;
    const char* description;
    Effect      effects[8];
    uint8_t     effect_count;
} Overseer;

/*--------------------------------------------------------------------------*\
                              MASTERY
\*--------------------------------------------------------------------------*/

/// MasteryHook
///
/// Level-1 (innate-shift) and level-3 (starting-power upgrade) hooks
/// per kingdom. Mastery-level-2 figurehead cards are NOT here — they
/// are normal CardTemplates in the CARD_* enum, injected into the
/// player's cardset by a level-2 hook.
///
/// Fields:
/// - Kingdom kingdom -> owning kingdom
/// - int level -> hook level (1 or 3)
/// - const char* name -> display name
/// - Effect effect -> mastery effect
///
typedef struct {
    Kingdom     kingdom;
    int         level; /* 1 or 3 */
    const char* name;
    Effect      effect;
} MasteryHook;

/*--------------------------------------------------------------------------*\
                              EVENT
\*--------------------------------------------------------------------------*/

/// EventTemplate
///
/// Event encountered during the map phase.
///
/// Fields:
/// - uint16_t id -> unique identifier
/// - const char* name -> display name
/// - const char* description -> event description
/// - EventId type -> event category
///
typedef struct {
    uint16_t    id;
    const char* name;
    const char* description;
    EventId     type;
} EventTemplate;

/// EventOption
///
/// Single choice within an event.
///
/// Fields:
/// - const char* option_text -> choice text
/// - Effect effect -> effect when chosen
///
typedef struct {
    const char* option_text;
    Effect      effect;
} EventOption;

/*--------------------------------------------------------------------------*\
                              META APPLICATION API
\*--------------------------------------------------------------------------*/

struct BattleState;
struct RunState;

/// meta_apply_relics
///
/// Register every held relic's effects onto the battle bus as
/// run-wide (`duration_turns = -1`) entries owned by the player.
///
/// Params:
/// - struct BattleState* bs  -> destination battle
/// - const struct RunState* run -> run state holding relic ids
///
void meta_apply_relics(struct BattleState* bs, const struct RunState* run);

/// meta_apply_innate
///
/// Register a kingdom's innate ability onto the bus as a run-wide
/// effect.
///
/// Params:
/// - struct BattleState* bs -> destination battle
/// - Kingdom             k  -> kingdom whose innate to apply
///
void meta_apply_innate(struct BattleState* bs, Kingdom k);

/// meta_apply_mastery
///
/// Phase 3 hook for mastery-level bodies; currently a no-op.
///
/// Params:
/// - struct BattleState* bs -> destination battle
/// - uint8_t             level -> mastery level
///
void meta_apply_mastery(struct BattleState* bs, uint8_t level);

/// meta_apply_chain
///
/// Register a chain level's penalty effects.
///
/// Params:
/// - struct BattleState* bs -> destination battle
/// - uint8_t             level -> chain level (1-3)
///
void meta_apply_chain(struct BattleState* bs, uint8_t level);

/// meta_apply_synergy
///
/// Register a cleared kingdom's synergy bonus onto the bus.
///
/// Params:
/// - struct BattleState* bs    -> destination battle
/// - Kingdom            cleared -> cleared kingdom id
///
void meta_apply_synergy(struct BattleState* bs, Kingdom cleared);

#endif /* META_H */