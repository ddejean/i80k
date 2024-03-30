// Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

#include "clk.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "cpu.h"
#include "devices.h"
#include "error.h"
#include "interrupts.h"
#include "list.h"
#include "scheduler.h"
#include "timer.h"

#define S_TO_NS(v) ((v)*1000000000)
#define MS_TO_NS(v) ((v)*1000000)
#define NS_TO_S(v) ((v) / 1000000000)

// Device behind the clock.
static struct timer *t;

// Kernel ticks.
uint64_t ticks;
// Kernel time since boot.
uint64_t now_ns;

// List of blocked processes.
struct list_node blocked = LIST_INITIAL_VALUE(blocked);

// Interruption request handler.
void clk_int_handler(void);

void clk_initialize(void) {
    t = timer_get("timer0");
    if (!t) {
        printf("Clock: no timer found.\n");
        return;
    }

    // Prepare the system to regularly count.
    ticks = 0;
    now_ns = 0;
    interrupts_handle(interrupts_from_irq(t->irq), KERNEL_CS, clk_int_handler);
    timer_set_alarm(t, t->freq / 100);
    irq_enable(t->irq);

    printf("Clock: frequency: %luHz, period: %dms, using IRQ %d\n", t->freq,
           CLOCK_INC_MS, t->irq);
}

// Wake-up all the processes that have passed the deadline.
static void clk_wakeup(void) {
    if (list_is_empty(&blocked)) {
        return;
    }

    // For all the elements in the blocked list, unblock those who passed the
    // deadline.
    struct task *t, *tmp;
    list_for_every_entry_safe(&blocked, t, tmp, struct task, node) {
        uint64_t deadline = *(uint64_t *)t->wait_state;
        if (deadline <= now_ns) {
            list_delete(&t->node);
            scheduler_wake_up(t);
        }
    }
}

void clk_handler(void) {
    // Increment the clock ticks and ack the interrupt as quickly as possible.
    ticks++;
    now_ns += MS_TO_NS(CLOCK_INC_MS);
    irq_ack(t->irq);

    // Wake-up all the waiting processes that passed the deadline.
    clk_wakeup();

    if ((ticks & 0x1) == 0) {
        schedule();
    }
}

int clk_gettime(clockid_t clockid, struct timespec *tp) {
    if (clockid != CLOCK_MONOTONIC || !tp) {
        return ERR_INVAL;
    }
    tp->tv_sec = NS_TO_S(now_ns);
    tp->tv_nsec = now_ns % S_TO_NS(1);
    return 0;
}

int clk_nanosleep(clockid_t clockid, int flags, const struct timespec *request,
                  struct timespec *remain) {
    (void)flags;
    if (clockid != CLOCK_MONOTONIC || !request || !remain) {
        return ERR_INVAL;
    }

    uint64_t deadline;
    if (flags & TIMER_ABSTIME) {
        // |request| contains an absolute time.
        deadline = S_TO_NS(request->tv_sec) + request->tv_nsec;
    } else {
        // request contains a delay relative to current clock.
        deadline = now_ns + S_TO_NS(request->tv_sec) + request->tv_nsec;
    }

    // If the deadline is in the past, nothing to do.
    if (deadline <= now_ns) {
        return 0;
    }

    // Block until the deadline is reached.
    scheduler_sleep_on(&blocked, &deadline);

    // We'll be unblocked only after the deadline passed, no remain.
    if (remain) {
        memset(remain, 0, sizeof(*remain));
    }

    return 0;
}