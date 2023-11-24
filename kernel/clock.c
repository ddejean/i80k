// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "clock.h"

#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"

// Count of iterations used in the delay calibration loop.
#define CLOCK_CAL_LOOPS 16384

// Average ns spent per loop iteration in __delay().
unsigned long ns_per_loop;

// Kernel ticks. Marked as volatile to prevent the kernel from doing any
// optimizations on it.
volatile unsigned long ticks;

// Interruption request handler.
void clock_int_handler(void);

// __delay iterates <loops> time doing nothing.
void __delay(unsigned long loops) {
    volatile unsigned long count = loops;
    do {
        count--;
    } while (count > 0);
}

static unsigned long clock_calibrate(void) {
    unsigned int val;

    // Use the PIT to mesure the time elapsed in the iteration loop.
    pit_set_alarm(PIT_TIMER0, 65535);
    __delay(CLOCK_CAL_LOOPS);
    val = pit_read(PIT_TIMER0);

    // Compute the ns spent in one loop iteration using the number of ns per
    // timer tick.
    return ((unsigned long)(65535 - val) * pit_ns_per_tick()) / CLOCK_CAL_LOOPS;
}

void clock_initialize(void) {
    // Calibrate the delay loop to have an effective implementation of
    // mdelay/udelay.
    ns_per_loop = clock_calibrate();
    printf("Clock: calibration loop measured %lu ns per iteration.\n",
           ns_per_loop);

    // Prepare the system to regularly count.
    ticks = 0;
    interrupts_handle(INT_IRQ0, clock_int_handler);
    pit_set_alarm(PIT_TIMER0, CLOCK_COUNTER);
    irq_enable(MASK_IRQ0);

    printf("Clock: frequency: %luHz, period: %dms, using IRQ0\n",
           (long unsigned int)PIT_FREQ, CLOCK_INC_MS);
}

unsigned long clock_now(void) {
    unsigned long now;
    // Atomic clock read.
    cli();
    now = ticks;
    sti();
    return now * CLOCK_INC_MS;
}

int clock_wait(unsigned long delay, wait_t type) {
    unsigned long due_time;
    due_time = clock_now() + delay;

    switch (type) {
        case POLL_WAIT:
            while (due_time > clock_now()) {
                hlt();
            }
            return 0;

        case BLOCK_WAIT:
        default:
            return -1;
    }
}
