#include <panic.h>
#include <stdio.h>
#include "../flanterm/src/flanterm.h"

extern struct flanterm_context *flantermctx;
extern void print_logo(void);

/**
 * @brief The function behind the kpanic macro
 * 
 * @param file The file (provided by macro)
 * @param func The function (provided by macro)
 * @param line The line (provided by macro)
 * @param fmt The format string
 * @param ... Any args used by the format string like %x
 */
__attribute__((noreturn)) void __kpanic(const char *file, const char *func, int line, const char *fmt, ...) {
    #ifdef __x86_64__
    __asm("cli");
    #endif
    flanterm_set_text_bg(flantermctx, 4, false);
    flanterm_clear(flantermctx, true);

    print_logo();

    // Print header
    printf("\n\n*** KERNEL PANIC ***\n");
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