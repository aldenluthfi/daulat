//!
//! Meta-layer activation: relics, innates, masteries, chains.
//! Each concept owns an Effect[] activated by its lifecycle condition.
//! Activation is idempotent — re-entering a kingdom re-applies the effect.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              RELIC APPLICATION
\*--------------------------------------------------------------------------*/

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

void meta_apply_mastery(BattleState* bs, uint8_t level) {
    (void)bs;
    (void)level;
}

/*--------------------------------------------------------------------------*\
                              CHAIN APPLICATION
\*--------------------------------------------------------------------------*/

void meta_apply_chain(BattleState* bs, uint8_t level) {
    const Chain* ch = chain_template(level);
    if (ch == NULL)
        return;
    for (uint8_t i = 0; i < ch->penalty_count; i++) {
        Effect e = ch->penalties[i];
        e.owner  = SIDE_PLAYER;
        bus_register(&bs->bus, &e);
    }
}

/*--------------------------------------------------------------------------*\
                              SYNERGY APPLICATION
\*--------------------------------------------------------------------------*/

void meta_apply_synergy(BattleState* bs, Kingdom cleared) {
    const Synergy* syn = synergy_template(cleared);
    if (syn == NULL)
        return;
    Effect e         = syn->bonus;
    e.owner          = SIDE_PLAYER;
    e.duration_turns = -1;
    bus_register(&bs->bus, &e);
}