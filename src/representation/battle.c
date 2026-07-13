//! battle.c
//!
//! Battle engine. Owns the board, the turn loop, all player actions, the
//! resolve and cascade pipeline, the movement generation kit, and the
//! subject registers effects use to self-filter during a firing.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                               SUBJECT REGISTERS
\*----------------------------------------------------------------------------*/

const PieceInfo      VOID_CELL = {};

static BattleState*  CURRENT_BATTLE;
static PieceInfo*    SUBJECT;
static PieceInfo*    VICTIM;
static Card*         SUBJECT_CARD;
static const Piece*  BUY_PIECE;
static Square        MOVE_FROM = {-1, -1};
static PieceInfo**   DAMAGERS;

static EngineState*  BATTLE_ENGINE;
static Side          HUMAN_SIDE;
static Side          ACTING_SIDE;
static unsigned int  BATTLE_RNG;
static PieceInfo*    FLIPPED_PIECE;
static PieceInfo*    DAMAGER_LIST[MAX_BOARD_SIZE + 1];

static const PieceID COMBO_RESULTS[] = {
    PIECE_SANG,
    PIECE_NORTHERN_CAVALRY,
    PIECE_HWACHA,
    PIECE_MEDEQ_SQUAD,
    PIECE_SULTANS_LEVY,
    PIECE_OLD_KING,
    PIECE_CATAPHRACT,
    PIECE_ROOK,
    PIECE_WAR_ELEPHANT,
    PIECE_HONORABLE_HORSE,
    PIECE_PROMOTED_BISHOP,
    PIECE_DAIMYO,
    PIECE_DRAGON,
    PIECE_CHANCELLOR,
    PIECE_SOVEREIGN_BANNER,
    PIECE_NONE,
};

static const PieceID RECIPES[][3] = {
    {PIECE_BING, PIECE_SABA, PIECE_SANG},
    {PIECE_FARAS, PIECE_BING, PIECE_NORTHERN_CAVALRY},
    {PIECE_PAO, PIECE_BISHOP, PIECE_HWACHA},

    {PIECE_MEDEQ, PIECE_MEDEQ, PIECE_MEDEQ_SQUAD},
    {PIECE_NEGUS_GUARD, PIECE_MEDEQ, PIECE_SULTANS_LEVY},

    {PIECE_MAKWANAM, PIECE_WAZIR, PIECE_OLD_KING},
    {PIECE_KNIGHT, PIECE_JAMAL, PIECE_CATAPHRACT},
    {PIECE_KYOSHA, PIECE_WAZIR, PIECE_ROOK},
    {PIECE_ZIRAAFA, PIECE_JAMAL, PIECE_WAR_ELEPHANT},

    {PIECE_KYOSHA, PIECE_KNIGHT, PIECE_HONORABLE_HORSE},
    {PIECE_BISHOP, PIECE_WAZIR, PIECE_PROMOTED_BISHOP},
    {PIECE_KINSHO, PIECE_GINSHO, PIECE_DAIMYO},
    {PIECE_ROOK, PIECE_WAZIR, PIECE_DRAGON},

    {PIECE_ROOK, PIECE_BISHOP, PIECE_CHANCELLOR},
    {PIECE_QUEEN, PIECE_KYOSHA, PIECE_SOVEREIGN_BANNER},

    {PIECE_NONE, PIECE_NONE, PIECE_NONE},
};

static int        KINGDOM_PLAYS[KINGDOM_COUNT];

static PieceInfo* REAP_LIST[2 * MAX_BOARD_SIZE];
static size_t     REAP_COUNT;

/// battle_reap
///
/// Frees every piece unlinked from the board since the last reap. Removal
/// only unlinks and queues a piece so effects may remove their own piece
/// while effect_fire is still walking the board; the actual free happens
/// here at turn and battle boundaries where no walk is in flight.
///
static void       battle_reap(void) {
    for (size_t i = 0; i < REAP_COUNT; i++) {
        PieceInfo* piece = REAP_LIST[i];

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            free(piece->piece->effects[slot].context);
        }

        free(piece->piece);
        free(piece);
    }

    REAP_COUNT = 0;
}

/// battle_current
///
/// Returns the battle the currently firing trigger belongs to. Valid
/// only while effect_fire is walking effects.
///
/// Return: current battle, nullptr outside a firing
///
BattleState* battle_current(void) {
    return CURRENT_BATTLE;
}

/// battle_subject
///
/// Returns the piece the currently firing trigger is about: the mover,
/// the priced piece, the damage dealer, or the flip candidate.
///
/// Return: subject piece, nullptr when the trigger has none
///
PieceInfo* battle_subject(void) {
    return SUBJECT;
}

/// battle_victim
///
/// Returns the piece receiving damage during a damage query. A nullptr
/// during QUERY_PIECE_DAMAGE_DEALT means an effective-value query.
///
/// Return: victim piece, nullptr outside damage queries
///
PieceInfo* battle_victim(void) {
    return VICTIM;
}

/// battle_subject_card
///
/// Returns the card the currently firing card trigger is about.
///
/// Return: subject card, nullptr outside card triggers
///
Card* battle_subject_card(void) {
    return SUBJECT_CARD;
}

/// battle_buy_piece
///
/// Returns the piece template being priced during a buy query, letting
/// pricing effects act on the piece's kingdom or id. Null outside a buy.
///
/// Return: the template under purchase, or null
///
const Piece* battle_buy_piece(void) {
    return BUY_PIECE;
}

/// battle_move_from
///
/// Returns the origin square of the move being reported by the current
/// ON_PIECE_MOVE firing.
///
/// Return: origin square, SQUARE_END outside ON_PIECE_MOVE
///
Square battle_move_from(void) {
    return MOVE_FROM;
}

/// battle_damagers
///
/// Returns the null-terminated list of pieces that dealt damage in the
/// current resolve, for post-flip attribution effects.
///
/// Return: damager list, nullptr outside resolve
///
PieceInfo** battle_damagers(void) {
    return DAMAGERS;
}

/*----------------------------------------------------------------------------*\
                                 BOARD INDEXING
\*----------------------------------------------------------------------------*/

/// square_index
///
/// Converts a square to its board cell index at the fixed stride of
/// twenty cells per row.
///
/// Params:
/// - square -> square to convert
///
/// Return: cell index into piece_board
///
static size_t square_index(Square square) {
    return (size_t) square.y * 20 + (size_t) square.x;
}

/*----------------------------------------------------------------------------*\
                                   LIFECYCLE
\*----------------------------------------------------------------------------*/

/// battle_player
///
/// Returns the player state of the given side.
///
/// Params:
/// - battle -> battle to look into
/// - side   -> side to fetch
///
/// Return: that side's player state
///
PlayerState* battle_player(BattleState* battle, Side side) {
    return side == SIDE_WHITE ? &battle->white : &battle->black;
}

/// battle_enemy
///
/// Returns the side opposing the given side.
///
/// Params:
/// - side -> side to oppose
///
/// Return: the opposing side
///
Side battle_enemy(Side side) {
    return side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
}

/// battle_find_king
///
/// Finds a side's king on the board.
///
/// Params:
/// - battle -> battle to search
/// - side   -> owning side
///
/// Return: the king, nullptr when it is not on the board
///
PieceInfo* battle_find_king(BattleState* battle, Side side) {
    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side &&
            cell->piece->id == PIECE_KING) {
            return cell;
        }
    }

    return nullptr;
}

/// battle_meter_gain
///
/// Adds to a side's meter and clamps it to the two hundred percent
/// ceiling, the shared path for card and effect meter rewards.
///
/// Params:
/// - battle -> battle the side plays in
/// - side   -> side gaining meter
/// - amount -> meter to add, may be negative
///
void battle_meter_gain(BattleState* battle, Side side, int amount) {
    PlayerState* player = battle_player(battle, side);
    int          max    = battle_meter_max(battle, side);

    player->meter += amount;

    if (player->meter > 2 * max) {
        player->meter = 2 * max;
    }
}

/// battle_draw
///
/// Draws cards into the side's hand from the run's unlocked, registered,
/// drawable card set. Only the human holds a hand; the AI plays no cards
/// in this build. Cards are sampled with replacement from the eligible
/// pool, each gated by QUERY_CARD_CAN_DRAW.
///
/// Params:
/// - battle -> battle whose hand is filled
/// - side   -> side drawing cards
/// - count  -> number of cards to draw
///
void battle_draw(BattleState* battle, Side side, size_t count) {
    if (side != HUMAN_SIDE) {
        return;
    }

    BattleState* saved_battle = CURRENT_BATTLE;
    Card*        saved_card   = SUBJECT_CARD;
    PlayerState* player       = battle_player(battle, side);

    CURRENT_BATTLE            = battle;

    CardID pool[CARD_COUNT];
    size_t pool_size = 0;

    for (CardID id = 0; id < CARD_COUNT; id++) {
        if (!BATTLE_ENGINE->run->cards[id] || !CARD_REGISTRY[id]) {
            continue;
        }

        bool can     = true;

        SUBJECT_CARD = (Card*) CARD_REGISTRY[id];
        effect_fire(battle, side, QUERY_CARD_CAN_DRAW, &can);

        if (can) {
            pool[pool_size] = id;
            pool_size++;
        }
    }

    for (size_t drawn = 0; drawn < count && pool_size > 0; drawn++) {
        size_t slot = 0;

        while (slot < MAX_DRAWN_CARDS && player->hand[slot]) {
            slot++;
        }

        if (slot >= MAX_DRAWN_CARDS) {
            break;
        }

        CardID pick = pool[(size_t) rand_r(&BATTLE_RNG) % pool_size];

        if (drawn == 0 && battle->modifier &&
            battle->modifier->id == MODIFIER_LUCKY_STRIKE) {
            pick = pool[0];

            for (size_t p = 1; p < pool_size; p++) {
                if (CARD_REGISTRY[pool[p]]->tier > CARD_REGISTRY[pick]->tier) {
                    pick = pool[p];
                }
            }
        }

        player->hand[slot] = (Card*) CARD_REGISTRY[pick];
    }

    SUBJECT_CARD   = saved_card;
    CURRENT_BATTLE = saved_battle;
}

/// turn_start
///
/// Starts a side's turn: restores actions, applies queried income, fires
/// ON_TURN_START, resets the same-kingdom combo counters, and draws the
/// side's hand.
///
/// Params:
/// - battle -> battle whose turn starts
/// - side   -> side beginning its turn
///
static void turn_start(BattleState* battle, Side side) {
    PlayerState* player = battle_player(battle, side);

    battle_reap();

    player->actions = 3;

    int income      = 10;

    CURRENT_BATTLE  = battle;
    effect_fire(battle, side, QUERY_CP_INCOME, &income);

    player->cp += income;

    effect_fire(battle, side, ON_TURN_START, (void*) (uintptr_t) battle->turn);
    CURRENT_BATTLE = nullptr;

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        KINGDOM_PLAYS[kingdom] = 0;
    }

    int draw       = 3;

    CURRENT_BATTLE = battle;
    effect_fire(battle, side, QUERY_CARD_DRAW_COUNT, &draw);
    CURRENT_BATTLE = nullptr;

    if (draw > 0) {
        battle_draw(battle, side, (size_t) draw);
    }
}

/// battle_finish
///
/// Ends the battle: emits the result line, frees everything the battle
/// owns, and detaches it from the engine.
///
/// Params:
/// - battle -> battle to finish
/// - winner -> side that won
///
static void battle_finish(BattleState* battle, Side winner) {
    CURRENT_BATTLE = battle;
    effect_fire(battle, winner, ON_BATTLE_END, (void*) (uintptr_t) winner);
    CURRENT_BATTLE = nullptr;

    protocol_emit("result won=%d", winner == HUMAN_SIDE ? 1 : 0);

    battle_free(battle);
    free(battle);

    BATTLE_ENGINE->battle = nullptr;

    run_battle_result(BATTLE_ENGINE, winner == HUMAN_SIDE);
}

/// battle_concede
///
/// Ends the battle immediately as a loss for the human, running the
/// standard finish and result pipeline.
///
/// Params:
/// - battle -> battle to concede
///
void battle_concede(BattleState* battle) {
    battle_finish(battle, HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE);
}

/// battle_free
///
/// Frees everything the battle owns: every live piece with its heap
/// piece copy and effect contexts, and both player effect lists.
///
/// Params:
/// - battle -> battle to deallocate
///
void battle_free(BattleState* battle) {
    battle_reap();

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL) {
            battle_remove(battle, cell);
        }
    }

    battle_reap();

    effect_clear(&battle->white.effects);
    effect_clear(&battle->black.effects);
}

/// battle_spawn
///
/// Places a new piece on the board by heap-copying the registry template
/// and allocating a zeroed context for each of its embedded effects,
/// storing the owning piece info in args[0] of each context.
///
/// Params:
/// - battle -> battle to spawn into
/// - id     -> registry piece to copy
/// - at     -> destination square, must be empty
/// - side   -> owning side
///
/// Return: the spawned piece, nullptr when the spawn is invalid
///
PieceInfo* battle_spawn(BattleState* battle, PieceID id, Square at, Side side) {
    const Piece* template = PIECE_REGISTRY[id];

    if (!template || !battle_in_bounds(battle, at) || battle_at(battle, at)) {
        return nullptr;
    }

    PieceInfo* info = malloc(sizeof(PieceInfo));
    Piece*     copy = malloc(sizeof(Piece));

    if (!info || !copy) {
        free(info);
        free(copy);
        return nullptr;
    }

    *copy        = *template;

    info->piece  = copy;
    info->square = at;
    info->side   = side;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (!copy->effects[slot].func) {
            continue;
        }

        copy->effects[slot].context          = calloc(1, sizeof(EffectContext));
        copy->effects[slot].context->args[0] = info;
    }

    battle->board.piece_board[square_index(at)] = info;

    return info;
}

/// battle_flip
///
/// Flips a piece to the opposing side, firing ON_PIECE_FLIP_PRE before
/// the toggle so effects may redirect or consume it, and ON_PIECE_FLIP
/// right after the toggle when it happened. ON_PIECE_FLIP_POST is fired
/// by the cascade once the step settles; callers flipping outside the
/// cascade fire it themselves.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to flip
///
void battle_flip(BattleState* battle, PieceInfo* piece) {
    PieceInfo* pick = piece;

    FLIPPED_PIECE   = nullptr;
    CURRENT_BATTLE  = battle;
    SUBJECT         = piece;

    effect_fire(battle, piece->side, ON_PIECE_FLIP_PRE, &pick);

    if (pick) {
        pick->side    = pick->side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

        FLIPPED_PIECE = pick;

        protocol_emit(
            "log flip piece=%s x=%d y=%d",
            pick->piece->name,
            pick->square.x,
            pick->square.y
        );

        effect_fire(battle, pick->side, ON_PIECE_FLIP, pick);
    }

    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;
}

/// battle_remove
///
/// Removes a piece from the board by unlinking its cell and queuing it for
/// the next reap. Unlinking is immediate so meter, movement, and spawning
/// see the removal at once; the free is deferred so an effect may remove
/// its own piece while effect_fire is still walking the board.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to remove
///
void battle_remove(BattleState* battle, PieceInfo* piece) {
    battle->board.piece_board[square_index(piece->square)] = nullptr;

    if (REAP_COUNT < 2 * MAX_BOARD_SIZE) {
        REAP_LIST[REAP_COUNT] = piece;
        REAP_COUNT++;
    }
}

/*----------------------------------------------------------------------------*\
                                    ACTIONS
\*----------------------------------------------------------------------------*/

/// battle_move
///
/// Moves an acting-side piece if the destination is in its generated
/// move list and the player can pay the queried action cost. Fires
/// ON_PIECE_MOVE with the origin square in the move register.
///
/// Params:
/// - battle -> battle to act in
/// - from   -> square of the piece to move
/// - to     -> destination square
///
/// Return: true when the move was performed
///
bool battle_move(BattleState* battle, Square from, Square to) {
    PieceInfo* piece = battle_at(battle, from);

    if (!piece || piece->side != ACTING_SIDE) {
        return false;
    }

    Square* moves = battle_moves(battle, piece);
    bool    legal = false;

    for (size_t i = 0; !(moves[i].x == -1 && moves[i].y == -1); i++) {
        if (moves[i].x == to.x && moves[i].y == to.y) {
            legal = true;
            break;
        }
    }

    if (!legal) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);

    int          cost   = 1;

    CURRENT_BATTLE      = battle;
    SUBJECT             = piece;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_ACTION_COST_MOVE, &cost);

    if (player->actions < cost) {
        CURRENT_BATTLE = nullptr;
        SUBJECT        = nullptr;
        return false;
    }

    player->actions     -= cost;

    PieceInfo* occupant  = battle_at(battle, to);

    if (occupant && occupant->side == piece->side) {
        battle->board.piece_board[square_index(from)] = occupant;
        battle->board.piece_board[square_index(to)]   = piece;

        occupant->square                              = from;
        piece->square                                 = to;
    } else {
        battle->board.piece_board[square_index(from)] = nullptr;
        piece->square                                 = to;
        battle->board.piece_board[square_index(to)]   = piece;
    }

    MOVE_FROM = from;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_MOVE, piece);
    MOVE_FROM      = SQUARE_END;

    Effect* moved  = effect_find_mark(&player->effects, MARK_MOVED, piece);

    if (moved) {
        moved->context->args[2] = (void*) (uintptr_t) battle->turn;
    } else {
        Effect mark = {
            .func      = eff_noop,
            .name      = "Moved",
            .trigger   = ON_TURN_START,
            .lasts_for = TURNS_2,
        };

        Effect* stamp = effect_attach(&player->effects, &mark);

        if (stamp) {
            stamp->context->args[0] = piece;
            stamp->context->args[1] = (void*) (uintptr_t) MARK_MOVED;
            stamp->context->args[2] = (void*) (uintptr_t) battle->turn;
        }
    }

    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;

    return true;
}

/// battle_challenge_allows_buy
///
/// Enforces the buy constraints of the active challenge: Pacifist Doctrine
/// forbids pieces valued above twenty, Solo Vanguard forbids a second
/// non-king piece on the human's side.
///
/// Params:
/// - battle   -> battle being played
/// - template -> piece being bought
///
/// Return: true when the purchase is permitted
///
static bool battle_challenge_allows_buy(
    BattleState* battle,
    const Piece* template
) {
    ChallengeRunID challenge = BATTLE_ENGINE->run->challenge;

    if (challenge == CHALLENGE_PACIFIST_DOCTRINE && template->value > 20) {
        return false;
    }

    if (challenge == CHALLENGE_SOLO_VANGUARD) {
        for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
            PieceInfo* cell = battle->board.piece_board[index];

            if (cell && cell != &VOID_CELL && cell->side == HUMAN_SIDE &&
                cell->piece->id != PIECE_KING) {
                return false;
            }
        }
    }

    return true;
}

/// battle_price
///
/// Computes the base currency cost of a piece before the buy queries,
/// applying the region rule: pieces native to the battle's home kingdom
/// cost forty percent less, foreign pieces twenty percent more. The king
/// and kingdomless pieces price at face value. The human's Trade Routes
/// relic drops the foreign markup for its own purchases.
///
/// Params:
/// - battle   -> battle whose node fixes the home kingdom
/// - template -> piece template being priced
/// - side     -> side making the purchase
///
/// Return: the region-adjusted base cost
///
static int battle_price(
    BattleState* battle,
    const Piece* template,
    Side         side
) {
    int cost = template->value;

    if (template->kingdom == KINGDOM_NONE || !battle->node ||
        !battle->node->kingdom) {
        return cost;
    }

    if (template->kingdom == battle->node->kingdom->id) {
        return cost * 60 / 100;
    }

    bool trade_routes =
        side == HUMAN_SIDE && BATTLE_ENGINE &&
        BATTLE_ENGINE->run->relics[RELIC_TRADE_ROUTES];

    return trade_routes ? cost : cost * 120 / 100;
}

/// battle_buy
///
/// Buys a piece onto any empty square, applying the pricing and action
/// cost queries, then fires ON_PIECE_BUY. The king and combination
/// results are never in the shop; the human seat may only buy pieces
/// unlocked in the run.
///
/// Params:
/// - battle -> battle to act in
/// - id     -> piece to buy
/// - at     -> destination square
///
/// Return: true when the purchase was performed
///
bool battle_buy(BattleState* battle, PieceID id, Square at) {
    const Piece* template = PIECE_REGISTRY[id];

    if (!template || id == PIECE_KING) {
        return false;
    }

    for (size_t i = 0; COMBO_RESULTS[i] != PIECE_NONE; i++) {
        if (COMBO_RESULTS[i] == id) {
            return false;
        }
    }

    if (ACTING_SIDE == HUMAN_SIDE && !BATTLE_ENGINE->run->pieces[id]) {
        return false;
    }

    if (ACTING_SIDE == HUMAN_SIDE &&
        !battle_challenge_allows_buy(battle, template)) {
        return false;
    }

    PlayerState* player      = battle_player(battle, ACTING_SIDE);

    int          cost        = battle_price(battle, template, ACTING_SIDE);
    int          action_cost = 1;

    CURRENT_BATTLE           = battle;
    BUY_PIECE                = template;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_CP_COST_BUY, &cost);
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_ACTION_COST_BUY, &action_cost);
    BUY_PIECE      = nullptr;
    CURRENT_BATTLE = nullptr;

    Effect* storm =
        effect_find_mark(&player->effects, CARD_PAWN_STORM, nullptr);

    if (storm && id == PIECE_PAWN && (uintptr_t) storm->context->args[2] > 0) {
        action_cost = 0;

        if ((uintptr_t) storm->context->args[2] == 1) {
            cost = 0;
        }
    }

    Effect* reforge = effect_find_mark(&player->effects, CARD_REFORGE, nullptr);

    bool reforge_ok =
        reforge && !reforge->context->args[3] &&
        (PieceID) (uintptr_t) reforge->context->args[2] == id &&
        battle->turn <= (size_t) (uintptr_t) reforge->context->args[4] + 1;

    if (reforge_ok) {
        cost = cost * 70 / 100;
    }

    if (player->cp < cost || player->actions < action_cost) {
        return false;
    }

    PieceInfo* piece = battle_spawn(battle, id, at, ACTING_SIDE);

    if (!piece) {
        return false;
    }

    player->cp      -= cost;
    player->actions -= action_cost;
    player->meter   += battle_value(battle, piece, nullptr);

    if (storm && id == PIECE_PAWN && (uintptr_t) storm->context->args[2] > 0) {
        storm->context->args[2] =
            (void*) ((uintptr_t) storm->context->args[2] - 1);
    }

    if (reforge_ok) {
        reforge->context->args[3] = (void*) 1;
    }

    CURRENT_BATTLE = battle;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_BUY, piece);
    CURRENT_BATTLE = nullptr;

    return true;
}

/// battle_combine
///
/// Combines the pieces on the two squares when they match an unlocked
/// recipe, removing both parents and spawning the result on the second
/// square, then fires ON_PIECE_COMBINE.
///
/// Params:
/// - battle -> battle to act in
/// - a      -> square of the first parent
/// - b      -> square of the second parent and result destination
///
/// Return: true when the combination was performed
///
bool battle_combine(BattleState* battle, Square a, Square b) {
    PieceInfo* first  = battle_at(battle, a);
    PieceInfo* second = battle_at(battle, b);

    if (!first || !second || first == second || first->side != ACTING_SIDE ||
        second->side != ACTING_SIDE) {
        return false;
    }

    PieceID result = PIECE_NONE;

    for (size_t i = 0; RECIPES[i][2] != PIECE_NONE; i++) {
        PieceID left  = RECIPES[i][0];
        PieceID right = RECIPES[i][1];

        if ((first->piece->id == left && second->piece->id == right) ||
            (first->piece->id == right && second->piece->id == left)) {
            result = RECIPES[i][2];
            break;
        }
    }

    if (result == PIECE_NONE) {
        return false;
    }

    if (ACTING_SIDE == HUMAN_SIDE && !BATTLE_ENGINE->run->pieces[result]) {
        return false;
    }

    PlayerState* player      = battle_player(battle, ACTING_SIDE);

    int          action_cost = 1;

    CURRENT_BATTLE           = battle;
    effect_fire(
        battle,
        ACTING_SIDE,
        QUERY_PIECE_ACTION_COST_COMBINE,
        &action_cost
    );
    CURRENT_BATTLE = nullptr;

    if (player->actions < action_cost) {
        return false;
    }

    int first_value  = battle_value(battle, first, nullptr);
    int second_value = battle_value(battle, second, nullptr);

    battle_remove(battle, first);
    battle_remove(battle, second);

    PieceInfo* piece = battle_spawn(battle, result, b, ACTING_SIDE);

    if (!piece) {
        return false;
    }

    player->actions -= action_cost;
    player->meter +=
        battle_value(battle, piece, nullptr) - first_value - second_value;

    CURRENT_BATTLE = battle;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_COMBINE, piece);
    CURRENT_BATTLE = nullptr;

    return true;
}

/// battle_play
///
/// Plays a card from the hand, paying its queried play cost, running its
/// immediate effects and attaching its durational ones, then fires
/// ON_CARD_PLAY and advances the same-kingdom combo chain. The a and b
/// targets are encoded per the card's documentation.
///
/// Params:
/// - battle -> battle to act in
/// - hand   -> hand index of the card to play
/// - a      -> first encoded target, card specific
/// - b      -> second encoded target, card specific
///
/// Return: true when the card was played
///
bool battle_play(BattleState* battle, size_t hand, long a, long b) {
    if (hand >= MAX_DRAWN_CARDS) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);
    Card*        card   = player->hand[hand];

    if (!card) {
        return false;
    }

    int cost       = card->play_cost;

    CURRENT_BATTLE = battle;
    SUBJECT_CARD   = card;
    effect_fire(battle, ACTING_SIDE, QUERY_CARD_PLAY_COST, &cost);

    if (player->cp < cost) {
        SUBJECT_CARD   = nullptr;
        CURRENT_BATTLE = nullptr;
        return false;
    }

    player->cp -= cost;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        const Effect* effect = &card->effects[slot];

        if (!effect->func) {
            continue;
        }

        if (effect->trigger == ON_CARD_PLAY) {
            EffectContext ctx = {};

            ctx.args[0]       = (void*) (uintptr_t) ACTING_SIDE;
            ctx.args[1]       = (void*) (uintptr_t) a;
            ctx.args[2]       = (void*) (uintptr_t) b;

            if (effect->func(&ctx, card) && effect->name) {
                protocol_emit(
                    "log effect name=\"%s\" "
                    "trigger=ON_CARD_PLAY",
                    effect->name
                );
            }

            continue;
        }

        Effect* attached = effect_attach(&player->effects, effect);

        if (!attached) {
            continue;
        }

        if (attached->func == eff_noop) {
            attached->context->args[1] = (void*) (uintptr_t) card->id;
            attached->context->args[2] = (void*) (uintptr_t) 3;
        } else {
            attached->context->args[0] = (void*) (uintptr_t) ACTING_SIDE;
            attached->context->args[1] = (void*) (uintptr_t) a;
            attached->context->args[2] = (void*) (uintptr_t) b;
        }
    }

    effect_fire(battle, ACTING_SIDE, ON_CARD_PLAY, card);

    if (card->id == CARD_QUEENS_GAMBIT) {
        battle_draw(battle, ACTING_SIDE, 3);
    }

    if (card->kingdom < KINGDOM_COUNT) {
        KINGDOM_PLAYS[card->kingdom]++;

        if (KINGDOM_PLAYS[card->kingdom] == 2) {
            player->cp += 15;
            protocol_emit(
                "log combo kingdom=%d refund=15",
                (int) card->kingdom
            );
        } else if (KINGDOM_PLAYS[card->kingdom] == 3) {
            protocol_emit("log climax kingdom=%d", (int) card->kingdom);
            KINGDOM_CLIMAX[card->kingdom](battle, ACTING_SIDE);
        }
    }

    player->hand[hand] = nullptr;

    SUBJECT_CARD       = nullptr;
    CURRENT_BATTLE     = nullptr;

    return true;
}

/// battle_sell
///
/// Sells a card from the hand for its queried sell value, then fires
/// ON_CARD_SELL.
///
/// Params:
/// - battle -> battle to act in
/// - hand   -> hand index of the card to sell
///
/// Return: true when the card was sold
///
bool battle_sell(BattleState* battle, size_t hand) {
    if (hand >= MAX_DRAWN_CARDS) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);
    Card*        card   = player->hand[hand];

    if (!card) {
        return false;
    }

    int value      = card->sell_cost;

    CURRENT_BATTLE = battle;
    SUBJECT_CARD   = card;
    effect_fire(battle, ACTING_SIDE, QUERY_CARD_SELL_COST, &value);

    player->cp += value;

    effect_fire(battle, ACTING_SIDE, ON_CARD_SELL, card);

    SUBJECT_CARD       = nullptr;
    CURRENT_BATTLE     = nullptr;

    player->hand[hand] = nullptr;

    return true;
}

/// battle_reclaim
///
/// Reclaims a friendly piece from the board for its queried reclaim
/// cost, removing it and refunding its value.
///
/// Params:
/// - battle -> battle to act in
/// - at     -> square of the piece to reclaim
///
/// Return: true when the reclaim was performed
///
bool battle_reclaim(BattleState* battle, Square at) {
    PieceInfo* piece = battle_at(battle, at);

    if (!piece || piece->side == ACTING_SIDE ||
        piece->piece->id == PIECE_KING) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);

    int          cost   = 30;

    CURRENT_BATTLE      = battle;
    SUBJECT             = piece;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_CP_COST_RECLAIM, &cost);
    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;

    if (player->cp < cost) {
        return false;
    }

    player->cp -= cost;

    battle_flip(battle, piece);

    return true;
}

/*----------------------------------------------------------------------------*\
                              RESOLVE AND CASCADE
\*----------------------------------------------------------------------------*/

/// battle_resolve
///
/// Resolves the attacker side's coverage into total meter damage on the
/// receiver, filling the damager register with every piece that dealt
/// damage.
///
/// Params:
/// - battle   -> battle to resolve
/// - attacker -> side whose pieces attack
///
/// Return: total damage dealt to the receiving meter
///
static int battle_resolve(BattleState* battle, Side attacker) {
    int    total    = 0;
    size_t damagers = 0;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* piece = battle->board.piece_board[index];

        if (!piece || piece == &VOID_CELL || piece->side != attacker) {
            continue;
        }

        Square* coverage = battle_attacks(battle, piece);
        int     dealt    = 0;

        Square  targets[MAX_BOARD_SIZE + 1];
        size_t  count = 0;

        while (!(coverage[count].x == -1 && coverage[count].y == -1)) {
            targets[count] = coverage[count];
            count++;
        }

        for (size_t i = 0; i < count; i++) {
            PieceInfo* victim = battle_at(battle, targets[i]);

            if (!victim || victim->side == attacker ||
                victim->side == SIDE_NEUTRAL) {
                continue;
            }

            dealt += battle_value(battle, piece, victim);
        }

        if (dealt > 0) {
            total                  += dealt;
            DAMAGER_LIST[damagers]  = piece;
            damagers++;
        }
    }

    DAMAGER_LIST[damagers] = nullptr;

    return total;
}

/// battle_cascade
///
/// Runs the flip cascade on the receiving side after meter damage:
/// while the meter is empty a flip candidate is chosen and flipped, the
/// meter refills through QUERY_METER_REFILL, and any remaining deficit
/// carries over. The gaining side's meter grows by however much its
/// maximum actually changed, so redirected or consumed flips account
/// for themselves. A receiver reduced to its lone king loses the battle.
///
/// Params:
/// - battle   -> battle to cascade in
/// - receiver -> side whose meter was damaged
///
/// Return: true when the battle ended by a king flip
///
static bool battle_cascade(BattleState* battle, Side receiver) {
    PlayerState* player = battle_player(battle, receiver);

    while (player->meter <= 0) {
        int        deficit = -player->meter;

        PieceInfo* candidates[MAX_BOARD_SIZE];
        size_t     count     = 0;
        size_t     non_kings = 0;

        for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
            PieceInfo* cell = battle->board.piece_board[index];

            if (!cell || cell == &VOID_CELL || cell->side != receiver ||
                cell->piece->id == PIECE_KING) {
                continue;
            }

            non_kings++;

            bool can       = true;

            CURRENT_BATTLE = battle;
            SUBJECT        = cell;
            effect_fire(battle, receiver, QUERY_PIECE_CAN_FLIP, &can);
            CURRENT_BATTLE = nullptr;
            SUBJECT        = nullptr;

            if (can) {
                candidates[count] = cell;
                count++;
            }
        }

        if (count == 0) {
            if (non_kings == 0) {
                PieceInfo* king = battle_find_king(battle, receiver);

                if (king) {
                    battle_flip(battle, king);
                }

                return true;
            }

            player->meter = 0;

            return false;
        }

        Side   gainer_side  = receiver == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

        PlayerState* gainer = battle_player(battle, gainer_side);
        int          max_before = battle_meter_max(battle, gainer_side);

        int flips =
            battle->modifier && battle->modifier->id == MODIFIER_BLOODBATH
                ? 2
                : 1;

        PieceInfo* toggled[2];
        int        toggled_count = 0;

        for (int f = 0; f < flips && count > 0; f++) {
            size_t pick = (size_t) rand_r(&BATTLE_RNG) % count;

            battle_flip(battle, candidates[pick]);

            if (FLIPPED_PIECE) {
                toggled[toggled_count] = FLIPPED_PIECE;
                toggled_count++;
            }

            candidates[pick] = candidates[count - 1];
            count--;
        }

        int refill     = battle_meter_max(battle, receiver);

        CURRENT_BATTLE = battle;
        effect_fire(battle, receiver, QUERY_METER_REFILL, &refill);
        CURRENT_BATTLE  = nullptr;

        player->meter   = refill - deficit;

        int gainer_max  = battle_meter_max(battle, gainer_side);

        gainer->meter  += gainer_max - max_before;

        if (gainer->meter > 2 * gainer_max) {
            gainer->meter = 2 * gainer_max;
        }

        for (int f = 0; f < toggled_count; f++) {
            CURRENT_BATTLE = battle;
            SUBJECT        = toggled[f];

            effect_fire(
                battle,
                toggled[f]->side,
                ON_PIECE_FLIP_POST,
                toggled[f]
            );

            CURRENT_BATTLE = nullptr;
            SUBJECT        = nullptr;
        }
    }

    return false;
}

/// battle_half_turn
///
/// Finishes one side's half of the round: fires ON_TURN_END, resolves
/// its attacks into the enemy meter, cascades flips, ticks its timed
/// effects, and zeroes its remaining actions.
///
/// Params:
/// - battle -> battle being played
/// - side   -> side whose half ends
///
/// Return: true when the battle ended during the cascade
///
static bool battle_half_turn(BattleState* battle, Side side) {
    Side         enemy_side = side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
    PlayerState* enemy      = battle_player(battle, enemy_side);
    PlayerState* actor      = battle_player(battle, side);

    CURRENT_BATTLE          = battle;

    for (size_t index = 0; index < MAX_DRAWN_CARDS; index++) {
        Card* card = actor->hand[index];

        if (!card) {
            continue;
        }

        int value    = card->sell_cost;

        SUBJECT_CARD = card;
        effect_fire(battle, side, QUERY_CARD_SELL_COST, &value);

        actor->cp += value;

        effect_fire(battle, side, ON_CARD_SELL, card);

        SUBJECT_CARD       = nullptr;
        actor->hand[index] = nullptr;

        protocol_emit("log autosell name=\"%s\" value=%d", card->name, value);
    }

    effect_fire(battle, side, ON_TURN_END, (void*) (uintptr_t) battle->turn);
    CURRENT_BATTLE = nullptr;

    DAMAGERS       = DAMAGER_LIST;
    int total      = battle_resolve(battle, side);

    CURRENT_BATTLE = battle;
    effect_fire(battle, enemy_side, QUERY_METER_DAMAGE_TAKEN, &total);
    CURRENT_BATTLE  = nullptr;

    enemy->meter   -= total;

    if (total > 0) {
        protocol_emit(
            "log damage side=%s amount=%d",
            enemy_side == SIDE_WHITE ? "white" : "black",
            total
        );
    }

    bool over = battle_cascade(battle, enemy_side);

    DAMAGERS  = nullptr;

    effect_tick(&battle_player(battle, side)->effects);

    battle_player(battle, side)->actions = 0;

    battle_reap();

    return over;
}

/// battle_damage
///
/// Reduces a side's meter by an amount then runs the flip cascade at
/// once, so card and relic meter damage flips pieces the moment it lands.
/// Fires QUERY_METER_DAMAGE_TAKEN first so receiver-side interceptors act.
///
/// Params:
/// - battle -> battle to damage in
/// - side   -> side whose meter takes the damage
/// - amount -> meter damage to apply
///
void battle_damage(BattleState* battle, Side side, int amount) {
    if (amount <= 0) {
        return;
    }

    BattleState* saved = CURRENT_BATTLE;
    int          dmg   = amount;

    CURRENT_BATTLE     = battle;
    effect_fire(battle, side, QUERY_METER_DAMAGE_TAKEN, &dmg);

    battle_player(battle, side)->meter -= dmg;

    if (dmg > 0) {
        protocol_emit(
            "log damage side=%s amount=%d",
            side == SIDE_WHITE ? "white" : "black",
            dmg
        );
    }

    battle_cascade(battle, side);

    CURRENT_BATTLE = saved;
}

/// battle_lunge
///
/// Force-moves a piece to an empty in-bounds destination, firing
/// ON_PIECE_MOVE, then resolves that one piece's attack coverage into the
/// enemy meter and cascades. Powers multi-strike card choreography such
/// as Crusade.
///
/// Params:
/// - battle -> battle to act in
/// - piece  -> the striking piece
/// - to     -> square to move to before striking
///
/// Return: total damage dealt to the enemy meter
///
int battle_lunge(BattleState* battle, PieceInfo* piece, Square to) {
    if (!piece) {
        return 0;
    }

    BattleState* saved = CURRENT_BATTLE;

    if ((to.x != piece->square.x || to.y != piece->square.y) &&
        battle_in_bounds(battle, to) && !battle_at(battle, to)) {
        Square from = piece->square;

        battle->board.piece_board[square_index(from)] = nullptr;
        piece->square                                 = to;
        battle->board.piece_board[square_index(to)]   = piece;

        CURRENT_BATTLE = battle;
        MOVE_FROM      = from;
        effect_fire(battle, piece->side, ON_PIECE_MOVE, piece);
        MOVE_FROM      = SQUARE_END;
        CURRENT_BATTLE = saved;
    }

    Square* coverage = battle_attacks(battle, piece);
    Square  targets[MAX_BOARD_SIZE + 1];
    size_t  count = 0;

    while (!(coverage[count].x == -1 && coverage[count].y == -1)) {
        targets[count] = coverage[count];
        count++;
    }

    Side enemy_side = battle_enemy(piece->side);
    int  dealt      = 0;

    for (size_t i = 0; i < count; i++) {
        PieceInfo* victim = battle_at(battle, targets[i]);

        if (!victim || victim->side == piece->side ||
            victim->side == SIDE_NEUTRAL) {
            continue;
        }

        dealt += battle_value(battle, piece, victim);
    }

    if (dealt > 0) {
        DAMAGER_LIST[0] = piece;
        DAMAGER_LIST[1] = nullptr;
        DAMAGERS        = DAMAGER_LIST;

        battle_damage(battle, enemy_side, dealt);

        DAMAGERS = nullptr;
    }

    return dealt;
}

/// battle_walk_run
///
/// Attaches the run's held relics to the seats at battle start. Relics go
/// on the human seat with the human as their beneficiary in args[0]; Soul
/// Shard observes the enemy, so it attaches to the enemy list while still
/// crediting the human. Runs before the meters are computed so value
/// editing relics fold into the starting meter maxima.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_run(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run) {
        return;
    }

    for (int id = 0; id < RELIC_COUNT; id++) {
        if (!run->relics[id]) {
            continue;
        }

        Side list =
            id == RELIC_SOUL_SHARD ? battle_enemy(HUMAN_SIDE) : HUMAN_SIDE;

        const Relic* relic = &RELIC_REGISTRY[id];

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            if (!relic->effects[slot].func) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, list)->effects,
                &relic->effects[slot]
            );

            if (attached) {
                attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;
            }
        }
    }
}

static const PieceID KINGDOM_BASIC[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = PIECE_BING,
    [KINGDOM_KEWARANI]  = PIECE_MEDEQ,
    [KINGDOM_ZARQAN]    = PIECE_WAZIR,
    [KINGDOM_HARUSHIMA] = PIECE_FUHYO,
    [KINGDOM_CAELAN]    = PIECE_PAWN,
};

/// battle_free_army
///
/// Spawns count free pieces of one type on random empty squares within a
/// chosen half of the board: the enemy reinforcements that pressure,
/// chains, and elite nodes grant, or the Traitor's Gambit intrusion into
/// the human's half. Occupied and void squares are skipped; a bounded
/// retry keeps a crowded board from looping forever.
///
/// Params:
/// - battle -> battle to place pieces in
/// - owner  -> side the pieces belong to
/// - half   -> side whose half the pieces spawn in
/// - id     -> piece type to spawn
/// - count  -> number of pieces to place
///
static void battle_free_army(
    BattleState* battle,
    Side         owner,
    Side         half,
    PieceID      id,
    size_t       count
) {
    if (id == PIECE_NONE) {
        return;
    }

    int8_t height = battle->board.height;
    int8_t low    = half == SIDE_BLACK ? 0 : (int8_t) (height / 2);
    int8_t high   = half == SIDE_BLACK ? (int8_t) (height / 2) : height;

    for (size_t placed = 0; placed < count; placed++) {
        for (int attempt = 0; attempt < 64; attempt++) {
            int8_t x = (int8_t) (rand_r(&BATTLE_RNG) % battle->board.width);
            int8_t y = (int8_t) (low + rand_r(&BATTLE_RNG) % (high - low));

            if (battle_spawn(battle, id, (Square) {x, y}, owner)) {
                break;
            }
        }
    }
}

/// battle_setup_armies
///
/// Grants the enemy its free starting reinforcements and attaches the
/// region's chain penalty to the human seat. The reinforcement count is
/// Vorath's pressure plus the Silver chain, the elite node, and the
/// liberation trial bonuses; the pieces are the region kingdom's basic
/// infantry. Runs before the meters compute so the extra pieces fold into
/// the enemy's starting maximum.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_setup_armies(BattleState* battle) {
    MapNode* node = battle->node;

    if (!node || !node->kingdom) {
        return;
    }

    KingdomID      kingdom = node->kingdom->id;
    ChainPenaltyID level   = node->kingdom->chain
        ? (ChainPenaltyID) (node->kingdom->chain - CHAIN_REGISTRY)
        : CHAIN_NONE;

    Difficulty difficulty = BATTLE_ENGINE->run->difficulty;
    size_t     count      = run_pressure(BATTLE_ENGINE->run, kingdom);

    if (difficulty >= DIFFICULTY_BOUND && difficulty <= DIFFICULTY_ENSLAVED) {
        count += 2;
    }

    if (level >= CHAIN_SILVER) {
        count++;
    }

    if (node->type == MAP_NODE_ELITE) {
        count++;
    }

    if (node->type == MAP_NODE_LIBERATION) {
        count += 2;
    }

    Side enemy = battle_enemy(HUMAN_SIDE);

    battle_free_army(battle, enemy, enemy, KINGDOM_BASIC[kingdom], count);

    if (BATTLE_ENGINE->run->challenge == CHALLENGE_THE_TRAITORS_GAMBIT) {
        battle_free_army(
            battle,
            enemy,
            HUMAN_SIDE,
            KINGDOM_BASIC[kingdom],
            3
        );
    }

    if (level < CHAIN_BRONZE) {
        return;
    }

    const ChainPenalty* chain = &CHAIN_REGISTRY[CHAIN_BRONZE];

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (!chain->effects[slot].func) {
            continue;
        }

        Effect* attached = effect_attach(
            &battle_player(battle, HUMAN_SIDE)->effects,
            &chain->effects[slot]
        );

        if (attached) {
            int penalty = difficulty >= DIFFICULTY_SHACKLED &&
                          difficulty <= DIFFICULTY_ENSLAVED
                ? 25
                : 10;

            attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;
            attached->context->args[1] = (void*) (uintptr_t) penalty;
        }
    }
}

/// battle_walk_synergies
///
/// Attaches the human's overseer synergies that apply in this region: a
/// synergy earned by clearing a kingdom's overseer fires while fighting in
/// that kingdom's lore-adjacent neighbour.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_synergies(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run || !battle->node || !battle->node->kingdom) {
        return;
    }

    KingdomID region = battle->node->kingdom->id;

    for (size_t source = 0; source < KINGDOM_COUNT; source++) {
        if (!run->synergies[source] || KINGDOM_ADJACENT[source] != region) {
            continue;
        }

        Effect* attached = effect_attach(
            &battle_player(battle, HUMAN_SIDE)->effects,
            &SYNERGY_REGISTRY[source]
        );

        if (attached) {
            attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;
        }
    }
}

/// battle_walk_innates
///
/// Attaches each kingdom innate the human has unlocked to the human seat,
/// and under the Enslaved difficulty attaches the region kingdom's innate
/// to the enemy seat so its army fights with its innate from battle one.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_innates(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run) {
        return;
    }

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        if (run_innate_ready(run, kingdom)) {
            KINGDOM_INNATE[kingdom](
                battle,
                HUMAN_SIDE,
                run->kingdoms[kingdom].mastery
            );
        }
    }

    if (run->difficulty == DIFFICULTY_ENSLAVED && battle->node &&
        battle->node->kingdom) {
        KingdomID kingdom = battle->node->kingdom->id;

        KINGDOM_INNATE[kingdom](
            battle,
            battle_enemy(HUMAN_SIDE),
            MASTERY_NONE
        );
    }
}

/// battle_walk_modifiers
///
/// Attaches the battle modifier's effects to both seats, one copy each,
/// so every copy fires only for its own side. Runs before the meters are
/// computed so a value-editing modifier folds into the starting maxima.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_modifiers(BattleState* battle) {
    if (!battle->modifier) {
        return;
    }

    Side sides[2] = {SIDE_WHITE, SIDE_BLACK};

    for (size_t seat = 0; seat < 2; seat++) {
        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            if (!battle->modifier->effects[slot].func) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, sides[seat])->effects,
                &battle->modifier->effects[slot]
            );

            if (attached) {
                attached->context->args[0] = (void*) (uintptr_t) sides[seat];
            }
        }
    }
}

/// battle_dense_terrain
///
/// Marks a fifth of the board as missing squares for the Dense Terrain
/// modifier, skipping any cell already holding a piece so the kings and
/// other placed pieces keep their squares.
///
/// Params:
/// - battle -> battle whose board gains voids
///
static void battle_dense_terrain(BattleState* battle) {
    if (!battle->modifier ||
        battle->modifier->id != MODIFIER_DENSE_TERRAIN) {
        return;
    }

    int voids = battle->board.width * battle->board.height * 20 / 100;

    for (int placed = 0; placed < voids; placed++) {
        int8_t x     = (int8_t) (rand_r(&BATTLE_RNG) % battle->board.width);
        int8_t y     = (int8_t) (rand_r(&BATTLE_RNG) % battle->board.height);

        size_t index = square_index((Square) {x, y});

        if (battle->board.piece_board[index]) {
            continue;
        }

        battle->board.piece_board[index] = (PieceInfo*) &VOID_CELL;
    }
}

/// battle_begin
///
/// Sets up a battle on the given campaign node: the human's seat from
/// the battle parity (white on even battles), board size and voids,
/// kings, free enemy pieces, run-walk effect attachment, meters,
/// starting currency, and the opening turn. When the human sits black,
/// the AI's first white turn plays out before control returns. A null
/// node builds the synthetic town-sized board used before the campaign
/// exists. Stores the battle on the engine.
///
/// Params:
/// - engine -> engine owning the run entering battle
/// - node   -> campaign node the battle takes place on
///
void battle_begin(EngineState* engine, MapNode* node) {
    BattleState* battle = calloc(1, sizeof(BattleState));

    BATTLE_ENGINE       = engine;
    engine->battle      = battle;

    HUMAN_SIDE  = engine->run->battles_fought % 2 ? SIDE_BLACK : SIDE_WHITE;

    int8_t size = 10;

    if (node && node->map) {
        size = node->map->type == MAP_TOWN       ? 12
               : node->map->type == MAP_PROVINCE ? 14
                                                 : 16;
    }

    battle->node          = node;
    battle->modifier      = node ? node->modifier : nullptr;

    int8_t width          = size;

    if (battle->modifier &&
        battle->modifier->id == MODIFIER_EXTENDED_FRONT) {
        width += 2;
    } else if (battle->modifier &&
               battle->modifier->id == MODIFIER_COMPRESSED) {
        width -= 2;
    }

    battle->board.width   = width;
    battle->board.height  = size;
    battle->turn          = 1;
    battle->white.effects = ll_init();
    battle->black.effects = ll_init();

    BATTLE_RNG            = (unsigned int)
        rng_mix(engine->run->seed, engine->run->battles_fought + 1);

    int8_t center = (int8_t) (battle->board.width / 2);

    battle_spawn(
        battle,
        PIECE_KING,
        (Square) {center, (int8_t) (battle->board.height - 1)},
        SIDE_WHITE
    );
    battle_spawn(battle, PIECE_KING, (Square) {center, 0}, SIDE_BLACK);

    battle_dense_terrain(battle);

    battle->white.cp    = 20;
    battle->black.cp    = 20;

    battle_walk_run(battle);
    battle_walk_modifiers(battle);
    battle_setup_armies(battle);
    battle_walk_innates(battle);
    battle_walk_synergies(battle);

    battle->white.meter = battle_meter_max(battle, SIDE_WHITE);
    battle->black.meter = battle_meter_max(battle, SIDE_BLACK);

    size_t vorath = engine->run->vorath_counter / 2;

    if (vorath > 0) {
        battle_meter_gain(
            battle,
            battle_enemy(HUMAN_SIDE),
            (int) (vorath * 20)
        );
    }

    ACTING_SIDE         = SIDE_WHITE;

    CURRENT_BATTLE      = battle;
    effect_fire(battle, SIDE_WHITE, ON_BATTLE_START, node);
    effect_fire(battle, SIDE_BLACK, ON_BATTLE_START, node);
    CURRENT_BATTLE = nullptr;

    turn_start(battle, SIDE_WHITE);

    if (HUMAN_SIDE == SIDE_BLACK) {
        ai_take_turn(battle);

        if (battle_half_turn(battle, SIDE_WHITE)) {
            battle_finish(battle, SIDE_WHITE);
            return;
        }

        ACTING_SIDE = SIDE_BLACK;
        turn_start(battle, SIDE_BLACK);
    }
}

/// battle_territory_finish
///
/// Ends the battle by territory count after turn ten: the side holding
/// more squares wins, ties losing for the human.
///
/// Params:
/// - battle -> battle to score and finish
///
static void battle_territory_finish(BattleState* battle) {
    int white_squares = 0;
    int black_squares = 0;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            Side holder    = battle_territory(battle, (Square) {x, y});

            white_squares += holder == SIDE_WHITE;
            black_squares += holder == SIDE_BLACK;
        }
    }

    protocol_emit(
        "log territory white=%d black=%d",
        white_squares,
        black_squares
    );

    Side winner;

    if (white_squares == black_squares) {
        winner = HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
    } else {
        winner = white_squares > black_squares ? SIDE_WHITE : SIDE_BLACK;
    }

    battle_finish(battle, winner);
}

/// battle_end_turn
///
/// Ends the human's turn and plays out the rest of the round in strict
/// white-first order. When the human sits white, the AI's black turn
/// follows before the next round starts; when the human sits black,
/// the round closes first and the AI's next white turn plays out before
/// control returns. After turn ten the battle ends by territory.
///
/// Params:
/// - battle -> battle whose human turn ends
///
void battle_end_turn(BattleState* battle) {
    Side ai_side = HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

    if (battle_half_turn(battle, HUMAN_SIDE)) {
        battle_finish(battle, HUMAN_SIDE);
        return;
    }

    if (HUMAN_SIDE == SIDE_WHITE) {
        ACTING_SIDE = ai_side;
        turn_start(battle, ai_side);
        ai_take_turn(battle);

        if (battle_half_turn(battle, ai_side)) {
            battle_finish(battle, ai_side);
            return;
        }
    }

    battle->turn++;

    if (battle->turn > 10) {
        battle_territory_finish(battle);
        return;
    }

    ACTING_SIDE = SIDE_WHITE;
    turn_start(battle, SIDE_WHITE);

    if (HUMAN_SIDE == SIDE_BLACK) {
        ai_take_turn(battle);

        if (battle_half_turn(battle, SIDE_WHITE)) {
            battle_finish(battle, SIDE_WHITE);
            return;
        }

        ACTING_SIDE = SIDE_BLACK;
        turn_start(battle, SIDE_BLACK);
    }
}

/*----------------------------------------------------------------------------*\
                                    QUERIES
\*----------------------------------------------------------------------------*/

/// battle_moves
///
/// Generates the piece's movement list: gates on QUERY_PIECE_CAN_MOVE,
/// runs the piece's mv, then lets QUERY_PIECE_MOVES effects edit the
/// list in place. The list lives in the shared scratch buffer.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to generate moves for
///
/// Return: SQUARE_END terminated move list
///
Square* battle_moves(BattleState* battle, PieceInfo* piece) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;

    CURRENT_BATTLE             = battle;
    SUBJECT                    = piece;

    mg_begin();

    bool can = true;
    effect_fire(battle, piece->side, QUERY_PIECE_CAN_MOVE, &can);

    Square* list = mg_end();

    if (can) {
        list = piece->piece->mv(battle, piece);
        effect_fire(battle, piece->side, QUERY_PIECE_MOVES, list);
    }

    CURRENT_BATTLE = saved_battle;
    SUBJECT        = saved_subject;

    return list;
}

/// battle_attacks
///
/// Generates the piece's attack coverage list: gates on
/// QUERY_PIECE_CAN_ATTACK, runs the piece's at, then lets
/// QUERY_PIECE_ATTACKS effects edit the list in place. Coverage includes
/// empty and enemy squares, never friendly ones.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to generate coverage for
///
/// Return: SQUARE_END terminated coverage list
///
Square* battle_attacks(BattleState* battle, PieceInfo* piece) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;

    CURRENT_BATTLE             = battle;
    SUBJECT                    = piece;

    mg_begin();

    bool can = true;
    effect_fire(battle, piece->side, QUERY_PIECE_CAN_ATTACK, &can);

    Square* list = mg_end();

    if (can) {
        list = piece->piece->at(battle, piece);
        effect_fire(battle, piece->side, QUERY_PIECE_ATTACKS, list);
    }

    CURRENT_BATTLE = saved_battle;
    SUBJECT        = saved_subject;

    return list;
}

/// battle_value
///
/// Computes the piece's damage against the given victim through the
/// damage queries. A nullptr victim performs an effective-value query
/// used for meters, pricing, and display; with a victim the defense
/// query runs afterwards with the victim as subject.
///
/// Params:
/// - battle -> battle the pieces live in
/// - piece  -> damage dealing piece
/// - victim -> damage receiving piece, nullptr for effective value
///
/// Return: queried damage or effective value
///
int battle_value(BattleState* battle, PieceInfo* piece, PieceInfo* victim) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;
    PieceInfo*   saved_victim  = VICTIM;

    int          damage        = piece->piece->value;

    CURRENT_BATTLE             = battle;
    SUBJECT                    = piece;
    VICTIM                     = victim;

    effect_fire(battle, piece->side, QUERY_PIECE_DAMAGE_DEALT, &damage);

    if (victim) {
        SUBJECT = victim;
        VICTIM  = piece;

        effect_fire(battle, victim->side, QUERY_PIECE_DAMAGE_TAKEN, &damage);
    }

    CURRENT_BATTLE = saved_battle;
    SUBJECT        = saved_subject;
    VICTIM         = saved_victim;

    return damage;
}

/// battle_meter_max
///
/// Computes a side's maximum meter as the sum of its pieces' effective
/// values.
///
/// Params:
/// - battle -> battle to measure
/// - side   -> side to measure
///
/// Return: the side's maximum meter
///
int battle_meter_max(BattleState* battle, Side side) {
    int total = 0;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side) {
            total += battle_value(battle, cell, nullptr);
        }
    }

    return total;
}

/// battle_territory
///
/// Determines which side holds the square by Chebyshev distance to the
/// nearest piece of each side. Neutral pieces hold no territory.
///
/// Params:
/// - battle -> battle to measure
/// - square -> square to classify
///
/// Return: holding side, SIDE_NEUTRAL when contested
///
Side battle_territory(BattleState* battle, Square square) {
    int nearest_white = MAX_BOARD_SIZE;
    int nearest_black = MAX_BOARD_SIZE;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (!cell || cell == &VOID_CELL || cell->side == SIDE_NEUTRAL) {
            continue;
        }

        int dx       = abs(cell->square.x - square.x);
        int dy       = abs(cell->square.y - square.y);
        int distance = dx > dy ? dx : dy;

        if (cell->side == SIDE_WHITE && distance < nearest_white) {
            nearest_white = distance;
        }

        if (cell->side == SIDE_BLACK && distance < nearest_black) {
            nearest_black = distance;
        }
    }

    if (nearest_white < nearest_black) {
        return SIDE_WHITE;
    }

    if (nearest_black < nearest_white) {
        return SIDE_BLACK;
    }

    return SIDE_NEUTRAL;
}

/// battle_at
///
/// Returns the live piece occupying the square.
///
/// Params:
/// - battle -> battle providing the board
/// - square -> square to look up
///
/// Return: occupying piece, nullptr when empty or missing
///
PieceInfo* battle_at(BattleState* battle, Square square) {
    if (!battle_in_bounds(battle, square)) {
        return nullptr;
    }

    return battle->board.piece_board[square_index(square)];
}

/// battle_in_bounds
///
/// Checks whether the square lies inside the active board and is not a
/// void cell.
///
/// Params:
/// - battle -> battle providing the board
/// - square -> square to check
///
/// Return: true when the square exists on this board
///
bool battle_in_bounds(BattleState* battle, Square square) {
    if (square.x < 0 || square.x >= battle->board.width || square.y < 0 ||
        square.y >= battle->board.height) {
        return false;
    }

    return battle->board.piece_board[square_index(square)] != &VOID_CELL;
}
