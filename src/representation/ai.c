//! ai.c
//!
//! Enemy turn logic. Implements the shared move scoring core, the five
//! kingdom archetypes with their behind-fallback behaviors, and the
//! Divination preview that reports the enemy's planned actions.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

static AIPlan AI_PLAN;

/// ai_reset
///
/// Clears the stored plan so a plan built for a previous battle can never
/// be replayed. Called when a battle begins.
///
void ai_reset(void) {
    AI_PLAN.valid = false;
    AI_PLAN.count = 0;
    AI_PLAN.side  = SIDE_WHITE;
}

/// ai_active_side
///
/// Determines which side the AI is playing. The turn engine zeroes a
/// side's actions when its half settles and grants the AI its actions
/// immediately before ai_take_turn, so the side with actions remaining
/// is the AI's.
///
/// Params:
/// - battle -> battle being played
///
/// Return: the AI's side this battle
///
static Side ai_active_side(BattleState* battle) {
    return battle->white.actions > 0 ? SIDE_WHITE : SIDE_BLACK;
}

/// ai_find_king
///
/// Finds a side's king on the board.
///
/// Params:
/// - battle -> battle to search
/// - side   -> owning side
///
/// Return: the king, nullptr when it is not on the board
///
static PieceInfo* ai_find_king(BattleState* battle, Side side) {
    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        Square     square = {(int8_t) (index % 20), (int8_t) (index / 20)};

        PieceInfo* cell   = battle_at(battle, square);

        if (cell && cell->side == side && cell->piece->id == PIECE_KING) {
            return cell;
        }
    }

    return nullptr;
}

/// ai_cell
///
/// Flat board index of a square, matching the engine's row-major layout.
///
/// Params:
/// - square -> board coordinate
///
/// Return: the piece_board index of the square
///
static size_t ai_cell(Square square) {
    return (size_t) square.y * 20 + (size_t) square.x;
}

/// ai_chebyshev
///
/// Chebyshev distance between two squares, the king-move metric the
/// advance heuristic minimises.
///
/// Params:
/// - from -> first square
/// - to   -> second square
///
/// Return: the greater of the axis distances
///
static int ai_chebyshev(Square from, Square to) {
    int dx = abs(from.x - to.x);
    int dy = abs(from.y - to.y);

    return dx > dy ? dx : dy;
}

/// ai_price
///
/// The centipawn cost the AI would pay to buy a template, mirroring the
/// engine's kingdom discount and surcharge. Buy-cost effects are not
/// applied at plan time; replay revalidates through battle_buy.
///
/// Params:
/// - battle   -> battle whose node sets the pricing kingdom
/// - template -> piece template to price
///
/// Return: the plan-time cost estimate
///
static int ai_price(BattleState* battle, const Piece* template) {
    int cost = template->value;

    if (template->kingdom == KINGDOM_NONE || !battle->node ||
        !battle->node->kingdom) {
        return cost;
    }

    if (template->kingdom == battle->node->kingdom->id) {
        return cost * 60 / 100;
    }

    return cost * 120 / 100;
}

/// ai_fingerprint
///
/// A hash of board occupancy only: piece identity and side per cell, never
/// centipawns, actions, or meter, which change legitimately at turn start.
/// A mismatch between build and execution means the board itself changed.
///
/// Params:
/// - battle -> battle to fingerprint
///
/// Return: the occupancy hash
///
static unsigned ai_fingerprint(BattleState* battle) {
    unsigned hash = 2166136261u;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        Square     square = {(int8_t) (index % 20), (int8_t) (index / 20)};

        PieceInfo* cell   = battle_at(battle, square);

        unsigned   key    = 0;

        if (cell) {
            key = (unsigned) (cell->side * 128 + cell->piece->id + 1);
        }

        hash ^= (unsigned) index + key;
        hash *= 16777619u;
    }

    return hash;
}

/// ai_plan_record
///
/// Appends one action to the stored plan when there is room.
///
/// Params:
/// - kind  -> buy or move
/// - piece -> piece bought (buys) or moved (moves)
/// - from  -> origin square (moves)
/// - to    -> destination or buy square
///
static void ai_plan_record(AIActionKind kind, PieceID piece, Square from,
                           Square to) {
    if (AI_PLAN.count >= MAX_AI_PLAN) {
        return;
    }

    AI_PLAN.actions[AI_PLAN.count].kind  = kind;
    AI_PLAN.actions[AI_PLAN.count].piece = piece;
    AI_PLAN.actions[AI_PLAN.count].from  = from;
    AI_PLAN.actions[AI_PLAN.count].to    = to;

    AI_PLAN.count++;
}

/// ai_plan_build
///
/// Recomputes the AI's intended turn from the current board and stores it.
/// The greedy loop matches ai_take_turn's live behaviour: buy a knight
/// adjacent to the king while affordable, otherwise advance the piece whose
/// best move most reduces the distance to the enemy king. Board mutation is
/// simulated on the real battle with reversible patches and phantom pieces
/// so no state persists, per the evaluation-versus-application rule.
///
/// Params:
/// - battle -> battle to plan for
/// - side   -> the AI's side
///
static void ai_plan_build(BattleState* battle, Side side) {
    Side       enemy_side = battle_enemy(side);
    PieceInfo* enemy_king = ai_find_king(battle, enemy_side);

    int        cp         = battle_player(battle, side)->cp + 10;
    int        actions    = 3;

    PieceInfo* phantoms[MAX_AI_PLAN];
    size_t     phantom_count           = 0;

    size_t     cell_index[2 * MAX_AI_PLAN];
    PieceInfo* cell_prev[2 * MAX_AI_PLAN];
    size_t     cell_count              = 0;

    PieceInfo* moved[MAX_AI_PLAN];
    Square     moved_from[MAX_AI_PLAN];
    size_t     moved_count             = 0;

    AI_PLAN.count = 0;

    while (actions > 0 && AI_PLAN.count < MAX_AI_PLAN) {
        PieceInfo*   king   = ai_find_king(battle, side);
        const Piece* knight = PIECE_REGISTRY[PIECE_KNIGHT];
        int          cost   = ai_price(battle, knight);

        bool         acted  = false;

        if (king && cp >= 30 && cp >= cost) {
            for (int8_t dy = -1; dy <= 1 && !acted; dy++) {
                for (int8_t dx = -1; dx <= 1 && !acted; dx++) {
                    if (dx == 0 && dy == 0) {
                        continue;
                    }

                    Square spot = {
                        (int8_t) (king->square.x + dx),
                        (int8_t) (king->square.y + dy),
                    };

                    if (!battle_in_bounds(battle, spot) ||
                        battle_at(battle, spot)) {
                        continue;
                    }

                    PieceInfo* ph =
                        battle_spawn(battle, PIECE_KNIGHT, spot, side);

                    if (!ph) {
                        continue;
                    }

                    phantoms[phantom_count++] = ph;
                    ai_plan_record(AI_ACT_BUY, PIECE_KNIGHT, spot, spot);

                    cp      -= cost;
                    actions -= 1;
                    acted    = true;
                }
            }
        }

        if (acted) {
            continue;
        }

        if (!enemy_king) {
            break;
        }

        PieceInfo* best_piece = nullptr;
        Square     best_dest  = {0, 0};
        int        best_gain  = 0;

        for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
            Square     square = {(int8_t) (index % 20), (int8_t) (index / 20)};

            PieceInfo* piece  = battle_at(battle, square);

            if (!piece || piece->side != side) {
                continue;
            }

            int     current = ai_chebyshev(piece->square, enemy_king->square);

            Square* moves   = battle_moves(battle, piece);

            for (size_t i = 0; !(moves[i].x == -1 && moves[i].y == -1); i++) {
                int distance = ai_chebyshev(moves[i], enemy_king->square);
                int gain     = current - distance;

                if (gain > best_gain) {
                    best_gain  = gain;
                    best_piece = piece;
                    best_dest  = moves[i];
                }
            }
        }

        if (!best_piece) {
            break;
        }

        Square from             = best_piece->square;
        size_t from_cell        = ai_cell(from);
        size_t dest_cell        = ai_cell(best_dest);

        cell_index[cell_count]  = from_cell;
        cell_prev[cell_count]   = battle->board.piece_board[from_cell];
        cell_count++;
        cell_index[cell_count]  = dest_cell;
        cell_prev[cell_count]   = battle->board.piece_board[dest_cell];
        cell_count++;

        moved[moved_count]      = best_piece;
        moved_from[moved_count] = from;
        moved_count++;

        battle->board.piece_board[dest_cell] = best_piece;
        battle->board.piece_board[from_cell] = nullptr;
        best_piece->square                   = best_dest;

        ai_plan_record(AI_ACT_MOVE, best_piece->piece->id, from, best_dest);

        actions -= 1;
    }

    while (cell_count > 0) {
        cell_count--;
        battle->board.piece_board[cell_index[cell_count]] =
            cell_prev[cell_count];
    }

    while (moved_count > 0) {
        moved_count--;
        moved[moved_count]->square = moved_from[moved_count];
    }

    for (size_t i = 0; i < phantom_count; i++) {
        PieceInfo* ph = phantoms[i];

        battle->board.piece_board[ai_cell(ph->square)] = nullptr;

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            free(ph->piece->effects[slot].context);
        }

        free(ph->piece);
        free(ph);
    }

    AI_PLAN.side        = side;
    AI_PLAN.valid       = true;
    AI_PLAN.fingerprint = ai_fingerprint(battle);
}

/// ai_ensure_plan
///
/// Guarantees a plan exists for the AI side before execution. A missing or
/// wrong-side plan is built silently; a plan whose board fingerprint no
/// longer matches is rebuilt and the reason is announced.
///
/// Params:
/// - battle -> battle to plan for
/// - side   -> the AI's side
///
static void ai_ensure_plan(BattleState* battle, Side side) {
    if (!AI_PLAN.valid || AI_PLAN.side != side) {
        ai_plan_build(battle, side);
        return;
    }

    if (AI_PLAN.fingerprint != ai_fingerprint(battle)) {
        protocol_emit("log ai replan reason=board-changed");
        ai_plan_build(battle, side);
    }
}

/// ai_apply_buy
///
/// Executes one planned buy against the live board.
///
/// Params:
/// - battle -> battle to act in
/// - action -> the planned buy
///
/// Return: true when the buy was performed
///
static bool ai_apply_buy(BattleState* battle, AIAction action) {
    if (!battle_buy(battle, action.piece, action.to)) {
        return false;
    }

    protocol_emit(
        "log ai buy piece=%s x=%d y=%d",
        PIECE_REGISTRY[action.piece]->name,
        action.to.x,
        action.to.y
    );

    return true;
}

/// ai_apply_move
///
/// Executes one planned move against the live board.
///
/// Params:
/// - battle -> battle to act in
/// - action -> the planned move
///
/// Return: true when the move was performed
///
static bool ai_apply_move(BattleState* battle, AIAction action) {
    if (!battle_move(battle, action.from, action.to)) {
        return false;
    }

    protocol_emit(
        "log ai move fx=%d fy=%d tx=%d ty=%d",
        action.from.x,
        action.from.y,
        action.to.x,
        action.to.y
    );

    return true;
}

/// ai_take_turn
///
/// Spends the AI side's actions by executing its stored plan. The plan is
/// built if Divination has not already stored one, and replayed action by
/// action. An action that has become illegal because the board changed
/// since planning triggers one replan from the current board, announced as
/// a reason, and replay restarts. The turn engine resolves and settles the
/// half afterwards.
///
/// Params:
/// - battle -> battle whose AI actions are spent
///
void ai_take_turn(BattleState* battle) {
    Side   side  = ai_active_side(battle);

    ai_ensure_plan(battle, side);

    size_t index = 0;
    int    guard = 0;

    while (index < AI_PLAN.count) {
        AIAction action = AI_PLAN.actions[index];

        bool     done   = action.kind == AI_ACT_BUY
                              ? ai_apply_buy(battle, action)
                              : ai_apply_move(battle, action);

        if (done) {
            index++;
            continue;
        }

        if (guard++ >= MAX_AI_PLAN) {
            break;
        }

        protocol_emit("log ai replan reason=illegal");
        ai_plan_build(battle, side);
        index = 0;
    }

    AI_PLAN.valid = false;
}

/// ai_plan
///
/// Backs the Divination card: builds and stores the enemy's plan for its
/// next turn, then reveals its ordered buys and moves along with the cards
/// it will hold. The AI plays no cards, so every projected card is tagged
/// as autosold, matching what the settle step does at the end of its turn.
///
/// Params:
/// - battle -> battle to preview the enemy turn of
///
void ai_plan(BattleState* battle) {
    Side side = battle_enemy(battle_human());

    ai_plan_build(battle, side);

    protocol_emit("log ai plan side=%d count=%zu", side, AI_PLAN.count);

    for (size_t index = 0; index < AI_PLAN.count; index++) {
        AIAction action = AI_PLAN.actions[index];

        if (action.kind == AI_ACT_BUY) {
            protocol_emit(
                "log ai plan buy piece=%d x=%d y=%d",
                action.piece,
                action.to.x,
                action.to.y
            );
        } else {
            protocol_emit(
                "log ai plan move fx=%d fy=%d tx=%d ty=%d",
                action.from.x,
                action.from.y,
                action.to.x,
                action.to.y
            );
        }
    }

    size_t count = 0;
    Card** cards = battle_next_hand(battle, side, &count);

    for (size_t index = 0; index < count; index++) {
        Card* card = cards[index];

        if (!card) {
            continue;
        }

        protocol_emit(
            "log ai plan card id=%d name=\"%s\" fate=autosell",
            card->id,
            card->name
        );
    }
}
