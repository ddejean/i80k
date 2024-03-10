// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "p8254.h"

#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "devices.h"

#define PIT_COUNTER(dev, t) (dev->port | (uint8_t)t)
#define PIT_MODE(dev) (dev->port | 3)

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

static inline void p8254_configure_counter(struct timer *t, uint8_t func,
                                           uint16_t value) {
    uint8_t mode, port;

    mode = MODE_COUNTER(t->index) | func | MODE_BOTH_BYTES | MODE_16BITS;
    port = PIT_COUNTER(t, t->index);

    // Configure the counter.
    outb(PIT_MODE(t), mode);
    outb(port, value & 0xff);
    outb(port, value >> 8);
}

void p8254_set_alarm(struct timer *t, uint16_t counter) {
    // Configure the counter to fire an alarm regularly.
    p8254_configure_counter(t, MODE_RATE_GEN, counter);
}

void p8254_set_freq(struct timer *t, uint32_t freq) {
    uint16_t divider = (uint16_t)(t->freq / freq);
    // Configure the counter to generate a square wave with a frequency of
    // PIT_FREQ/divider.
    p8254_configure_counter(t, MODE_SQUARE_WAVE, divider);
}

uint16_t p8254_read(struct timer *t) {
    uint8_t mode, port, low, high;
    mode = MODE_COUNTER(t->index) | MODE_LATCH;
    port = PIT_COUNTER(t, t->index);

    outb(PIT_MODE(t), mode);
    low = inb(port);
    high = inb(port);
    return high << 8 | low;
}

uint32_t p8254_ns_per_tick(struct timer *t) { return 1000000000ul / t->freq; }
