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

/**
 * @brief Sets up the CSPRNG
 * 
 * @return true init worked
 * @return false init did not work and the CSPRNG failed to init
 */
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
    sha512_bytes((void*)&dirtystuff, 8, (void*)&state);
    printf("SHA512 called called!\n");

    memset(&dirtystuff, 0, 8);
    init = true;
    return true;
}

/**
 * @brief Adds entropy to the CSPRNG
 * 
 * @param data The data
 * @param size Size of the data
 * @return true Data was added
 * @return false Data was not added
 */
bool csprng_addentropy(void* data, uint64_t size) {
    if (!init) return false;
    if (!range_is_mapped((uintptr_t)data, size)) return false;
    // Hash the data
    uint64_t hasheddata[8] = {0};
    sha512_bytes(data, 64, (void*)&hasheddata);

    // Mix the hashed data with the current state
    for (int i = 0; i < 8; i++) {
        state[i] = state[i] ^ hasheddata[i];
    }
    memset(hasheddata, 0, 64);
    return true;
}

/**
 * @brief Gets a random cryptographically secure number
 * 
 * @param out The output, must be 64 bytes large.
 * @return true Number was generated
 * @return false Number was not generated
 */
bool csprng_getrand(uint8_t *out) {
    uint64_t output[8];
    uint64_t newstate[8];

    if (!init) return false;
    if (!range_is_mapped((uintptr_t)out, 64)) return false;
    
    sha512_bytes((void*)&state,64, (void*)&output);

    sha512_bytes((void*)&output, 64, (void*)&newstate);

    // Add some extra entropy
    uint64_t tmpbuf;
    random_u64(&tmpbuf);
    csprng_addentropy(&tmpbuf, 8);

    memcpy(state, newstate, 64);

    memcpy(out, output, 64);

    memset(output, 0, 64);
    memset(newstate, 0, 64);
    return true;
}