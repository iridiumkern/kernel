#include <sec/hashes.h>
#include <sec/random.h>
#include <stdbool.h>
#include <stddef.h>

bool sss_sha512(const void *input, size_t input_size, void *output) {
    sha512_bytes(input, input_size, output);
    // Always returns true since SHA512_bytes cannot fail, somehow, lol. Imma look into that later.
    return true;
}

bool sss_get512randsecure(void *output) {
    return csprng_getrand(output);
}