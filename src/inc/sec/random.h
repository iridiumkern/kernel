#pragma once

#include <stdint.h>
#include <stdbool.h>

// Quick and dirty entropy, good enough to seed a CSPRNG, but should not be considered a secure value within itself
bool random_u64(uint64_t *out);

// Sets up the CSPRNG
bool csprng_init(void);

// Adds and mixes entropy to the CSPRNG state
bool csprng_addentropy(void* data, uint64_t size);

// Gets a (cryptographically) secure random number
// *data MUST be 64 bytes large (uint8_t data[64])
bool csprng_getrand(uint8_t *data);