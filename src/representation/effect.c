//! effect.c
//!
//! Effect system core. Handles attaching heap copies of effect templates
//! with fresh contexts, firing triggers across both player lists and all
//! live pieces, ticking timed durations, and locating bookkeeping marks.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

static const char* const TRIGGER_NAME[] = {
    "QUERY_CARD_DRAW_COUNT",
    "QUERY_CARD_PLAY_COST",
    "QUERY_CARD_SELL_COST",
    "QUERY_CARD_CAN_DRAW",
    "QUERY_CARD_CAN_PLAY",
    "QUERY_CARD_TARGETS",

    "QUERY_PIECE_ACTION_COST_MOVE",
    "QUERY_PIECE_ACTION_COST_BUY",
    "QUERY_PIECE_ACTION_COST_COMBINE",

    "QUERY_PIECE_CP_COST_BUY",
    "QUERY_PIECE_CP_COST_RECLAIM",

    "QUERY_PIECE_CAN_FLIP",
    "QUERY_PIECE_CAN_MOVE",
    "QUERY_PIECE_CAN_ATTACK",
    "QUERY_PIECE_CAN_BUY",

    "QUERY_PIECE_MOVES",
    "QUERY_PIECE_ATTACKS",

    "QUERY_PIECE_DAMAGE_DEALT",
    "QUERY_PIECE_DAMAGE_TAKEN",
    "QUERY_PIECE_VALUE",

    "QUERY_PIECE_HAS_MOVED",
    "QUERY_PIECE_HAS_FLIPPED",

    "QUERY_METER_DAMAGE_TAKEN",
    "QUERY_METER_REFILL",
    "QUERY_METER_AMOUNT",
    "QUERY_FLIP_COUNT",

    "QUERY_CP_INCOME",
    "QUERY_ENEMY_ARMY_COUNT",
    "QUERY_SQUARE_OWNER",

    "QUERY_BOARD_DIMENSION",
    "QUERY_BOARD_STATE",
    "QUERY_HAND_STATE",
    "ON_BOARD_BUILD",

    "ON_PIECE_FLIP_PRE",
    "ON_PIECE_FLIP",
    "ON_PIECE_FLIP_POST",
    "ON_PIECE_BUY",
    "ON_PIECE_MOVE",
    "ON_PIECE_COMBINE",

    "ON_CARD_PLAY",
    "ON_CARD_SELL",
    "ON_CARDS_DRAWN",
    "ON_CARD_TARGET_SELECTED",
    "ON_COMBO_DOUBLE",
    "ON_COMBO_CLIMAX",

    "ON_TURN_START",
    "ON_TURN_END",

    "ON_BATTLE_SETUP",
    "ON_BATTLE_START",
    "ON_BATTLE_END",
    "ON_CASCADE_END",

    "ON_EVENT_CHOOSE",
    "QUERY_EVENT_TARGETS",
    "ON_EVENT_TARGET_SELECTED",

    "QUERY_ARCHIVE_REVEAL_COUNT",

    "QUERY_NEXT_HAND",
    "ON_ITEM_ACTIVATE",

    "ON_MAP_ENTER",

    "QUERY_RECIPE_ALLOWED",
};

/// effect_trigger_name
///
/// Returns the protocol log spelling of a trigger, the single source the
/// battle and run fire paths share so their effect-fire lines agree.
///
/// Params:
/// - trigger -> trigger to name
///
/// Return: the trigger's log string
///
const char* effect_trigger_name(EffectTrigger trigger) {
    return TRIGGER_NAME[trigger];
}

/// effect_run
///
/// Runs one effect when its trigger matches, emitting the effect-fire
/// log line only when the effect reports it actually applied.
/// Self-filtered and unnamed fires stay silent.
///
/// Params:
/// - effect  -> effect to consider
/// - trigger -> trigger being fired
/// - x       -> queried value, type defined per trigger
///
static void effect_run(Effect* effect, EffectTrigger trigger, void* x) {
    if (!effect->func || effect->trigger != trigger) {
        return;
    }

    bool applied = effect->func(effect->context, x);

    if (applied && effect->name) {
        protocol_emit(
            "log effect name=\"%s\" trigger=%s",
            effect->name,
            effect_trigger_name(trigger)
        );
    }
}

/// effect_attach
///
/// Heap-copies the given effect template into the list and allocates a
/// zeroed context for the copy. The list owns both allocations until the
/// effect expires or the list is cleared.
///
/// Params:
/// - list   -> list to attach the copy to
/// - effect -> const template to copy
///
/// Return: pointer to the attached copy, nullptr on allocation failure
///
Effect* effect_attach(LinkedList* list, const Effect* effect) {
    Effect* copy = malloc(sizeof(Effect));

    if (!copy) {
        return nullptr;
    }

    *copy         = *effect;
    copy->context = calloc(1, sizeof(EffectContext));

    if (!copy->context) {
        free(copy);
        return nullptr;
    }

    ll_push(list, copy);

    return copy;
}

/// effect_fire
///
/// Fires every effect matching the trigger, walking the fired side's
/// player list only, then every live piece's embedded effects. List
/// membership is the side scoping; pieces self-filter via context. x is
/// the value the trigger is computing and mutates in place through each
/// matching effect.
///
/// Params:
/// - battle  -> battle to walk for effects
/// - side    -> subject side whose list is walked
/// - trigger -> trigger to match effects against
/// - x       -> queried value, type defined per trigger
///
void effect_fire(
    BattleState*  battle,
    Side          side,
    EffectTrigger trigger,
    void*         x
) {
    LinkedList* list =
        side == SIDE_WHITE ? &battle->white.effects : &battle->black.effects;

    LLNode* node = list->head;

    while (node) {
        effect_run(node->data, trigger, x);

        node = node->next;
    }

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (!cell || cell == &VOID_CELL) {
            continue;
        }

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            effect_run(&cell->piece->effects[slot], trigger, x);
        }
    }
}

/// effect_tick
///
/// Decrements every timed duration in the list by one turn and reaps
/// expired effects, freeing both the effect and its context. Effects
/// lasting the entire battle or run are left untouched.
///
/// Params:
/// - list -> list to tick
///
void effect_tick(LinkedList* list) {
    LLNode* node = list->head;

    while (node) {
        LLNode* next   = node->next;
        Effect* effect = node->data;

        if (effect->lasts_for == TURNS_1) {
            free(effect->context);
            ll_poll(list, node);
        } else if (effect->lasts_for <= TURNS_10) {
            effect->lasts_for = (EffectDuration) (effect->lasts_for - 1);
        }

        node = next;
    }
}

/// effect_clear
///
/// Removes every effect from the list, freeing each effect and its
/// context. The list itself remains valid and empty afterwards.
///
/// Params:
/// - list -> list to clear
///
void effect_clear(LinkedList* list) {
    LLNode* node = list->head;

    while (node) {
        Effect* effect = node->data;

        free(effect->context);

        node = node->next;
    }

    ll_free(list);
}

/// eff_noop
///
/// Marker effect body that does nothing when fired. Mark effects use it
/// so bookkeeping state can live in a list without altering any query.
///
/// Params:
/// - context -> the mark's private context, unused
/// - x       -> queried value, unused
///
/// Return: false, a mark never applies to a query
///
bool eff_noop(EffectContext* context, void* x) {
    (void) context;
    (void) x;

    return false;
}

/// effect_find_mark
///
/// Finds the first mark effect in the list whose context matches the
/// given subject and tag. Marks store their subject in args[0] and their
/// tag in args[1] by convention.
///
/// Params:
/// - list    -> list to search
/// - tag     -> tag identifying the mark kind
/// - subject -> subject pointer the mark must reference
///
/// Return: matching effect, nullptr when none matches
///
Effect* effect_find_mark(LinkedList* list, uintptr_t tag, void* subject) {
    LLNode* node = list->head;

    while (node) {
        Effect* effect = node->data;

        if (effect->func == eff_noop && effect->context->args[0] == subject &&
            (uintptr_t) effect->context->args[1] == tag) {
            return effect;
        }

        node = node->next;
    }

    return nullptr;
}
