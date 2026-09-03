/**
 * @file hexdump.c
 * @author apixeldev
 * @brief Contains an xxd like function (hexdump)
 * @version 0.1
 * @date 2026-08-29
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/**
 * @brief XXD like formatted output
 * 
 * @param data The data being dumped
 * @param len The length of the data being dumped
 */
void hexdump(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;

    for (size_t i = 0; i < len; i += 16) {
        printf("%08X  ", (unsigned int)i);

        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02X ", p[i + j]);
            } else {
                printf("   ");
            }
            if (j == 7) printf(" ");
        }

        printf(" |");

        for (size_t j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = p[i + j];
            printf("%c", (c >= 0x20 && c <= 0x7e) ? c : '.');
        }

        printf("|\n");
    }
}