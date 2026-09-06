#include <x86_64/apic.h>
#include <scheduler.h>
#include <stddef.h>
#include <stdint.h>

static uint64_t ticks = 0;

typedef struct lapic_timer_frame {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;

    uint64_t rsp;
    uint64_t ss;
} lapic_timer_frame_t;

void lapic_timer_drv(lapic_timer_frame_t *frame) {
    ticks++;
    // Check if we are in userland or not.
    if (frame->cs & 0x03) {
        thread_t *current = get_current_thread();
        if (current == NULL) {
            lapic_eoi();
            return;
        }
    }
    lapic_eoi();
}