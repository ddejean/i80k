// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "clock.h"

#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"

// Kernel ticks.
unsigned long ticks;

// Interruption request handler.
void clock_int_handler(void);

void clock_initialize(void) {
    printf("Clock: frequency: %luHz, period: %dms, using IRQ0\n",
           (long unsigned int)PIT_FREQ, CLOCK_INC_MS);

    ticks = 0;
    interrupts_handle(INT_IRQ0, clock_int_handler);
    pit_set_alarm(PIT_TIMER0, CLOCK_COUNTER);
    irq_enable(MASK_IRQ0);
}

void clock_handler(void) {
    irq_ack();
    ticks++;
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
