#ifndef RTSE_SHA256_H
#define RTSE_SHA256_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
    uint32_t state[8];
    uint64_t bit_count;
    uint8_t block[64];
    size_t block_used;
} ts_sha256_ctx;
void ts_sha256_init(ts_sha256_ctx *ctx);
void ts_sha256_update(ts_sha256_ctx *ctx, const void *data, size_t length);
void ts_sha256_final(ts_sha256_ctx *ctx, uint8_t digest[32]);
void ts_sha256_hex(const uint8_t digest[32], char output[65]);
#endif
