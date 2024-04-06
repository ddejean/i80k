// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "kthread.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "cpu.h"
#include "error.h"
#include "scheduler.h"

// Helper to fill the initial stack.
struct bootstrap_stack {
    uint16_t flags;
    uint16_t es;
    uint16_t ds;
    uint16_t di;
    uint16_t si;
    uint16_t bp;
    void (*boostrap)(int (*fn)(void));
    uint16_t _ignored;
    int (*kthread_start)(void);
};

// Kernel idle task, does nothing except pause waiting for interrupts.
int _kernel_idle() {
    while (1) {
        hlt();
    }
    // This function is never supposed to exit.
    return 0;
}

void _kthread_bootstrap(int (*fn)(void)) {
    int status = fn();
    exit(status);
}

void kthread_initialize(void) {
    // Start the background thread.
    kthread_start(_kernel_idle, 510, -32767);
}

int kthread_start(int (*fn)(void), size_t sz, int prio) {
    struct task *new;
    struct bootstrap_stack *bst;

    if (sz < sizeof(struct bootstrap_stack)) {
        printf("kthread: stack too small to start a kernel thread (%u bytes)\n",
               sz);
        return ERR_INVAL;
    }

    uint8_t *stack = malloc(sz);
    if (!stack) {
        printf("kthread: failed to allocate the stack\n");
        return ERR_NO_MEM;
    }

    new = calloc(1, sizeof(*new));
    if (!new) {
        printf("kthread: failed to allocate task\n");
        return ERR_NO_MEM;
    }

    // Put bootstrap values in the bootstrap stack.
    bst = (struct bootstrap_stack *)(stack + sz - sizeof(*bst));
    bst->flags = INTERRUPT_ENABLE_FLAG;
    bst->es = KERNEL_DS;
    bst->ds = KERNEL_DS;
    bst->boostrap = _kthread_bootstrap;
    bst->kthread_start = fn;

    // Initialize the process structure.
    new->stack = stack;
    new->ctx.ss = KERNEL_SS;
    new->ctx.sp = bst;

    // Add the task to the ready list for later scheduling.
    return scheduler_queue_new(new, prio);
}
