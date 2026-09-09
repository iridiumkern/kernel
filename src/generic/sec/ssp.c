#include <sec/random.h>
#include <stdint.h>
#include <stdbool.h>
#include <panic.h>

uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;

__attribute__((no_stack_protector)) bool sspsetup(void) {
    uint64_t data[8];

    if (!csprng_getrand((uint8_t *)data)) return false;

    __stack_chk_guard = data[0];
    return true;
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
    kpanic("SSP triggered!\n");
}