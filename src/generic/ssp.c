#include <panic.h>
#include <stdint.h>
#include <stdbool.h>

uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;
static bool hasbeeninit = false;

void sspsetup(void) {
    (void)hasbeeninit;
    /* TODO: Replace static guard with CSPRNG output. */
    hasbeeninit = true;
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
    kpanic("SSP triggered!\n");
}