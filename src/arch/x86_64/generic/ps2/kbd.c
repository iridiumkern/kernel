#include <stdbool.h>
#include <stdio.h>
#include <x86_64/io.h>

extern void lapic_eoi(void);

/**
 * @brief A simple PS2 keyboard driver, improvements soon!
 * 
 */
void ps2_kbd_drv(void) {
    uint8_t scancode = inb(0x60);
    bool released = scancode & 0x80;
    if (released) {
        lapic_eoi();
        return;
    }
    printf("SCANCODE: %x\n", scancode);
    lapic_eoi();
}