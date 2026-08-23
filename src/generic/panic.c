#include <panic.h>
#include <stdio.h>
#include "lib/flanterm/src/flanterm.h"

extern struct flanterm_context *flantermctx;

__attribute__((noreturn))
void __kpanic(const char *file, const char *func, int line, const char *fmt, ...) {
    #ifdef __x86_64__
    __asm("cli");
    #endif
    flanterm_clear(flantermctx, true);

    // Print header
    printf("*** KERNEL PANIC ***\n");
    printf("%s:%d in %s():\n", file, line, func);

    // Print the formatted message
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    // Halt forever
    for (;;) {
    }
}