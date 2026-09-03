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

/**
 * @brief Checks if the checksum of an RSDP/XSDP is valid to its considered length
 * 
 * @param ptr The ptr
 * @param len Length
 * @return int 1 if invalid, 0 if valid
 */
static int checksum_valid(const void *ptr, size_t len) {
    const uint8_t *bytes = (const uint8_t *)ptr;
    uint8_t sum = 0;

    for (size_t i = 0; i < len; i++) {
        sum += bytes[i]; // uint8_t wraps on overflow, which is what we want
    }

    return sum == 0;
}

/**
 * @brief Checks if a SDP is valid
 * 
 * @param rsdp The SDP is question (RSDP/XSDP)
 * @return true Valid
 * @return false Invalid
 */
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

/**
 * @brief Checksum on a SDT header
 * 
 * @param tableHeader The table header
 * @return true Valid
 * @return false Invalid
 */
bool doChecksum(struct SDT_header *tableHeader) {
    unsigned char sum = 0;

    for (uint32_t i = 0; i < tableHeader->Length; i++) {
        sum += ((char *) tableHeader)[i];
    }

    return sum == 0;
}

/**
 * @brief Finds an entry in the SDT
 * 
 * @param name The 4 character name of the ACPI table
 * @param RootSDT The SDT (RSDT/XSDT)
 * @return void* The pointer to the table (NULL if invalid)
 */
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

    return NULL;
}

/**
 * @brief Parses the ACPI headers
 * 
 * @return acpi_ret If properly initialized
 */
acpi_ret parse_acpi(void) {
    // Grab the RSDP
    if (rsdp_request.response == NULL) {
        return ACPI_MISSING;
    }

    // Grab the pointer to the RSDP/XSDP
    printf("RSDP pointer: %llx\n", rsdp_request.response->address);
    if (sdp_valid(rsdp_request.response->address) == false) {
        return ACPI_ERROR;
    }
    
    spd_pointer = rsdp_request.response->address;

    if (((struct RSDP_t*)rsdp_request.response->address)->Revision == 0) {
        // ACPI 1.0
        struct RSDP_t *rsdp = rsdp_request.response->address;
        struct RSDT_t *rsdt = (struct RSDT_t*)(uintmax_t)(rsdp->RsdtAddress + krnl.hhdm_offset);
        krnl.acpi2 = false;

        if (!doChecksum(&rsdt->h)) {
            kpanic("RSDT invalid!\n");
        }

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
        if (!facp) {
            // FACP should be present, if not either the kernel is broken or something else isnt working
            kpanic("FACP is missing or equal to NULL!\n");
        }
        parse_facp(facp);

        // Find (and init) the MADT
        void* madt = findEntry("APIC", rsdt);
        madt_parse(madt);
    } else if (((struct RSDP_t*)rsdp_request.response->address)->Revision == 2) {
        // ACPI 2.0 or above
        struct XSDP_t *xsdp = rsdp_request.response->address;
        struct XSDT_t *xsdt = (struct XSDT_t*)(uintmax_t)(xsdp->XsdtAddress + krnl.hhdm_offset);
        printf("ACPI 2.0+ system\n");
        krnl.acpi2 = true;

        if (!doChecksum(&xsdt->h)) {
            kpanic("XSDT invalid!\n");
        }

        printf("RSDT at: %llx\n", xsdt);
        printf("OEMID: ");
        putchar_ft(xsdp->OEMID[0]);
        putchar_ft(xsdp->OEMID[1]);
        putchar_ft(xsdp->OEMID[2]);
        putchar_ft(xsdp->OEMID[3]);
        putchar_ft(xsdp->OEMID[4]);
        putchar_ft(xsdp->OEMID[5]);
        putchar_ft('\n');

        // Find FACP.
        void* facp = findEntry("FACP", xsdt);
        parse_facp(facp);

        // Find (and init) the MADT
        void* madt = findEntry("APIC", xsdt);
        madt_parse(madt);
    } else {
        kpanic("ACPI reports as version %llx\nOnly supported versions are 0 and 2.\n", ((struct RSDP_t*)rsdp_request.response->address)->Revision);
    }

    return ACPI_OK;
}