#include <stdio.h>
#include <panic.h>
#include <acpi/facp.h>
#include <acpi/sdt.h>
#include <kernel.h>

void parse_facp(void* facpptr) {
    if (!facpptr) {
        kpanic("FACP pointer is NULL!\nPTR = %llx\n", facpptr);
    }
    struct FADT *fadt = facpptr;

    if (fadt->h.Revision == 2) {
        printf("DSDT at: %llx\n", krnl.hhdm_offset + fadt->X_Dsdt);
    } else if (fadt->h.Revision == 0 || fadt->h.Revision == 1) {
        printf("DSDT at: %llx\n", krnl.hhdm_offset + fadt->Dsdt);
    }

    printf("PreferredPowerManagementProfile: ");
    switch (fadt->PreferredPowerManagementProfile) {
        case 0:
            printf("Unspecified\n");
            break;
        case 1:
            printf("Desktop\n");
            break;
        case 2:
            printf("Mobile\n");
            break;
        case 3:
            printf("Workstation\n");
            break;
        case 4:
            printf("Enterprise Server\n");
            break;
        case 5:
            printf("SOHO Server\n");
            break;
        case 6:
            printf("Aplliance PC\n");
            break;
        case 7:
            printf("Performance Server\n");
            break;
        default:
            kpanic("PreferredPowerManagementProfile = %x\nInvalid behavior!\n", fadt->PreferredPowerManagementProfile);
    }
}