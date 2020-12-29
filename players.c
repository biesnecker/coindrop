#include "players.h"

#include "board.h"

bool always_roll(const state* s, void* data) {
    (void)(s);
    (void)(data);

    return true;
}

bool never_roll(const state* s, void* data) {
    (void)(s);
    (void)(data);

    return false;
}

bool zero_roller(const state* s, void* data) {
    (void)(data);
    return BOARD_N_FILLED(s->board) == 0;
}

bool one_roller(const state* s, void* data) {
    (void)(data);
    return BOARD_N_FILLED(s->board) < 2;
}

bool two_roller(const state* s, void* data) {
    (void)(data);
    return BOARD_N_FILLED(s->board) < 3;
}

bool three_roller(const state* s, void* data) {
    (void)(data);
    return BOARD_N_FILLED(s->board) < 4;
}

bool four_roller(const state* s, void* data) {
    (void)(data);
    return BOARD_N_FILLED(s->board) < 5;
}

bool aggressive_early(const state* s, void* data) {
    (void)(data);
    if (s->coins > 5) {
        return BOARD_N_FILLED(s->board) < 4;
    } else {
        return BOARD_N_FILLED(s->board) < 2;
    }
}

bool aggressive_late(const state* s, void* data) {
    (void)(data);
    if (s->coins < 5) {
        return BOARD_N_FILLED(s->board) < 4;
    } else {
        return BOARD_N_FILLED(s->board) < 2;
    }
}