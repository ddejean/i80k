// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>

#ifndef _TIMER_H_
#define _TIMER_H_

struct timer {
    // Timers I/O port.
    uint16_t port;
    // IRQ the timer is wired to.
    int irq;
    // Timer's index on the generator.
    int index;
    // Timer's base frequency.
    uint32_t freq;

    // Timer's implementation operations.
    uint32_t (*ns_per_tick)(struct timer *t);
    uint16_t (*read)(struct timer *t);
    void (*set_alarm)(struct timer *t, uint16_t count);
    void (*set_freq)(struct timer *t, uint32_t freq);
};

// timer_get returns the time named |name|.
struct timer *timer_get(const char *name);

// timer_ns_per_tick returns the duration in nanoseconds between two timer
// ticks.
uint32_t timer_ns_per_tick(struct timer *t);

// timer_read returns the current value of the timer.
uint16_t timer_read(struct timer *t);

// timer_set_alarm sets the timer to fire an interruption after |count| ticks.
void timer_set_alarm(struct timer *t, uint16_t count);

// timer_set_freq configures the timer to fire an interruption at frequency
// |freq|.
void timer_set_freq(struct timer *t, uint32_t freq);

#endif  // _TIMER_H_
