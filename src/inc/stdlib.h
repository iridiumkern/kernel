#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Sets up the kernel heap
 * 
 */
void kheap_init(void);

/**
 * @brief Allocates memory
 * 
 * @param size Size needed
 * @return void* The pointer to the start of said memory
 */
void *kmalloc(size_t size);

/**
 * @brief Frees memory
 * 
 * @param ptr Pointer to previously allocated memory
 */
void kfree(void *ptr);