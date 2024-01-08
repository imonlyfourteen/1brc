/*
    gcc -O2 -march=native 1brc.c -o 1brc
    time ./1brc measurements.txt

    Single thread, file mmap, data specific hash function,
    custom "%.1f"-format floats parsing.

    https://github.com/gunnarmorling/1brc
    https://github.com/imonlyfourteen/1brc

    " The task is to write a Java program which reads the file, calculates 
      the min, mean, and max temperature value per weather station, and emits 
      the results on stdout like this (i.e. sorted alphabetically by station 
      name, and the result values per station in the format <min>/<mean>/<max>, 
      rounded to one fractional digit):
      
        {Abha=-23.0/18.0/59.2, Abidjan=-16.2/26.0/67.3, ...
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct Station {
    int64_t sum;
    int32_t n;
    int32_t min;
    int32_t max;
    char name[32];
} stations[0x10000];

char* mmap_file_read(char* name, uint64_t* len) {
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error: %m\n");
        exit(1);
    }
    struct stat fs;
    fstat(fd, &fs);
    *len = fs.st_size;
    return mmap(0, *len, PROT_READ, MAP_PRIVATE, fd, 0);
}

static inline void parse_line(char **s) {
    char *p = *s;
    uint16_t id=0;
    
    /* hash compute, unroll part of the loop */
    #define ROL(x, n)  (x<<n | x>>(16-n))
    id = ROL(id, 2) ^ *p++;
    id = ROL(id, 2) ^ *p++;
    id = ROL(id, 2) ^ *p++;
    while (*p != ';') {
        id = ROL(id, 2) ^ *p++;
    }
    
    /* copy name */
    if (stations[id].name[0] == '\0') {
        memcpy(stations[id].name, *s, p - *s);
    }

    /* skip ';' */
    p++;
    
    /* read temperature, format is "%.1f" */
    int32_t t = 0, 
            sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    }
    t = *p++ - '0';
    do {
        if (*p == '.') {
            /* read the fractional digit */
            t = t * 10 + p[1] - '0';
            p += 3;
            break; 
        }
        t = t * 10 + *p++ - '0';
    } while (1);
    t *= sign;

    /* line done, move to next one */
    *s = p;
    
    /* update station */
    stations[id].sum += t;
    stations[id].n += 1;
    #define min(a, b) (a) <= (b) ? (a) : (b);
    #define max(a, b) (a) >= (b) ? (a) : (b);
    stations[id].min = min(stations[id].min, t);
    stations[id].max = max(stations[id].max, t);
}

int compar(const void *x, const void *y) {
    /* station sort function */
    const struct Station *a=x, *b=y;
    if (a->n == 0) return b->n == 0 ? 0 : 1;
    if (b->n == 0) return -1;
    return strcmp(a->name, b->name);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <measurements.txt>\n", argv[0]);
        exit(1);
    }
    
    /* init stations' min/max */
    for (int i=0; i<0x10000; i++) {
        stations[i].min = +10000;
        stations[i].max = -10000;
    }
    
    /* mmap the file */
    uint64_t len;
    char *fmap = mmap_file_read(argv[1], &len);
    char *line = fmap;
    
    /* parse lines and fill in stations data */
    while (line < fmap + len) {
        parse_line(&line);
    }
    
    /* sort and print result */
    qsort(stations, 0x10000, sizeof(struct Station), compar);
    int stat_cnt = 0, line_cnt = 0;
    puts("Count; Station; Min; Mean; Max");
    for (int i=0; stations[i].n > 0; i++) {
        float mean = stations[i].sum / 10.0f / stations[i].n;
        printf("%d; %s; %.1f; %.1f; %.1f\n",
               stations[i].n,
               stations[i].name,
               stations[i].min / 10.0f,
               mean,
               stations[i].max / 10.0f);
        stat_cnt += 1;
        line_cnt += stations[i].n;
    }
    
    fprintf(stderr, "Stations count (413): %d\n", stat_cnt);
    fprintf(stderr, "Lines count: %d (%s)\n", line_cnt, line_cnt < 1e9 ? "too few" : "ok");
}
