#include <x86_64/cpuid.h>
#include <stdbool.h>
#include <stdint.h>

#define CR4_SMEP (1ULL << 20)
#define CR4_SMAP (1ULL << 21)

extern void gdt_init(uint64_t kernel_stack_top);
extern void idt_init(void);
extern void PIC_disable(void);
extern int init_serial(void);

static uint8_t kernel_stack[16384];

static inline uint64_t read_cr4(void) {
	uint64_t value;

	__asm__ volatile (
		"mov %0, cr4"
		: "=r"(value)
	);

	return value;
}

static inline void write_cr4(uint64_t value) {
	__asm__ volatile (
		"mov cr4, %0"
		:
		: "r"(value)
		: "memory"
	);
}

void kinit(void) {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;

	gdt_init((uint64_t)(kernel_stack + sizeof(kernel_stack)));

	PIC_disable();
	idt_init();

	init_serial();

	cpuid(0, &eax, &ebx, &ecx, &edx);

	uint32_t max_leaf = eax;

	if (max_leaf < 7)
		return;

	cpuid(7, &eax, &ebx, &ecx, &edx);

	uint64_t cr4 = read_cr4();

	if (ebx & (1U << 7)) cr4 |= CR4_SMEP;

	if (ebx & (1U << 20)) cr4 |= CR4_SMAP;

	write_cr4(cr4);
}