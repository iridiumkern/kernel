#include <stdint.h>
#include <stdio.h>

struct stackframe {
	struct stackframe* rbp;
	uint64_t rip;
};

void TraceStackTrace(unsigned int MaxFrames) {
	struct stackframe *stk;
	__asm ("mov %0,rbp" : "=r"(stk) ::);
	printf("Stack trace:\n");
	for(unsigned int frame = 0; stk && frame < MaxFrames; ++frame) {
		// Unwind to previous stack frame
		printf("    0x%llx\n", stk->rip);
		stk = stk->rbp;
	}
}