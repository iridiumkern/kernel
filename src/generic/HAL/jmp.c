#ifdef __x86_64__
#include <x86_64/priv/jmp.h>
#endif
#include <stdlib.h>

void* allocjmp(void) {
    #ifdef __x86_64__
    return kmalloc(sizeof(jmpctx_t));
    #endif
}

int setjmp(void* ctx) {
    #ifdef __x86_64__
    return ksetjmp(ctx);
    #endif
}

__attribute__((noreturn)) void longjmp(void* ctx, int value) {
    #ifdef __x86_64__
    klongjmp(ctx, value);
    #endif
}