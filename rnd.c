/*
    Fast PRNG, see https://www.pcg-random.org/
    
    Note the global `random_state`.
    
    https://github.com/imonlyfourteen/1brc
*/

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

struct {
    /* PRNG state */
    uint64_t state;
    uint64_t incr;
} random_state;

void rnd_init() {
    /* Initializes random state */
    int fd = open("/dev/random", 0);
    read(fd, &random_state.state, sizeof(uint64_t));
    read(fd, &random_state.incr, sizeof(uint64_t));
    close(fd);
    random_state.incr |= 1;
}

float rnd() {
    /* Returns pseudo random float in range of (0;1),
       excluding 0 and 1; advances random state */
    uint32_t r;
    random_state.state *= 0x5851f42d4c957f2d;
    random_state.state += random_state.incr;
    r = random_state.state >> (32 + 9);
    r = (127U << 23) | r | !r;
    return *(float*)&r - 1.0f;
}

uint32_t random_int(uint32_t lim) {
    /* Returns pseudo random integer in range of [0;lim),
       including 0 and excluding lim; at max only 2^23 - 1
       random values could be obtained, see rnd() */
    return rnd() * lim;
}

float random_gauss(float sigma, float mu) {
    /* Returns normally distributed pseudo random float;
       uses Box–Muller transform to convert from uniform
       distribution */
    static float a, u1, u2=0, s=0, r;
    if (!s) {
        if (!u2) {
            /* only first time */
            u2 = rnd();
        }
        u1 = rnd();
        s = sqrtf(-2 * logf(u1));
        a = 2 * M_PI * u2;
        u2 = u1;
        r = s * cosf(a);
    } else {
        r = s * sinf(a);
        s = 0;
    }
    return r * sigma + mu;
}

float random_lognorm(float sigma, float mu) {
    /* Returns log-normally distributed pseudo random float */
    return expf(mu + sigma * random_gauss(1.0f, 0.0f));
}
