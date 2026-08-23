#include <stdio.h>
#include <stddef.h>
#include <acpi/types.h>
#include <acpi/madt.h>
#include <panic.h>
#include <stdint.h>

acpi_ret madt_parse(struct madt* madt) {
    if (!madt) {
        kpanic("MADT is set to NULL\n");
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