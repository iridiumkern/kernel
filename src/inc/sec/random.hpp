#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Quick and random entropy, secure but should not be considered a CSPRNG equal
 * 
 * @param out The output of the 64bits of random entropy
 * @return true The data is valid/usable
 * @return false The data is invalid/unusable
 */
bool random_u64(uint64_t *out);

/**
 * @brief Sets up the CSPRNG
 * 
 * @return true CSPRNG is now safe to use
 * @return false CSPRNG init failed
 */
bool csprng_init(void);

/**
 * @brief Adds entropy to the CSPRNG
 * 
 * @param data The data to add
 * @param size The size of the data
 * @return true Entropy was added
 * @return false Entropy was not added
 */
bool csprng_addentropy(void* data, uint64_t size);

/**
 * @brief Gets a Cryptographically Secure random value
 * 
 * @param data 64 bytes of data (a uint8_t out[64] would work just fine)
 * @return true Data is secure/valid
 * @return false Data is possibly corrupted, wrong, or was not filled out
 */
bool csprng_getrand(uint8_t *data);