#pragma once

void* allocjmp(void);
int setjmp(void** nctx);
__attribute__((noreturn)) void longjmp(void* ctx, int value);