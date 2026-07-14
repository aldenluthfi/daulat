//! forward.h
//!
//! This file contains forward declarations for all types in the codebase.
//! By declaring types before they are fully defined, it enables circular
//! references between representation.h and protocol.h without circular
//! includes.
//!
//! Created: 12/06/2026
//! Author : Alden Luthfi

/*----------------------------------------------------------------------------*\
                              DATA_STRUCTURE.H
\*----------------------------------------------------------------------------*/

typedef struct LLNode        LLNode;
typedef struct LinkedList    LinkedList;
typedef struct DGEdge        DGEdge;
typedef struct DGNode        DGNode;
typedef struct DirectedGraph DirectedGraph;

/*----------------------------------------------------------------------------*\
                               REPRESENTATION.H
\*----------------------------------------------------------------------------*/

typedef enum EffectTrigger    EffectTrigger;
typedef enum EffectDuration   EffectDuration;
typedef enum PieceID          PieceID;
typedef enum Side             Side;
typedef enum MoveClass        MoveClass;
typedef enum CardID           CardID;
typedef enum UnlockTier       UnlockTier;
typedef enum TargetKind       TargetKind;
typedef enum MasteryLevel     MasteryLevel;
typedef enum KingdomID        KingdomID;
typedef enum RelicID          RelicID;
typedef enum BattleModifierID BattleModifierID;
typedef enum BoardTraitID     BoardTraitID;
typedef enum MapNodeID        MapNodeID;
typedef enum MapTypeID        MapTypeID;
typedef enum AIArchetypeID    AIArchetypeID;
typedef enum Difficulty       Difficulty;
typedef enum ChallengeRunID   ChallengeRunID;
typedef enum ChainPenaltyID   ChainPenaltyID;
typedef enum OverseerTypeID   OverseerTypeID;
typedef enum EventID          EventID;
typedef enum EventChoice      EventChoice;

typedef struct EffectContext  EffectContext;
typedef struct Effect         Effect;
typedef struct Board          Board;
typedef struct Square         Square;
typedef struct Piece          Piece;
typedef struct PieceInfo      PieceInfo;
typedef struct Card           Card;
typedef struct CardTarget     CardTarget;
typedef struct KingdomState   KingdomState;
typedef struct Relic          Relic;
typedef struct BattleModifier BattleModifier;
typedef struct BoardTrait     BoardTrait;
typedef struct MapNode        MapNode;
typedef struct MapState       MapState;
typedef struct EventState     EventState;
typedef struct ChainPenalty   ChainPenalty;
typedef struct DifficultyMode DifficultyMode;
typedef struct ChallengeRun   ChallengeRun;
typedef struct PlayerState    PlayerState;
typedef struct BattleState    BattleState;
typedef struct RunState       RunState;
typedef struct EngineState    EngineState;

/*----------------------------------------------------------------------------*\
                                  PROTOCOL.H
\*----------------------------------------------------------------------------*/

typedef struct Protocol Protocol;

typedef enum ScreenID   ScreenID;
typedef struct Screen   Screen;
