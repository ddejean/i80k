// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "clock.h"

#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "debug.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"

// The clock counter defines how many periods the counter has to wait before
// firing and interrupt. The PIT clock frequency is 1.25Mhz and we want it to
// fire every 20 ms.
#define CLOCK_COUNTER (PIT_FREQ / 50)
// The clock increment is the count of ms spent between each interrupt.
#define CLOCK_INC_MS 20

// Kernel ticks.
unsigned long ticks;

// Interruption request handler.
void clock_int_handler(void);

void clock_initialize(void) {
    printk("Clock: frequency: %luHz, period: %dms, using IRQ0\r\n",
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
