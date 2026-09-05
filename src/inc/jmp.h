#pragma once

/**
 * @brief Allocates the ctx pointer
 * 
 * @return void* The allocated CTX pointer
 */
void* allocjmp(void);

/**
 * @brief Sets up the jmp
 * 
 * @param nctx The context
 * @return int The return value (should be 0 unless longjmp was called)
 */
int setjmp(void** nctx);

/**
 * @brief JMPs back to the RIP setup with setjmp
 * 
 * @param ctx The context setup by allocjmp and filled by setjmp earlier
 * @param value The value to return to the setjmp if check.
 */
__attribute__((noreturn)) void longjmp(void* ctx, int value);