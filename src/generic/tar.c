#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <panic.h>
#include <tar.h>

struct tar_wrapper *rootentry = NULL;

uintptr_t getsize(const char *in) {
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;
}

uintptr_t tar_init(uintptr_t address) {
    unsigned int i;
    rootentry = kmalloc(sizeof(struct tar_wrapper));

    if (rootentry == NULL) {
        kpanic("KMALLOC OOM!\n");
    }

    struct tar_wrapper *next = rootentry;

    for (i = 0; ; i++) {
        memcpy(&next->h, (const void *)address, 512);

        if (next->h.filename[0] == '\0')
            break;

        uintptr_t size = getsize(next->h.size);
        uintptr_t blocks = (size + 511) / 512;

        // Sets next->address (where the data is)
        next->address = address + 512;
        
        address += 512 + blocks * 512;
        next->next = kmalloc(sizeof(struct tar_wrapper));

        if (next->next == NULL) break;

        next = next->next;
    }

    next->next = NULL;
    return i;
}

struct tar_wrapper *tar_getfile(const char* name) {
    struct tar_wrapper *next = rootentry;
    while (next != NULL) {
        if (memcmp(name, next->h.filename, strlen(name)) == 0) {
            return next;
        }
        next = next->next;
    }

    return NULL;
}