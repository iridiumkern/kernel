/**
 * @file init.c
 * @author apixeldev
 * @brief Contains the code to init x86_64 systems
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdio.h>

extern int gdt_init(void);
extern void idt_init();
extern void PIC_disable(void);
extern int init_serial();

/**
 * @brief Sets up x86_64 specific hardware
 * 
 */
void kinit(void) {
    printf("kinit called.\n");
    gdt_init();
    printf("GDT: setup\n");
    idt_init();
    printf("IDT: setup\n");

    PIC_disable();

    init_serial();
}