//! meta.c
//!
//! Meta-layer activation: relics, innates, masteries, chains.
//! Each concept owns an Effect[] activated by its lifecycle condition.
//! Activation is idempotent — re-entering a kingdom re-applies the effect.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              RELIC APPLICATION
\*--------------------------------------------------------------------------*/

/// meta_apply_relics
///
/// Register all effects from the run's relic collection.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - const RunState* run -> run state with relics
///
void meta_apply_relics(BattleState* bs, const RunState* run) {
    for (uint8_t i = 0; i < run->relic_count; i++) {
        const RelicTemplate* relic = run->relics[i];
        if (relic == NULL)
            continue;
        for (uint8_t j = 0; j < relic->effect_count; j++) {
            Effect effect         = relic->effects[j];
            effect.owner          = SIDE_PLAYER;
            effect.duration_turns = -1;
            bus_register(&bs->bus, &effect);
        }
    }
}

/*--------------------------------------------------------------------------*\
                              INNATE APPLICATION
\*--------------------------------------------------------------------------*/

/// meta_apply_innate
///
/// Register the innate ability for a kingdom.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - Kingdom kingdom -> kingdom whose innate to apply
///
void meta_apply_innate(BattleState* bs, Kingdom kingdom) {
    const InnateTemplate* innate = innate_template(kingdom);
    if (innate == NULL)
        return;
    for (uint8_t i = 0; i < innate->effect_count; i++) {
        Effect effect         = innate->effects[i];
        effect.owner          = SIDE_PLAYER;
        effect.duration_turns = -1;
        bus_register(&bs->bus, &effect);
    }
}

/*--------------------------------------------------------------------------*\
                              MASTERY APPLICATION
\*--------------------------------------------------------------------------*/

/// meta_apply_mastery
///
/// Apply mastery-level hooks. Currently a no-op stub.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint8_t level -> mastery level
///
void meta_apply_mastery(BattleState* bs, uint8_t level) {
    (void)bs;
    (void)level;
}

/*--------------------------------------------------------------------------*\
                              CHAIN APPLICATION
\*--------------------------------------------------------------------------*/

/// meta_apply_chain
///
/// Register penalty effects for a chain level.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - uint8_t level -> chain level
///
void meta_apply_chain(BattleState* bs, uint8_t level) {
    const Chain* chain = chain_template(level);
    if (chain == NULL)
        return;
    for (uint8_t i = 0; i < chain->penalty_count; i++) {
        Effect effect = chain->penalties[i];
        effect.owner  = SIDE_PLAYER;
        bus_register(&bs->bus, &effect);
    }
}

/*--------------------------------------------------------------------------*\
                              SYNERGY APPLICATION
\*--------------------------------------------------------------------------*/

/// meta_apply_synergy
///
/// Register the synergy bonus for a cleared kingdom.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - Kingdom cleared -> cleared kingdom
///
void meta_apply_synergy(BattleState* bs, Kingdom cleared) {
    const Synergy* synergy = synergy_template(cleared);
    if (synergy == NULL)
        return;
    Effect effect         = synergy->bonus;
    effect.owner          = SIDE_PLAYER;
    effect.duration_turns = -1;
    bus_register(&bs->bus, &effect);
}