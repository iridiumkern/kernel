/**
 * @file vmm.c
 * @author apixeldev
 * @brief Simple VMM, not the best but its decent.
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <panic.h>

#include <kernel.h>
#include <pmm.h>
#include <x86_64/vmm.h>

#define PT_ENTRIES 512
#define PAGE_MASK  0x000FFFFFFFFFF000ULL
#define PAGE_SIZE 4096

#define VMM_USER_START   0x0000000000001000ULL
#define VMM_USER_END     0x00007FFFFFFFF000ULL

#define VMM_KERNEL_START 0xFFFF800100000000ULL
#define VMM_KERNEL_END   0xFFFFFFFFFFFFF000ULL

/**
 * @brief Reads the CR3
 * 
 * @return uint64_t CR3 phys addr
 */
uint64_t read_cr3(void) {
    uint64_t cr3;

    __asm__ volatile (
        "mov %0, cr3"
        : "=r"(cr3)
    );

    return cr3;
}

/**
 * @brief Writes a new CR3
 * 
 * @param cr3 The CR3 to write
 */
void write_cr3(uint64_t cr3) {
    __asm__ volatile (
        "mov cr3, %0"
        :
        : "r"(cr3)
        : "memory"
    );
}

/**
 * @brief Uses HHDM to turn a physical address into a virtual one (not suitable for general use)
 * 
 * @param phys The phys addr
 * @return uint64_t* R/W pointer to said memory
 */
static inline uint64_t *phys_to_virt(uint64_t phys) {
    return (uint64_t *)(uintptr_t)(phys + krnl.hhdm_offset);
}

/**
 * @brief Allocates a new table for the VMM
 * 
 * @return uint64_t The physical address
 */
static uint64_t vmm_alloc_table(void) {
    uint64_t phys = pmm_alloc();

    if (!phys)
        kpanic("VMM: out of physical memory\n");

    uint64_t *table = phys_to_virt(phys);

    for (uint64_t i = 0; i < PT_ENTRIES; i++)
        table[i] = 0;

    return phys;
}

/**
 * @brief Grabs the PML4 index of a pointer
 * 
 * @param virt The memory address
 * @return uint64_t The PML4 index
 */
static inline uint64_t pml4_index(uint64_t virt) {
    return (virt >> 39) & 0x1FF;
}

/**
 * @brief Grabs the PDPT index of a page
 * 
 * @param virt The page
 * @return uint64_t The index
 */
static inline uint64_t pdpt_index(uint64_t virt) {
    return (virt >> 30) & 0x1FF;
}

/**
 * @brief Grabs the PD index
 * 
 * @param virt The virtual address
 * @return uint64_t The PD table
 */
static inline uint64_t pd_index(uint64_t virt) {
    return (virt >> 21) & 0x1FF;
}

/**
 * @brief Grabs the PT index
 * 
 * @param virt The virtual address
 * @return uint64_t The PT index
 */
static inline uint64_t pt_index(uint64_t virt) {
    return (virt >> 12) & 0x1FF;
}

/**
 * @brief Maps a page
 * 
 * @param virt The virtual page that is being mapped onto
 * @param phys The physical memory referenced by the page
 * @param flags The flags for the MMU
 * @return true it worked
 * @return false it failed
 */
bool vmm_map(uint64_t virt, uint64_t phys, uint64_t flags) {
    if (virt & (VMM_PAGE_SIZE - 1))
        return false;

    if (phys & (VMM_PAGE_SIZE - 1))
        return false;

    uint64_t cr3 = read_cr3();
    uint64_t pml4_phys = cr3 & PAGE_MASK;

    uint64_t *pml4 = phys_to_virt(pml4_phys);

    /*
     * PML4
     */
    uint64_t pml4_i = pml4_index(virt);

    if (!(pml4[pml4_i] & VMM_P)) {
        uint64_t table = vmm_alloc_table();

        pml4[pml4_i] = table | VMM_P | VMM_RW;
    }

    uint64_t *pdpt =
        phys_to_virt(pml4[pml4_i] & PAGE_MASK);

    /*
     * PDPT
     */
    uint64_t pdpt_i = pdpt_index(virt);

    if (!(pdpt[pdpt_i] & VMM_P)) {
        uint64_t table = vmm_alloc_table();

        pdpt[pdpt_i] = table | VMM_P | VMM_RW;
    }

    uint64_t *pd =
        phys_to_virt(pdpt[pdpt_i] & PAGE_MASK);

    /*
     * Page Directory
     */
    uint64_t pd_i = pd_index(virt);

    if (!(pd[pd_i] & VMM_P)) {
        uint64_t table = vmm_alloc_table();

        pd[pd_i] = table | VMM_P | VMM_RW;
    }

    uint64_t *pt =
        phys_to_virt(pd[pd_i] & PAGE_MASK);

    /*
     * Page Table
     */
    uint64_t pt_i = pt_index(virt);

    if (pt[pt_i] & VMM_P)
        return false;

    pt[pt_i] = (phys & PAGE_MASK) | flags | VMM_P;

    /*
     * Make the new translation visible.
     */
    __asm__ volatile (
        "invlpg [%0]"
        :
        : "r"(virt)
        : "memory"
    );

    return true;
}

/**
 * @brief Unmaps a page
 * 
 * @param virt The virtual address
 * @return true It worked
 * @return false It failed
 */
bool vmm_unmap(uint64_t virt) {
    if (virt & (VMM_PAGE_SIZE - 1))
        return false;

    uint64_t cr3 = read_cr3();
    uint64_t pml4_phys = cr3 & PAGE_MASK;

    uint64_t *pml4 = phys_to_virt(pml4_phys);

    uint64_t pml4_i = pml4_index(virt);

    if (!(pml4[pml4_i] & VMM_P))
        return false;

    uint64_t *pdpt =
        phys_to_virt(pml4[pml4_i] & PAGE_MASK);

    uint64_t pdpt_i = pdpt_index(virt);

    if (!(pdpt[pdpt_i] & VMM_P))
        return false;

    uint64_t *pd =
        phys_to_virt(pdpt[pdpt_i] & PAGE_MASK);

    uint64_t pd_i = pd_index(virt);

    if (!(pd[pd_i] & VMM_P))
        return false;

    uint64_t *pt =
        phys_to_virt(pd[pd_i] & PAGE_MASK);

    uint64_t pt_i = pt_index(virt);

    if (!(pt[pt_i] & VMM_P))
        return false;

    pt[pt_i] = 0;

    __asm__ volatile (
        "invlpg [%0]"
        :
        : "r"(virt)
        : "memory"
    );

    return true;
}

/**
 * @brief Gets the physical address of a virtual pointer
 * 
 * @param virt The virtual pointer
 * @return uint64_t The physical address
 */
uint64_t vmm_get_phys(uint64_t virt) {
    uint64_t cr3 = read_cr3();
    uint64_t pml4_phys = cr3 & PAGE_MASK;

    uint64_t *pml4 = phys_to_virt(pml4_phys);

    uint64_t pml4_i = pml4_index(virt);

    if (!(pml4[pml4_i] & VMM_P))
        return 0;

    uint64_t *pdpt =
        phys_to_virt(pml4[pml4_i] & PAGE_MASK);

    uint64_t pdpt_i = pdpt_index(virt);

    if (!(pdpt[pdpt_i] & VMM_P))
        return 0;

    uint64_t *pd =
        phys_to_virt(pdpt[pdpt_i] & PAGE_MASK);

    uint64_t pd_i = pd_index(virt);

    if (!(pd[pd_i] & VMM_P))
        return 0;

    /*
     * We aren't supporting 2 MiB pages yet.
     */
    if (pd[pd_i] & VMM_PS)
        return (pd[pd_i] & 0x000FFFFFFFE00000ULL) |
               (virt & 0x1FFFFFULL);

    uint64_t *pt =
        phys_to_virt(pd[pd_i] & PAGE_MASK);

    uint64_t pt_i = pt_index(virt);

    if (!(pt[pt_i] & VMM_P))
        return 0;

    return (pt[pt_i] & PAGE_MASK) |
           (virt & 0xFFF);
}

/**
 * @brief Maps multiple pages
 * 
 * @param virt Virtual target (base)
 * @param phys The physical target (base)
 * @param pages The amount of pages to map
 * @param flags The flags for all pages
 */
void vmm_map_pages(uint64_t virt, uint64_t phys, uint64_t pages, uint64_t flags) {
    for (uint64_t i = 0; i < pages; i++) {
        if (!vmm_map(virt + (i * PAGE_SIZE), phys + (i * PAGE_SIZE), flags)) {
            kpanic("VMM: failed to map page\n");
        }
    }
}

/**
 * @brief Unmaps multiple pages
 * 
 * @param virt The virtual address (base)
 * @param pages The amount of pages to unmap
 */
bool vmm_free_pages(uint64_t virt, uint64_t pages) {
    for (uint64_t i = 0; i < pages; i++) {
        if (!vmm_unmap(virt + (i * PAGE_SIZE))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Finds free pages
 * 
 * @param pages The amount of pages needed
 * @param user Is it for the userland
 * @return uint64_t The start (base) address
 */
uint64_t vmm_find_free_pages(uint64_t pages, bool user) {
    if (pages == 0)
        return 0;

    uint64_t start = user ? VMM_USER_START : VMM_KERNEL_START;
    uint64_t end   = user ? VMM_USER_END   : VMM_KERNEL_END;

    uint64_t consecutive = 0;
    uint64_t found = 0;

    for (uint64_t virt = start; virt <= end; virt += PAGE_SIZE) {
        if (vmm_get_phys(virt) == 0) {
            if (consecutive == 0)
                found = virt;

            consecutive++;

            if (consecutive == pages)
                return found;
        } else {
            consecutive = 0;
        }
    }

    return 0;
}

/**
 * @brief Checks if a page is mapped
 * 
 * @param virt The virtual address
 * @return true Mapped
 * @return false unmapped
 */
bool vmm_is_page_mapped(uint64_t virt) {
    if (virt & (PAGE_SIZE - 1))
        return false;

    return vmm_get_phys(virt) != 0;
}

/**
 * @brief Sets up the VMM
 * 
 */
void vmm_init(void) {
    uint64_t cr3 = read_cr3();

    printf("VMM: CR3: %lx\n", cr3 & PAGE_MASK);
    printf("VMM: HHDM: %lx\n", krnl.hhdm_offset);
    printf("VMM: initialized\n");
}