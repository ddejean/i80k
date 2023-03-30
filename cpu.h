// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

// CPU flags
#define CARRY_FLAG (1 << 0)
#define PARITY_FLAG (1 << 2)
#define AUX_CARRY_FLAG (1 << 4)
#define ZERO_FLAG (1 << 6)
#define SIGN_FLAG (1 << 7)
#define TRAP_FLAG (1 << 8)
#define INTERRUPT_ENABLE_FLAG (1 << 9)
#define DIRECTION_FLAG (1 << 10)
#define OVERFLOW_FLAG (1 << 11)

// outb emits <data> on io address <port>.
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(data));
}

// inb retries a byte from io address <port>.
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__("inb %1, %0" : "=a"(data) : "dN"(port));
    return data;
}

// cli clears the interrupts flag which disables the interruptions.
static inline void cli(void) { __asm__ __volatile__("cli"); }

// sti sets the interrupts flag which enables the interruptions.
static inline void sti(void) { __asm__ __volatile__("sti"); }

// hlt pauses the processor until an interruption.
static inline void hlt(void) { __asm__ __volatile__("hlt"); }

#endif  // _CPU_H_
