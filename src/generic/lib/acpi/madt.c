#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <acpi/sdt.h>
#include <acpi/madt.h>

struct madt_entry *madt_next(struct madt *madt, struct madt_entry *entry) {
    uint8_t *start = (uint8_t *)madt + sizeof(struct madt);
    uint8_t *end   = (uint8_t *)madt + madt->h.Length;

    if (entry == NULL)
        return (struct madt_entry *)start;

    if (entry->length < sizeof(struct madt_entry))
        return NULL;

    uint8_t *next = (uint8_t *)entry + entry->length;

    if (next + sizeof(struct madt_entry) > end)
        return NULL;

    return (struct madt_entry *)next;
}

bool madt_entry_valid(struct madt *madt, struct madt_entry *entry) {
    uint8_t *start = (uint8_t *)madt + sizeof(struct madt);
    uint8_t *end   = (uint8_t *)madt + madt->h.Length;
    uint8_t *ptr   = (uint8_t *)entry;

    if (ptr < start)
        return false;

    if (ptr + sizeof(struct madt_entry) > end)
        return false;

    if (entry->length < sizeof(struct madt_entry))
        return false;

    if (ptr + entry->length > end)
        return false;

    return true;
}