/**
 * @file jmp.c
 * @author apixeldev
 * @brief Contains klongjmp and ksetjmp
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <x86_64/priv/jmp.h>

/*
This provides the structs needed for storing registers in klongjmp and ksetjmp
These are used for specific error handling when needed in noncritial parts of the kernel
For error recovery. Though in some cases they might need to run kpanic and will return an error code specifically for that
So the failing case is easier to find than the generic handler being used
*/

__attribute__((naked)) int ksetjmp(jmpctx_t *ctx) {
    __asm__ volatile (
        "mov QWORD PTR [rdi + 0],  rbx\n\t"
        "mov QWORD PTR [rdi + 8],  rbp\n\t"
        "mov QWORD PTR [rdi + 16], r12\n\t"
        "mov QWORD PTR [rdi + 24], r13\n\t"
        "mov QWORD PTR [rdi + 32], r14\n\t"
        "mov QWORD PTR [rdi + 40], r15\n\t"

        "mov QWORD PTR [rdi + 48], rsp\n\t"

        "mov rax, QWORD PTR [rsp]\n\t"
        "mov QWORD PTR [rdi + 56], rax\n\t"

        "xor eax, eax\n\t"
        "ret\n\t"
    );
}

__attribute__((naked)) __attribute__((noreturn)) void klongjmp(jmpctx_t *ctx, int value) {
    __asm__ volatile (
        "mov rbx, [rdi + 0]\n\t"
        "mov rbp, [rdi + 8]\n\t"
        "mov r12, [rdi + 16]\n\t"
        "mov r13, [rdi + 24]\n\t"
        "mov r14, [rdi + 32]\n\t"
        "mov r15, [rdi + 40]\n\t"

        /* Restore stack pointer */
        "mov rsp, [rdi + 48]\n\t"

        /* longjmp(value): 0 becomes 1 */
        "mov eax, esi\n\t"
        "test eax, eax\n\t"
        "jnz 1f\n\t"
        "mov eax, 1\n\t"

        /* Jump to saved RIP */
        "1:\n\t"
        "jmp [rdi + 56]\n\t"
    );
}