/**
 * @file exception.c
 * @author apixeldev
 * @brief Handles exceptions for page faults and the like.
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>
#include "../../src/generic/lib/flanterm/src/flanterm.h"

extern struct flanterm_context *flantermctx;

struct interrupt_frame {
    uint64_t exception_code;
    uint64_t errorcode;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;

    // Priv change
    uint64_t rsp;
    uint64_t ss;
};

static inline uint64_t read_cr2(void) {
    uint64_t value;
    __asm__ volatile ("mov %0, cr2" : "=r"(value));
    return value;
}

/**
 * @brief Handles exceptions
 *
 */
__attribute__((noreturn))
void exception_handler(struct interrupt_frame *frame) {
    __asm__ volatile ("cli");

    flanterm_clear(flantermctx, true);

    printf("EXCEPTION!\n");
    if (frame->exception_code == 14) {
        printf("PAGE FAULT!\n");
        printf("CR2:       %lx\n", read_cr2());
    } else {
        printf("Exception code: %lx\n", frame->exception_code);
    }
    printf("Error code:    %lx\n", frame->errorcode);
    printf("RIP:           %lx\n", frame->rip);
    printf("CS:            %lx\n", frame->cs);
    printf("RFLAGS:        %lx\n", frame->rflags);
    printf("RSP:           %lx\n", frame->rsp);
    printf("SS:            %lx\n", frame->ss);

    printf("\nSystem halted.\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }

    __builtin_unreachable();
}