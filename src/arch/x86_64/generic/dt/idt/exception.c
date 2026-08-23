#include <stdio.h>
#include "../../src/generic/lib/flanterm/src/flanterm.h"

extern struct flanterm_context *flantermctx;

/**
 * @brief Handles exceptions
 * 
 */
__attribute__((noreturn)) void exception_handler() {
    __asm("cli");
    flanterm_clear(flantermctx, true);
    printf("EXCEPTION!\n");
    __asm("hlt"); // Completely hangs the computer
    __builtin_unreachable();
}