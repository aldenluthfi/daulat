//! eff_registry.c
//!
//! Effect-function lookup table. Maps every `EffectFuncId` to its
//! live function pointer so the save/load codec can serialize an
//! id on disk and rehydrate the `Effect.apply` pointer on load.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              FORWARD DECLARATIONS
\*--------------------------------------------------------------------------*/

/* Relics */
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

/* Chains */
void eff_chain_bronze(struct EffectCtx*, const EffectArg*, size_t);
void eff_chain_silver(struct EffectCtx*, const EffectArg*, size_t);
void eff_chain_gold(struct EffectCtx*, const EffectArg*, size_t);

/* Innates */
void eff_innate_bulwark(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_reclaim(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_double_time(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_royal_sub(struct EffectCtx*, const EffectArg*, size_t);
void eff_innate_conquerors_reward(struct EffectCtx*, const EffectArg*, size_t);

/* Figureheads */
void eff_fh_mingzhu(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_tomohito(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_selassie(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_timur(struct EffectCtx*, const EffectArg*, size_t);
void eff_fh_isabella(struct EffectCtx*, const EffectArg*, size_t);

/* Synergies */
void eff_syn_longwei(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_harushima(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_kewarani(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_zarqan(struct EffectCtx*, const EffectArg*, size_t);
void eff_syn_caelan(struct EffectCtx*, const EffectArg*, size_t);

/* Mastery hooks */
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

/* Overseers */
void eff_overseer_iron_strategist(struct EffectCtx*, const EffectArg*, size_t);
void eff_overseer_eternal_recursion(
    struct EffectCtx*, const EffectArg*, size_t
);
void eff_overseer_caravan_of_conquest(
    struct EffectCtx*, const EffectArg*, size_t
);
void eff_overseer_many_faced_king(struct EffectCtx*, const EffectArg*, size_t);
void eff_overseer_crowned_heretic(struct EffectCtx*, const EffectArg*, size_t);

/* Vorath Memory */
void eff_vorath_memory_tally(struct EffectCtx*, const EffectArg*, size_t);
void eff_vorath_memory_apply(struct EffectCtx*, const EffectArg*, size_t);

/*--------------------------------------------------------------------------*\
                              REGISTRY TABLE
\*--------------------------------------------------------------------------*/

static const EffectFunc EFFECT_FUNCS[EFFECT_FUNC_COUNT] = {
    [EFFECT_FUNC_TODO]                       = eff_todo,

    [EFFECT_FUNC_MERCHANTS_LEDGER]           = eff_merchants_ledger,
    [EFFECT_FUNC_MINTED_COIN]                = eff_minted_coin,
    [EFFECT_FUNC_TAX_STAMP]                  = eff_tax_stamp,
    [EFFECT_FUNC_BULK_DISCOUNT]              = eff_bulk_discount,
    [EFFECT_FUNC_WAR_CHEST]                  = eff_war_chest,
    [EFFECT_FUNC_TRADE_ROUTES]               = eff_trade_routes,
    [EFFECT_FUNC_SOUL_SHARD]                 = eff_soul_shard,
    [EFFECT_FUNC_VETERANS_BOND]              = eff_veterans_bond,
    [EFFECT_FUNC_DEAD_MANS_PACT]             = eff_dead_mans_pact,
    [EFFECT_FUNC_IRON_KING]                  = eff_iron_king,
    [EFFECT_FUNC_BLOODTHIRST]                = eff_bloodthirst,
    [EFFECT_FUNC_LAST_BREATH]                = eff_last_breath,
    [EFFECT_FUNC_TACTICIANS_SCROLL]          = eff_tacticians_scroll,
    [EFFECT_FUNC_LIBRARIANS_NOTES]           = eff_librarians_notes,
    [EFFECT_FUNC_COUNTRY_SEAL]               = eff_country_seal,
    [EFFECT_FUNC_DEEP_HAND]                  = eff_deep_hand,
    [EFFECT_FUNC_GILDED_ARCHIVE]             = eff_gilded_archive,
    [EFFECT_FUNC_ALCHEMISTS_KIT]             = eff_alchemists_kit,
    [EFFECT_FUNC_MASTERS_NOTES]              = eff_masters_notes,
    [EFFECT_FUNC_PHILOSOPHERS_STONE]         = eff_philosophers_stone,
    [EFFECT_FUNC_INHERITED_POWER]            = eff_inherited_power,
    [EFFECT_FUNC_EAGLE_EYE]                  = eff_eagle_eye,
    [EFFECT_FUNC_SURVEYORS_MAP]              = eff_surveyors_map,
    [EFFECT_FUNC_FORWARD_COMMAND]            = eff_forward_command,
    [EFFECT_FUNC_FORTIFIED_LINE]             = eff_fortified_line,
    [EFFECT_FUNC_WARLORDS_BANNER]            = eff_warlords_banner,

    [EFFECT_FUNC_CHAIN_BRONZE]               = eff_chain_bronze,
    [EFFECT_FUNC_CHAIN_SILVER]               = eff_chain_silver,
    [EFFECT_FUNC_CHAIN_GOLD]                 = eff_chain_gold,

    [EFFECT_FUNC_INNATE_BULWARK]             = eff_innate_bulwark,
    [EFFECT_FUNC_INNATE_RECLAIM]             = eff_innate_reclaim,
    [EFFECT_FUNC_INNATE_DOUBLE_TIME]         = eff_innate_double_time,
    [EFFECT_FUNC_INNATE_ROYAL_SUB]           = eff_innate_royal_sub,
    [EFFECT_FUNC_INNATE_CONQUERORS_REWARD]   = eff_innate_conquerors_reward,

    [EFFECT_FUNC_FH_MINGZHU]                 = eff_fh_mingzhu,
    [EFFECT_FUNC_FH_TOMOHITO]                = eff_fh_tomohito,
    [EFFECT_FUNC_FH_SELASSIE]                = eff_fh_selassie,
    [EFFECT_FUNC_FH_TIMUR]                   = eff_fh_timur,
    [EFFECT_FUNC_FH_ISABELLA]                = eff_fh_isabella,

    [EFFECT_FUNC_SYN_LONGWEI]                = eff_syn_longwei,
    [EFFECT_FUNC_SYN_HARUSHIMA]              = eff_syn_harushima,
    [EFFECT_FUNC_SYN_KEWARANI]               = eff_syn_kewarani,
    [EFFECT_FUNC_SYN_ZARQAN]                 = eff_syn_zarqan,
    [EFFECT_FUNC_SYN_CAELAN]                 = eff_syn_caelan,

    [EFFECT_FUNC_MASTERY_L1_LONGWEI]         = eff_mastery_l1_longwei,
    [EFFECT_FUNC_MASTERY_L1_HARUSHIMA]       = eff_mastery_l1_harushima,
    [EFFECT_FUNC_MASTERY_L1_KEWARANI]        = eff_mastery_l1_kewarani,
    [EFFECT_FUNC_MASTERY_L1_ZARQAN]          = eff_mastery_l1_zarqan,
    [EFFECT_FUNC_MASTERY_L1_CAELAN]          = eff_mastery_l1_caelan,
    [EFFECT_FUNC_MASTERY_L2_LONGWEI]         = eff_mastery_l2_longwei,
    [EFFECT_FUNC_MASTERY_L2_HARUSHIMA]       = eff_mastery_l2_harushima,
    [EFFECT_FUNC_MASTERY_L2_KEWARANI]        = eff_mastery_l2_kewarani,
    [EFFECT_FUNC_MASTERY_L2_ZARQAN]          = eff_mastery_l2_zarqan,
    [EFFECT_FUNC_MASTERY_L2_CAELAN]          = eff_mastery_l2_caelan,
    [EFFECT_FUNC_MASTERY_L3_LONGWEI]         = eff_mastery_l3_longwei,
    [EFFECT_FUNC_MASTERY_L3_HARUSHIMA]       = eff_mastery_l3_harushima,
    [EFFECT_FUNC_MASTERY_L3_KEWARANI]        = eff_mastery_l3_kewarani,
    [EFFECT_FUNC_MASTERY_L3_ZARQAN]          = eff_mastery_l3_zarqan,
    [EFFECT_FUNC_MASTERY_L3_CAELAN]          = eff_mastery_l3_caelan,

    [EFFECT_FUNC_OVERSEER_IRON_STRATEGIST]   = eff_overseer_iron_strategist,
    [EFFECT_FUNC_OVERSEER_ETERNAL_RECURSION] = eff_overseer_eternal_recursion,
    [EFFECT_FUNC_OVERSEER_CARAVAN_OF_CONQUEST] =
        eff_overseer_caravan_of_conquest,
    [EFFECT_FUNC_OVERSEER_MANY_FACED_KING]   = eff_overseer_many_faced_king,
    [EFFECT_FUNC_OVERSEER_CROWNED_HERETIC]   = eff_overseer_crowned_heretic,

    [EFFECT_FUNC_VORATH_MEMORY_TALLY]        = eff_vorath_memory_tally,
    [EFFECT_FUNC_VORATH_MEMORY_APPLY]        = eff_vorath_memory_apply,
};

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

EffectFunc eff_lookup(EffectFuncId id) {
    if (id < 0 || id >= EFFECT_FUNC_COUNT) {
        log_warn("eff_lookup: out-of-range id %d", id);
        return NULL;
    }
    EffectFunc func = EFFECT_FUNCS[id];
    return (func != NULL) ? func : eff_todo;
}
