#pragma once

#include <stdint.h>

/**
 * @brief Sets up the PMM
 * 
 */
void pmm_init(void);

/**
 * @brief Allocates a page
 * 
 * @return uint64_t The page phys addr
 */
uint64_t pmm_alloc(void);

/**
 * @brief Frees a page
 * 
 * @param address The pages phys addr
 */
void pmm_free(uint64_t address);

/**
 * @brief Allocates contig phys pages
 * 
 * @param pages The page count needed
 * @return uint64_t The "base" page, 0 if a failure
 */
uint64_t pmm_alloc_pages(uint64_t pages);

/**
 * @brief Frees multiple (contig) pages
 * 
 * @param address The "base" page
 * @param pages The count to free
 */
void pmm_free_pages(uint64_t address, uint64_t pages);