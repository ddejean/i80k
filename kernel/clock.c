// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "clock.h"

#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "devices.h"
#include "interrupts.h"
#include "timer.h"

// Kernel ticks. Marked as volatile to prevent the kernel from doing any
// optimizations on it.
volatile unsigned long ticks;

// Interruption request handler.
void clock_int_handler(void);

void clock_initialize(void) {
    struct timer *t = timer_get("timer0");
    if (!t) {
        printf("Clock: no timer found.\n");
        return;
    }

    // Prepare the system to regularly count.
    ticks = 0;
    interrupts_handle(interrupts_from_irq(t->irq), KERNEL_CS,
                      clock_int_handler);
    timer_set_alarm(t, t->freq / 100);
    irq_enable(t->irq);

    printf("Clock: frequency: %luHz, period: %dms, using IRQ %d\n", t->freq,
           CLOCK_INC_MS, t->irq);
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
