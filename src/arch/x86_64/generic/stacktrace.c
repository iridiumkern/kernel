#include <sec/memory.h>
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
	for (unsigned int frame = 0; stk && frame < MaxFrames; ++frame) {
		uintptr_t addr = (uintptr_t)stk;
	
		if ((addr & (sizeof(uintptr_t) - 1)) != 0 ||
			!range_is_mapped(addr, sizeof(struct stackframe))) {
			printf("	<invalid frame at 0x%llx>\n",
				   (unsigned long long)addr);
			break;
		}
	
		struct stackframe *next = stk->rbp;
		printf("	0x%llx\n", (unsigned long long)stk->rip);
	
		if (next && (uintptr_t)next <= addr)
			break;
	
		stk = next;
	}
}