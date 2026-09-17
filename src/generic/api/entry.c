#include <panic.h>
#include <scheduler.h>
#include <debug.h>
#include <sec/random.h>
#include <stdint.h>
#include <stdio.h>

bool apisyscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    (void)arg3;
    (void)arg4;
    (void)arg5;
    (void)arg6;
    if (arg1 == 0) {
        printf("%s", arg2);
        return true;
    } else if (arg1 == 255) {
        // 0x40 is the PID0 only API
        thread_t *cur = get_current_thread();
        if (!cur || !cur->owner) return false;
        if (cur->owner->pid != 0) return false;

        if (arg2 == 0) {
            // Add entropy in arg3, 64bits of entropy
            printf("ADDING ENTROPY!\n");
            if (!csprng_addentropy(&arg3, 8)) return false;
        }
    }
    return false;
}