#pragma once

#include <stdarg.h>

__attribute__((noreturn))
void __kpanic(const char *file, const char *func, int line, const char *fmt, ...);

#define kpanic(fmt, ...) \
    __kpanic(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)