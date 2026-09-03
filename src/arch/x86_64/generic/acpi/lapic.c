/**
 * @file lapic.c
 * @author apixeldev
 * @brief Sets up the lapic and lapic timer (calibrated with the PIT)
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <x86_64/io.h>
#include <stdio.h>
#include <stdint.h>

#define LAPIC_ID            0x020
#define LAPIC_VERSION       0x030
#define LAPIC_TPR           0x080
#define LAPIC_EOI           0x0B0
#define LAPIC_SVR           0x0F0

#define LAPIC_ESR           0x280
#define LAPIC_ICR_LOW       0x300
#define LAPIC_ICR_HIGH      0x310

#define LAPIC_LVT_TIMER     0x320
#define LAPIC_LVT_PERF      0x340
#define LAPIC_LVT_LINT0     0x350
#define LAPIC_LVT_LINT1     0x360
#define LAPIC_LVT_ERROR     0x370

#define LAPIC_TIMER_INIT    0x380
#define LAPIC_TIMER_CURRENT 0x390
#define LAPIC_TIMER_DIV     0x3E0

#define LAPIC_LDR           0x0D0
#define LAPIC_DFR           0x0E0

#define LAPIC_SVR_ENABLE    0x100
#define LAPIC_LVT_MASKED    0x10000
#define LAPIC_LVT_PERIODIC  0x20000

#define LAPIC_CPUFOCUS      0x200
#define LAPIC_NMI           (4 << 8)

#define PIT_CHANNEL0       0x40
#define PIT_COMMAND        0x43

#define PIT_FREQUENCY      1193182ULL
#define PIT_MODE_ONESHOT   0x00
#define PIT_ACCESS_LOHI    0x30

static volatile uint32_t *lapic;
static uint64_t pit_ticks;

/**
 * @brief Prepares how long the PIT should sleep for, calibration for the system timer
 * 
 * @param usec The amount of nanoseconds that it should sleep for
 */
void pit_prepare_sleep(uint32_t usec) {
    uint64_t ticks = (PIT_FREQUENCY * usec) / 1000000ULL;

    if (ticks == 0)
        ticks = 1;

    if (ticks > 65536)
        ticks = 65536;

    outb(PIT_COMMAND, 0x30);

    pit_ticks = (ticks == 65536) ? 0 : (uint16_t)ticks;

    outb(PIT_CHANNEL0, pit_ticks & 0xFF);
    outb(PIT_CHANNEL0, pit_ticks >> 8);
}

/**
 * @brief Performs the sleep that was prepared
 * 
 */
void pit_perform_sleep(void) {
    outb(PIT_COMMAND, 0xE2);

    while (!(inb(PIT_CHANNEL0) & 0x80))
        __asm__ volatile ("pause");
}

/**
 * @brief Reads from the lapic
 * 
 * @param reg The lapic register being read
 * @return uint32_t The value in the register
 */
static inline uint32_t lapic_read(uint32_t reg) {
    return lapic[reg / sizeof(uint32_t)];
}

/**
 * @brief Writes to an lapic register
 * 
 * @param reg The register being written to
 * @param value The value to write to the register
 */
static inline void lapic_write(uint32_t reg, uint32_t value) {
    lapic[reg / sizeof(uint32_t)] = value;
}

/**
 * @brief Calibrates and sets up the lapic timer
 * 
 */
void apic_start_timer(void) {
    lapic_write(LAPIC_TIMER_DIV, 0x3);

    lapic_write(LAPIC_TIMER_INIT, 0xFFFFFFFF);

    pit_prepare_sleep(1000);
    pit_perform_sleep();

    lapic_write(LAPIC_LVT_TIMER, LAPIC_LVT_MASKED);

    uint32_t ticks_in_1ms = 0xFFFFFFFF - lapic_read(LAPIC_TIMER_CURRENT);

    printf("LAPIC: %u ticks / 1ms\n", ticks_in_1ms);

    // Enable the timer on vector 0x20
    lapic_write(LAPIC_LVT_TIMER, 0x20 | LAPIC_LVT_PERIODIC);

    lapic_write(LAPIC_TIMER_DIV, 0x3);
    lapic_write(LAPIC_TIMER_INIT, ticks_in_1ms);
}

/**
 * @brief EOI for an lapic
 * 
 */
void lapic_eoi(void) {
    lapic_write(LAPIC_EOI, 0);
}

/**
 * @brief Sets up an lapic
 * 
 * @param lapic_virtual The virtual address of the lapic in question
 */
void lapic_init(uint64_t lapic_virtual) {
    lapic = (volatile uint32_t *)lapic_virtual;

    printf("LAPIC ID: %x\n", lapic_read(LAPIC_ID) >> 24);

    printf("LAPIC Version: %x\n", lapic_read(LAPIC_VERSION));

    lapic_write(LAPIC_TPR, 0);

    lapic_write(LAPIC_SVR, LAPIC_SVR_ENABLE | 0xFF);

    lapic_write(LAPIC_EOI, 0);

    apic_start_timer();

    printf("LAPIC: initialized\n");
}