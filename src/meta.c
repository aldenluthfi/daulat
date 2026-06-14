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
        const RelicTemplate* r = run->relics[i];
        if (r == NULL)
            continue;
        for (uint8_t j = 0; j < r->effect_count; j++) {
            Effect e         = r->effects[j];
            e.owner          = SIDE_PLAYER;
            e.duration_turns = -1;
            bus_register(&bs->bus, &e);
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
    const InnateTemplate* inn = innate_template(kingdom);
    if (inn == NULL)
        return;
    for (uint8_t i = 0; i < inn->effect_count; i++) {
        Effect e         = inn->effects[i];
        e.owner          = SIDE_PLAYER;
        e.duration_turns = -1;
        bus_register(&bs->bus, &e);
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
    const Chain* ch = chain_template(level);
    if (ch == NULL)
        return;
    for (uint8_t i = 0; i < ch->penalty_count; i++) {
        Effect e    = ch->penalties[i];
        e.owner    = SIDE_PLAYER;
        bus_register(&bs->bus, &e);
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
    const Synergy* syn = synergy_template(cleared);
    if (syn == NULL)
        return;
    Effect e         = syn->bonus;
    e.owner          = SIDE_PLAYER;
    e.duration_turns = -1;
    bus_register(&bs->bus, &e);
}