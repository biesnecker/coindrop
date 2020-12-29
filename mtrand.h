#pragma once

#include <stdint.h>

#define MT_STATE_SIZE 624U

typedef struct mtrand {
    uint32_t state[MT_STATE_SIZE];
    unsigned next;
} mtrand;

void mtrand_init(mtrand* m);

void mtrand_init_with_seed(mtrand* m, uint32_t seed);

uint32_t mtrand_gen(mtrand* m);

uint32_t mtrand_gen_uniform(mtrand* m, uint32_t n);