#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "lib/flanterm/src/flanterm.h"
#include "lib/flanterm/src/flanterm_backends/fb.h"

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

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] =
    LIMINE_REQUESTS_END_MARKER;

static void hcf(void) {
    for (;;) {
    }
}

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

    struct flanterm_context *ctx = flanterm_fb_init(
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

    if (ctx == NULL) {
        hcf();
    }

    flanterm_clear(ctx, true);

    flanterm_write(ctx, "Iridium", 8);

    hcf();
}