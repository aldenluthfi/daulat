//! events.c
//!
//! Narrative event data. Each event is an Event item carrying its name,
//! flavour text, and two options; every option's behaviour is expressed as
//! effects. An ON_EVENT_CHOOSE effect runs the instant the option is taken
//! and acts on run state through the public run_* helpers; any other trigger
//! is a run-persistent effect re-attached to the human seat each battle from
//! the recorded choice. Parameters are baked into each effect's context so a
//! handful of generic bodies cover every event.
//!
//! Created: 14/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                              IMMEDIATE EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_ev_vorath
///
/// Reduces the Global Vorath Counter by the baked amount.
///
/// Params:
/// - context -> args[0] amount to reduce
/// - x       -> EngineState* owning the run
///
/// Return: true, the reduction always applies
///
static bool eff_ev_vorath(EffectContext* context, void* x) {
    EngineState* engine = x;

    run_reduce_vorath(engine->run, (size_t) (uintptr_t) context->args[0]);

    return true;
}

/// eff_ev_reveal
///
/// Reveals the next baked number of hidden nodes on the active map, folding
/// the peek through the shared node-visibility state so masked identities
/// and content become visible.
///
/// Params:
/// - context -> args[0] nodes revealed
/// - x       -> EngineState* owning the run
///
/// Return: true, the peek always reveals
///
static bool eff_ev_reveal(EffectContext* context, void* x) {
    run_node_reveal(x, (size_t) (uintptr_t) context->args[0]);

    return true;
}

/// eff_ev_elite
///
/// Begins an elite battle on the event node, stashing the reward granted on
/// a win.
///
/// Params:
/// - context -> args[0] reward kind, args[1]/args[2] the offered relics
/// - x       -> EngineState* owning the run
///
/// Return: true, the elite battle always begins
///
static bool eff_ev_elite(EffectContext* context, void* x) {
    run_begin_elite(
        x,
        (int) (intptr_t) context->args[0],
        (RelicID) (uintptr_t) context->args[1],
        (RelicID) (uintptr_t) context->args[2]
    );

    return true;
}

/*----------------------------------------------------------------------------*\
                            TYPED TARGET EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_ev_tg_card
///
/// Advertises every held card as a TARGET_CARD candidate, excluding cards
/// already picked in an earlier step so a multi-card removal never offers
/// the same card twice.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* candidate list to fill
///
/// Return: true, the list always builds
///
static bool eff_ev_tg_card(EffectContext* context, void* x) {
    (void) context;

    RunState*         run   = run_engine()->run;
    const CardTarget* picks = run_pending_picks();
    CardTarget*       list  = x;

    for (CardID id = 0; id < CARD_COUNT; id++) {
        if (!run->cards[id]) {
            continue;
        }

        bool picked = false;

        for (size_t p = 0; picks[p].kind != TARGET_NONE; p++) {
            if (picks[p].kind == TARGET_CARD && picks[p].value == (int) id) {
                picked = true;
            }
        }

        if (!picked) {
            card_target_push(list, TARGET_CARD, (int) id);
        }
    }

    return true;
}

/// eff_ev_tg_piece
///
/// Advertises every unlocked piece type as a TARGET_PIECE_TYPE candidate,
/// restricted to the baked kingdom unless it is KINGDOM_NONE.
///
/// Params:
/// - context -> args[1] kingdom filter, KINGDOM_NONE for any
/// - x       -> CardTarget* candidate list to fill
///
/// Return: true, the list always builds
///
static bool eff_ev_tg_piece(EffectContext* context, void* x) {
    RunState*   run     = run_engine()->run;
    KingdomID   kingdom = (KingdomID) (uintptr_t) context->args[1];
    CardTarget* list    = x;

    for (PieceID id = 0; id < PIECE_COUNT; id++) {
        const Piece* piece = PIECE_REGISTRY[id];

        if (!piece || !run->pieces[id]) {
            continue;
        }

        if (kingdom != KINGDOM_NONE && piece->kingdom != kingdom) {
            continue;
        }

        card_target_push(list, TARGET_PIECE_TYPE, (int) id);
    }

    return true;
}

/// eff_ev_tg_relic
///
/// Advertises the two baked relics as TARGET_RELIC candidates.
///
/// Params:
/// - context -> args[0]/args[1] the two offered relics
/// - x       -> CardTarget* candidate list to fill
///
/// Return: true, the list always builds
///
static bool eff_ev_tg_relic(EffectContext* context, void* x) {
    CardTarget* list = x;

    card_target_push(list, TARGET_RELIC, (int) (uintptr_t) context->args[0]);
    card_target_push(list, TARGET_RELIC, (int) (uintptr_t) context->args[1]);

    return true;
}

/// eff_ev_tg_node
///
/// Advertises the active map's selectable battle nodes as TARGET_NODE
/// candidates, the nodes a skip event may forgo.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* candidate list to fill
///
/// Return: true, the list always builds
///
static bool eff_ev_tg_node(EffectContext* context, void* x) {
    (void) context;

    run_targets_battle_nodes(x);

    return true;
}

/// eff_ev_tg_modifier
///
/// Advertises the battle nodes whose modifier is still hidden as TARGET_NODE
/// candidates, the battles a modifier-reveal event may scout.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* candidate list to fill
///
/// Return: true, the list always builds
///
static bool eff_ev_tg_modifier(EffectContext* context, void* x) {
    (void) context;

    run_targets_modifier_nodes(x);

    return true;
}

/// eff_ev_tg_figurehead
///
/// Advertises the chained kingdoms as TARGET_FIGUREHEAD candidates.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* candidate list to fill
///
/// Return: true, the list always builds
///
static bool eff_ev_tg_figurehead(EffectContext* context, void* x) {
    (void) context;

    run_targets_figureheads(x);

    return true;
}

/// eff_ev_sel_remove
///
/// Removes every card picked in the interaction from the run's card set.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* the picks list
///
/// Return: true, the removal always applies
///
static bool eff_ev_sel_remove(EffectContext* context, void* x) {
    (void) context;

    RunState*   run   = run_engine()->run;
    CardTarget* picks = x;

    for (size_t p = 0; picks[p].kind != TARGET_NONE; p++) {
        if (picks[p].kind != TARGET_CARD) {
            continue;
        }

        run->cards[picks[p].value] = false;

        protocol_emit("log offering removed=%d", picks[p].value);
    }

    return true;
}

/// eff_ev_sel_relic
///
/// Grants the picked relic to the run.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* the picks list
///
/// Return: true when a relic was picked
///
static bool eff_ev_sel_relic(EffectContext* context, void* x) {
    (void) context;

    CardTarget* picks = x;

    if (picks[0].kind != TARGET_RELIC) {
        return false;
    }

    run_engine()->run->relics[picks[0].value] = true;

    protocol_emit("log relic id=%d", picks[0].value);

    return true;
}

/// eff_ev_sel_pick
///
/// Records the picked piece type for the pending event so its run-persistent
/// value or cost effect reads it each battle.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* the picks list
///
/// Return: true, the pick always records
///
static bool eff_ev_sel_pick(EffectContext* context, void* x) {
    (void) context;

    CardTarget* picks = x;

    run_event_pick(picks[0].value);

    return true;
}

/// eff_ev_sel_skip
///
/// Skips the picked battle node.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* the picks list
///
/// Return: true when a node was picked
///
static bool eff_ev_sel_skip(EffectContext* context, void* x) {
    (void) context;

    CardTarget* picks = x;

    if (picks[0].kind != TARGET_NODE) {
        return false;
    }

    run_skip_node((size_t) picks[0].value);

    return true;
}

/// eff_ev_sel_modifier
///
/// Reveals the picked battle node's modifier without revealing the node
/// itself, the resolution of a modifier-reveal event.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* the picks list
///
/// Return: true when a node was picked
///
static bool eff_ev_sel_modifier(EffectContext* context, void* x) {
    (void) context;

    CardTarget* picks = x;

    if (picks[0].kind != TARGET_NODE) {
        return false;
    }

    run_reveal_modifier(run_engine(), (size_t) picks[0].value);

    return true;
}

/// eff_ev_sel_figurehead
///
/// Removes one chain level from the picked figurehead.
///
/// Params:
/// - context -> unused
/// - x       -> CardTarget* the picks list
///
/// Return: true when a figurehead was picked
///
static bool eff_ev_sel_figurehead(EffectContext* context, void* x) {
    (void) context;

    CardTarget* picks = x;

    if (picks[0].kind != TARGET_FIGUREHEAD) {
        return false;
    }

    run_unchain((KingdomID) picks[0].value);

    return true;
}

/*----------------------------------------------------------------------------*\
                              PERSISTENT EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_val_kingdom
///
/// Adds the baked value to every spawning piece of the baked kingdom.
///
/// Params:
/// - context -> args[1] kingdom, args[2] added value
/// - x       -> int* the spawning piece's value
///
/// Return: true when the piece belonged to the kingdom
///
static bool eff_val_kingdom(EffectContext* context, void* x) {
    const Piece* piece = battle_buy_piece();

    if (!piece || piece->kingdom != (KingdomID) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x += (int) (intptr_t) context->args[2];

    return true;
}

/// eff_val_piece
///
/// Adds the baked value to every spawning piece of the baked identity.
///
/// Params:
/// - context -> args[1] piece id, args[2] added value
/// - x       -> int* the spawning piece's value
///
/// Return: true when the piece matched the identity
///
static bool eff_val_piece(EffectContext* context, void* x) {
    const Piece* piece = battle_buy_piece();

    if (!piece || piece->id != (PieceID) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x += (int) (intptr_t) context->args[2];

    return true;
}

/// eff_val_pawn
///
/// Adds the baked value to every spawning pawn-grade piece.
///
/// Params:
/// - context -> args[2] added value
/// - x       -> int* the spawning piece's value
///
/// Return: true when the piece was pawn-grade
///
static bool eff_val_pawn(EffectContext* context, void* x) {
    const Piece* piece = battle_buy_piece();

    if (!piece || !piece_is_pawn(piece->id)) {
        return false;
    }

    *(int*) x += (int) (intptr_t) context->args[2];

    return true;
}

/// eff_cost_kingdom
///
/// Discounts the buy cost of the baked kingdom's pieces by the baked
/// percent.
///
/// Params:
/// - context -> args[1] kingdom, args[2] percent discount
/// - x       -> int* the buy cost
///
/// Return: true when the piece belonged to the kingdom
///
static bool eff_cost_kingdom(EffectContext* context, void* x) {
    const Piece* piece = battle_buy_piece();

    if (!piece || piece->kingdom != (KingdomID) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x = *(int*) x * (100 - (int) (intptr_t) context->args[2]) / 100;

    return true;
}

/// eff_cost_piece
///
/// Discounts the buy cost of the baked piece identity by the baked percent.
///
/// Params:
/// - context -> args[1] piece id, args[2] percent discount
/// - x       -> int* the buy cost
///
/// Return: true when the piece matched the identity
///
static bool eff_cost_piece(EffectContext* context, void* x) {
    const Piece* piece = battle_buy_piece();

    if (!piece || piece->id != (PieceID) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x = *(int*) x * (100 - (int) (intptr_t) context->args[2]) / 100;

    return true;
}

/// eff_ev_income_once
///
/// Adds bonus starting currency on the opening turn. Carried by a ONE_BATTLE
/// event effect so it applies to exactly the next battle then is consumed.
///
/// Params:
/// - context -> args[2] currency granted
/// - x       -> int* the acting side's turn income
///
/// Return: true on the opening turn
///
static bool eff_ev_income_once(EffectContext* context, void* x) {
    if (battle_current()->turn != 1) {
        return false;
    }

    *(int*) x += (int) (intptr_t) context->args[2];

    return true;
}

/// eff_ev_income_map
///
/// Adds bonus starting currency on the opening turn of every battle fought
/// in the baked kingdom's region.
///
/// Params:
/// - context -> args[1] kingdom, args[2] currency granted
/// - x       -> int* the acting side's turn income
///
/// Return: true on the opening turn in the kingdom
///
static bool eff_ev_income_map(EffectContext* context, void* x) {
    BattleState* battle = battle_current();

    if (battle->turn != 1 || !battle->node || !battle->node->kingdom ||
        battle->node->kingdom->id != (KingdomID) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x += (int) (intptr_t) context->args[2];

    return true;
}

/// eff_enemy_army
///
/// Adds the baked free-piece count to the enemy army while fighting in the
/// baked kingdom's region.
///
/// Params:
/// - context -> args[1] kingdom, args[2] extra pieces
/// - x       -> int* the enemy reinforcement count
///
/// Return: true when the battle is in the kingdom's region
///
static bool eff_enemy_army(EffectContext* context, void* x) {
    BattleState* battle = battle_current();

    if (!battle->node || !battle->node->kingdom ||
        battle->node->kingdom->id != (KingdomID) (uintptr_t) context->args[1]) {
        return false;
    }

    *(int*) x += (int) (intptr_t) context->args[2];

    return true;
}

/*----------------------------------------------------------------------------*\
                             EFFECT SHORTHANDS
\*----------------------------------------------------------------------------*/

#define EV_VORATH(n)                                                           \
    {                                                                          \
        .func = eff_ev_vorath, .name = "Vorath Reduction",                     \
        .trigger = ON_EVENT_CHOOSE, .lasts_for = ENTIRE_BATTLE,                \
        .context = &(EffectContext) {                                          \
            .args = {(void*) (uintptr_t) (n) }                                 \
        }                                                                      \
    }

#define EV_REVEAL(n)                                                           \
    {                                                                          \
        .func = eff_ev_reveal, .name = "Reveal", .trigger = ON_EVENT_CHOOSE,   \
        .lasts_for = ENTIRE_BATTLE, .context = &(EffectContext) {              \
            .args = {(void*) (intptr_t) (n) }                                  \
        }                                                                      \
    }

#define EV_TG_CARD                                                             \
    {.func      = eff_ev_tg_card,                                              \
     .trigger   = QUERY_EVENT_TARGETS,                                         \
     .lasts_for = ENTIRE_BATTLE}

#define EV_SEL_REMOVE                                                          \
    {.func      = eff_ev_sel_remove,                                           \
     .name      = "Card Removal",                                              \
     .trigger   = ON_EVENT_TARGET_SELECTED,                                    \
     .lasts_for = ENTIRE_BATTLE}

#define EV_TG_RELIC(a, b)                                                      \
    {                                                                          \
        .func = eff_ev_tg_relic, .trigger = QUERY_EVENT_TARGETS,               \
        .lasts_for = ENTIRE_BATTLE, .context = &(EffectContext) {              \
            .args = {(void*) (uintptr_t) (a), (void*) (uintptr_t) (b) }        \
        }                                                                      \
    }

#define EV_SEL_RELIC                                                           \
    {.func      = eff_ev_sel_relic,                                            \
     .name      = "Relic Offer",                                               \
     .trigger   = ON_EVENT_TARGET_SELECTED,                                    \
     .lasts_for = ENTIRE_BATTLE}

#define EV_TG_PIECE(k)                                                         \
    {                                                                          \
        .func = eff_ev_tg_piece, .trigger = QUERY_EVENT_TARGETS,               \
        .lasts_for = ENTIRE_BATTLE, .context = &(EffectContext) {              \
            .args = { nullptr, (void*) (uintptr_t) (k) }                       \
        }                                                                      \
    }

#define EV_SEL_PICK                                                            \
    {.func      = eff_ev_sel_pick,                                             \
     .name      = "Piece Focus",                                               \
     .trigger   = ON_EVENT_TARGET_SELECTED,                                    \
     .lasts_for = ENTIRE_BATTLE}

#define EV_TG_NODE                                                             \
    {.func      = eff_ev_tg_node,                                              \
     .trigger   = QUERY_EVENT_TARGETS,                                         \
     .lasts_for = ENTIRE_BATTLE}

#define EV_SEL_SKIP                                                            \
    {.func      = eff_ev_sel_skip,                                             \
     .name      = "Skip Battle",                                               \
     .trigger   = ON_EVENT_TARGET_SELECTED,                                    \
     .lasts_for = ENTIRE_BATTLE}

#define EV_TG_MODIFIER                                                         \
    {.func      = eff_ev_tg_modifier,                                          \
     .trigger   = QUERY_EVENT_TARGETS,                                         \
     .lasts_for = ENTIRE_BATTLE}

#define EV_SEL_MODIFIER                                                        \
    {.func      = eff_ev_sel_modifier,                                         \
     .name      = "Modifier Reveal",                                           \
     .trigger   = ON_EVENT_TARGET_SELECTED,                                    \
     .lasts_for = ENTIRE_BATTLE}

#define EV_TG_FIGUREHEAD                                                       \
    {.func      = eff_ev_tg_figurehead,                                        \
     .trigger   = QUERY_EVENT_TARGETS,                                         \
     .lasts_for = ENTIRE_BATTLE}

#define EV_SEL_FIGUREHEAD                                                      \
    {.func      = eff_ev_sel_figurehead,                                       \
     .name      = "Chain Removal",                                             \
     .trigger   = ON_EVENT_TARGET_SELECTED,                                    \
     .lasts_for = ENTIRE_BATTLE}

#define EV_VAL_PIECE_PICK(v)                                                   \
    {                                                                          \
        .func = eff_val_piece, .name = "Value Bonus",                          \
        .trigger = QUERY_PIECE_VALUE, .lasts_for = ENTIRE_BATTLE,              \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                nullptr,                                                       \
                (void*) (intptr_t) (-1),                                       \
                (void*) (intptr_t) (v)                                         \
            }                                                                  \
        }                                                                      \
    }

#define EV_COST_PIECE_PICK(p)                                                  \
    {                                                                          \
        .func = eff_cost_piece, .name = "Cost Discount",                       \
        .trigger = QUERY_PIECE_CP_COST_BUY, .lasts_for = ENTIRE_BATTLE,        \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                nullptr,                                                       \
                (void*) (intptr_t) (-1),                                       \
                (void*) (intptr_t) (p)                                         \
            }                                                                  \
        }                                                                      \
    }

#define EV_ELITE(kind, a, b)                                                   \
    {                                                                          \
        .func = eff_ev_elite, .name = "Elite Battle",                          \
        .trigger = ON_EVENT_CHOOSE, .lasts_for = ENTIRE_BATTLE,                \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                (void*) (intptr_t) (kind),                                     \
                (void*) (uintptr_t) (a),                                       \
                (void*) (uintptr_t) (b)                                        \
            }                                                                  \
        }                                                                      \
    }

#define EV_VAL_KINGDOM(k, v)                                                   \
    {                                                                          \
        .func = eff_val_kingdom, .name = "Value Bonus",                        \
        .trigger = QUERY_PIECE_VALUE, .lasts_for = ENTIRE_BATTLE,              \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                nullptr,                                                       \
                (void*) (uintptr_t) (k),                                       \
                (void*) (intptr_t) (v)                                         \
            }                                                                  \
        }                                                                      \
    }

#define EV_VAL_PIECE(p, v)                                                     \
    {                                                                          \
        .func = eff_val_piece, .name = "Value Bonus",                          \
        .trigger = QUERY_PIECE_VALUE, .lasts_for = ENTIRE_BATTLE,              \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                nullptr,                                                       \
                (void*) (uintptr_t) (p),                                       \
                (void*) (intptr_t) (v)                                         \
            }                                                                  \
        }                                                                      \
    }

#define EV_VAL_PAWN(v)                                                         \
    {                                                                          \
        .func = eff_val_pawn, .name = "Value Bonus",                           \
        .trigger = QUERY_PIECE_VALUE, .lasts_for = ENTIRE_BATTLE,              \
        .context = &(EffectContext) {                                          \
            .args = { nullptr, nullptr, (void*) (intptr_t) (v) }               \
        }                                                                      \
    }

#define EV_COST_KINGDOM(k, p)                                                  \
    {                                                                          \
        .func = eff_cost_kingdom, .name = "Cost Discount",                     \
        .trigger = QUERY_PIECE_CP_COST_BUY, .lasts_for = ENTIRE_BATTLE,        \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                nullptr,                                                       \
                (void*) (uintptr_t) (k),                                       \
                (void*) (intptr_t) (p)                                         \
            }                                                                  \
        }                                                                      \
    }

#define EV_ENEMY_ARMY(k, n)                                                    \
    {                                                                          \
        .func = eff_enemy_army, .name = "Enemy Reinforcement",                 \
        .trigger = QUERY_ENEMY_ARMY_COUNT, .lasts_for = ENTIRE_BATTLE,         \
        .context = &(EffectContext) {                                          \
            .args = {                                                          \
                nullptr,                                                       \
                (void*) (uintptr_t) (k),                                       \
                (void*) (intptr_t) (n)                                         \
            }                                                                  \
        }                                                                      \
    }

/*----------------------------------------------------------------------------*\
                                 EVENT DATA
\*----------------------------------------------------------------------------*/

static const Event EVENTS[EVENT_COUNT] = {
    [EVENT_SCHOLARS_OFFER] =
        {.name = "The Scholar's Offer",
         .desc = "A sage weighs knowledge against fortune.",
         .id   = EVENT_SCHOLARS_OFFER,
         .options =
             {{.text    = "All Longwei pieces gain +3 value this run",
               .effects = {EV_VAL_KINGDOM(KINGDOM_LONGWEI, 3)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_VETERANS_BOND, RELIC_TAX_STAMP),
                    EV_SEL_RELIC}}}},

    [EVENT_DRAGON_COURT_TRIBUTE] =
        {.name = "Dragon Court Tribute",
         .desc = "The court demands a tribute of steel or paper.",
         .id   = EVENT_DRAGON_COURT_TRIBUTE,
         .options =
             {{.text = "Remove 1 card; all Longwei pieces +5 value this run",
               .effects =
                   {EV_TG_CARD,
                    EV_SEL_REMOVE,
                    EV_VAL_KINGDOM(KINGDOM_LONGWEI, 5)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_BULK_DISCOUNT, RELIC_FORTIFIED_LINE),
                    EV_SEL_RELIC}}}},

    [EVENT_DEFECTOR] =
        {.name = "The Defector",
         .desc = "A turncoat offers intelligence or a favour.",
         .id   = EVENT_DEFECTOR,
         .options =
             {{.text    = "Reveal the next battle's modifier",
               .effects = {EV_TG_MODIFIER, EV_SEL_MODIFIER}},
              {.text    = "Reduce Global Vorath Counter by 1",
               .effects = {EV_VORATH(1)}}}},

    [EVENT_JANGGI_ELDER] =
        {.name = "The Janggi Elder",
         .desc = "An old master hones one lineage of pieces.",
         .id   = EVENT_JANGGI_ELDER,
         .options =
             {{.text = "One Longwei piece type gains +5 value this run",
               .effects =
                   {EV_TG_PIECE(KINGDOM_LONGWEI),
                    EV_SEL_PICK,
                    EV_VAL_PIECE_PICK(5)}},
              {.text    = "Reduce Global Vorath Counter by 1",
               .effects = {EV_VORATH(1)}}}},

    [EVENT_CANNON_SALUTE] =
        {.name = "Cannon Salute",
         .desc = "The artillery corps salutes a passing commander.",
         .id   = EVENT_CANNON_SALUTE,
         .options =
             {{.text    = "All Pao gain +5 value this run",
               .effects = {EV_VAL_PIECE(PIECE_PAO, 5)}},
              {.text    = "Reduce Global Vorath Counter by 2",
               .effects = {EV_VORATH(2)}}}},

    [EVENT_MANSAS_COURT] =
        {.name = "Mansa's Court",
         .desc = "The Mansa asks a price for safe passage.",
         .id   = EVENT_MANSAS_COURT,
         .options =
             {{.text    = "Remove 1 card from card set",
               .effects = {EV_TG_CARD, EV_SEL_REMOVE}},
              {.text    = "Refuse: Kewarani enemies gain +1 piece this run",
               .effects = {EV_ENEMY_ARMY(KINGDOM_KEWARANI, 1)}}}},

    [EVENT_SALT_ROAD_MERCHANT] =
        {.name = "Salt Road Merchant",
         .desc = "A trader spreads his wares across the sand.",
         .id   = EVENT_SALT_ROAD_MERCHANT,
         .options =
             {{.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_SOUL_SHARD, RELIC_GILDED_ARCHIVE),
                    EV_SEL_RELIC}},
              {.text    = "All Medeq gain +3 value this run",
               .effects = {EV_VAL_PIECE(PIECE_MEDEQ, 3)}}}},

    [EVENT_STOLEN_GUARD] =
        {.name = "The Stolen Guard",
         .desc = "A captured guard can be freed by force.",
         .id   = EVENT_STOLEN_GUARD,
         .options =
             {{.text    = "Elite battle. Win: remove 1 Bronze chain",
               .effects = {EV_ELITE(1, 0, 0)}},
              {.text    = "Reduce Global Vorath Counter by 1",
               .effects = {EV_VORATH(1)}}}},

    [EVENT_CAMEL_CARAVAN] =
        {.name = "The Camel Caravan",
         .desc = "Caravaneers bargain over the season's rates.",
         .id   = EVENT_CAMEL_CARAVAN,
         .options =
             {{.text    = "Kewarani pieces cost 15% less this run",
               .effects = {EV_COST_KINGDOM(KINGDOM_KEWARANI, 15)}},
              {.text    = "Reduce Global Vorath Counter by 2",
               .effects = {EV_VORATH(2)}}}},

    [EVENT_FEAST_OF_YOD_ABEGA] =
        {.name = "Feast of Yod Abeba",
         .desc = "A festival tempts the army to linger.",
         .id   = EVENT_FEAST_OF_YOD_ABEGA,
         .options =
             {{.text    = "Skip the next battle node",
               .effects = {EV_TG_NODE, EV_SEL_SKIP}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_LAST_BREATH, RELIC_WARLORDS_BANNER),
                    EV_SEL_RELIC}}}},

    [EVENT_WARLORDS_CHALLENGE] =
        {.name = "The Warlord's Challenge",
         .desc = "A warlord dares you to single combat.",
         .id   = EVENT_WARLORDS_CHALLENGE,
         .options =
             {{.text = "Elite battle. Win: choose 1 relic from 2",
               .effects =
                   {EV_ELITE(0, RELIC_WARLORDS_BANNER, RELIC_IRON_KING)}},
              {.text    = "Reduce Global Vorath Counter by 2",
               .effects = {EV_VORATH(2)}}}},

    [EVENT_BAZAAR_OF_SAMARKAND] =
        {.name = "Bazaar of Samarkand",
         .desc = "The grand bazaar buys back your surplus.",
         .id   = EVENT_BAZAAR_OF_SAMARKAND,
         .options =
             {{.text = "Remove 2 cards; Zarqan pieces cost 15% less this run",
               .effects =
                   {EV_TG_CARD,
                    EV_TG_CARD,
                    EV_SEL_REMOVE,
                    EV_COST_KINGDOM(KINGDOM_ZARQAN, 15)}},
              {.text    = "Reduce Global Vorath Counter by 2",
               .effects = {EV_VORATH(2)}}}},

    [EVENT_MIRAGE] =
        {.name = "The Mirage",
         .desc = "A fortified vision shimmers on the horizon.",
         .id   = EVENT_MIRAGE,
         .options =
             {{.text = "Elite (Citadel enemies). Win: choose 1 relic from 2",
               .effects =
                   {EV_ELITE(0, RELIC_TRADE_ROUTES, RELIC_MERCHANTS_LEDGER)}},
              {
                  .text = "Gain +15 cp at the start of the next battle",
                  .effects =
                      {
                          {
                              .func      = eff_ev_income_once,
                              .name      = "Bonus Currency",
                              .trigger   = QUERY_CP_INCOME,
                              .lasts_for = ONE_BATTLE,
                              .context =
                                  &(EffectContext) {
                                      .args = {
                                          nullptr,
                                          nullptr,
                                          (void*) (intptr_t) 15,
                                      },
                                  },
                          },
                      },
              }}},

    [EVENT_SPY_REPORT] =
        {.name = "The Spy's Report",
         .desc = "An informant sells you the enemy's plans.",
         .id   = EVENT_SPY_REPORT,
         .options =
             {{.text    = "Reduce Global Vorath Counter by 2",
               .effects = {EV_VORATH(2)}},
              {.text    = "Reduce Global Vorath Counter by 1",
               .effects = {EV_VORATH(1)}}}},

    [EVENT_DESERT_CROSSING] =
        {.name = "The Desert Crossing",
         .desc = "A guide can scout ahead or lift a burden.",
         .id   = EVENT_DESERT_CROSSING,
         .options =
             {{.text = "Reveal 3 map nodes ahead", .effects = {EV_REVEAL(3)}},
              {.text    = "Remove 1 Bronze chain from any figurehead",
               .effects = {EV_TG_FIGUREHEAD, EV_SEL_FIGUREHEAD}}}},

    [EVENT_RONIN] =
        {.name = "The Ronin",
         .desc = "A masterless swordsman offers his service.",
         .id   = EVENT_RONIN,
         .options =
             {{.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_EAGLE_EYE, RELIC_FORWARD_COMMAND),
                    EV_SEL_RELIC}},
              {.text    = "All Harushima pieces gain +3 value this run",
               .effects = {EV_VAL_KINGDOM(KINGDOM_HARUSHIMA, 3)}}}},

    [EVENT_SPY_NETWORK] =
        {.name = "The Spy Network",
         .desc = "A network trades secrets for coin or sight.",
         .id   = EVENT_SPY_NETWORK,
         .options =
             {{.text    = "Reveal the next 3 map nodes",
               .effects = {EV_REVEAL(3)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_MINTED_COIN, RELIC_SURVEYORS_MAP),
                    EV_SEL_RELIC}}}},

    [EVENT_BURNING_PORT] =
        {.name = "The Burning Port",
         .desc = "A burning dock forces a hard bargain.",
         .id   = EVENT_BURNING_PORT,
         .options =
             {{.text = "Remove 2 cards; Harushima pieces cost 15% less",
               .effects =
                   {EV_TG_CARD,
                    EV_TG_CARD,
                    EV_SEL_REMOVE,
                    EV_COST_KINGDOM(KINGDOM_HARUSHIMA, 15)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_COUNTRY_SEAL, RELIC_FORTIFIED_LINE),
                    EV_SEL_RELIC}}}},

    [EVENT_FORGE_MASTER] =
        {.name = "The Forge Master",
         .desc = "A smith retools one line of your army.",
         .id   = EVENT_FORGE_MASTER,
         .options =
             {{.text = "One piece type costs 20% less this run",
               .effects =
                   {EV_TG_PIECE(KINGDOM_NONE),
                    EV_SEL_PICK,
                    EV_COST_PIECE_PICK(20)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_VETERANS_BOND, RELIC_TACTICIANS_SCROLL),
                    EV_SEL_RELIC}}}},

    [EVENT_VETERAN_LANCE] =
        {.name = "The Veteran Lance",
         .desc = "A decorated lancer drills your spears.",
         .id   = EVENT_VETERAN_LANCE,
         .options =
             {{.text    = "All Kyosha gain +5 value this run",
               .effects = {EV_VAL_PIECE(PIECE_KYOSHA, 5)}},
              {.text    = "Reduce Global Vorath Counter by 1",
               .effects = {EV_VORATH(1)}}}},

    [EVENT_TOURNAMENT] =
        {.name = "The Tournament",
         .desc = "A grand tourney offers glory or gold.",
         .id   = EVENT_TOURNAMENT,
         .options =
             {{.text = "Elite battle. Win: choose 1 relic from 2",
               .effects =
                   {EV_ELITE(0, RELIC_INHERITED_POWER, RELIC_BLOODTHIRST)}},
              {.text = "Gain +20 cp at the start of the next battle",
               .effects =
                   {
                       {
                           .func      = eff_ev_income_once,
                           .name      = "Bonus Currency",
                           .trigger   = QUERY_CP_INCOME,
                           .lasts_for = ONE_BATTLE,
                           .context =
                               &(EffectContext) {
                                   .args = {
                                       nullptr,
                                       nullptr,
                                       (void*) (intptr_t) 20,
                                   },
                               },
                       },
                   }}}},

    [EVENT_CHURCH_BLESSING] =
        {.name = "The Church's Blessing",
         .desc = "The cathedral blesses your treasury or your saints.",
         .id   = EVENT_CHURCH_BLESSING,
         .options =
             {{.text    = "Caelan pieces cost 10% less this run",
               .effects = {EV_COST_KINGDOM(KINGDOM_CAELAN, 10)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_GILDED_ARCHIVE, RELIC_BULK_DISCOUNT),
                    EV_SEL_RELIC}}}},

    [EVENT_SIEGE_ENGINEER] =
        {.name = "The Siege Engineer",
         .desc = "An engineer sharpens tools or cannon.",
         .id   = EVENT_SIEGE_ENGINEER,
         .options =
             {{.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_MASTERS_NOTES, RELIC_ALCHEMISTS_KIT),
                    EV_SEL_RELIC}},
              {.text    = "All Pao gain +5 value this run",
               .effects = {EV_VAL_PIECE(PIECE_PAO, 5)}}}},

    [EVENT_PRETENDER] =
        {.name = "The Pretender",
         .desc = "A rival claimant mirrors your own banner.",
         .id   = EVENT_PRETENDER,
         .options =
             {{.text    = "Elite (mirror). Win: remove 1 Silver chain",
               .effects = {EV_ELITE(1, 0, 0)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_DEAD_MANS_PACT, RELIC_TRADE_ROUTES),
                    EV_SEL_RELIC}}}},

    [EVENT_ROYAL_DECREE] =
        {.name = "The Royal Decree",
         .desc = "The crown decrees a sacrifice for a boon.",
         .id   = EVENT_ROYAL_DECREE,
         .options =
             {{.text = "Remove 1 card; all Caelan pieces +3 value this run",
               .effects =
                   {EV_TG_CARD,
                    EV_SEL_REMOVE,
                    EV_VAL_KINGDOM(KINGDOM_CAELAN, 3)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_IRON_KING, RELIC_BLOODTHIRST),
                    EV_SEL_RELIC}}}},

    [EVENT_QUEENS_FAVOR] =
        {.name = "The Queen's Favor",
         .desc = "The queen funds your campaign across her lands.",
         .id   = EVENT_QUEENS_FAVOR,
         .options =
             {{.text = "Gain +25 cp each remaining battle this map",
               .effects =
                   {
                       {
                           .func      = eff_ev_income_map,
                           .name      = "Map Currency",
                           .trigger   = QUERY_CP_INCOME,
                           .lasts_for = ENTIRE_BATTLE,
                           .context =
                               &(EffectContext) {
                                   .args = {
                                       nullptr,
                                       (void*) (uintptr_t) KINGDOM_CAELAN,
                                       (void*) (intptr_t) 25},
                               },
                       }
                   }},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_VETERANS_BOND, RELIC_MINTED_COIN),
                    EV_SEL_RELIC}}}},

    [EVENT_WANDERING_PIECE] =
        {.name = "The Wandering Piece",
         .desc = "A lone piece wanders in from the wastes.",
         .id   = EVENT_WANDERING_PIECE,
         .options =
             {{.text    = "Reduce Global Vorath Counter by 2",
               .effects = {EV_VORATH(2)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_VETERANS_BOND, RELIC_DEEP_HAND),
                    EV_SEL_RELIC}}}},

    [EVENT_VORATHS_DECREE] =
        {.name = "Vorath's Decree",
         .desc = "Vorath's heralds demand a sacrifice.",
         .id   = EVENT_VORATHS_DECREE,
         .options =
             {{.text    = "Remove 1 card; reduce Global Vorath Counter by 3",
               .effects = {EV_VORATH(3), EV_TG_CARD, EV_SEL_REMOVE}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_SOUL_SHARD, RELIC_FORWARD_COMMAND),
                    EV_SEL_RELIC}}}},

    [EVENT_DESERTER] =
        {.name = "The Deserter",
         .desc = "A deserter's tale rallies the rank and file.",
         .id   = EVENT_DESERTER,
         .options =
             {{.text    = "Remove 1 card; all pawns gain +3 value this run",
               .effects = {EV_TG_CARD, EV_SEL_REMOVE, EV_VAL_PAWN(3)}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_WARLORDS_BANNER, RELIC_DEAD_MANS_PACT),
                    EV_SEL_RELIC}}}},

    [EVENT_ARCHIVE] =
        {.name = "The Archive",
         .desc = "Two vaults of relics stand open before you.",
         .id   = EVENT_ARCHIVE,
         .options =
             {{.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(
                        RELIC_PHILOSOPHERS_STONE,
                        RELIC_INHERITED_POWER
                    ),
                    EV_SEL_RELIC}},
              {.text = "Choose 1 relic from 2",
               .effects =
                   {EV_TG_RELIC(RELIC_MASTERS_NOTES, RELIC_ALCHEMISTS_KIT),
                    EV_SEL_RELIC}}}},
};

const Event* const EVENT_REGISTRY[EVENT_COUNT] = {
    [EVENT_SCHOLARS_OFFER]       = &EVENTS[EVENT_SCHOLARS_OFFER],
    [EVENT_DRAGON_COURT_TRIBUTE] = &EVENTS[EVENT_DRAGON_COURT_TRIBUTE],
    [EVENT_DEFECTOR]             = &EVENTS[EVENT_DEFECTOR],
    [EVENT_JANGGI_ELDER]         = &EVENTS[EVENT_JANGGI_ELDER],
    [EVENT_CANNON_SALUTE]        = &EVENTS[EVENT_CANNON_SALUTE],
    [EVENT_MANSAS_COURT]         = &EVENTS[EVENT_MANSAS_COURT],
    [EVENT_SALT_ROAD_MERCHANT]   = &EVENTS[EVENT_SALT_ROAD_MERCHANT],
    [EVENT_STOLEN_GUARD]         = &EVENTS[EVENT_STOLEN_GUARD],
    [EVENT_CAMEL_CARAVAN]        = &EVENTS[EVENT_CAMEL_CARAVAN],
    [EVENT_FEAST_OF_YOD_ABEGA]   = &EVENTS[EVENT_FEAST_OF_YOD_ABEGA],
    [EVENT_WARLORDS_CHALLENGE]   = &EVENTS[EVENT_WARLORDS_CHALLENGE],
    [EVENT_BAZAAR_OF_SAMARKAND]  = &EVENTS[EVENT_BAZAAR_OF_SAMARKAND],
    [EVENT_MIRAGE]               = &EVENTS[EVENT_MIRAGE],
    [EVENT_SPY_REPORT]           = &EVENTS[EVENT_SPY_REPORT],
    [EVENT_DESERT_CROSSING]      = &EVENTS[EVENT_DESERT_CROSSING],
    [EVENT_RONIN]                = &EVENTS[EVENT_RONIN],
    [EVENT_SPY_NETWORK]          = &EVENTS[EVENT_SPY_NETWORK],
    [EVENT_BURNING_PORT]         = &EVENTS[EVENT_BURNING_PORT],
    [EVENT_FORGE_MASTER]         = &EVENTS[EVENT_FORGE_MASTER],
    [EVENT_VETERAN_LANCE]        = &EVENTS[EVENT_VETERAN_LANCE],
    [EVENT_TOURNAMENT]           = &EVENTS[EVENT_TOURNAMENT],
    [EVENT_CHURCH_BLESSING]      = &EVENTS[EVENT_CHURCH_BLESSING],
    [EVENT_SIEGE_ENGINEER]       = &EVENTS[EVENT_SIEGE_ENGINEER],
    [EVENT_PRETENDER]            = &EVENTS[EVENT_PRETENDER],
    [EVENT_ROYAL_DECREE]         = &EVENTS[EVENT_ROYAL_DECREE],
    [EVENT_QUEENS_FAVOR]         = &EVENTS[EVENT_QUEENS_FAVOR],
    [EVENT_WANDERING_PIECE]      = &EVENTS[EVENT_WANDERING_PIECE],
    [EVENT_VORATHS_DECREE]       = &EVENTS[EVENT_VORATHS_DECREE],
    [EVENT_DESERTER]             = &EVENTS[EVENT_DESERTER],
    [EVENT_ARCHIVE]              = &EVENTS[EVENT_ARCHIVE],
};
