#include <debug.h>
#include <sec/random.h>
#include <stdint.h>
#include <stdio.h>

uint64_t apisyscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;
    (void)arg6;
    if (arg1 == 0) {
        // CSPRNG test
        uint64_t out[8];
        csprng_getrand((uint8_t*)&out);
        hexdump((void*)&out, 64);
        return 0;
    }
    printf("%s", arg1);
    return 0;
}