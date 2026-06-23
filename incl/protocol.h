//! protocol.h
//!
//! This file is the main header file for src/protocol. It controls the main
//! text based protocol logic for the game including screen transitions and
//! command handling.
//!
//! Created: 12/06/2026
//! Author : Alden Luthfi

/*----------------------------------------------------------------------------*\
                                  SCREEN IDS
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
/// SCREEN_MAP:
///
/// This screen shows the current progress of the tiers of the kingdom selected
/// by the campaign screen. In this screen the player can select a node and
/// choose to battle. This screen only has access back to the campaign screen.
///
enum ScreenID {
    SCREEN_TITLE,
    SCREEN_CAMPAIGN,
    SCREEN_MAP,
    SCREEN_BATTLE,
    SCREEN_CODEX,
    SCREEN_SETTINGS,
    SCREEN_COUNT
};

/*----------------------------------------------------------------------------*\
                                 SCREEN STRUCT
\*----------------------------------------------------------------------------*/

/// Screen
///
/// Represents a game screen with a name and a variadic handler function.
/// The handler processes commands specific to this screen and may emit
/// screen transitions or trigger game state changes.
///
struct Screen {
    char* name;
    void  (*handle)(EngineState* engine, ...);
};

/*----------------------------------------------------------------------------*\
                                 SCREENS
\*----------------------------------------------------------------------------*/

/// SCREEN_REGISTRY
///
/// Global array containing all screen definitions indexed by ScreenID.
/// Each entry provides the screen name and handler function.
///
extern const Screen SCREEN_REGISTRY[SCREEN_COUNT];
