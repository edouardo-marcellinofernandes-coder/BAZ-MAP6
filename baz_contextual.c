// baz_contextual.c - BASE150000 contextual zone encoder
// Author: Edouardo-Marcellino Fernandes
// Motto: "By the fog, the impossible dies."
#include <stdio.h>
#include <stdlib.h>
#include "baz_semantic_mapping.h"

typedef struct {
    int zone_id;
    int zone_base;
    int row;
    int depth;
} baz_context_t;

void baz_context_init(baz_context_t *ctx, int index) {
    // Identify zone boundaries (simple version for demo)
    if (index < BAZ_ZONE_LATIN_START) ctx->zone_id = 0;
    else if (index < BAZ_ZONE_HEALTH) ctx->zone_id = 1;
    else if (index < BAZ_ZONE_BALISEE1) ctx->zone_id = 2;
    else if (index < BAZ_ZONE_DIGITS) ctx->zone_id = 3;
    else if (index < BAZ_ZONE_BALISEE2) ctx->zone_id = 4;
    else if (index < BAZ_ZONE_MATHPIC) ctx->zone_id = 5;
    else if (index < BAZ_ZONE_BALISEE3) ctx->zone_id = 6;
    else if (index < BAZ_ZONE_PUNCT) ctx->zone_id = 7;
    else ctx->zone_id = 8;
    ctx->zone_base = ctx->zone_id * 10000;
    ctx->row = index - ctx->zone_base;
    ctx->depth = 0;
}

int baz_get_zone(int index) {
    if (index < BAZ_ZONE_LATIN_START) return 0;
    if (index < BAZ_ZONE_HEALTH) return 1;
    if (index < BAZ_ZONE_BALISEE1) return 2;
    if (index < BAZ_ZONE_DIGITS) return 3;
    if (index < BAZ_ZONE_BALISEE2) return 4;
    if (index < BAZ_ZONE_MATHPIC) return 5;
    if (index < BAZ_ZONE_BALISEE3) return 6;
    if (index < BAZ_ZONE_PUNCT) return 7;
    return 8;
}

size_t baz_contextual_encode(const int *indices, size_t n, int *out) {
    // Demo: 2 digits for same-zone, 6 digits for cross-zone
    int prev_zone = baz_get_zone(indices[0]);
    size_t o = 0;
    for (size_t i = 0; i < n; ++i) {
        int zone = baz_get_zone(indices[i]);
        if (zone == prev_zone && o+2 <= n*6) {
            out[o++] = (indices[i] % 100); // row only
        } else if (o+6 <= n*6) {
            out[o++] = (zone * 100000) + indices[i]; // full index as 6 digits
            prev_zone = zone;
        }
    }
    return o;
}

size_t baz_contextual_decode(const int *encoded, size_t n, int *decoded) {
    // Demo: check if entry is <100 (row only) or >99999 (full)
    size_t d = 0;
    int curr_zone = 0; // Assume starting zone 0
    for (size_t i = 0; i < n; ++i) {
        if (encoded[i] < 100) {
            decoded[d++] = curr_zone * 10000 + encoded[i];
        } else {
            curr_zone = encoded[i] / 100000;
            decoded[d++] = encoded[i] % 100000;
        }
    }
    return d;
}

int main(void) {
    int indices[] = {10000, 10001, 20200, 20201, 50000, 50001};
    size_t n = sizeof(indices)/sizeof(indices[0]);
    int encoded[36] = {0};
    int decoded[6] = {0};
    size_t en = baz_contextual_encode(indices, n, encoded);
    printf("Encoded: ");
    for (size_t i=0; i<en; ++i) printf("%d ", encoded[i]);
    printf("\n");
    size_t dn = baz_contextual_decode(encoded, en, decoded);
    printf("Decoded: ");
    for (size_t i=0; i<dn; ++i) printf("%d ", decoded[i]);
    printf("\n");
    printf("Compression ratio: %.2f\n", (double)en/dn);
    return 0;
}
