#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BOARD_N_FILLED(b) (__builtin_popcount((uint32_t)b))

#define BOARD_SET_SLOT(b, idx) ((uint8_t)((b) | (1U << (idx))))

#define BOARD_CHECK_SLOT(b, idx) (((b) & (1U << (idx))) > 0)