#include <stdint.h>

extern int gdt_init(uint64_t kernel_stack_top);
extern void idt_init();
extern void PIC_disable(void);
extern int init_serial();

static uint8_t kernel_stack[16384];

void kinit(void) {
    gdt_init((uint64_t)&kernel_stack[sizeof(kernel_stack)]);
    PIC_disable();
    idt_init();

    init_serial();
}