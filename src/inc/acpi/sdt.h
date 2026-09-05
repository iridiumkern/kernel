#pragma once

#include <stdint.h>

/**
 * @brief Stander Table Header for any table other than the RSDP and XSDP
 * 
 */
struct SDT_header {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__ ((packed));

/**
 * @brief Contains pointers to all tables
 * 
 */
struct RSDT_t {
    struct SDT_header h;
    uint32_t PointerToOtherSDT[];
} __attribute__ ((packed));

/**
 * @brief Contains pointers to all tables
 * 
 */
struct XSDT_t {
    struct SDT_header h;
    uint64_t PointerToOtherSDT[];
}__attribute__ ((packed));