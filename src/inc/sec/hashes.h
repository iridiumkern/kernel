#pragma once

#include <stddef.h>

// SHA512
void sha512_bytes(const void *src, size_t n_bytes, void *dst_bytes32);

#define crypto_hash_sha512_BYTES 64