// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "scheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "board.h"
#include "cpu.h"
#include "ctx.h"
#include "error.h"
#include "list.h"

// Process state.
enum task_state {
    READY,
    RUNNING,
    WAITING,
    ZOMBIE,
};

// Represents a process.
struct task {
    // List node of the processes list.
    struct list_node node;

    // Process context.
    struct context ctx;
    // Process ID.
    unsigned int pid;
    // Process stack.
    uint16_t *stack;

    // Process state.
    enum task_state state;
    // Process return value.
    int status;
};

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

// Current running process.
struct task *current;

// Next available PID.
static unsigned int next_pid;

// List or ready processes.
struct list_node ready = LIST_INITIAL_VALUE(ready);

// List of killed processes.
struct list_node zombies = LIST_INITIAL_VALUE(zombies);

void scheduler_initialize() {
    current = calloc(1, sizeof(struct task));
    current->pid = next_pid++;
    current->state = RUNNING;
}

void schedule() {
    struct task *prev, *next;

    if (!current || list_is_empty(&ready)) {
        // Scheduler not initialized or the ready list is empty, nothing to do.
        return;
    }

    // Get the next process to run.
    next = list_remove_head_type(&ready, struct task, node);

    // Switch processes internally.
    prev = current;
    current = next;

    // If the current process is still running, queue it to the ready list for
    // future run.
    if (prev->state == RUNNING) {
        prev->state = READY;
        list_add_before(&ready, &prev->node);
    }

    // Switch to the next process.
    next->state = RUNNING;
    ctx_switch(&prev->ctx, &next->ctx);
}

void _kthread_bootstrap(int (*fn)(void)) {
    int status = fn();
    exit(status);
}

int scheduler_kthread_start(int (*fn)(void), size_t sz) {
    struct task *new;
    struct bootstrap_stack *bst;

    new = calloc(1, sizeof(*new));
    if (!new) {
        printf("scheduler: failed to allocate task\n");
        return ERR_NO_MEM;
    }

    // 2K stack seems to be a minimum.
    new->stack = calloc(sz, sizeof(uint8_t));
    if (!new->stack) {
        printf("scheduler: failed to allocate %u bytes stack\n", sz);
        free(new);
        return ERR_NO_MEM;
    }

    // Put bootstrap values in the bootstrap stack.
    bst = (struct bootstrap_stack *)(((char *)new->stack) + sz - sizeof(*bst));
    bst->flags = INTERRUPT_ENABLE_FLAG;
    bst->es = KERNEL_DS;
    bst->ds = KERNEL_DS;
    bst->boostrap = _kthread_bootstrap;
    bst->kthread_start = fn;

    // Initialize the process structure.
    new->ctx.ss = KERNEL_SS;
    new->ctx.sp = bst;
    new->pid = next_pid++;
    new->state = READY;

    list_add_before(&ready, &new->node);

    return 0;
}

void scheduler_exit(int status) {
    if (!current) {
        // Scheduler not initialized, nothing to do.
        return;
    }
    if (current->pid == 0) {
        printf("scheduler: can't exit kernel main process\n");
        return;
    }

    // Mark the process as killed to let schedule handle the removal.
    current->state = ZOMBIE;
    current->status = status;

    // Process exited, let other processes wait on it.
    list_add_before(&zombies, &current->node);

    schedule();
}
