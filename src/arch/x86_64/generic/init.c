extern int gdt_init(void);
extern void idt_init();
extern void PIC_disable(void);
extern int init_serial();

void kinit(void) {
    gdt_init();
    PIC_disable();
    idt_init();

    init_serial();
}