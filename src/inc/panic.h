#pragma once

#include <stdarg.h>

/**
 * @brief The wrapper for the kpanic macro
 * 
 * @param file The file where the exception happened
 * @param func The function where the exeception happened
 * @param line The line in the file where the exception happened
 * @param fmt Format string
 * @param ... Args
 */
__attribute__((noreturn)) void __kpanic(const char *file, const char *func, int line, const char *fmt, ...);

/**
 * @brief Wrapper for __kpanic
 * 
 */
#define kpanic(fmt, ...) \
    __kpanic(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)