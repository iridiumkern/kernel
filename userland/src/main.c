#include <string.h>
#include <stdint.h>

// This is expected to be provided by every architectures libc impl
extern uint64_t syscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

void puts(const char* string) {
	syscall(0,0,(uint64_t)string, strlen(string), 0, 0);
}

__attribute__((noreturn)) void _start(void) {
	static const char *teststr = "Hello from PID0!\n";
	puts(teststr);

	for (;;)
		__asm__ volatile ("pause");
}