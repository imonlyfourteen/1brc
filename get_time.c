#include <time.h>

#include "get_time.h"

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
