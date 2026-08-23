#include <stdio.h>

extern int gdt_init(void);
extern void idt_init();

void kinit(void) {
    printf("kinit called.\n");
    gdt_init();
    printf("GDT: setup\n");
    idt_init();
    printf("IDT: setup\n");
}