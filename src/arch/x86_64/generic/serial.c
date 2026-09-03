/**
 * @file serial.c
 * @author apixeldev
 * @brief Sets up the PC serial port
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <x86_64/io.h>

#define PORT 0x3f8          // COM1

/**
 * @brief Sets up the serial port
 * 
 * @return int 1 if it fails, 0 if it works
 */
int init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if(inb(PORT + 0) != 0xAE) {
      return 1;
   }

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   outb(PORT + 4, 0x0F);
   return 0;
}

/**
 * @brief Checks if the serial port transmit is empty
 * 
 * @return int empty or not (1 is empty or true)
 */
int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

/**
 * @brief Writes a character to the serial port
 * 
 * @param a Character to write
 */
void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(PORT,a);
}
