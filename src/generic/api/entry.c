#include <sec/memory.h>
#include <sec/random.h>
#include <scheduler.h>
#include <panic.h>
#include <debug.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __x86_64__
#define CPUID_7_EBX_SMAP (1U << 20)

static inline bool cpu_has_smap(void) {
    uint32_t eax, ebx, ecx, edx;

    __asm(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(7), "c"(0)
    );

    return (ebx & CPUID_7_EBX_SMAP) != 0;
}
#endif

static inline void usermemoryacc(void) {
    #ifdef __x86_64__
    if (cpu_has_smap()) __asm("stac" ::: "memory");
    #endif
}

static inline void usermemorylock(void) {
    #ifdef __x86_64__
    if (cpu_has_smap()) __asm("clac" ::: "memory");
    #endif
}

bool apisyscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    (void)arg4;
    (void)arg5;
    (void)arg6;
    if (arg1 == 0) {
        // Checks if the page is mapped and if the page is userland
        if (!range_is_mapped(arg2, arg3) || !range_is_user(arg2, arg3)) {
            return false;
        }

        // Enables userland memory access then prints the string, then disables userland memory access
        usermemoryacc();
        for (uint64_t i = 0; i < arg3; i++) {
            putchar_ft(*(const char*)arg2++);
        }
        usermemorylock();
        return true;
    } else if (arg1 == 1) {
    } else if (arg1 == 255) {
        // 0x40 is the PID0 only API
        thread_t *cur = get_current_thread();
        if (!cur || !cur->owner) return false;
        if (cur->owner->pid != 0) return false;
    }
    return false;
}