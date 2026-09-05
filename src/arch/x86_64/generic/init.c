#include <stdio.h>

extern int gdt_init(void);
extern void idt_init();
extern void PIC_disable(void);
extern int init_serial();

void kinit(void) {
    printf("kinit called.\n");
    gdt_init();
    printf("GDT: setup\n");
    idt_init();
    printf("IDT: setup\n");

    PIC_disable();

    init_serial();
}