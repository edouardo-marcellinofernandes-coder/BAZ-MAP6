/*
 * BAZiSoooo BASE150000 v1.0 (C99) - FIXED 128-bit logic
 * Encoder/Decoder for BASE150000 ecosystem
 * "By the fog, the impossible dies"
 * Author: Edouardo-Marcellino Fernandes
 * Email: edouardo-marcellino.fernandes@laposte.fr
 *
 * Fully corrected version: safe 128-bit arithmetic, proper decoder power
 * multiplication, and cleaned-up IPv6 conversion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "baz_semantic_mapping.h"

typedef struct {
    unsigned long long hi;  /* most significant 64 bits */
    unsigned long long lo;  /* least significant 64 bits */
} baz_uint128_t;

/* ---------- Safe 128-bit arithmetic helpers ---------- */

/* Multiply a 128-bit number by a 32-bit unsigned scalar (b < 2^32).
 * Result fits within 128 bits as long as the high limb of a is ≤ (2^96-1)/b,
 * which is always true for our use (BASE^exponent where exponent ≤ ~10). */
static baz_uint128_t baz_mul128_u32(baz_uint128_t a, uint32_t b) {
    uint64_t a0 = a.lo & 0xFFFFFFFF;          /* low 32 bits of lo */
    uint64_t a1 = a.lo >> 32;                 /* high 32 bits of lo */
    uint64_t a2 = a.hi & 0xFFFFFFFF;          /* low 32 bits of hi */
    uint64_t a3 = a.hi >> 32;                 /* high 32 bits of hi (should be 0 for our values) */

    uint64_t p0 = a0 * b;                     /* product of lowest limb */
    uint64_t p1 = a1 * b;                     /* product shifted by 32 */
    uint64_t p2 = a2 * b;                     /* product shifted by 64 */
    uint64_t p3 = a3 * b;                     /* product shifted by 96 */

    uint64_t lo, hi;

    /* Combine p0 and p1 into lower 64 bits, with carry */
    uint64_t mid = p1 + (p0 >> 32);
    lo = (p0 & 0xFFFFFFFFULL) | ((mid & 0xFFFFFFFFULL) << 32);
    uint64_t carry = (mid >> 32) + (p1 > (UINT64_MAX - (p0 >> 32)) ? 1 : 0);

    /* Add p2 (shifted 64) and p3 (shifted 96) to high part */
    hi = p2 + carry;
    carry = (hi < carry) ? 1 : 0;
    hi += p3 << 32;

    baz_uint128_t result = { hi, lo };
    return result;
}

/* Add two 128-bit numbers, return sum. */
static baz_uint128_t baz_add128(baz_uint128_t a, baz_uint128_t b) {
    baz_uint128_t sum;
    sum.lo = a.lo + b.lo;
    sum.hi = a.hi + b.hi + ((sum.lo < a.lo) ? 1 : 0);
    return sum;
}

/* ---------- Base-150000 division (128-bit / 150000) ---------- */
static unsigned int baz_divmod_128_by_150000(baz_uint128_t *val) {
    const unsigned long long BASE = BAZ_BASE;  /* 150000ULL */

    /* First, handle high limb */
    unsigned long long q_hi = val->hi / BASE;
    unsigned long long r_hi = val->hi % BASE;

    /* Combine remainder from hi with the upper part of lo */
    unsigned long long temp = (r_hi << 32) | (val->lo >> 32);
    unsigned long long q_mid = temp / BASE;
    unsigned long long r_mid = temp % BASE;

    /* Combine with the lower part of lo */
    temp = (r_mid << 32) | (val->lo & 0xFFFFFFFFULL);
    unsigned long long q_lo = temp / BASE;
    unsigned long long rem  = temp % BASE;

    /* Build the new quotient */
    val->hi = q_hi;
    val->lo = (q_mid << 32) | q_lo;

    return (unsigned int)rem;
}

/* ---------- UTF-8 output helpers ---------- */
static void baz_codepoint_to_utf8(unsigned int cp, char *buf, size_t *len) {
    if (cp <= 0x7F) {
        buf[0] = (char)cp; *len = 1;
    } else if (cp <= 0x7FF) {
        buf[0] = 0xC0 | (cp >> 6); buf[1] = 0x80 | (cp & 0x3F); *len = 2;
    } else if (cp <= 0xFFFF) {
        buf[0] = 0xE0 | (cp >> 12);
        buf[1] = 0x80 | ((cp >> 6) & 0x3F);
        buf[2] = 0x80 | (cp & 0x3F); *len = 3;
    } else if (cp <= 0x10FFFF) {
        buf[0] = 0xF0 | (cp >> 18);
        buf[1] = 0x80 | ((cp >> 12) & 0x3F);
        buf[2] = 0x80 | ((cp >> 6) & 0x3F);
        buf[3] = 0x80 | (cp & 0x3F); *len = 4;
    } else {
        buf[0] = '?'; *len = 1;
    }
}

static void baz_print_encoded(const unsigned int *symbols, size_t count) {
    char utf8[5]; size_t len;
    for (size_t i = 0; i < count; ++i) {
        if (symbols[i] >= BAZ_TABLE_SIZE) {
            printf("[ERR:%u]", symbols[i]);
            continue;
        }
        baz_codepoint_to_utf8(baz_int_to_cp[symbols[i]], utf8, &len);
        fwrite(utf8, 1, len, stdout);
    }
    printf("\n");
}

/* ---------- Encoding (128-bit → base-150000 digits) ---------- */
void baz_encode_128(baz_uint128_t val, unsigned int *out, size_t *count) {
    *count = 0;
    if (val.hi == 0 && val.lo == 0) {
        out[0] = 0;
        *count = 1;
        return;
    }
    while (val.hi || val.lo) {
        if (*count >= 64) break;  /* safety limit (~22 symbols for 128 bits) */
        out[*count] = baz_divmod_128_by_150000(&val);
        (*count)++;
    }
}

/* ---------- Decoding (base-150000 digits → 128-bit) ---------- */
baz_uint128_t baz_decode_128(const unsigned int *symbols, size_t count) {
    baz_uint128_t val = {0, 0};         /* result accumulator */
    baz_uint128_t power = {0, 1};       /* starts as BASE^0 = 1 */

    for (size_t i = 0; i < count; ++i) {
        if (symbols[i] >= BAZ_BASE)     /* invalid symbol – skip */
            continue;

        /* val += symbols[i] * power */
        baz_uint128_t term = baz_mul128_u32(power, (uint32_t)symbols[i]);
        val = baz_add128(val, term);

        /* power *= BAZ_BASE for the next digit */
        power = baz_mul128_u32(power, (uint32_t)BAZ_BASE);
    }
    return val;
}

/* ---------- IPv6 ↔ 128-bit conversion ---------- */
void baz_ipv6_to_uint128(const unsigned char *ipv6_bytes, baz_uint128_t *val) {
    /* Correct byte-by-byte big-endian construction */
    val->hi = 0;
    val->lo = 0;
    for (int i = 0; i < 16; ++i) {
        val->hi = (val->hi << 8) | (val->lo >> 56);
        val->lo = (val->lo << 8) | ipv6_bytes[i];
    }
}

void baz_uint128_to_ipv6(const baz_uint128_t *val, unsigned char *ipv6_bytes) {
    baz_uint128_t v = *val;
    for (int i = 15; i >= 0; --i) {
        ipv6_bytes[i] = (unsigned char)(v.lo & 0xFF);
        v.lo = (v.lo >> 8) | (v.hi << 56);
        v.hi >>= 8;
    }
}

/* ---------- High‑level IPv6 encode / decode ---------- */
void baz_ipv6_encode(const unsigned char *ipv6_bytes, unsigned int *out, size_t *count) {
    baz_uint128_t val;
    baz_ipv6_to_uint128(ipv6_bytes, &val);
    baz_encode_128(val, out, count);
}

int baz_ipv6_decode(const unsigned int *symbols, size_t count, unsigned char *ipv6_bytes) {
    baz_uint128_t val = baz_decode_128(symbols, count);
    baz_uint128_to_ipv6(&val, ipv6_bytes);
    return 0;
}

/* ---------- Demo main ---------- */
int main(void) {
    printf("BAZiSoooo BASE150000 v1.0 (fixed 128-bit) | By the fog, the impossible dies\n");

    /* Example IPv6: 2001:0db8:85a3::8a2e:0370:7334 */
    unsigned char ipv6[16] = {
        0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00,
        0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34
    };

    unsigned int symbols[64];
    size_t count = 0;

    baz_ipv6_encode(ipv6, symbols, &count);
    printf("Encoded IPv6 (%zu BAZ symbols): ", count);
    baz_print_encoded(symbols, count);

    unsigned char decoded[16] = {0};
    baz_ipv6_decode(symbols, count, decoded);

    printf("Decoded IPv6: ");
    for (int i = 0; i < 16; ++i) {
        printf("%02x", decoded[i]);
        if (i % 2 == 1 && i < 15) printf(":");
    }
    printf("\n");

    /* Verify round‑trip */
    int match = memcmp(ipv6, decoded, 16) == 0;
    printf("Round-trip verification: %s\n", match ? "SUCCESS" : "FAILED");

    return 0;
}
