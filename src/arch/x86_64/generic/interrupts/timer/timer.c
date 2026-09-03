/**
 * @file timer.c
 * @author apixeldev
 * @brief Handler for the timer
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <x86_64/apic.h>

static uint64_t ticks = 0;

/**
 * @brief A simpler handler for an lapic timer
 * 
 */
void lapic_timer_drv(void) {
    ticks++;
    lapic_eoi();
}