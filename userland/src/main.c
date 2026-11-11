#include <string.h>
#include <stdint.h>

// This is expected to be provided by every architectures libc impl
extern uint64_t syscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

void puts(const char* str) {
	syscall(0, 2, 0, (uint64_t)str, (uint64_t)strlen(str), 0);
}

__attribute__((noreturn)) void _start(void) {
	puts("Hello from PID0!\n");
	// Open file
	uint64_t fd = syscall(0, 0, (uint64_t)"kernelinfo/test.log", 0, 0, 0);
	char buf[512] = {'\0'};

	// Read file into buffer
	syscall(0, 1, fd, (uint64_t)buf, 512, 0);

	// Print out buffer
	puts(buf);

	// Zero the buffer
	memset(buf, 0, 512);

	// Write a new string into the buffer
	memcpy(buf, "Hello, this file is being written to!\n", 39);

	// Write the buffer to the VFS
	syscall(0, 2, fd, (uint64_t)buf, 512, 0);

	// Zero the buffer
	memset(buf, 0, 512);

	// Read the file into the buffer
	syscall(0, 1, fd, (uint64_t)buf, 512, 0);

	// Print out the buffer
	puts(buf);

	// Close the file
	syscall(0, 3, fd, 0, 0, 0);

	for (;;)
		__asm__ volatile ("pause");
}