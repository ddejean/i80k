// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "clock.h"

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "devices.h"
#include "interrupts.h"
#include "timer.h"

// Device behind the clock.
static struct timer *t;

// Kernel ticks. Marked as volatile to prevent the kernel from doing any
// optimizations on it.
uint64_t ticks;

// Interruption request handler.
void clock_int_handler(void);

void clock_initialize(void) {
    t = timer_get("timer0");
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

void clock_handler(void) {
    ticks++;
    irq_ack(t->irq);
}

uint64_t clock_now(void) {
    uint64_t now;
    // Atomic clock read.
    cli();
    now = ticks;
    sti();
    return now * CLOCK_INC_MS;
}
