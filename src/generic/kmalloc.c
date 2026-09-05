#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <pmm.h>
#ifdef __x86_64__
#include <x86_64/vmm.h>
#endif
#include <stdlib.h>

#define KHEAP_INITIAL_PAGES 16
#define KHEAP_GROW_PAGES    16

#define ALIGNMENT 16ULL

typedef struct heap_block {
    size_t size;
    bool free;

    struct heap_block *next;
    struct heap_block *prev;
} heap_block_t;

static heap_block_t *heap_head = NULL;

static uint64_t heap_start = 0;
static uint64_t heap_end = 0;

static size_t align_up(size_t value) {
    return (value + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

static heap_block_t *find_free_block(size_t size) {
    heap_block_t *block = heap_head;

    while (block) {
        if (block->free && block->size >= size)
            return block;

        block = block->next;
    }

    return NULL;
}

static void split_block(heap_block_t *block, size_t size) {
    if (block->size < size + sizeof(heap_block_t) + ALIGNMENT)
        return;

    heap_block_t *new_block =
        (heap_block_t *)((uint8_t *)(block + 1) + size);

    new_block->size =
        block->size - size - sizeof(heap_block_t);

    new_block->free = true;

    new_block->next = block->next;
    new_block->prev = block;

    if (block->next)
        block->next->prev = new_block;

    block->next = new_block;
    block->size = size;
}

static void merge_next(heap_block_t *block) {
    heap_block_t *next = block->next;

    if (!next || !next->free)
        return;

    block->size += sizeof(heap_block_t) + next->size;
    block->next = next->next;

    if (block->next)
        block->next->prev = block;
}

static bool heap_grow(size_t required) {
    size_t pages = (required + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;

    if (pages < KHEAP_GROW_PAGES)
        pages = KHEAP_GROW_PAGES;

    uint64_t phys = pmm_alloc_pages(pages);

    if (!phys)
        return false;

    uint64_t virt = vmm_find_free_pages(pages, false);

    if (!virt) {
        pmm_free_pages(phys, pages);
        return false;
    }

    vmm_map_pages(
        virt,
        phys,
        pages,
        VMM_P | VMM_RW | VMM_G
    );

    heap_block_t *new_block = (heap_block_t *)virt;

    new_block->size =
        pages * VMM_PAGE_SIZE - sizeof(heap_block_t);

    new_block->free = true;
    new_block->next = NULL;
    new_block->prev = NULL;

    if (!heap_head) {
        heap_head = new_block;
    } else {
        heap_block_t *last = heap_head;

        while (last->next)
            last = last->next;

        last->next = new_block;
        new_block->prev = last;
    }

    if (!heap_start || virt < heap_start)
        heap_start = virt;

    if (virt + pages * VMM_PAGE_SIZE > heap_end)
        heap_end = virt + pages * VMM_PAGE_SIZE;

    return true;
}

void kheap_init(void) {
    heap_head = NULL;
    heap_start = 0;
    heap_end = 0;
}

void *kmalloc(size_t size) {
    if (size == 0)
        return NULL;

    size = align_up(size);

    heap_block_t *block = find_free_block(size);

    if (!block) {
        if (!heap_grow(size))
            return NULL;

        block = find_free_block(size);

        if (!block)
            return NULL;
    }

    split_block(block, size);

    block->free = false;

    return (void *)(block + 1);
}

void kfree(void *ptr) {
    if (!ptr)
        return;

    heap_block_t *block =
        ((heap_block_t *)ptr) - 1;

    block->free = true;

    merge_next(block);

    if (block->prev && block->prev->free) {
        heap_block_t *prev = block->prev;

        merge_next(prev);
        block = prev;
    }

    merge_next(block);
}
