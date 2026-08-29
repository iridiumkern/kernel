/**
 * @file main.c
 * @author apixeldev
 * @brief Architecture independent entry point for the kernel
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdlib.h>
#include <kernel.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdio.h>
#include <acpi/sdp.h>
#include "lib/flanterm/src/flanterm.h"
#include "lib/flanterm/src/flanterm_backends/fb.h"

#include <pmm.h>
#include <jmp.h>

#ifdef __x86_64__
#include <x86_64/vmm.h>
#endif

#define VERSION_STRING MAJORVER "." MINORVER "-" GIT_HASH " (" GIT_BRANCH ")"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] =
    LIMINE_BASE_REVISION(6);

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] =
    LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST_ID,
    .revision = 0,
    .stack_size = 1048576 // 1 MiB
};

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] =
    LIMINE_REQUESTS_END_MARKER;

kernel_info_t krnl = {0};

/**
 * @brief Halts the CPU
 * 
 */
static void hcf(void) {
    printf("hcf(): called\n");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

struct flanterm_context *flantermctx = NULL;

// Sets up hardware
// This should be on all architectures.
extern void kinit(void);
extern void print_logo(void);

/**
 * @brief The entry for the kernel
 * 
 */
void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision)) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *framebuffer =
        framebuffer_request.response->framebuffers[0];

    flantermctx = flanterm_fb_init(
        NULL,
        NULL,

        (uint32_t *)framebuffer->address,
        framebuffer->width,
        framebuffer->height,
        framebuffer->pitch,

        framebuffer->red_mask_size,
        framebuffer->red_mask_shift,

        framebuffer->green_mask_size,
        framebuffer->green_mask_shift,

        framebuffer->blue_mask_size,
        framebuffer->blue_mask_shift,

        NULL, /* canvas */

        NULL, /* ANSI colours */
        NULL, /* ANSI bright colours */

        NULL, /* default background */
        NULL, /* default foreground */
        NULL, /* bright background */
        NULL, /* bright foreground */

        NULL, /* font */
        0,    /* font width */
        0,    /* font height */
        0,    /* font spacing */

        0,    /* font scale X */
        0,    /* font scale Y */

        0,    /* margin */

        FLANTERM_FB_ROTATE_0,

        true  /* autoflush */
    );

    if (flantermctx == NULL) {
        hcf();
    }

    flanterm_clear(flantermctx, true);

    krnl.hhdm_offset = hhdm_request.response->offset;

    print_logo();
    printf("Iridium %s\n", VERSION_STRING);
    printf("Iridium is brought to you under the GPLv3!\n");
    printf("For more information please read the LICENSE file shipped with this copy of the OS.\n");
    
    kinit();

    printf("kinit: returned\n");
    pmm_init();
    #ifdef __x86_64__
    vmm_init();
    #endif

    kheap_init();

    // Test setjmp and longjmp
    void* ctx = allocjmp();
    int value = 0;

    value = setjmp(ctx);

    if(value == 0) {
        longjmp(ctx, 1);
    } else {
        printf("CTX WAS SET TO %d\n", value);
    }
    kfree(ctx);

    parse_acpi();
    hcf();
}