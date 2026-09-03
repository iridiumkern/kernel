#pragma once

#include <stdint.h>
#include <stdbool.h>

#define VMM_PAGE_SIZE 4096ULL

#define VMM_P        (1ULL << 0)
#define VMM_RW       (1ULL << 1)
#define VMM_US       (1ULL << 2)
#define VMM_PWT      (1ULL << 3)
#define VMM_PCD      (1ULL << 4)
#define VMM_A        (1ULL << 5)
#define VMM_D        (1ULL << 6)
#define VMM_PS       (1ULL << 7)
#define VMM_G        (1ULL << 8)
#define VMM_NX       (1ULL << 63)

uint64_t read_cr3(void);
void write_cr3(uint64_t cr3);
bool vmm_map(uint64_t virt, uint64_t phys, uint64_t flags);
bool vmm_unmap(uint64_t virt);
uint64_t vmm_get_phys(uint64_t virt);
void vmm_map_pages(uint64_t virt, uint64_t phys, uint64_t pages, uint64_t flags);
bool vmm_free_pages(uint64_t virt, uint64_t pages);
bool vmm_is_page_mapped(uint64_t virt);
void vmm_init(void);

uint64_t vmm_find_free_pages(uint64_t pages, bool user);