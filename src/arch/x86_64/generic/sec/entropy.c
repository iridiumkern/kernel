#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;

    __asm__ volatile (
        "cpuid"
        : "=a"(a),
          "=b"(b),
          "=c"(c),
          "=d"(d)
        : "a"(leaf)
        : "cc"
    );

    *eax = a;
    *ebx = b;
    *ecx = c;
    *edx = d;
}

// Reads from the TSC
uint64_t rdtsc(void) {
    uint32_t lo, hi;

    __asm__ volatile (
        "rdtsc"
        : "=a"(lo), "=d"(hi)
    );

    return ((uint64_t)hi << 32) | lo;
}

static inline bool rdrand64(uint64_t *out) {
    unsigned char ok;

    __asm__ volatile (
        "rdrand %0\n"
        "setc %1"
        : "=r"(*out), "=qm"(ok)
        :
        : "cc"
    );

    return ok;
}

// Generic function for random unsigned 64bit integers
bool random_u64(uint64_t *out) {
    uint64_t jitter1 = rdtsc();

    for (volatile uint64_t i = 0; i < (jitter1 & 0x3FF); i++);

    uint64_t jitter2 = rdtsc();

    uint64_t jitter = jitter1 ^ jitter2;

    uint32_t eax, ebx, ecx, edx;

    cpuid(1, &eax, &ebx, &ecx, &edx);

    if (ecx & (1U << 30)) {
        uint64_t value;

        if (rdrand64(&value)) {
            *out = jitter ^ value;
        } else {
            *out = jitter;
        }
    } else {
        *out = jitter;
    }

    return true;
}