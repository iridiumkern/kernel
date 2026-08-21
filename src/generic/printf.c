#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "lib/flanterm/src/flanterm.h"

extern struct flanterm_context *flantermctx;

static void putstr(const char *s) {
    flanterm_write(flantermctx, s, __builtin_strlen(s));
}

__attribute__((noinline)) static void putchar_ft(char c) {
    // Fixes a bug on AARCH64 where referencing &c directly fails
    if (c == '\n') {
        char cr = '\r';
        flanterm_write(flantermctx, &cr, 1);
    }
    char buf[1];
    buf[0] = c;
    flanterm_write(flantermctx, buf, 1);
}

static void print_uint(uint64_t val, unsigned base, int upper) {
    char buf[32];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    if (val == 0) {
        putchar_ft('0');
        return;
    }

    while (val > 0) {
        buf[i++] = digits[val % base];
        val /= base;
    }

    while (i > 0) {
        putchar_ft(buf[--i]);
    }
}

static void print_int(int64_t val, unsigned base) {
    if (val < 0) {
        putchar_ft('-');
        print_uint((uint64_t)(-val), base, 0);
    } else {
        print_uint((uint64_t)val, base, 0);
    }
}

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = 0;

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            putchar_ft(fmt[i]);
            count++;
            continue;
        }

        i++; // skip '%'

        switch (fmt[i]) {
            case 'c': {
                char c = (char)va_arg(args, int);
                putchar_ft(c);
                count++;
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s) s = "(null)";
                putstr(s);
                count += (int)__builtin_strlen(s);
                break;
            }
            case 'd':
            case 'i': {
                int64_t val = va_arg(args, int);
                print_int(val, 10);
                count++;
                break;
            }
            case 'u': {
                uint64_t val = va_arg(args, unsigned int);
                print_uint(val, 10, 0);
                count++;
                break;
            }
            case 'x': {
                uint64_t val = va_arg(args, unsigned int);
                print_uint(val, 16, 0);
                count++;
                break;
            }
            case 'X': {
                uint64_t val = va_arg(args, unsigned int);
                print_uint(val, 16, 1);
                count++;
                break;
            }
            case 'p': {
                void *ptr = va_arg(args, void *);
                putstr("0x");
                print_uint((uint64_t)(uintptr_t)ptr, 16, 0);
                count++;
                break;
            }
            case '%': {
                putchar_ft('%');
                count++;
                break;
            }
            default: {
                // Unknown specifier, print literally
                putchar_ft('%');
                putchar_ft(fmt[i]);
                count += 2;
                break;
            }
        }
    }

    va_end(args);
    return count;
}