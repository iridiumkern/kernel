#include <sec/memory.hpp>
#include <sec/random.hpp>
#include <scheduler.hpp>
#include <panic.hpp>
#include <debug.hpp>
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

extern "C" bool apisyscall(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
	bool ret = false;
	(void)arg6;
	// Output
	if (arg1 == 0) {
		if (arg2 == 0) {
			// Write to VFS
			// Arg3 is FD
			// Arg4 is buffer pointer
			// Arg5 is size
			// User is set to true to make sure its safe
			usermemoryacc();
			ret = vfs::write(arg3, (void*)arg4, arg5, true);
			usermemorylock();
		}
	}
	return ret;
}
