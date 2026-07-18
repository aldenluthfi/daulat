//! engine.c
//!
//! Engine lifecycle and persistence. Initializes and frees the master
//! game state, encodes and decodes the text save format, and finalizes
//! runs by advancing masteries and the cleared difficulty.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <string.h>

#include <forward.h>
#include <prelude.h>

/// engine_init
///
/// Initializes a fresh engine state with no masteries, no cleared
/// difficulty, no run, no battle, and the title screen active.
///
/// Params:
/// - engine -> engine state to initialize
///
void engine_init(EngineState* engine) {
    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        engine->masteries[kingdom] = MASTERY_NONE;
    }

    engine->cleared = DIFFICULTY_NONE;
    engine->screen  = &SCREEN_REGISTRY[SCREEN_TITLE];
    engine->run     = nullptr;
    engine->battle  = nullptr;
}

/// engine_free
///
/// Frees everything the engine owns: the active battle if any, then the
/// active run if any.
///
/// Params:
/// - engine -> engine state to deallocate
///
void engine_free(EngineState* engine) {
    if (engine->battle) {
        battle_free(engine->battle);
        free(engine->battle);
        engine->battle = nullptr;
    }

    if (engine->run) {
        run_free(engine->run);
        engine->run = nullptr;
    }
}

/*----------------------------------------------------------------------------*\
                                 PERSISTENCE
\*----------------------------------------------------------------------------*/

/// save_map
///
/// Returns a run's map for a kingdom and tier index.
///
/// Params:
/// - run     -> run holding the maps
/// - kingdom -> kingdom index
/// - tier    -> tier index
///
/// Return: the requested MapState
///
static MapState* save_map(RunState* run, size_t kingdom, size_t tier) {
    KingdomState* state = &run->kingdoms[kingdom];

    return tier == 0   ? state->town_map
           : tier == 1 ? state->province_map
                       : state->country_map;
}

/// write_bits
///
/// Writes a keyed line of one bit character per boolean flag.
///
/// Params:
/// - file  -> file to write to
/// - key   -> line key
/// - flags -> boolean array
/// - count -> number of flags
///
static void
write_bits(FILE* file, const char* key, const bool* flags, size_t count) {
    fprintf(file, "%s=", key);

    for (size_t i = 0; i < count; i++) {
        fputc(flags[i] ? '1' : '0', file);
    }

    fputc('\n', file);
}

/// engine_save
///
/// Writes the engine state to the given path in the text save format.
/// Battles are never saved; saving is rejected while one is active.
///
/// Params:
/// - engine -> engine state to save
/// - path   -> file path to write to
///
/// Return: true when the save was written
///
bool engine_save(EngineState* engine, const char* path) {
    if (engine->battle) {
        return false;
    }

    FILE* file = fopen(path, "w");

    if (!file) {
        return false;
    }

    fprintf(file, "daulat version=1\n");
    fprintf(
        file,
        "masteries=%d,%d,%d,%d,%d\n",
        engine->masteries[0],
        engine->masteries[1],
        engine->masteries[2],
        engine->masteries[3],
        engine->masteries[4]
    );
    fprintf(file, "cleared=%d\n", engine->cleared);

    RunState* run = engine->run;

    if (!run) {
        fprintf(file, "run=0\n");
        fclose(file);
        return true;
    }

    fprintf(file, "run=1\n");
    fprintf(file, "seed=%zu\n", run->seed);
    fprintf(file, "difficulty=%d\n", run->difficulty);
    fprintf(file, "challenge=%d\n", run->challenge);
    fprintf(file, "vorath=%zu\n", run->vorath_counter);
    fprintf(file, "battles=%zu\n", run->battles_fought);

    write_bits(file, "relics", run->relics, RELIC_COUNT);
    write_bits(file, "pieces", run->pieces, PIECE_COUNT);
    write_bits(file, "cards", run->cards, CARD_COUNT);
    write_bits(file, "synergies", run->synergies, KINGDOM_COUNT);

    fprintf(file, "chains=");
    for (size_t k = 0; k < KINGDOM_COUNT; k++) {
        int level = run->kingdoms[k].chain
                        ? (int) (run->kingdoms[k].chain - CHAIN_REGISTRY)
                        : -1;

        fprintf(file, "%s%d", k ? "," : "", level);
    }
    fprintf(file, "\n");

    fprintf(file, "ever=");
    for (size_t k = 0; k < KINGDOM_COUNT; k++) {
        fprintf(file, "%s%d", k ? "," : "", run->kingdoms[k].ever_chained);
    }
    fprintf(file, "\n");

    fprintf(file, "liberation=");
    for (size_t k = 0; k < KINGDOM_COUNT; k++) {
        fprintf(file, "%s%zu", k ? "," : "", run->liberation_at[k]);
    }
    fprintf(file, "\n");

    for (size_t k = 0; k < KINGDOM_COUNT; k++) {
        for (size_t t = 0; t < 3; t++) {
            MapState* map = save_map(run, k, t);

            fprintf(file, "map k=%zu t=%zu cleared=", k, t);
            for (size_t i = 0; i < map->nodes.vertices_count; i++) {
                MapNode* node = map->nodes.nodes[i].data;
                fputc(node->cleared ? '1' : '0', file);
            }

            fprintf(file, " revealed=");
            for (size_t i = 0; i < map->nodes.vertices_count; i++) {
                MapNode* node = map->nodes.nodes[i].data;
                fputc(node->revealed ? '1' : '0', file);
            }

            fprintf(file, "\n");
        }
    }

    for (size_t i = 0; i < EVENT_COUNT; i++) {
        if (run->events[i] != NO_CHOICE) {
            fprintf(
                file,
                "event i=%zu choice=%d\n",
                i,
                run->events[i]
            );
        }
    }

    for (size_t i = 0; i < EVENT_COUNT; i++) {
        if (run->event_picks[i] >= 0) {
            fprintf(
                file,
                "eventpick i=%zu value=%d\n",
                i,
                run->event_picks[i]
            );
        }
    }

    fclose(file);
    return true;
}

/// apply_bits
///
/// Applies a bit string to a boolean array up to its count.
///
/// Params:
/// - flags -> boolean array to fill
/// - count -> array length
/// - bits  -> bit string
///
static void apply_bits(bool* flags, size_t count, const char* bits) {
    for (size_t i = 0; i < count && bits[i]; i++) {
        flags[i] = bits[i] == '1';
    }
}

/// engine_load
///
/// Reads the engine state from the given path, rebuilding the campaign
/// maps from the saved seed and reapplying saved bits and events. A
/// missing file yields a fresh profile.
///
/// Params:
/// - engine -> engine state to load into
/// - path   -> file path to read from
///
/// Return: true when an existing save was loaded
///
bool engine_load(EngineState* engine, const char* path) {
    FILE* file = fopen(path, "r");

    if (!file) {
        return false;
    }

    char   line[512];
    size_t saved_seed = RNG_SEED;
    int    saved_diff = DIFFICULTY_FREE;

    while (fgets(line, sizeof(line), file)) {
        int    ints[KINGDOM_COUNT];
        size_t seed;
        int    difficulty;
        int    challenge;
        size_t value;
        int    k;
        int    t;
        int    choice;
        char   bits[512];
        char   revealed[512];

        if (sscanf(
                line,
                "masteries=%d,%d,%d,%d,%d",
                &ints[0],
                &ints[1],
                &ints[2],
                &ints[3],
                &ints[4]
            ) == 5) {
            for (size_t i = 0; i < KINGDOM_COUNT; i++) {
                engine->masteries[i] = (MasteryLevel) ints[i];
            }
        } else if (sscanf(line, "cleared=%d", &ints[0]) == 1) {
            engine->cleared = (Difficulty) ints[0];
        } else if (strncmp(line, "seed=", 5) == 0 &&
                   sscanf(line, "seed=%zu", &seed) == 1) {
            saved_seed = seed;
        } else if (sscanf(line, "difficulty=%d", &difficulty) == 1) {
            saved_diff = difficulty;
        } else if (sscanf(line, "challenge=%d", &challenge) == 1) {
            run_new(
                engine,
                saved_seed,
                (Difficulty) saved_diff,
                (ChallengeRunID) challenge
            );
        } else if (!engine->run) {
            continue;
        } else if (sscanf(line, "vorath=%zu", &value) == 1) {
            engine->run->vorath_counter = value;
        } else if (sscanf(line, "battles=%zu", &value) == 1) {
            engine->run->battles_fought = value;
        } else if (sscanf(line, "relics=%511s", bits) == 1 &&
                   strncmp(line, "relics=", 7) == 0) {
            apply_bits(engine->run->relics, RELIC_COUNT, bits);
        } else if (sscanf(line, "pieces=%511s", bits) == 1 &&
                   strncmp(line, "pieces=", 7) == 0) {
            apply_bits(engine->run->pieces, PIECE_COUNT, bits);
        } else if (sscanf(line, "cards=%511s", bits) == 1 &&
                   strncmp(line, "cards=", 6) == 0) {
            apply_bits(engine->run->cards, CARD_COUNT, bits);
        } else if (sscanf(line, "synergies=%511s", bits) == 1 &&
                   strncmp(line, "synergies=", 10) == 0) {
            apply_bits(engine->run->synergies, KINGDOM_COUNT, bits);
        } else if (sscanf(
                       line,
                       "chains=%d,%d,%d,%d,%d",
                       &ints[0],
                       &ints[1],
                       &ints[2],
                       &ints[3],
                       &ints[4]
                   ) == 5) {
            for (size_t i = 0; i < KINGDOM_COUNT; i++) {
                engine->run->kingdoms[i].chain =
                    ints[i] < 0 ? nullptr
                                : (ChainPenalty*) &CHAIN_REGISTRY[ints[i]];
            }
        } else if (sscanf(
                       line,
                       "ever=%d,%d,%d,%d,%d",
                       &ints[0],
                       &ints[1],
                       &ints[2],
                       &ints[3],
                       &ints[4]
                   ) == 5) {
            for (size_t i = 0; i < KINGDOM_COUNT; i++) {
                engine->run->kingdoms[i].ever_chained = ints[i];
            }
        } else if (strncmp(line, "liberation=", 11) == 0) {
            char* cursor = line + 11;

            for (size_t i = 0; i < KINGDOM_COUNT; i++) {
                engine->run->liberation_at[i]  = strtoul(cursor, &cursor, 10);
                cursor                        += *cursor == ',';
            }
        } else if (sscanf(
                       line,
                       "map k=%d t=%d cleared=%511s revealed=%511s",
                       &k,
                       &t,
                       bits,
                       revealed
                   ) == 4) {
            MapState* map = save_map(engine->run, (size_t) k, (size_t) t);

            for (size_t i = 0; i < map->nodes.vertices_count; i++) {
                MapNode* node  = map->nodes.nodes[i].data;

                node->cleared  = bits[i] == '1';
                node->revealed = revealed[i] == '1';
            }
        } else if (sscanf(line, "event i=%d choice=%d", &k, &choice) == 2 &&
                   strncmp(line, "event ", 6) == 0) {
            engine->run->events[k] = (EventChoice) choice;
        } else if (sscanf(line, "eventpick i=%d value=%d", &k, &choice) == 2) {
            engine->run->event_picks[k] = choice;
        }
    }

    fclose(file);
    return true;
}

/*----------------------------------------------------------------------------*\
                                  FINALIZE
\*----------------------------------------------------------------------------*/

/// engine_finalize_run
///
/// Concludes the active run: advances masteries for kingdoms that were
/// never chained when Vorath was defeated, bumps the cleared difficulty,
/// and frees the run.
///
/// Params:
/// - engine     -> engine owning the run
/// - vorath_won -> whether the run ended by defeating Vorath
///
void engine_finalize_run(EngineState* engine, bool vorath_won) {
    RunState* run = engine->run;

    if (!run) {
        return;
    }

    if (vorath_won) {
        for (size_t k = 0; k < KINGDOM_COUNT; k++) {
            if (!run->kingdoms[k].ever_chained &&
                engine->masteries[k] < MASTERY_LEVEL_3) {
                engine->masteries[k] =
                    (MasteryLevel) (engine->masteries[k] + 1);
            }
        }

        if (engine->cleared == DIFFICULTY_NONE ||
            run->difficulty > engine->cleared) {
            engine->cleared = run->difficulty;
        }
    }

    run_free(run);
    engine->run = nullptr;
}
