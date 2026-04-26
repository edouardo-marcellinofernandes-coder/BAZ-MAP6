// BAZiSoooo.c - BASE150000 encoder/decoder, IPv6 utility
// Author: Edouardo-Marcellino Fernandes
// Email: edouardo-marcellino.fernandes@laposte.fr
// Motto: "By the fog, the impossible dies."
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "baz_semantic_mapping.h"

typedef struct {
    uint64_t hi, lo;
} baz_uint128_t;

// Encodes a 128-bit integer in base-150000
size_t baz_encode_128(const baz_uint128_t *val, unsigned int *symbols, size_t maxlen) {
    baz_uint128_t v = *val;
    size_t idx = 0;
    while ((v.hi != 0 || v.lo != 0) && idx < maxlen) {
        // Division: v / BAZ_BASE
        uint64_t rem = v.lo % BAZ_BASE;
        symbols[idx++] = (unsigned int)rem;
        // Update: v = v / BAZ_BASE
        uint64_t new_lo = v.lo / BAZ_BASE;
        uint64_t carry = v.hi % BAZ_BASE;
        v.hi /= BAZ_BASE;
        v.lo = new_lo + carry * ((uint64_t)1 << 32);
    }
    return idx;
}

// Decodes a base-150000 symbol array to 128-bit integer
void baz_decode_128(const unsigned int *symbols, size_t len, baz_uint128_t *out) {
    baz_uint128_t v = {0};
    for (size_t i = len; i-- > 0;) {
        // v = v * BAZ_BASE + symbols[i]
        uint64_t hi = v.hi * BAZ_BASE + ((v.lo * BAZ_BASE) >> 32);
        uint64_t lo = (v.lo * BAZ_BASE) + symbols[i];
        v.hi = hi + (lo >> 32);
        v.lo = lo & 0xFFFFFFFF;
    }
    *out = v;
}

// IPv6 encode: 16-byte -> base-150000 symbols
size_t baz_ipv6_encode(const uint8_t ipv6[16], unsigned int *symbols, size_t maxlen) {
    baz_uint128_t v = {0};
    for (int i = 0; i < 8; ++i) v.hi = (v.hi << 8) | ipv6[i];
    for (int i = 8; i < 16; ++i) v.lo = (v.lo << 8) | ipv6[i];
    return baz_encode_128(&v, symbols, maxlen);
}

void baz_ipv6_decode(const unsigned int *symbols, size_t len, uint8_t ipv6[16]) {
    baz_uint128_t v = {0};
    baz_decode_128(symbols, len, &v);
    for (int i = 7; i >= 0; --i) { ipv6[i] = v.hi & 0xFF; v.hi >>= 8; }
    for (int i = 15; i >= 8; --i) { ipv6[i] = v.lo & 0xFF; v.lo >>= 8; }
}

// Convert codepoint to UTF-8 bytes
int baz_codepoint_to_utf8(unsigned int cp, char *utf8) {
    if (cp <= 0x7F) {
        utf8[0] = cp; return 1;
    } else if (cp <= 0x7FF) {
        utf8[0] = 0xC0 | ((cp >> 6) & 0x1F);
        utf8[1] = 0x80 | (cp & 0x3F); return 2;
    } else if (cp <= 0xFFFF) {
        utf8[0] = 0xE0 | ((cp >> 12) & 0x0F);
        utf8[1] = 0x80 | ((cp >> 6) & 0x3F);
        utf8[2] = 0x80 | (cp & 0x3F); return 3;
    } else {
        utf8[0] = 0xF0 | ((cp >> 18) & 0x07);
        utf8[1] = 0x80 | ((cp >> 12) & 0x3F);
        utf8[2] = 0x80 | ((cp >> 6) & 0x3F);
        utf8[3] = 0x80 | (cp & 0x3F); return 4;
    }
}

// Pretty-print encoded sequence
void baz_print_encoded(const unsigned int *symbols, size_t len) {
    char u8[5];
    for (size_t i = 0; i < len; ++i) {
        int n = baz_codepoint_to_utf8(baz_int_to_cp[symbols[i]], u8);
        u8[n] = 0; printf("%s", u8);
    }
    printf("\n");
}

// Demo main()
int main(void) {
    uint8_t example[16] = {0x20,0x01,0x0d,0xb8,0x85,0xa3,0x00,0x00,0x00,0x00,0x8a,0x2e,0x03,0x70,0x73,0x34};
    unsigned int symbols[32];
    size_t n = baz_ipv6_encode(example, symbols, 32);
    printf("Encoded symbols: ");
    for (size_t i=0; i<n; ++i) printf("%u ", symbols[i]);
    printf("\nBAZiSoooo UTF-8: ");
    baz_print_encoded(symbols, n);
    uint8_t out[16] = {0};
    baz_ipv6_decode(symbols, n, out);
    printf("Decoded IPv6: ");
    for (int i=0; i<16; ++i) printf("%02x%c", out[i], i==15?'\n':':');
    return 0;
}
