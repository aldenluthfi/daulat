//! protocol.h
//!
//! This file is the main header file for src/protocol. It controls the main
//! text based protocol logic for the game including screen transitions and
//! command handling.
//!
//! Created: 12/06/2026
//! Author : Alden Luthfi

/*----------------------------------------------------------------------------*\
                                   PROTOCOL.C
\*----------------------------------------------------------------------------*/

/// PROTOCOL_LINE_MAX
///
/// Maximum length in bytes of a single protocol input line including the
/// trailing newline and terminating NUL.
///
#define PROTOCOL_LINE_MAX 256

/// PROTOCOL_MAX_ARGS
///
/// Maximum number of whitespace-separated tokens in one protocol command.
///
#define PROTOCOL_MAX_ARGS 16

struct Protocol {
    EngineState* engine;
    FILE*        in;
    FILE*        out;
};

void        protocol_bind(Protocol* protocol);
void        protocol_run(Protocol* protocol);
void        protocol_emit(const char* format, ...);

const char* arg_value(int argc, char** argv, const char* key);
long        arg_long(int argc, char** argv, const char* key, long fallback);

/*----------------------------------------------------------------------------*\
                                    SCREEN.C
\*----------------------------------------------------------------------------*/

/// ScreenID
///
/// This enum defines the screens that is in the game. The protocol is a
/// stateful one, so each screen handles commands differently
///
/// SCREEN_TITLE:
///
/// This screen is the main title screen here the player can create a new run
/// or load an existing one, the only screen accessible from this screen is
/// the settings screen.
///
/// SCREEN_CAMPAIGN:
///
/// This screen is the main screen of the game, it shows all the kingdoms that
/// can be picked, this is the screen where the player which kingdom they want
/// to progress the map of. This screen also has access to the codex screen and
/// the settings screen
///
/// SCREEN_CODEX:
///
/// This screen shows all of the cards, piece and relic the player has unlocked
/// as well as the info that is shown on the screen.
///
/// SCREEN_MAP:
///
/// This screen shows the current progress of the tiers of the kingdom selected
/// by the campaign screen. In this screen the player can select a node and
/// choose to battle. This screen only has access back to the campaign screen.
///
/// SCREEN_BATTLE:
///
/// This screen shows the ongoing battle. This screen has no access to any of
/// the other screens.
///
/// SCREEN_SETTINGS:
///
/// This screen shows the settings state of the game. The settings page also
/// holds the button where the player can reset all progress in the run or game
/// This screen has a back button that takes the player back to the previous
/// screen
enum ScreenID {
    SCREEN_TITLE,
    SCREEN_CAMPAIGN,
    SCREEN_CODEX,
    SCREEN_MAP,
    SCREEN_BATTLE,
    SCREEN_SETTINGS,
    SCREEN_COUNT
};

/// Screen
///
/// Represents a game screen with a name and a variadic handler function.
/// The handler processes commands specific to this screen and may emit
/// screen transitions or trigger game state changes. Handlers are always
/// invoked as handle(engine, (int) argc, (char**) argv) and must va_arg
/// exactly that pair.
///
struct Screen {
    char*   name;
    Screen* prev;
    size_t  cursor;

    void    (*handle)(EngineState* engine, ...);
};

/// SCREEN_REGISTRY
///
/// Global array containing all screen definitions indexed by ScreenID.
/// Each entry provides the screen name and handler function. Mutable at
/// runtime because prev and cursor track back-navigation state.
///
extern Screen SCREEN_REGISTRY[SCREEN_COUNT];

void          screen_goto(EngineState* engine, ScreenID id);
