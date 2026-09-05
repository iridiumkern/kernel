#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Holds context for the kernel to use, basically the kernels state
 * 
 */
typedef struct kernel_info_t {
    uint64_t hhdm_offset;
    bool acpi2;
}kernel_info_t;

// Kernel state
extern kernel_info_t krnl;