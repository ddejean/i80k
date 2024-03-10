// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "timer.h"

// Count of iterations used in the delay calibration loop.
#define CLOCK_CAL_LOOPS 20000

// Average ns spent per loop iteration in __delay().
unsigned long ns_per_loop;

// __delay iterates <loops> time doing nothing.
void __delay(unsigned long loops) {
    volatile unsigned long count = loops;
    do {
        count--;
    } while (count > 0);
}

void delay_calibrate(struct timer *t) {
    unsigned int val;

    // Use the PIT to mesure the time elapsed in the iteration loop.
    timer_set_alarm(t, 65535);
    __delay(CLOCK_CAL_LOOPS);
    val = timer_read(t);

    // Compute the ns spent in one loop iteration using the number of ns per
    // timer tick.
    ns_per_loop =
        ((unsigned long)(65535 - val) * timer_ns_per_tick(t)) / CLOCK_CAL_LOOPS;
}

void udelay(unsigned long delay) {
    unsigned long loops = (delay * 1000) / ns_per_loop;
    if (!loops) {
        __delay(1);
        return;
    }
    __delay(loops);
}

void mdelay(unsigned long delay) {
    while (delay--) {
        __delay(1000000);
    }
}