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

/// SAVE_PATH
///
/// Default profile save file path used by the load and save commands.
///
static const char* const SAVE_PATH = "daulat.sav";

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

    return piece->side == SIDE_WHITE ? (char) toupper(letter) : letter;
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
        human->actions
    );
}

/// emit_board
///
/// Emits the board as one row line per rank: middle dots for empty squares,
/// spaces for void squares, and piece letters per piece_letter. An enemy
/// piece the human's board view hides (Fog of War, Fog Coast) renders as ?,
/// so the human knows a piece is there but not what it is.
///
/// Params:
/// - engine -> engine owning the battle
///
static void emit_board(EngineState* engine) {
    BattleState* battle = engine->battle;

    battle_board_view(battle);

    for (int8_t y = 0; y < battle->board.height; y++) {
        char cells[41];
        int  len = 0;

        for (int8_t x = 0; x < battle->board.width; x++) {
            size_t     index = (size_t) (y * 20 + x);
            PieceInfo* cell  = battle->board.piece_board[index];

            if (cell == &VOID_CELL) {
                cells[len++] = ' ';
            } else if (!cell) {
                cells[len++] = "·"[0];
                cells[len++] = "·"[1];
            } else if (!battle->board.visible[index]) {
                cells[len++] = '?';
            } else {
                cells[len++] = piece_letter(cell);
            }
        }

        cells[len] = '\0';

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

/// emit_hand
///
/// Emits one line per card in the human's hand: index, id, name, tier,
/// and play and sell costs. A card the hand-state query has hidden masks
/// its id and name, leaving only the kingdom and tier visible.
///
/// Params:
/// - engine -> engine owning the battle
///
static void emit_hand(EngineState* engine) {
    BattleState* battle = engine->battle;
    Side side = engine->run->battles_fought % 2 ? SIDE_BLACK : SIDE_WHITE;

    PlayerState* human = battle_player(battle, side);

    battle_hand_view(battle, side);

    for (size_t i = 0; i < MAX_DRAWN_CARDS; i++) {
        Card* card = human->hand[i];

        if (!card) {
            continue;
        }

        protocol_emit(
            "card i=%zu id=%d name=\"%s\" kingdom=%d tier=%d play=%d sell=%d",
            i,
            human->hand_visible[i] ? card->id : -1,
            human->hand_visible[i] ? card->name : "?",
            card->kingdom,
            card->tier,
            card->play_cost,
            card->sell_cost
        );
    }
}

/// emit_next
///
/// Emits one line per card in the human's projected next hand: index, id,
/// name, kingdom, tier, and play and sell costs. The projection is the
/// player's own preview, so no cards are masked.
///
/// Params:
/// - engine -> engine owning the battle
///
static void emit_next(EngineState* engine) {
    BattleState* battle = engine->battle;
    Side side = engine->run->battles_fought % 2 ? SIDE_BLACK : SIDE_WHITE;

    size_t count = 0;
    Card** cards = battle_next_hand(battle, side, &count);

    for (size_t i = 0; i < count; i++) {
        Card* card = cards[i];

        if (!card) {
            continue;
        }

        protocol_emit(
            "next i=%zu id=%d name=\"%s\" kingdom=%d tier=%d play=%d sell=%d",
            i,
            card->id,
            card->name,
            card->kingdom,
            card->tier,
            card->play_cost,
            card->sell_cost
        );
    }
}

/// emit_activatable
///
/// Emits one line per activatable held item for the human: index, name, and
/// whether it may be fired now. The index is the ordinal the activate
/// command selects by.
///
/// Params:
/// - engine -> engine owning the battle
///
static void emit_activatable(EngineState* engine) {
    BattleState* battle = engine->battle;
    Side side = engine->run->battles_fought % 2 ? SIDE_BLACK : SIDE_WHITE;

    Effect* items[RELIC_COUNT];
    size_t  count = battle_item_list(
        battle,
        side,
        items,
        RELIC_COUNT
    );

    for (size_t i = 0; i < count; i++) {
        protocol_emit(
            "item i=%zu name=\"%s\" usable=%d",
            i,
            items[i]->name ? items[i]->name : "?",
            battle_item_usable(battle, items[i]) ? 1 : 0
        );
    }
}

/// handle_title
///
/// Handles the title screen: new starts a fresh run and load restores a
/// saved profile, both entering the campaign screen.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_title(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    if (strcmp(argv[0], "new") == 0) {
        size_t     seed = (size_t) arg_long(argc, argv, "seed", 0);

        Difficulty difficulty =
            (Difficulty) arg_long(argc, argv, "difficulty", DIFFICULTY_FREE);

        ChallengeRunID challenge =
            (ChallengeRunID) arg_long(argc, argv, "challenge", CHALLENGE_NONE);

        run_new(engine, seed, difficulty, challenge);

        screen_goto(engine, SCREEN_CAMPAIGN);
        run_emit_kingdoms(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "load") == 0) {
        if (!engine_load(engine, SAVE_PATH) || !engine->run) {
            protocol_emit("error msg=\"no save\"");
            return;
        }

        screen_goto(engine, SCREEN_CAMPAIGN);
        run_emit_kingdoms(engine);
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
static Square
battle_square_arg(int argc, char** argv, const char* key_x, const char* key_y) {
    return (Square){
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

    if (strcmp(argv[0], "moves") == 0 || strcmp(argv[0], "attacks") == 0) {
        Square     at    = battle_square_arg(argc, argv, "x", "y");
        PieceInfo* piece = battle_at(battle, at);

        if (!piece) {
            protocol_emit("error msg=\"no piece there\"");
            return;
        }

        emit_squares(
            strcmp(argv[0], "moves") == 0 ? battle_moves(battle, piece)
                                          : battle_attacks(battle, piece)
        );
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
        PieceID id = (PieceID) arg_long(argc, argv, "piece", PIECE_COUNT);
        Square  at = battle_square_arg(argc, argv, "x", "y");

        if (id >= PIECE_COUNT || !battle_buy(battle, id, at)) {
            protocol_emit("error msg=\"illegal buy\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "hand") == 0) {
        emit_hand(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "next") == 0) {
        emit_next(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "activatable") == 0) {
        emit_activatable(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "activate") == 0) {
        Side side =
            engine->run->battles_fought % 2 ? SIDE_BLACK : SIDE_WHITE;
        size_t index = (size_t) arg_long(argc, argv, "i", -1);

        if (!battle_item_activate(battle, side, index)) {
            protocol_emit("error msg=\"not activatable\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "play") == 0) {
        size_t index = (size_t) arg_long(argc, argv, "i", -1);

        if (!battle_play(battle, index)) {
            protocol_emit("error msg=\"illegal play\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "target") == 0) {
        size_t index = (size_t) arg_long(argc, argv, "i", -1);

        if (!battle_card_target(battle, index)) {
            protocol_emit("error msg=\"no pending target\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "sell") == 0) {
        size_t index = (size_t) arg_long(argc, argv, "i", -1);

        if (!battle_sell(battle, index)) {
            protocol_emit("error msg=\"illegal sell\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "reclaim") == 0) {
        Square at = battle_square_arg(argc, argv, "x", "y");

        if (!battle_reclaim(battle, at)) {
            protocol_emit("error msg=\"illegal reclaim\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "end") == 0) {
        battle_end_turn(battle);

        if (engine->battle) {
            emit_battle_state(engine);
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "concede") == 0) {
        battle_concede(battle);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

/// handle_campaign
///
/// Handles the campaign hub: the kingdom list, entering a kingdom's map,
/// the Vorath finale, the codex and settings screens, saving, and
/// returning to the title.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_campaign(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    if (strcmp(argv[0], "kingdoms") == 0) {
        run_emit_kingdoms(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "enter") == 0) {
        KingdomID kingdom =
            (KingdomID) arg_long(argc, argv, "id", KINGDOM_COUNT);

        if (kingdom >= KINGDOM_COUNT) {
            protocol_emit("error msg=\"bad kingdom\"");
            return;
        }

        run_enter_map(engine, kingdom);
        screen_goto(engine, SCREEN_MAP);
        run_emit_map(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "vorath") == 0) {
        if (!run_enter_vorath(engine)) {
            protocol_emit("error msg=\"vorath not ready\"");
            return;
        }

        screen_goto(engine, SCREEN_BATTLE);
        emit_battle_state(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "codex") == 0) {
        screen_goto(engine, SCREEN_CODEX);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "settings") == 0) {
        screen_goto(engine, SCREEN_SETTINGS);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "save") == 0) {
        if (!engine_save(engine, SAVE_PATH)) {
            protocol_emit("error msg=\"save failed\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "title") == 0) {
        screen_goto(engine, SCREEN_TITLE);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

/// handle_map
///
/// Handles the map screen: the node list, selecting a node, and resolving
/// the event, offering, and relic choices a selected node leaves pending.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_map(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    if (strcmp(argv[0], "nodes") == 0) {
        run_emit_map(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "select") == 0) {
        size_t index = (size_t) arg_long(argc, argv, "i", -1);

        if (!run_select_node(engine, index)) {
            protocol_emit("error msg=\"not selectable\"");
            return;
        }

        if (engine->battle) {
            screen_goto(engine, SCREEN_BATTLE);
            emit_battle_state(engine);
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "choose") == 0) {
        EventChoice choice =
            argc > 1 && argv[1][0] == 'b' ? CHOICE_B : CHOICE_A;

        run_event_choose(engine, choice);

        if (engine->battle) {
            screen_goto(engine, SCREEN_BATTLE);
            emit_battle_state(engine);
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "target") == 0) {
        size_t index = (size_t) arg_long(argc, argv, "i", -1);

        if (!run_event_target(engine, index)) {
            protocol_emit("error msg=\"no pending target\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "remove") == 0) {
        CardID card = (CardID) arg_long(argc, argv, "card", CARD_COUNT);

        if (card >= CARD_COUNT) {
            protocol_emit("error msg=\"bad card\"");
            return;
        }

        run_offering(engine, card);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "relic") == 0) {
        RelicID relic = (RelicID) arg_long(argc, argv, "id", RELIC_COUNT);

        if (relic >= RELIC_COUNT) {
            protocol_emit("error msg=\"bad relic\"");
            return;
        }

        run_relic_pick(engine, relic);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "back") == 0) {
        screen_goto(engine, SCREEN_CAMPAIGN);
        run_emit_kingdoms(engine);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

/// handle_codex
///
/// Handles the codex screen: listing pieces, cards, and relics, showing
/// one entry's detail, and returning to the campaign.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_codex(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    if (strcmp(argv[0], "pieces") == 0) {
        for (PieceID id = 0; id < PIECE_COUNT; id++) {
            if (PIECE_REGISTRY[id]) {
                protocol_emit(
                    "piece id=%d name=\"%s\"",
                    id,
                    PIECE_REGISTRY[id]->name
                );
            }
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "cards") == 0) {
        for (CardID id = 0; id < CARD_COUNT; id++) {
            if (CARD_REGISTRY[id]) {
                protocol_emit(
                    "card id=%d name=\"%s\"",
                    id,
                    CARD_REGISTRY[id]->name
                );
            }
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "relics") == 0) {
        for (RelicID id = 0; id < RELIC_COUNT; id++) {
            if (RELIC_REGISTRY[id].name) {
                protocol_emit(
                    "relic id=%d name=\"%s\"",
                    id,
                    RELIC_REGISTRY[id].name
                );
            }
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "info") == 0) {
        long piece = arg_long(argc, argv, "piece", -1);
        long card  = arg_long(argc, argv, "card", -1);

        if (piece >= 0 && piece < PIECE_COUNT && PIECE_REGISTRY[piece]) {
            protocol_emit(
                "info name=\"%s\" desc=\"%s\"",
                PIECE_REGISTRY[piece]->name,
                PIECE_REGISTRY[piece]->desc
            );
        } else if (card >= 0 && card < CARD_COUNT && CARD_REGISTRY[card]) {
            protocol_emit(
                "info name=\"%s\" desc=\"%s\"",
                CARD_REGISTRY[card]->name,
                CARD_REGISTRY[card]->desc
            );
        } else {
            protocol_emit("error msg=\"no entry\"");
            return;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "back") == 0) {
        screen_goto(engine, SCREEN_CAMPAIGN);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

/// handle_settings
///
/// Handles the settings screen: showing the profile, resetting the run or
/// the whole profile, and returning to the campaign.
///
/// Params:
/// - engine -> engine owning the screen
/// - ...    -> the (int) argc, (char**) argv pair
///
static void handle_settings(EngineState* engine, ...) {
    SCREEN_ARGS(engine, argc, argv);

    if (strcmp(argv[0], "show") == 0) {
        protocol_emit(
            "settings cleared=%d masteries=%d,%d,%d,%d,%d",
            engine->cleared,
            engine->masteries[0],
            engine->masteries[1],
            engine->masteries[2],
            engine->masteries[3],
            engine->masteries[4]
        );
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "reset") == 0 && argc > 1 &&
        strcmp(argv[1], "run") == 0) {
        if (engine->run) {
            run_free(engine->run);
            engine->run = nullptr;
        }

        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "reset") == 0 && argc > 1 &&
        strcmp(argv[1], "all") == 0) {
        engine_free(engine);
        engine_init(engine);
        protocol_emit("ok");
        return;
    }

    if (strcmp(argv[0], "back") == 0) {
        screen_goto(engine, SCREEN_CAMPAIGN);
        protocol_emit("ok");
        return;
    }

    protocol_emit("error msg=\"unknown command\"");
}

Screen SCREEN_REGISTRY[SCREEN_COUNT] = {
    [SCREEN_TITLE]    = {"title", nullptr, 0, handle_title},
    [SCREEN_CAMPAIGN] = {"campaign", nullptr, 0, handle_campaign},
    [SCREEN_CODEX]    = {"codex", nullptr, 0, handle_codex},
    [SCREEN_MAP]      = {"map", nullptr, 0, handle_map},
    [SCREEN_BATTLE]   = {"battle", nullptr, 0, handle_battle},
    [SCREEN_SETTINGS] = {"settings", nullptr, 0, handle_settings},
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
