// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "pit.h"

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "pit.h"

#define PIT_COUNTER(t) (PORT_PIT | (uint8_t)t)
#define PIT_MODE (PORT_PIT | 3)

#define MODE_16BITS 0
#define MODE_BCD 1
#define MODE_INT_TC (0 << 1)
#define MODE_ONE_SHOT (1 << 1)
#define MODE_RATE_GEN (2 << 1)
#define MODE_SQUARE_WAVE (3 << 1)
#define MODE_SOFT_TRIG (4 << 1)
#define MODE_HW_TRIG (5 << 1)
#define MODE_LATCH (0 << 4)
#define MODE_LEAST_SIG_BYTE (1 << 4)
#define MODE_MOST_SIG_BYTE (2 << 4)
#define MODE_BOTH_BYTES (3 << 4)
#define MODE_COUNTER(t) (((uint8_t)t) << 6)

static inline void pit_configure_counter(timer_t timer, uint8_t func,
                                         uint16_t value) {
    uint8_t mode, port;

    mode = MODE_COUNTER(timer) | func | MODE_BOTH_BYTES | MODE_16BITS;
    port = PIT_COUNTER(timer);

    // Configure the counter.
    outb(PIT_MODE, mode);
    outb(port, value & 0xff);
    outb(port, value >> 8);
}

void pit_set_alarm(timer_t timer, uint16_t counter) {
    // Configure the counter to fire an alarm regularly.
    pit_configure_counter(timer, MODE_RATE_GEN, counter);
}

void pit_freq_gen(timer_t timer, uint32_t freq) {
    uint16_t divider = (uint16_t)((uint32_t)PIT_FREQ / freq);
    // Configure the counter to generate a square wave with a frequency of
    // PIT_FREQ/divider.
    pit_configure_counter(timer, MODE_SQUARE_WAVE, divider);
}

uint16_t pit_read(timer_t timer) {
    uint8_t mode, port, low, high;
    mode = MODE_COUNTER(timer) | MODE_LATCH;
    port = PIT_COUNTER(timer);

    outb(PIT_MODE, mode);
    low = inb(port);
    high = inb(port);
    return high << 8 | low;
}

uint32_t pit_ns_per_tick(void) { return 1000000000ul / PIT_FREQ; }
