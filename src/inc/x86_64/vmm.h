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

/**
 * @brief Read the current CR3
 * 
 * @return uint64_t The current CR3
 */
uint64_t read_cr3(void);

/**
 * @brief Write a new CR3
 * 
 * @param cr3 The new CR3
 */
void write_cr3(uint64_t cr3);

/**
 * @brief Map a page
 * 
 * @param virt The virtual address
 * @param phys The physical address being mapped
 * @param flags The flags being used
 * @return true It worked
 * @return false It failed
 */
bool vmm_map(uint64_t virt, uint64_t phys, uint64_t flags);

/**
 * @brief Unmap a page
 * 
 * @param virt The pages virtual address
 * @return true It unmapped
 * @return false It did not unmap/was already unmapped
 */
bool vmm_unmap(uint64_t virt);

/**
 * @brief Get the physical address of a page
 * 
 * @param virt The virtual address
 * @return uint64_t The physical address
 */
uint64_t vmm_get_phys(uint64_t virt);

/**
 * @brief Map multiple pages
 * 
 * @param virt The "base" virtual page
 * @param phys The "base" physical page
 * @param pages The amount of pages to map
 * @param flags The flags for every page
 */
void vmm_map_pages(uint64_t virt, uint64_t phys, uint64_t pages, uint64_t flags);

/**
 * @brief Unmap/free pages.
 * 
 * @param virt The virtual "base" page
 * @param pages The amount of pages to unmap
 * @return true They were unmapped
 * @return false They were not unmapped
 */
bool vmm_free_pages(uint64_t virt, uint64_t pages);

/**
 * @brief Check if a page is mapped
 * 
 * @param virt The virtual address
 * @return true It is mapped
 * @return false It is unmapped
 */
bool vmm_is_page_mapped(uint64_t virt);

/**
 * @brief Sets up the VMM
 * 
 */
void vmm_init(void);

/**
 * @brief Looks for a free range of contigious virtual pages
 * 
 * @param pages The count
 * @param user Are they for userland or not
 * @return uint64_t The "base" page, 0 if something failed
 */
uint64_t vmm_find_free_pages(uint64_t pages, bool user);