#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <acpi/sdt.h>
#include <acpi/types.h>

#define MADT_TYPE_LOCAL_APIC       0
#define MADT_TYPE_IO_APIC          1
#define MADT_TYPE_ISO              2
#define MADT_TYPE_NMI_SOURCE       3
#define MADT_TYPE_LOCAL_NMI        4
#define MADT_TYPE_LAPIC_OVERRIDE   5
#define MADT_TYPE_X2APIC           9

#define MADT_CPU_ENABLED           (1u << 0)
#define MADT_CPU_ONLINE_CAPABLE    (1u << 1)

#define MADT_POLARITY_MASK         0x3
#define MADT_TRIGGER_MASK          0xC

#define MADT_POLARITY_DEFAULT      0x0
#define MADT_POLARITY_HIGH         0x1
#define MADT_POLARITY_LOW          0x3

#define MADT_TRIGGER_DEFAULT       0x0
#define MADT_TRIGGER_EDGE          0x4
#define MADT_TRIGGER_LEVEL         0xC

struct madt {
    struct SDT_header h;

    uint32_t lapicaddr;
    uint32_t flags;
} __attribute__((packed));

struct madt_entry {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));

/* Type 0 */
struct madt_local_apic {
    uint8_t  type;
    uint8_t  length;

    uint8_t  processor_id;
    uint8_t  apic_id;
    uint32_t flags;
} __attribute__((packed));

/* Type 1 */
struct madt_io_apic {
    uint8_t  type;
    uint8_t  length;

    uint8_t  id;
    uint8_t  reserved;
    uint32_t address;
    uint32_t gsi_base;
} __attribute__((packed));

/* Type 2 */
struct madt_iso {
    uint8_t  type;
    uint8_t  length;

    uint8_t  bus;
    uint8_t  source;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed));

/* Type 3 */
struct madt_nmi_source {
    uint8_t  type;
    uint8_t  length;

    uint8_t  nmi_source;
    uint8_t  reserved;
    uint16_t flags;
    uint32_t gsi;
} __attribute__((packed));

/* Type 4 */
struct madt_local_nmi {
    uint8_t  type;
    uint8_t  length;

    uint8_t  processor_id;
    uint16_t flags;
    uint8_t  lint;
} __attribute__((packed));

/* Type 5 */
struct madt_lapic_override {
    uint8_t  type;
    uint8_t  length;

    uint16_t reserved;
    uint64_t address;
} __attribute__((packed));

/* Type 9 */
struct madt_x2apic {
    uint8_t  type;
    uint8_t  length;

    uint16_t reserved;
    uint32_t x2apic_id;
    uint32_t flags;
    uint32_t acpi_id;
} __attribute__((packed));

struct madt_entry *madt_next(struct madt *madt, struct madt_entry *entry);
bool madt_entry_valid(struct madt *madt, struct madt_entry *entry);

// To be implemented by each architecture.
// As these actions are for the most part architecture inherent.
acpi_ret madt_parse(struct madt* madt);