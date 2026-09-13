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

#include "lib/flanterm/src/flanterm_backends/fb.h"
#include <acpi/sdp.h>
#include <stdlib.h>
#include <kernel.h>
#include <limine.h>
#include <stdio.h>

#include <sec/random.h>
#include <scheduler.h>
#include <string.h>
#include <debug.h>
#include <panic.h>
#include <pmm.h>
#include <tar.h>

#ifdef __x86_64__
#include <x86_64/schedarch.h>
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
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
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
extern void sspsetup(void);
extern uintptr_t tar_init(uintptr_t address);
extern void jump_usermode(uint64_t ip, uint64_t sp);

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
    
    kinit();
    pmm_init();
    #ifdef __x86_64__
    vmm_init();
    #endif

    kheap_init();
    
    if (!csprng_init()) {
        kpanic("CSPRNG_INIT FAILED!\n");
    } else {
        printf("csprng_init returned!\n");
    }
    sspsetup();

    parse_acpi();

    // Load userland.tar
    if (module_request.response->module_count != 1) {
        // Kernel should only have one module, a tar file
        kpanic("Passed more than one file! Only file should be userland.tar!\n");
    }
    tar_init((uintptr_t)(module_request.response->modules[0]->address));
    struct tar_wrapper *usr = tar_getfile("userland/test.bin");
    uint8_t *data = (uint8_t*)usr->address;
    #ifdef __x86_64__
    uint64_t phys = pmm_alloc();
    vmm_map(0x10000, phys, VMM_P | VMM_US | VMM_RW);
    memcpy((void*)0x10000, data, 2);
    hexdump((void*)0x10000, 2);

    // Create the new proc (in this case it should be PID0)
    process_t *proc = addproc();
    if (proc->pid != 0) {
        kpanic("First allocated process did not have PID0! PID was: %d\n", proc->pid);
    }
    thread_t *thrd = addthrd(proc->pid);
    thrd->archdata = kmalloc(sizeof(regs_thread_state_t));
    thrd->instruction_ptr = 0x10000; 
    thrd->stack_ptr = 0x10000 + 4096;
    thrd->state = THREAD_RUNNING;

    jump_usermode(thrd->instruction_ptr, thrd->stack_ptr);
    #endif
    hcf();
}