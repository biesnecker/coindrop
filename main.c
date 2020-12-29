#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "players.h"
#include "state.h"

#define MAX_PLAYERS 8
#define N_THREADS (1U << 3)
#define N_ROUNDS (1U << 20)
#define N_ROUNDS_PER_THREAD (N_ROUNDS >> 3)

#include "mtrand.h"

typedef struct player {
    const char* label;
    bool (*callback)(const state* s, void* data);
    void* data;
} player;

typedef struct threaddata {
    player* player_types;
    unsigned n_player_types;
    unsigned* wins;
    unsigned* played;
    pthread_mutex_t lock;
    unsigned n_rounds;
} threaddata;

typedef struct randomstate {
    mtrand m;
    uint32_t num;
    uint8_t remaining;
} randomstate;

static unsigned rolldie(randomstate* rs) {
    while (true) {
        if (rs->remaining < 1) {
            rs->num = mtrand_gen(&rs->m);
            rs->remaining = 10;
        }
        uint32_t t = rs->num & 0x7;
        rs->num >>= 3;
        rs->remaining -= 1;
        if (t < 6) {
            return (unsigned)t;
        }
    }
}

// Returns true if the player can go again (that is, didn't hit an occupied
// slot), and updates the board and player coins based on the result.
static bool do_dice_roll(randomstate* rs, uint8_t* board, int* player_coins) {
    unsigned roll = rolldie(rs);
    if (roll == 5) {
        // Rolled a six.
        --(*player_coins);
        return true;
    } else if (!BOARD_CHECK_SLOT(*board, roll)) {
        // Empty slot.
        --(*player_coins);
        *board = BOARD_SET_SLOT(*board, roll);
        return true;
    } else {
        // Full slot.
        *player_coins += BOARD_N_FILLED(*board);
        *board = 0;
        return false;
    }
    return false;
}

static unsigned play_game(randomstate* rs,
                          const player** players,
                          int n_players,
                          int starting_coins) {
    assert(1 < n_players && n_players <= MAX_PLAYERS);
    assert(starting_coins > 0);

    uint8_t board = 0;

    int coins[n_players];
    for (int i = 0; i < n_players; ++i) {
        coins[i] = starting_coins;
    }

    unsigned turn = 0;
    state s = {0};

    while (true) {
        if (do_dice_roll(rs, &board, &coins[turn])) {
            if (coins[turn] == 0) {
                return turn;
            }
            // Update the state for the current decision.
            s.board = board;
            s.coins = coins[turn];
            if (players[turn]->callback(&s, players[turn]->data)) {
                // Don't increment the turn if the player wants to roll.
                continue;
            }
        }
        turn = (turn + 1) % n_players;
    }
    assert(false); // Should never be reached.
    return 0;
}

static void* run_game_thread(void* arg) {
    threaddata* td = (threaddata*)arg;

    unsigned npt = td->n_player_types;

    unsigned wins[npt];
    unsigned played[npt];
    unsigned player_type_order[npt];

    for (unsigned i = 0; i < npt; ++i) {
        wins[i] = 0;
        played[i] = 0;
        player_type_order[i] = i;
    }

    randomstate rs = {0};
    mtrand_init(&rs.m);

    const player* players[4] = {0};

    for (unsigned i = 0; i < td->n_rounds; ++i) {

        // Shuffle the order of the player types.
        for (unsigned j = npt - 1; j > 0; --j) {
            unsigned r = (unsigned)mtrand_gen_uniform(&rs.m, j + 1);
            unsigned t = player_type_order[j];
            player_type_order[j] = player_type_order[r];
            player_type_order[r] = t;
        }

        // Choose the first four to play.
        for (unsigned j = 0; j < 4; ++j) {
            players[j] = &td->player_types[player_type_order[j]];
            played[player_type_order[j]] += 1;
        }

        unsigned winner = play_game(&rs, players, 4, 10);

        int winner_idx = player_type_order[winner];

        wins[winner_idx] += 1;
    }

    pthread_mutex_lock(&td->lock);

    for (unsigned i = 0; i < npt; ++i) {
        td->played[i] += played[i];
        td->wins[i] += wins[i];
    }

    pthread_mutex_unlock(&td->lock);
    pthread_exit(NULL);
}

int main(void) {
    player player_types[] = {
        {.label = "Always Roll", .callback = &always_roll, .data = NULL},
        {.label = "Never Roll", .callback = &never_roll, .data = NULL},
        {.label = "Roll if 0", .callback = &zero_roller, .data = NULL},
        {.label = "Roll if 1", .callback = &one_roller, .data = NULL},
        {.label = "Roll if 2", .callback = &two_roller, .data = NULL},
        {.label = "Roll if 3", .callback = &three_roller, .data = NULL},
        {.label = "Roll if 4", .callback = &four_roller, .data = NULL},
        {.label = "Aggressive Early",
         .callback = &aggressive_early,
         .data = NULL},
        {.label = "Aggressive Late",
         .callback = &aggressive_late,
         .data = NULL}};
    unsigned n_player_types = sizeof(player_types) / sizeof(player);

    unsigned wins[n_player_types];
    unsigned played[n_player_types];

    for (unsigned i = 0; i < n_player_types; ++i) {
        wins[i] = 0;
        played[i] = 0;
    }

    threaddata td = {.player_types = player_types,
                     .n_player_types = n_player_types,
                     .wins = wins,
                     .played = played,
                     .n_rounds = N_ROUNDS_PER_THREAD};

    int rc;
    rc = pthread_mutex_init(&td.lock, NULL);
    if (rc) {
        fprintf(stderr, "error: pthread_mutex_init: %d\n", rc);
        return EXIT_FAILURE;
    }

    pthread_t threads[N_THREADS] = {0};
    for (unsigned i = 0; i < N_THREADS; ++i) {
        rc = pthread_create(&threads[i], NULL, &run_game_thread, &td);
        if (rc) {
            fprintf(stderr, "error: pthread_create: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    for (unsigned i = 0; i < N_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Final results:\n");
    for (unsigned i = 0; i < n_player_types; ++i) {
        printf("\t%s: %d / %d (%.2f)\n",
               player_types[i].label,
               wins[i],
               played[i],
               (double)(wins[i]) / (double)(played[i]));
    }

    return EXIT_SUCCESS;
}