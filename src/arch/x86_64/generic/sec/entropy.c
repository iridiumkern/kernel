#include <immintrin.h>
#include <stdbool.h>
#include <stdint.h>
#include <cpuid.h>

static inline uint64_t rdtsc(void) {
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

bool random_u64(uint64_t *out) {
    uint64_t jitter1 = rdtsc();

    for (volatile uint64_t i = 0; i < (jitter1 & 0x3FF); i++);

    uint64_t jitter2 = rdtsc();

    uint64_t jitter = jitter1 ^ jitter2;

    uint32_t eax, ebx, ecx, edx;

    __cpuid(1, eax, ebx, ecx, edx);

    if (ecx & bit_RDRND) {
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