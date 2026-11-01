#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct interrupt_frame {
	uint64_t exception_code;
	uint64_t errorcode;
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;

	// Priv change
	uint64_t rsp;
	uint64_t ss;
};

#ifdef __cplusplus
}
#endif