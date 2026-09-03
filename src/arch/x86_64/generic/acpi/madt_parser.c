/**
 * @file madt_parser.c
 * @author apixeldev
 * @brief Sets up the MADT (well more accurate to say that it parses it)
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <debug.h>
#include <x86_64/vmm.h>
#include <stdio.h>
#include <stddef.h>
#include <acpi/types.h>
#include <acpi/madt.h>
#include <panic.h>
#include <stdint.h>
#include <kernel.h>

extern void lapic_init(uint64_t lapic_virtual);
extern void ioapic_init(uint64_t ioapic_virtual, uint32_t gsi_base, uint8_t bsp_lapic_id);
extern void ioapic_register_iso(uint8_t source, uint32_t gsi);

/**
 * @brief Parses the MADT
 * 
 * @param madt The pointer to the MADT
 * @return acpi_ret The state of the madt parser
 */
acpi_ret madt_parse(struct madt* madt) {
    if (!madt) {
        // Used for when a feature and or table or other thing is not there/available
        return ACPI_MISSING;
    }

    uint32_t eax, ebx, ecx, edx;

    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );

    uint32_t bsp_apic_id = ebx >> 24;

    // Find the first MADT entry
    struct madt_entry *entry = madt_next(madt, NULL);

    // Parses all entries that the madt parser can find
    while (entry != NULL) {
        // If a MADT entry is corrupt we panic
        // Since if this basic ACPI table is corrupted it is very possible that other tables are broken
        if (!madt_entry_valid(madt, entry)) {
            kpanic("MADT entry is invalid!\n");
        }

        // Sets up an LAPIC
        if (entry->type == MADT_TYPE_LOCAL_APIC) {
            struct madt_local_apic *lapic = (struct madt_local_apic *)entry;

            // Sets up the BSP lapic
            if (lapic->apic_id == bsp_apic_id) {
                if (madt->lapicaddr == 0) {
                    kpanic("madt->lapicaddr == 0");
                }
                // Map the lapic address
                uint64_t page = vmm_find_free_pages(1, true);
                vmm_map(page, madt->lapicaddr, VMM_P | VMM_RW);

                lapic_init(page);
            }
        } else if (entry->type == MADT_TYPE_ISO) {
            // Registers an ISO
            struct madt_iso *iso = (struct madt_iso*)entry;
            ioapic_register_iso(iso->source, iso->gsi);
        } else if (entry->type == MADT_TYPE_IO_APIC) {
            struct madt_io_apic *ioapic = (struct madt_io_apic*)entry;
            
            // Maps the IOAPIC and sets it up
            uint64_t page = vmm_find_free_pages(1, true);
            vmm_map(page, ioapic->address, VMM_P | VMM_RW);
            ioapic_init(page, ioapic->gsi_base, bsp_apic_id);
        }

        // Find the next entry
        entry = madt_next(madt, entry);
    }
    
    return ACPI_OK;
}