/* 
    PCG PRNG, custom float to "%.1f" formatting,
    buffered write using write()
    
    https://github.com/imonlyfourteen/1brc
    https://github.com/gunnarmorling/1brc
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

struct Station {
    /* A station */
    char *name;
    float mean;
};

#include "rnd.h"
#include "funcs.h"
#include "get_time.h"
#include "stations.h"
#include "stations_rand.h" /* Use `Use `generate_stations_rand_h` to generate `stations_rand.h` */

int main(int argc, char **argv) {
    size_t N;
    int err = 0, random_names = 0;
    
    if (argc < 2) {
        err = 1;
    } else {
        random_names = argv[1][1] == 'r';
        if (random_names) {
            if (argc == 2) {
                err = 1;
            }
            argv++;
        }
    }

    if (err) {
        fprintf(stderr, "Usage: %s [-r] <n>\n", argv[0]);
        fprintf(stderr, "           -r  - UTF-8 station names from static 10k random pool\n");
        fprintf(stderr, "            n  - measurements to genegate\n");
        exit(1);
    }

    N = strtoul(argv[1], NULL, 0);
    
    rnd_init();

    float temperature;
    #define BUF_SZ (10 * 1024*1024)
    uint8_t *wbuf = malloc(BUF_SZ);
    size_t stat_i, 
           name_len,
           w = 0;
    
    size_t num_st = random_names ? NUM_ST_R : NUM_ST;
    struct Station *st = random_names ? stations_rand : stations;

    get_time();

    for (size_t i=0; i<N; i++) {
        stat_i = random_int(num_st);
        temperature = random_gauss(10, st[stat_i].mean);
        
        name_len = strlen(st[stat_i].name);
        memcpy(wbuf + w, st[stat_i].name, name_len);
        w += name_len;
        wbuf[w++] = ';';
        w += float1_to_str(temperature, wbuf + w);
        wbuf[w++] = '\n';
        if (w > BUF_SZ - 200) {
            write(1, wbuf, w);
            w = 0;
        }
    }
    
    if (w) {
        write(1, wbuf, w);
    }

    fprintf(stderr, "%.3f seconds\n", get_time());
}
