#pragma once

#include <stdint.h>

/**
 * @brief Output data to a port
 * 
 * @param port The port
 * @param val The data
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("out %w1, %b0" : : "a"(val), "Nd"(port) : "memory");
}

/**
 * @brief Get data from a port
 * 
 * @param port The port
 * @return uint8_t The data
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile (
        "in al, dx"
        : "=a"(ret)
        : "d"(port)
        : "memory"
    );
    return ret;
}