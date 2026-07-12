//! screen.c
//!
//! Screen registry and handlers. Each screen owns a handler that
//! interprets the commands available on it and emits the resulting
//! state, with screen_goto driving transitions and back-navigation.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <ctype.h>
#include <string.h>

#include <forward.h>
#include <prelude.h>

/// SCREEN_ARGS
///
/// Prologue macro extracting the uniform (int, char**) pair every
/// screen handler is invoked with. Declares argc and argv in the
/// handler's scope; va_start must run inside the variadic function
/// itself, so this cannot be a helper function.
///
#define SCREEN_ARGS(engine, argc, argv)                                        \
    int    argc;                                                               \
    char** argv;                                                               \
    do {                                                                       \
        va_list screen_args;                                                   \
        va_start(screen_args, engine);                                         \
        argc = va_arg(screen_args, int);                                       \
        argv = va_arg(screen_args, char**);                                    \
        va_end(screen_args);                                                   \
    } while (0)

/// piece_letter
///
/// Maps a live piece to its one-character board representation:
/// uppercase for white, lowercase for black, with the knight using n to
/// avoid colliding with the king.
///
/// Params:
/// - piece -> piece to represent
///
/// Return: board character for the piece
///
static char piece_letter(PieceInfo* piece) {
    char letter = piece->piece->id == PIECE_KNIGHT
                ? 'n'
                : (char) tolower(piece->piece->name[0]);

    return piece->side == SIDE_WHITE
         ? (char) toupper(letter)
         : letter;
}

/// emit_battle_state
///
/// Emits the one-line battle summary from the human's perspective: the
/// human's seat, turn, centipawns, both meters, and remaining actions.
/// The human's seat follows the battle parity: white on even battles.
///
/// Params:
/// - engine -> engine owning the battle
///
static void emit_battle_state(EngineState* engine) {
    BattleState* battle = engine->battle;
    bool         black  = engine->run->battles_fought % 2;

    PlayerState* human  = black ? &battle->black : &battle->white;
    PlayerState* enemy  = black ? &battle->white : &battle->black;

    protocol_emit(
        "state side=%s turn=%zu cp=%d meter=%d enemy_meter=%d "
        "actions=%d",
        black ? "black" : "white",
        battle->turn,
        human->cp,
        human->meter,
        enemy->meter,
        human->actions);
}

/// emit_board
///
/// Emits the board as one row line per rank: dots for empty squares,
/// X for void squares, and piece letters per piece_letter.
///
/// Params:
/// - engine -> engine owning the battle
///
static void emit_board(EngineState* engine) {
    BattleState* battle = engine->battle;

    for (int8_t y = 0; y < battle->board.height; y++) {
        char cells[21];

        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* cell = battle->board.piece_board[y * 20 + x];

            if (cell == &VOID_CELL) {
                cells[x] = 'X';
            } else if (!cell) {
                cells[x] = '.';
            } else {
                cells[x] = piece_letter(cell);
            }
        }

        cells[battle->board.width] = '\0';

        protocol_emit("row y=%d cells=%s", y, cells);
    }
}

/// emit_squares
///
/// Emits one square line per entry of a SQUARE_END terminated list.
///
/// Params:
/// - list -> list to emit
///
static void emit_squares(const Square* list) {
    for (size_t i = 0; !(list[i].x == -1 && list[i].y == -1); i++) {
        protocol_emit("square x=%d y=%d", list[i].x, list[i].y);
    }
}

/// handle_stub
///
/// Placeholder handler shared by screens without a real handler yet.
/// Rejects every command with an error line.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_stub(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    (void) engine;
    (void) argc;
    (void) argv;

    protocol_emit("error msg=\"unknown command\"");
}

/// handle_title
///
/// Handles the title screen: new starts a run and enters the synthetic
/// battle until the campaign exists.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_title(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    if (strcmp(argv[0], "new") == 0) {
        size_t seed = (size_t) arg_long(argc, argv, "seed", 0);

        Difficulty difficulty = (Difficulty)
            arg_long(argc, argv, "difficulty", DIFFICULTY_FREE);

        ChallengeRunID challenge = (ChallengeRunID)
            arg_long(argc, argv, "challenge", CHALLENGE_NONE);

        run_new(engine, seed, difficulty, challenge);
        battle_begin(engine, nullptr);

        if (!engine->battle) {
            protocol_emit("ok");
            return;
        }

        screen_goto(engine, SCREEN_BATTLE);
        emit_battle_state(engine);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

/// battle_square_arg
///
/// Reads a pair of coordinate arguments into a square.
///
/// Params:
/// - argc  -> token count
/// - argv  -> token array
/// - key_x -> key of the x coordinate
/// - key_y -> key of the y coordinate
///
/// Return: parsed square, { -1, -1 } when either key is missing
///
static Square battle_square_arg(int argc, char** argv,
                                const char* key_x, const char* key_y) {
    return (Square) {
        (int8_t) arg_long(argc, argv, key_x, -1),
        (int8_t) arg_long(argc, argv, key_y, -1),
    };
}

/// handle_battle
///
/// Handles the battle screen: state and board dumps, move and coverage
/// queries, the move and buy actions, ending the turn, and conceding.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_battle(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    BattleState* battle = engine->battle;

    if (strcmp(argv[0], "state") == 0) {
        emit_battle_state(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "board") == 0) {
        emit_board(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "moves") == 0
        || strcmp(argv[0], "attacks") == 0) {
        Square     at    = battle_square_arg(argc, argv, "x", "y");
        PieceInfo* piece = battle_at(battle, at);

        if (!piece) {
            protocol_emit("error msg=\"no piece there\"");
            return;
        }

        emit_squares(strcmp(argv[0], "moves") == 0
                     ? battle_moves(battle, piece)
                     : battle_attacks(battle, piece));
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "move") == 0) {
        Square from = battle_square_arg(argc, argv, "fx", "fy");
        Square to   = battle_square_arg(argc, argv, "tx", "ty");

        if (!battle_move(battle, from, to)) {
            protocol_emit("error msg=\"illegal move\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "buy") == 0) {
        PieceID id = (PieceID) arg_long(argc, argv, "piece",
                                        PIECE_COUNT);
        Square  at = battle_square_arg(argc, argv, "x", "y");

        if (id >= PIECE_COUNT || !battle_buy(battle, id, at)) {
            protocol_emit("error msg=\"illegal buy\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "end") == 0) {
        battle_end_turn(battle);

        if (engine->battle) {
            emit_battle_state(engine);
        } else {
            screen_goto(engine, SCREEN_TITLE);
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "concede") == 0) {
        protocol_emit("result won=0");

        battle_free(battle);
        free(battle);
        engine->battle = nullptr;

        screen_goto(engine, SCREEN_TITLE);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

Screen SCREEN_REGISTRY[SCREEN_COUNT] = {
    [SCREEN_TITLE]    = { "title",    nullptr, 0, handle_title  },
    [SCREEN_CAMPAIGN] = { "campaign", nullptr, 0, handle_stub   },
    [SCREEN_CODEX]    = { "codex",    nullptr, 0, handle_stub   },
    [SCREEN_MAP]      = { "map",      nullptr, 0, handle_stub   },
    [SCREEN_BATTLE]   = { "battle",   nullptr, 0, handle_battle },
    [SCREEN_SETTINGS] = { "settings", nullptr, 0, handle_stub   },
};

/// screen_goto
///
/// Transitions the engine to the given screen, recording the previous
/// screen for back-navigation and announcing the change.
///
/// Params:
/// - engine -> engine to transition
/// - id     -> screen to activate
///
void screen_goto(EngineState* engine, ScreenID id) {
    Screen* next   = &SCREEN_REGISTRY[id];

    next->prev     = engine->screen;
    engine->screen = next;

    protocol_emit("screen %s", next->name);
}
