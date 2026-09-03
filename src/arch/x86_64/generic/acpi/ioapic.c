/**
 * @file ioapic.c
 * @author apixeldev
 * @brief A simple ioapic setup for x86_64
 * @version 0.1
 * @date 2026-09-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <x86_64/io.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Holds the ISO information (interrupt source override)
 * 
 */
typedef struct {
    uint8_t source;
    uint32_t gsi;
} ioapic_iso_t;

#define IOAPIC_REGSEL  0x00
#define IOAPIC_WINDOW  0x10

#define IOAPIC_ID      0x00
#define IOAPIC_VER     0x01
#define MAX_ISOS 32

static uintptr_t base = 0;
static uint32_t gsi_base = 0;

static ioapic_iso_t isos[MAX_ISOS];
static size_t iso_count = 0;

/**
 * @brief Reads from a register
 * 
 * @param reg The register
 * @return uint32_t The data/info stored in said register
 */
static inline uint32_t ioapic_read(uint8_t reg) {
    volatile uint32_t *regsel = (volatile uint32_t *)(base + IOAPIC_REGSEL);
    volatile uint32_t *window = (volatile uint32_t *)(base + IOAPIC_WINDOW);

    *regsel = reg;
    return *window;
}

/**
 * @brief Writes to a register
 * 
 * @param reg The register in question
 * @param value Value to write to the register
 */
static inline void ioapic_write(uint8_t reg, uint32_t value) {
    volatile uint32_t *regsel = (volatile uint32_t *)(base + IOAPIC_REGSEL);
    volatile uint32_t *window = (volatile uint32_t *)(base + IOAPIC_WINDOW);

    *regsel = reg;
    *window = value;
}

/**
 * @brief Registers an ISO
 * 
 * @param source An override
 * @param gsi The GSI
 */
void ioapic_register_iso(uint8_t source, uint32_t gsi) {
    if (iso_count >= MAX_ISOS)
        return;

    isos[iso_count].source = source;
    isos[iso_count].gsi = gsi;

    iso_count++;
}

/**
 * @brief Routes a GSI (for example GSI 1 would be the PS/2 keyboard)
 * 
 * @param gsi The GSI
 * @param vector The vector in the IDT
 * @param bsp_lapic_id The lapicid of the lapic it should be routed to (the BSP in our case though this is meant to be modular)
 */
void ioapic_route_gsi(uint32_t gsi, uint8_t vector, uint8_t bsp_lapic_id) {
    uint32_t index = gsi - gsi_base;

    uint8_t low_reg  = 0x10 + (index * 2);
    uint8_t high_reg = low_reg + 1;

    // Route to BSP LAPIC
    ioapic_write(high_reg, (uint32_t)bsp_lapic_id << 24);

    // Fixed delivery, physical destination, active-high,
    // edge-triggered, unmasked.
    ioapic_write(low_reg, vector);
}

/**
 * @brief Grabs the GSI of a source
 * 
 * @param source The source
 * @return uint32_t The GSI that should be used to map
 */
uint32_t ioapic_get_gsi(uint8_t source) {
    for (size_t i = 0; i < iso_count; i++) {
        if (isos[i].source == source)
            return isos[i].gsi;
    }

    // No override: ISA IRQ maps directly to the same GSI.
    return source;
}

/**
 * @brief Sets up an ioAPIC
 * 
 * @param ioapic_virtual The (mapped) ioapic address
 * @param gsi_base_glb The base GSI to setup
 * @param bsp_lapic_id The lapic of the BSP (or target processor for this ioapic)
 */
void ioapic_init(uint64_t ioapic_virtual, uint32_t gsi_base_glb, uint8_t bsp_lapic_id) {
    base = ioapic_virtual;
    gsi_base = gsi_base_glb;

    uint32_t version = ioapic_read(IOAPIC_VER);
    uint32_t max_redirection = (version >> 16) & 0xFF;
    uint32_t redirection_count = max_redirection + 1;

    for (uint32_t i = 0; i < redirection_count; i++) {
        uint8_t low_reg  = 0x10 + (i * 2);
        uint8_t high_reg = low_reg + 1;

        // Route to BSP LAPIC.
        ioapic_write(high_reg, (uint32_t)bsp_lapic_id << 24);

        // Mask all interrupts from ioapic
        ioapic_write(low_reg, 1u << 16);
    }

    // Maps in the PS/2 keyboard
    outb(0x64, 0xAA);

    while (!(inb(0x64) & 1));

    uint8_t result = inb(0x60);

    if (result == 0x55) {
        // 8042 exists
        ioapic_route_gsi(ioapic_get_gsi(1), 0x21, bsp_lapic_id);
    }
    (void)gsi_base;
}