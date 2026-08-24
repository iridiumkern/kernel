#pragma once

#include <stdint.h>

void pmm_init(void);

uint64_t pmm_alloc(void);

void pmm_free(uint64_t address);

uint64_t pmm_alloc_pages(uint64_t pages);

void pmm_free_pages(uint64_t address, uint64_t pages);