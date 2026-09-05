#pragma once

#include <stdint.h>

typedef struct {
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
} jmpctx_t;

/**
 * @brief Sets up the CTX for klongjmp to use
 * 
 * @param ctx The pointer to the context
 * @return int The value of the return, 0 unless klongjmp was called
 */
int ksetjmp(jmpctx_t *ctx);

/**
 * @brief Jumps to where the context that klongjmp setup was
 * 
 * @param ctx The context to be used
 * @param value The value that should be returned to when ksetjmp was used.
 */
__attribute__((noreturn)) void klongjmp(jmpctx_t *ctx, int value);