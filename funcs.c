#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>

size_t float1_to_str(float f, uint8_t *to) {
    /* Converts a float value to "%.1f" format */
    uint8_t buf[100], *p = buf+99, n = 0;
    if (f < 0) { 
        n = 1;
        f *= -1;
    }
    int i = f * 10 + 0.5;
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
    size_t len = buf+99 - p;
    memcpy(to, p+1, len);
    return len;
}

ssize_t ucp2utf8(uint32_t cp, uint8_t *buf) {
    /*  Unicode code point to UTF-8 conversion
    
        From https://www.unicode.org/versions/Unicode15.0.0/UnicodeStandard-15.0.pdf:
    
        Unicode codespace:      A range of integers from 0 to 10FFFF.
        Code point:             Any value in the Unicode codespace.
        Unicode scalar value:   Any Unicode code point except high-surrogate 
                                and low-surrogate code points (D800..DFFF).
        UTF-8 encoding form:    The Unicode encoding form that assigns each Unicode 
                                scalar value to an unsigned byte sequence of one 
                                to four bytes in length.
        Code unit:              The minimal bit combination that can represent a unit 
                                of encoded text for processing or interchange.
                                Code units are particular units of computer storage. 
                                Other character encoding standards typically use code 
                                units defined as 8-bit units—that is, octets. 
        Code unit sequence:     An ordered sequence of one or more code units.
                                When the code unit is an 8-bit unit, a code unit sequence 
                                may also be referred to as a byte sequence.
                                Informally, a code unit sequence is itself just referred 
                                to as a string, and a byte sequence is referred to 
                                as a byte string.
        Unicode 8-bit string:   A Unicode string containing only UTF-8 code units.
    */
    if (cp < 0x80) {
        buf[0] = cp;
        return 1;
    } 
    if (cp < 0x800) {
        if (cp >= 0xd800 && cp <= 0xdfff) { return -1; /* surrogates */ }
        buf[1] = 0b10000000 | (cp & 0x3f);
        buf[0] = 0b11000000 | (cp >> 6);
        return 2;
    } 
    if (cp < 0x10000) {
        buf[2] = 0b10000000 | (cp & 0x3f);
        buf[1] = 0b10000000 | (cp >> 6 & 0x3f);
        buf[0] = 0b11100000 | (cp >> 12);
        return 3;
    } 
    if (cp < 0x110000) {
        buf[3] = 0b10000000 | (cp & 0x3f);
        buf[2] = 0b10000000 | (cp >> 6 & 0x3f);
        buf[1] = 0b10000000 | (cp >> 12 & 0x3f);
        buf[0] = 0b11110000 | (cp >> 18);
        return 4;
    } 
    return -2;
}

void byte2xcode(uint8_t b, uint8_t **p) {
    /* Converts a byte to "\xHH" string representation */
	uint8_t n;

    (*p)[0] = '\\';
    (*p)[1] = 'x';
    
    n = b & 0x0f;
    if (n > 9) { n += 7; }
    (*p)[3] = '0' + n;
    
    n = b >> 4;
    if (n > 9) { n += 7; }
    (*p)[2] = '0' + n;

    *p += 4;
}
