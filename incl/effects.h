//! effects.h
//!
//! Forward declarations for every effect handler symbol referenced
//! by `EFF(NAME, name)` in data tables. Keeping the declarations
//! here means data files don't have to pull each handler's source
//! header individually; the EFF macro expands cleanly because the
//! function names are known to the compiler.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef EFFECTS_H
#define EFFECTS_H

#include "effect.h"

/*--------------------------------------------------------------------------*\
                              RELICS (26)
\*--------------------------------------------------------------------------*/

void eff_merchants_ledger(struct EffectCtx*, const EffectArg*, size_t);
void eff_minted_coin(struct EffectCtx*, const EffectArg*, size_t);
void eff_tax_stamp(struct EffectCtx*, const EffectArg*, size_t);
void eff_bulk_discount(struct EffectCtx*, const EffectArg*, size_t);
void eff_war_chest(struct EffectCtx*, const EffectArg*, size_t);
void eff_trade_routes(struct EffectCtx*, const EffectArg*, size_t);
void eff_soul_shard(struct EffectCtx*, const EffectArg*, size_t);
void eff_veterans_bond(struct EffectCtx*, const EffectArg*, size_t);
void eff_dead_mans_pact(struct EffectCtx*, const EffectArg*, size_t);
void eff_iron_king(struct EffectCtx*, const EffectArg*, size_t);
void eff_bloodthirst(struct EffectCtx*, const EffectArg*, size_t);
void eff_last_breath(struct EffectCtx*, const EffectArg*, size_t);
void eff_tacticians_scroll(struct EffectCtx*, const EffectArg*, size_t);
void eff_librarians_notes(struct EffectCtx*, const EffectArg*, size_t);
void eff_country_seal(struct EffectCtx*, const EffectArg*, size_t);
void eff_deep_hand(struct EffectCtx*, const EffectArg*, size_t);
void eff_gilded_archive(struct EffectCtx*, const EffectArg*, size_t);
void eff_alchemists_kit(struct EffectCtx*, const EffectArg*, size_t);
void eff_masters_notes(struct EffectCtx*, const EffectArg*, size_t);
void eff_philosophers_stone(struct EffectCtx*, const EffectArg*, size_t);
void eff_inherited_power(struct EffectCtx*, const EffectArg*, size_t);
void eff_eagle_eye(struct EffectCtx*, const EffectArg*, size_t);
void eff_surveyors_map(struct EffectCtx*, const EffectArg*, size_t);
void eff_forward_command(struct EffectCtx*, const EffectArg*, size_t);
void eff_fortified_line(struct EffectCtx*, const EffectArg*, size_t);
void eff_warlords_banner(struct EffectCtx*, const EffectArg*, size_t);

/*--------------------------------------------------------------------------*\
                              CHAINS (3)
\*--------------------------------------------------------------------------*/

void eff_chain_bronze(struct EffectCtx*, const EffectArg*, size_t);
void eff_chain_silver(struct EffectCtx*, const EffectArg*, size_t);
void eff_chain_gold(struct EffectCtx*, const EffectArg*, size_t);

/*--------------------------------------------------------------------------*\
                              INNATES (5)
\*--------------------------------------------------------------------------*/

void eff_innate_bulwark(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_reclaim(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_double_time(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_royal_sub(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_conquerors_reward(
    struct EffectCtx*, const EffectArg*, size_t
);

/*--------------------------------------------------------------------------*\
                              FIGUREHEADS (5)
\*--------------------------------------------------------------------------*/

void eff_fh_mingzhu(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_tomohito(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_selassie(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_timur(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_isabella(struct EffectCtx*, const EffectArg*, size_t);

/*--------------------------------------------------------------------------*\
                              SYNERGIES (5)
\*--------------------------------------------------------------------------*/

void eff_syn_longwei(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_harushima(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_kewarani(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_zarqan(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_caelan(struct EffectCtx*, const EffectArg*, size_t);

/*--------------------------------------------------------------------------*\
                              MASTERY HOOKS (15)
\*--------------------------------------------------------------------------*/

void eff_mastery_l1_longwei(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l1_harushima(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l1_kewarani(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l1_zarqan(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l1_caelan(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l2_longwei(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l2_harushima(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l2_kewarani(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l2_zarqan(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l2_caelan(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l3_longwei(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l3_harushima(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l3_kewarani(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l3_zarqan(struct EffectCtx*, const EffectArg*, size_t);
void eff_mastery_l3_caelan(struct EffectCtx*, const EffectArg*, size_t);

/*--------------------------------------------------------------------------*\
                              OVERSEERS (5)
\*--------------------------------------------------------------------------*/

void eff_overseer_iron_strategist(
    struct EffectCtx*, const EffectArg*, size_t
);
void eff_overseer_eternal_recursion(
    struct EffectCtx*, const EffectArg*, size_t
);
void eff_overseer_caravan_of_conquest(
    struct EffectCtx*, const EffectArg*, size_t
);
void eff_overseer_many_faced_king(
    struct EffectCtx*, const EffectArg*, size_t
);
void eff_overseer_crowned_heretic(
    struct EffectCtx*, const EffectArg*, size_t
);

/*--------------------------------------------------------------------------*\
                              VORATH MEMORY
\*--------------------------------------------------------------------------*/

void eff_vorath_memory_tally(struct EffectCtx*, const EffectArg*, size_t);
void eff_vorath_memory_apply(struct EffectCtx*, const EffectArg*, size_t);

#endif /* EFFECTS_H */
