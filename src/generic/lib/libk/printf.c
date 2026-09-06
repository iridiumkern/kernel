#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "../flanterm/src/flanterm.h"

extern struct flanterm_context *flantermctx;

#ifdef __x86_64__
extern void write_serial(char a);
#endif

static void putstr(const char *s) {
    flanterm_write(flantermctx, s, __builtin_strlen(s));
    #ifdef __x86_64__
    while (*s != '\0') {
        write_serial(*s++);
    }
    #endif
}

__attribute__((noinline)) void putchar_ft(char c) {
    // Fixes a bug on AARCH64 where referencing &c directly fails
    if (c == '\n') {
        char cr = '\r';
        flanterm_write(flantermctx, &cr, 1);
        #ifdef __x86_64__
        write_serial(cr);
        #endif
    }
    char buf[1];
    buf[0] = c;
    flanterm_write(flantermctx, buf, 1);
    #ifdef __x86_64__
    write_serial(buf[0]);
    #endif
    if (c == '\b') {
        char cr = ' ';
        flanterm_write(flantermctx, &cr, 1);
        #ifdef __x86_64__
        write_serial(cr);
        #endif
        cr = '\b';
        flanterm_write(flantermctx, &cr, 1);
        #ifdef __x86_64__
        write_serial(cr);
        #endif
    }
}

static int print_uint_padded(uint64_t val, unsigned base, int upper, int width, int zero_pad) {
    char buf[32];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val > 0) {
            buf[i++] = digits[val % base];
            val /= base;
        }
    }

    int digit_count = i;
    int pad = width - digit_count;
    if (pad < 0) pad = 0;

    char pad_char = zero_pad ? '0' : ' ';
    for (int p = 0; p < pad; p++) {
        putchar_ft(pad_char);
    }

    while (i > 0) {
        putchar_ft(buf[--i]);
    }

    return digit_count + pad;
}

static int print_int_padded(int64_t val, unsigned base, int width, int zero_pad) {
    int count = 0;
    uint64_t uval;

    if (val < 0) {
        putchar_ft('-');
        count++;
        uval = (uint64_t)(-val);
        if (width > 0) width--; // account for the '-' already printed
    } else {
        uval = (uint64_t)val;
    }

    count += print_uint_padded(uval, base, 0, width, zero_pad);
    return count;
}

int vprintf(const char *fmt, va_list args) {
    int count = 0;

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            putchar_ft(fmt[i]);
            count++;
            continue;
        }

        i++; // skip '%'

        // Parse optional zero-pad flag and width, e.g. %02X
        int zero_pad = 0;
        int width = 0;
        if (fmt[i] == '0') {
            zero_pad = 1;
            i++;
        }
        while (fmt[i] >= '0' && fmt[i] <= '9') {
            width = width * 10 + (fmt[i] - '0');
            i++;
        }

        // Parse optional length modifier: l or ll
        int length_mod = 0; // 0 = none, 1 = l, 2 = ll
        if (fmt[i] == 'l') {
            length_mod = 1;
            i++;
            if (fmt[i] == 'l') {
                length_mod = 2;
                i++;
            }
        }

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
                int64_t val;
                if (length_mod == 2)
                    val = va_arg(args, long long);
                else if (length_mod == 1)
                    val = va_arg(args, long);
                else
                    val = va_arg(args, int);
                count += print_int_padded(val, 10, width, zero_pad);
                break;
            }
            case 'u': {
                uint64_t val;
                if (length_mod == 2)
                    val = va_arg(args, unsigned long long);
                else if (length_mod == 1)
                    val = va_arg(args, unsigned long);
                else
                    val = va_arg(args, unsigned int);
                count += print_uint_padded(val, 10, 0, width, zero_pad);
                break;
            }
            case 'x': {
                uint64_t val;
                if (length_mod == 2)
                    val = va_arg(args, unsigned long long);
                else if (length_mod == 1)
                    val = va_arg(args, unsigned long);
                else
                    val = va_arg(args, unsigned int);
                count += print_uint_padded(val, 16, 0, width, zero_pad);
                break;
            }
            case 'X': {
                uint64_t val;
                if (length_mod == 2)
                    val = va_arg(args, unsigned long long);
                else if (length_mod == 1)
                    val = va_arg(args, unsigned long);
                else
                    val = va_arg(args, unsigned int);
                count += print_uint_padded(val, 16, 1, width, zero_pad);
                break;
            }
            case 'p': {
                void *ptr = va_arg(args, void *);
                putstr("0x");
                count += 2 + print_uint_padded((uint64_t)(uintptr_t)ptr, 16, 0, width, zero_pad);
                break;
            }
            case '%': {
                putchar_ft('%');
                count++;
                break;
            }
            default: {
                putchar_ft('%');
                if (length_mod == 1) { putchar_ft('l'); count++; }
                if (length_mod == 2) { putstr("ll"); count += 2; }
                putchar_ft(fmt[i]);
                count += 2;
                break;
            }
        }
    }

    return count;
}

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vprintf(fmt, args);
    va_end(args);
    return ret;
}