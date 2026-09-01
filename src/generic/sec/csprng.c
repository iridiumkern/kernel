#include <sec/memory.h>
#include <sec/hashes.h>
#include <sec/random.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <debug.h>

static uint64_t state[8] = {0};
static bool init = false;

bool csprng_init(void) {
    if (init) return false;
    // Just in case some attacker messes with BSS for example
    memset(&state, 0, 64);
    printf("Cleared state\n");

    uint64_t dirtystuff = 0;
    // Quick and dirty entropy, not secure on its own, though assumed to be part of the secret state
    if (!random_u64(&dirtystuff)) return false;
    printf("random_u64 called\n");

    // Use SHA512 inputing dirtystuff and outputting into state
    if (crypto_hash_sha512((void*)&state, (void*)&dirtystuff, 8) != 0) return false;
    printf("SHA512 called called!\n");

    memset(&dirtystuff, 0, 8);
    init = true;
    return true;
}

bool csprng_addentropy(void* data, uint64_t size) {
    if (!init) return false;
    if (!range_is_mapped((uintptr_t)data, size)) return false;
    // Hash the data
    uint64_t hasheddata[8] = {0};
    if (crypto_hash_sha512((void*)&hasheddata, data, size) != 0) return false;

    // Mix the hashed data with the current state
    for (int i = 0; i < 8; i++) {
        state[i] = state[i] ^ hasheddata[i];
    }
    memset(hasheddata, 0, 64);
    return true;
}

// Data must be 64 bytes
bool csprng_getrand(uint8_t *data) {
    uint64_t output[8];
    uint64_t newstate[8];

    if (!init) return false;
    if (!range_is_mapped((uintptr_t)data, 64)) return false;
    
    if (crypto_hash_sha512((void*)&output, (void*)&state, 64) != 0) {
        memset(&output, 0, 64);
        memset(&newstate, 0, 64);
        return false;
    }

    if (crypto_hash_sha512((void*)&newstate,(void*)&output, 64) != 0) {
        memset(&output, 0, 64);
        memset(&newstate, 0, 64);
        return false;
    }

    memcpy(state, newstate, 64);

    memcpy(data, output, 64);

    memset(output, 0, 64);
    memset(newstate, 0, 64);
    return true;
}