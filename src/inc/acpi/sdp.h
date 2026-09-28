#pragma once

#include <stdint.h>
#include <acpi/types.h>
#include <acpi/sdt.h>
#include <stdbool.h>

/**
 * @brief RSDP for older ACPI based devices (rare to find)
 * 
 */
struct RSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));

/**
 * @brief XSDP for most modern ACPI devices
 * 
 */
struct XSDP_t {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;      // deprecated since version 2.0

    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

/**
 * @brief Parses ACPI
 * 
 * @return acpi_ret The ACPI parser
 */
acpi_ret parse_acpi(void);

/**
 * @brief Checks a table
 * 
 * @param tableHeader The SDT header
 * @return true Is valid
 * @return false Is not valid
 */
bool doChecksum(struct SDT_header *tableHeader);