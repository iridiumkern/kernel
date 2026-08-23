#pragma once

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("out %w1, %b0" : : "a"(val), "Nd"(port) : "memory");
}

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