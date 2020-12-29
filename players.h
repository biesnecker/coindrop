#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "state.h"

/*
 * Always choose to roll no matter what.
 */
bool always_roll(const state* s, void* data);

/*
 * Never choose to roll no matter what.
 */
bool never_roll(const state* s, void* data);

/*
 * Roll if board has zero coins.
 */
bool zero_roller(const state* s, void* data);

/*
 * Roll if board has zero coins.
 */
bool one_roller(const state* s, void* data);

/*
 * Roll if board has zero coins.
 */
bool two_roller(const state* s, void* data);

/*
 * Roll if board has zero coins.
 */
bool three_roller(const state* s, void* data);

/*
 * Roll if board has zero coins.
 */
bool four_roller(const state* s, void* data);

/*
 * Aggressive when coins > 5, otherwise cautious.
 */
bool aggressive_early(const state* s, void* data);

/*
 * Aggressive when coins < 5, otherwise cautious.
 */
bool aggressive_late(const state* s, void* data);