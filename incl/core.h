//!
//! Base types shared by all other headers.
//! Position, EffectArg, and all enums live here to break circular deps.
//! Layer 0 — included by every other header.
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef CORE_H
#define CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*--------------------------------------------------------------------------*\
                              TIERS
\*--------------------------------------------------------------------------*/

typedef enum {
    TIER_DISTRICT,
    TIER_TOWN,
    TIER_PROVINCE,
    TIER_COUNTRY,
    TIER_CAPSTONE,
    TIER_KING
} Tier;

/*--------------------------------------------------------------------------*\
                              KINGDOMS
\*--------------------------------------------------------------------------*/

typedef enum {
    KINGDOM_LONGWEI,
    KINGDOM_HARUSHIMA,
    KINGDOM_KEWARANI,
    KINGDOM_ZARQAN,
    KINGDOM_CAELAN,
    KINGDOM_NONE
} Kingdom;

/*--------------------------------------------------------------------------*\
                              SIDES
\*--------------------------------------------------------------------------*/

typedef enum { SIDE_PLAYER, SIDE_ENEMY, SIDE_NEUTRAL } Side;

/*--------------------------------------------------------------------------*\
                              BATTLE RESULT
\*--------------------------------------------------------------------------*/

typedef enum {
    BATTLE_IN_PROGRESS,
    BATTLE_PLAYER_WON,
    BATTLE_ENEMY_WON,
    BATTLE_DRAW
} BattleResult;

/*--------------------------------------------------------------------------*\
                              POSITION
\*--------------------------------------------------------------------------*/

/// A coordinate on the board. (0,0) is player-side corner.
typedef struct {
    int8_t x, y;
} Position;

/*--------------------------------------------------------------------------*\
                              EFFECT ARGUMENT
\*--------------------------------------------------------------------------*/

typedef enum {
    EARG_INT,
    EARG_FLOAT,
    EARG_PIECE_REF,
    EARG_CARD_REF,
    EARG_TMPL_REF,
    EARG_KINGDOM,
    EARG_TIER,
    EARG_POS,
    EARG_SIDE,
    EARG_MOVEGEN_REF
} EffectArgType;

/// A typed argument passed to an EffectFunc or MoveGenFunc.
typedef struct {
    EffectArgType type;
    union {
        int i;
        float f;
        uint32_t piece_id;
        uint32_t card_id;
        uint16_t tmpl_id;
        Kingdom kingdom;
        Tier tier;
        Position pos;
        Side side;
        uint16_t movegen_id;
    } v;
} EffectArg;

#endif /* CORE_H */
