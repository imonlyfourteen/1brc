/*
    Generates `stations_rand.h` with 10_000 random UTF-8 names
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "rnd.h"
#include "funcs.h"

void generate_utf8_name(uint8_t **r) {
    /*  From CreateMeasurements3.java:
            len = (int)(4 + 2500 * pow(rnd() - 0.372, 7)
    */
    uint32_t len;
    uint32_t n = 0, cp;
    uint8_t buf[4];
    size_t b;
    
    do {
        /* log-normally distributed len with 
           mean ~= 8 and mode ~= 4 */
        len = random_lognorm(0.68, 1.84);
    } while (len == 0 || len > 100);
    
    while (1) {
        cp = random_int(UCP_MAX);
        if (cp == ';' || cp == '\n')
            continue;
        b = ucp2utf8(cp, buf);
        if (b > 0) {
            if (b > len)
                continue;
            if (n + b > len)
                break;
            for (int i=0; i<b; i++) {
                byte2xcode(buf[i], r);
            }
            n += b;
        }
    }
}

void generate_temperature(uint8_t **r) {
    /*  From CreateMeasurements3.java:
            // Guesstimate mean temperature using cosine of latitude
            var avgTemp = (float) (30 * Math.cos(Math.toRadians(lat))) - 10;
    */
    float t = 50.0f * rnd() - 20.0f;
    size_t len;
    len = float1_to_str(t, *r);
    *r += len;
}

size_t make_a_record(uint8_t *r) {
    uint8_t *ir = r;
    *r++ = '{';
    *r++ = '"';
    generate_utf8_name(&r);
    *r++ = '"';
    *r++ = ',';
    *r++ = ' ';
    generate_temperature(&r);
    *r++ = '}';
    *r++ = ',';
    *r++ = '\n';
    *r = '\0';
    return r - ir;
}

int main() {
    rnd_init();
    
    uint8_t head[] = "struct Station stations_rand[] = {\n";
    uint8_t tail[] = "};\n\nconst size_t NUM_ST_R = "
                     "sizeof(stations_rand) / sizeof(struct Station);";
    uint8_t record[500] = "    ",
           *rec = record + 4;
    size_t len;
    
    int fd = open("stations_rand.h", O_CREAT | O_WRONLY | O_TRUNC, 0664);
    
    write(fd, head, sizeof(head)-1);
    for (int i=0; i<10000; i++) {
        len = make_a_record(rec);
        write(fd, record, len+4);
    }
    write(fd, tail, sizeof(tail)-1);

    close(fd);
}
