#include <stdbool.h>
#include <stdint.h>

#ifdef __x86_64__
#include <x86_64/vmm.h>
#endif

//static inline uint64_t align_up(uint64_t addr, uint64_t alignment) {
//    return (addr + alignment - 1) & ~(alignment - 1);
//}

static inline uint64_t align_down(uint64_t addr, uint64_t alignment) {
    return addr & ~(alignment - 1);
}

bool range_is_mapped(uintptr_t start, uintptr_t size) {
    #ifdef __x86_64__
    if (size == 0)
        return true;

    uintptr_t first = align_down(start, 0x1000);
    uintptr_t last  = align_down(start + size - 1, 0x1000);

    for (uintptr_t page = first; page <= last; page += 0x1000) {
        if (!vmm_is_page_mapped(page))
            return false;
    }

    return true;
    #else
    (void)start;
    (void)end;
    return false;
    #endif
}