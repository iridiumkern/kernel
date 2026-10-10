#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Checks if memory is mapped
 * 
 * @param start Start of memory range (does not need to be aligned)
 * @param size The size
 * @return true It is mapped
 * @return false It is not mapped
 */
bool range_is_mapped(uintptr_t start, uintptr_t size);

/**
 * @brief Checks if a page is mapped/setup for the userland
 * 
 * @param start The start of the memory range (does not need alignment)
 * @param size The size
 * @return true The memory range is mapped as user
 * @return false The memory range is not user
 */
bool range_is_user(uintptr_t start, uintptr_t size);