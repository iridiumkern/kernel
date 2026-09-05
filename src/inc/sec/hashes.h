#pragma once

#include <stddef.h>

/**
 * @brief SHA512
 * 
 * @param src Source
 * @param n_bytes Size of the source
 * @param dst_bytes32 The output (64 bytes of data)
 */
void sha512_bytes(const void *src, size_t n_bytes, void *dst_bytes);

#define crypto_hash_sha512_BYTES 64