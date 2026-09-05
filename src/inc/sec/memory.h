#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Checks if memory is mapped
 * 
 * @param start Start of memory range (does not need to be aligned)
 * @param end End of range (does not have to be aligned)
 * @return true It is mapped
 * @return false It is not mapped
 */
bool range_is_mapped(uintptr_t start, uintptr_t end);