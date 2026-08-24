#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <panic.h>
#include <limine.h>

#define PAGE_SIZE       4096ULL
#define MAX_PHYS_MEM    (64ULL * 1024 * 1024 * 1024)
#define MAX_PAGES       (MAX_PHYS_MEM / PAGE_SIZE)
#define BITMAP_SIZE     ((MAX_PAGES + 7) / 8)

static uint8_t pmm_bitmap[BITMAP_SIZE];

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

static inline void pmm_set(uint64_t page) {
    pmm_bitmap[page / 8] |= (uint8_t)(1 << (page % 8));
}

static inline void pmm_clear(uint64_t page) {
    pmm_bitmap[page / 8] &= (uint8_t)~(1 << (page % 8));
}

static inline int pmm_test(uint64_t page) {
    return (pmm_bitmap[page / 8] >> (page % 8)) & 1;
}

static inline void pmm_clear_range(uint64_t base, uint64_t length) {
    uint64_t first = (base + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t last = (base + length) / PAGE_SIZE;

    if (last > MAX_PAGES)
        last = MAX_PAGES;

    for (uint64_t page = first; page < last; page++)
        pmm_clear(page);
}

void pmm_init(void) {
    if (!memmap.response)
        kpanic("PMM: memory map is NULL\n");

    /*
     * Start with every physical page allocated.
     *
     * This means we only ever expose memory that Limine explicitly
     * identifies as usable.
     */
    for (uint64_t i = 0; i < BITMAP_SIZE; i++)
        pmm_bitmap[i] = 0xff;

    for (uint64_t i = 0; i < memmap.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap.response->entries[i];

        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;

        pmm_clear_range(entry->base, entry->length);
    }

    /*
     * Page zero should never be handed out.
     */
    pmm_set(0);

    printf("PMM: initialized\n");
}

uint64_t pmm_alloc(void) {
    for (uint64_t page = 0; page < MAX_PAGES; page++) {
        if (!pmm_test(page)) {
            pmm_set(page);
            return page * PAGE_SIZE;
        }
    }

    return 0;
}

void pmm_free(uint64_t address) {
    if (address & (PAGE_SIZE - 1))
        kpanic("PMM: attempted to free unaligned physical address\n");

    uint64_t page = address / PAGE_SIZE;

    if (page >= MAX_PAGES)
        kpanic("PMM: attempted to free invalid physical address\n");

    if (!pmm_test(page))
        kpanic("PMM: double free\n");

    pmm_clear(page);
}

uint64_t pmm_alloc_pages(uint64_t pages) {
    if (pages == 0)
        return 0;

    uint64_t consecutive = 0;
    uint64_t start = 0;

    for (uint64_t page = 0; page < MAX_PAGES; page++) {
        if (!pmm_test(page)) {
            if (consecutive == 0)
                start = page;

            consecutive++;

            if (consecutive == pages) {
                for (uint64_t i = start; i < start + pages; i++)
                    pmm_set(i);

                return start * PAGE_SIZE;
            }
        } else {
            consecutive = 0;
        }
    }

    return 0;
}

void pmm_free_pages(uint64_t address, uint64_t pages) {
    if (pages == 0)
        return;

    if (address & (PAGE_SIZE - 1))
        kpanic("PMM: attempted to free unaligned physical address\n");

    uint64_t start = address / PAGE_SIZE;

    if (start >= MAX_PAGES || pages > MAX_PAGES - start)
        kpanic("PMM: attempted to free invalid physical range\n");

    for (uint64_t i = 0; i < pages; i++) {
        uint64_t page = start + i;

        if (!pmm_test(page))
            kpanic("PMM: double free\n");

        pmm_clear(page);
    }
}