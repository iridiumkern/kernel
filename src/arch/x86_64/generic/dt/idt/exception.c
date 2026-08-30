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

static const char* decode_exception(uint64_t exception) {
    switch (exception) {
        case 0: return "DIVERR"; // Division error
        case 1: return "DBG"; // Debug
        case 2: return "NMI"; // NMI
        case 3: return "BRKPNT"; // Breakpoint
        case 4: return "OVRFLW"; // Overflow
        case 5: return "BNDRGEEXC"; // Bound Range Exceeded
        case 6: return "OPCODE"; // Invalid Opcode
        case 7: return "DEVNAVAIL"; // Device not available
        case 8: return "DBLFLT"; // Double Fault
        case 9: return "CPSGOV"; // Coprocessor Segment Overrun
        case 10: return "TSS"; // Invalid TSS
        case 11: return "SEGNP"; // Segment not present
        case 12: return "SSF"; // Stack Segment Fault
        case 13: return "GPF"; // General Protection fault
        case 14: return "PGF"; // Page Fault
        case 16: return "FPE"; // Floating Point Exception
        case 17: return "ALGCHK"; // Alignment Check
        case 18: return "MACHK"; // Machine Check
        case 19: return "SIMDFPE"; // SIMD Floating Point Exception
        case 20: return "VRTEXC"; // Virtualization Exception
        case 21: return "CTRLPE"; // Control Protection Exception
        case 28: return "HYPINJ"; // Hypervisor Injection
        case 29: return "VMMCOM"; // VMM Communication Exception
        case 30: return "SECURE"; // Security Exception
        default: return "RSRVD"; // Reserved
    }
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
    printf("Exception: %s (%lu)\n", decode_exception(frame->exception_code), frame->exception_code);
    printf("Error code:    %lu\n", frame->errorcode);
    printf("RIP:           0x%lx\n", frame->rip);
    printf("CS:            0x%lx\n", frame->cs);
    printf("RFLAGS:        0x%lx\n", frame->rflags);
    if (frame->cs & 0x03) {
        printf("RSP:           0x%lx\n", frame->rsp);
        printf("SS:            0x%lx\n", frame->ss);
    }
    if (frame->exception_code == 14) {
        printf("CR2:       0x%lx\n", read_cr2());
    }
    printf("\nSystem halted.\n");

    for (;;) {
        __asm__ volatile ("hlt");
    }

    __builtin_unreachable();
}