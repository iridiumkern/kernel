#pragma once

#include <stdarg.h>

/**
 * @brief Formatted printing wrapper for vprintf
 * 
 * @param fmt The format string
 * @param ... The arguments being passed
 * @return int The amount of characters printed
 */
int printf(const char *fmt, ...);

/**
 * @brief Formatted printing function
 * 
 * @param fmt The format
 * @param args All the arguments
 * @return int Amount of characters printed
 */
int vprintf(const char *fmt, va_list args);

/**
 * @brief A simple putchar
 * 
 * @param c The character
 */
void putchar_ft(char c);