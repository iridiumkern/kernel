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

    while (entry != NULL) {
        if (!madt_entry_valid(madt, entry)) {
            kpanic("MADT entry is invalid!\n");
        }

        if (entry->type == MADT_TYPE_LOCAL_APIC) {
            struct madt_local_apic *lapic = (struct madt_local_apic *)entry;

            if (lapic->apic_id == bsp_apic_id) {
                printf("Found BSP lapic, CPU %x!\n", bsp_apic_id);
                if (madt->lapicaddr == 0) {
                    kpanic("madt->lapicaddr == 0");
                } else {
                    printf("madt->lapicaddr == %llx\n", madt->lapicaddr);
                }
                uint64_t page = vmm_find_free_pages(1, true);
                vmm_map(page, madt->lapicaddr, VMM_P | VMM_RW);

                lapic_init(page);
            }
        } else if (entry->type == MADT_TYPE_ISO) {
            struct madt_iso *iso = (struct madt_iso*)entry;
            printf("ISO Info Dump:\n");
            printf("\tBus: %x\n", iso->bus);
            printf("\tSource: %x\n", iso->source);
            printf("\tGSI: %lx\n", iso->gsi);
            printf("\tflags: %x\n", iso->flags);
        }

        // Find the next entry
        entry = madt_next(madt, entry);
    }
    
    return ACPI_OK;
}