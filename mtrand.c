#include "mtrand.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _MT_M 397
#define _MT_FIRST_HALF (MT_STATE_SIZE - _MT_M)

static void _twist(mtrand* m) {
    unsigned i;
    for (i = 0; i < _MT_FIRST_HALF; i++) {
        uint32_t bits =
            (m->state[i] & 0x80000000) | (m->state[i + 1] & 0x7fffffff);
        m->state[i] =
            m->state[i + _MT_M] ^ (bits >> 1) ^ ((bits & 1) * 0x9908b0df);
    }
    for (; i < MT_STATE_SIZE - 1; i++) {
        uint32_t bits =
            (m->state[i] & 0x80000000) | (m->state[i + 1] & 0x7fffffff);
        m->state[i] = m->state[i - _MT_FIRST_HALF] ^ (bits >> 1) ^
                      ((bits & 1) * 0x9908b0df);
    }
    uint32_t bits = (m->state[i] & 0x80000000) | (m->state[0] & 0x7fffffff);
    m->state[i] = m->state[_MT_M - 1] ^ (bits >> 1) ^ ((bits & 1) * 0x9908b0df);
    m->next = 0;
}

void mtrand_init(mtrand* m) {
    uint32_t seed = 1337;
    int random_data = open("/dev/urandom", O_RDONLY);
    if (random_data >= 0) {
        uint32_t tseed = 0;
        if (read(random_data, &seed, sizeof(tseed)) >= 0) {
            seed = tseed;
        }
    }
    mtrand_init_with_seed(m, seed);
}

void mtrand_init_with_seed(mtrand* m, uint32_t seed) {
    m->next = 0;
    memset(m->state, 0, sizeof(uint32_t) * MT_STATE_SIZE);
    m->state[0] = seed;
    for (uint32_t i = 1; i < MT_STATE_SIZE; ++i) {
        m->state[i] =
            1812433253UL * (m->state[i - 1] ^ (m->state[i - 1] >> 30)) + i;
    }
    _twist(m);
}

uint32_t mtrand_gen(mtrand* m) {
    if (m->next >= MT_STATE_SIZE) {
        _twist(m);
    }

    uint32_t res = m->state[m->next++];
    res ^= res >> 11;
    res ^= (res << 7) & 0x9D2C5680;
    res ^= (res << 15) & 0xEFC60000;
    res ^= res >> 18;
    return res;
}

uint32_t mtrand_gen_uniform(mtrand* m, uint32_t n) {
    uint32_t r, min;
    if (n < 2) {
        return 0;
    }
    min = -n % n;
    for (;;) {
        r = mtrand_gen(m);
        if (r >= min)
            break;
    }
    return r % n;
}