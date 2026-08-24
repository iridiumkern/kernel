#include <stdint.h>
#include <x86_64/apic.h>

static uint64_t ticks = 0;

void lapic_timer_drv(void) {
    ticks++;
    lapic_eoi();
}