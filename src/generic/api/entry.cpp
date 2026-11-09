#include <sec/memory.hpp>
#include <sec/random.hpp>
#include <scheduler.hpp>
#include <panic.hpp>
#include <stdint.h>
#include <vfs.hpp>

using namespace security::memory;

#ifdef __x86_64__
#define CPUID_7_EBX_SMAP (1U << 20)

static inline bool cpu_has_smap(void) {
	uint32_t eax, ebx, ecx, edx;

	__asm("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(7), "c"(0));

	return (ebx & CPUID_7_EBX_SMAP) != 0;
}
#endif

static inline void usermemoryacc(void) {
	#ifdef __x86_64__
	if (cpu_has_smap()) __asm("stac" ::: "memory");
	#endif
}

static inline void usermemorylock(void) {
	#ifdef __x86_64__
	if (cpu_has_smap()) __asm("clac" ::: "memory");
	#endif
}

extern "C" uint64_t apisyscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
	uint64_t ret = 0;
	(void)arg6;
	// I/O
	if (arg1 == 0) {
		if (arg2 == 0) {
			// Arg3 is the string to open
			// Arg4 is the Filesystem ID
			usermemoryacc();
			ret = vfs::open(&ret, (const char*)arg3, arg4);
			usermemorylock();
		} else if (arg2 == 1) {
			// Arg3 is the file descriptor
			// Arg4 is the buffer
			// Arg5 is the buffer length
			usermemoryacc();
			ret = vfs::read(arg3, (void*)arg4, arg5);
			usermemorylock();
		} else if (arg2 == 2) {
			// Arg3 is the file descriptor
			// Arg4 is the buffer
			// Arg5 is the buffer length
			usermemoryacc();
			ret = vfs::write(arg3, (void*)arg4, arg5, true);
			usermemorylock();	
		} else if (arg2 == 3) {
			// Closes a file descriptor
			ret = vfs::close(arg3);
		}
	}

	return ret;
}
