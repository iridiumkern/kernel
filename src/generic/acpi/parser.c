#include <acpi/madt.h>
#include <string.h>
#include <acpi/facp.h>
#include <acpi/sdt.h>
#include <stdbool.h>
#include <stdio.h>
#include <panic.h>
#include <stddef.h>
#include <kernel.h>
#include <acpi/sdp.h>
#include <limine.h>
#include <acpi/types.h>
#include <debug.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 6,
};

static int checksum_valid(const void *ptr, size_t len) {
    const uint8_t *bytes = (const uint8_t *)ptr;
    uint8_t sum = 0;

    for (size_t i = 0; i < len; i++) {
        sum += bytes[i]; // uint8_t wraps on overflow, which is what we want
    }

    return sum == 0;
}

bool sdp_valid(const struct RSDP_t *rsdp) {
    // First 20 bytes = the ACPI 1.0 portion of the structure (common to both)
    if (!checksum_valid(rsdp, sizeof(struct RSDP_t))) {
        return false;
    }

    if (rsdp->Revision >= 2) {
        const struct XSDP_t *xsdp = (const struct XSDP_t *)rsdp;
        if (!checksum_valid(xsdp, xsdp->Length)) {
            return false;
        }
    }

    return true;
}

void* spd_pointer = NULL;

bool doChecksum(struct SDT_header *tableHeader) {
    unsigned char sum = 0;

    for (uint32_t i = 0; i < tableHeader->Length; i++) {
        sum += ((char *) tableHeader)[i];
    }

    return sum == 0;
}

void *findEntry(const char* name, void *RootSDT) {
    if (krnl.acpi2) {
        struct XSDT_t *xsdt = (struct XSDT_t *) RootSDT;
        int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

        for (int i = 0; i < entries; i++) {
            struct SDT_header *h = (struct SDT_header *)(xsdt->PointerToOtherSDT[i] + krnl.hhdm_offset);
            if (!memcmp(h->Signature, name, 4))
                return (void *) h;
        }
    } else {
        // ACPI 1.0
        struct RSDT_t *rsdt = (struct RSDT_t *) RootSDT;
        int entries = (rsdt->h.Length - sizeof(rsdt->h)) / 4;

        for (int i = 0; i < entries; i++) {
            struct SDT_header *h = (struct SDT_header *)(uintmax_t)(rsdt->PointerToOtherSDT[i] + krnl.hhdm_offset);
            if (!memcmp(h->Signature, name, 4))
                return (void *) h;
        }
    }

    // No FACP found
    return NULL;
}

acpi_ret parse_acpi(void) {
    // Grab the RSDP
    if (rsdp_request.response == NULL) {
        kpanic("No RSDP/XSDP!\nPointer of RSDP request: %llx\n", rsdp_request.response);
    }

    // Grab the pointer to the RSDP
    printf("RSDP pointer: %llx\n", rsdp_request.response->address);
    if (sdp_valid(rsdp_request.response->address) == false) {
        kpanic("SDP corrupted\nSDP reported version: %llx\n", ((struct RSDP_t*)rsdp_request.response->address)->Revision);
    }
    
    spd_pointer = rsdp_request.response->address;

    if (((struct RSDP_t*)rsdp_request.response->address)->Revision == 0) {
        // ACPI 1.0
        struct RSDP_t *rsdp = rsdp_request.response->address;
        struct RSDT_t *rsdt = (struct RSDT_t*)(uintmax_t)(rsdp->RsdtAddress + krnl.hhdm_offset);
        krnl.acpi2 = false;

        printf("ACPI 1.0 system, some features may not be supported!\n");
        printf("RSDT at: %llx\n", rsdt);
        printf("OEMID: ");
        putchar_ft(rsdp->OEMID[0]);
        putchar_ft(rsdp->OEMID[1]);
        putchar_ft(rsdp->OEMID[2]);
        putchar_ft(rsdp->OEMID[3]);
        putchar_ft(rsdp->OEMID[4]);
        putchar_ft(rsdp->OEMID[5]);
        putchar_ft('\n');

        // Find FACP.
        // No need to check if the facp is null as the parser does for us, if so it panics.
        void* facp = findEntry("FACP", rsdt);
        parse_facp(facp);

        // Find (and init) the MADT
        void* madt = findEntry("APIC", rsdt);
        madt_parse(madt);
    } else if (((struct RSDP_t*)rsdp_request.response->address)->Revision == 2) {
        // ACPI 2.0 or above
        printf("ACPI 2.0+ system\n");
        krnl.acpi2 = true;
    } else {
        kpanic("ACPI reports as version %llx\nOnly supported versions are 0 and 2.\n", ((struct RSDP_t*)rsdp_request.response->address)->Revision);
    }

    return ACPI_OK;
}