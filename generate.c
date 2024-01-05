/* 
    gcc -O2 -march=native generate.c -lm -o generate 
    ./generate 1000000000 > measurements.txt
    
    PCG PRNG, custom float to "%.1f" formatting,
    buffered write using write().
    
    https://github.com/gunnarmorling/1brc
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "stations.h"

struct {
    /* PRNG state */
    uint64_t state;
    uint64_t incr;
} rnd_state;

void rnd_init() {
    /* Initializes random state */
    int fd = open("/dev/random", 0);
    read(fd, &rnd_state.state, sizeof(uint64_t));
    read(fd, &rnd_state.incr, sizeof(uint64_t));
    close(fd);
    rnd_state.incr |= 1;
}

float rnd() {
    /* Returns pseudo random float in range of (0;1),
       excluding 0 and 1; advances random state */
    uint32_t r;
    rnd_state.state *= 0x5851f42d4c957f2d;
    rnd_state.state += rnd_state.incr;
    r = rnd_state.state >> (32 + 9);
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

size_t float1_to_str(float f, char *to) {
    char buf[100], *p = buf+99, n = 0;
    if (f < 0) { 
        n = 1;
        f *= -1;
    }
    int i = f * 10 + 0.5;
    *p-- = '\n';
    *p-- = i % 10 + '0';
    *p-- = '.';
    i /= 10;
    do {
        *p-- = i % 10 + '0';
        i /= 10;
    } while (i);
    if (n) {
        *p-- = '-';
    }
    *p = ';';
    size_t len = buf+100 - p;
    memcpy(to, p, len);
    return len;
}

typedef struct timespec tspec;

void timespec_diff(const tspec *t0, const tspec *t1, tspec *dt) {
    dt->tv_nsec = t1->tv_nsec - t0->tv_nsec;
    dt->tv_sec  = t1->tv_sec  - t0->tv_sec;
    if (dt->tv_sec > 0 && dt->tv_nsec < 0) {
        dt->tv_sec -= 1;
        dt->tv_nsec += 1e9;
    }
}    

float get_time() {
    static tspec t0, t1, dt;
    t0 = t1;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    timespec_diff(&t0, &t1, &dt);
    return dt.tv_sec + dt.tv_nsec / 1e9;
}

int main(int argc, char **argv) {
    size_t N;
    
    if (argc > 1) {
        N = strtoul(argv[1], NULL, 0);
    } else {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        exit(1);
    }
    
    rnd_init();

    float temperature;
    #define BUFSZ (10 * 1024*1024)
    char *wbuf = malloc(BUFSZ);
    size_t stat_i, 
           name_len,
           w = 0;

    get_time();

    for (size_t i=0; i<N; i++) {
        stat_i = random_int(NUM_ST);
        temperature = random_gauss(10, stations[stat_i].mean);
        
        name_len = strlen(stations[stat_i].name);
        memcpy(wbuf + w, stations[stat_i].name, name_len);
        w += name_len;
        w += float1_to_str(temperature, wbuf + w);
        if (w > BUFSZ - 100) {
            write(1, wbuf, w);
            w = 0;
        }
    }
    
    if (w) {
        write(1, wbuf, w);
    }

    fprintf(stderr, "%.3f seconds\n", get_time());
}
