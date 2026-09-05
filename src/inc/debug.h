#pragma once

#include <stddef.h>

/**
 * @brief Prints out XXD like formatted hex
 * 
 * @param data The start of the data
 * @param len The amount being printed out
 */
void hexdump(const void *data, size_t len);