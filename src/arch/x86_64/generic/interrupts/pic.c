#include <stdint.h>
#include <x86_64/io.h>

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_DATA	(PIC1+1)
#define PIC2_DATA	(PIC2+1)

/**
 * @brief Disables the PIC
 * 
 */
void PIC_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}