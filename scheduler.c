// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "scheduler.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "board.h"
#include "cpu.h"
#include "debug.h"
#include "hwalloc.h"
#include "interrupts.h"
#include "mem.h"

struct task;
struct task {
    // Stack pointer.
    uint16_t *sp;
    // Stack segment.
    uint16_t ss;
    // Process identifier.
    unsigned int pid;
    // Process priority.
    int priority;
    // Memory descriptor for the stack.
    struct hw_page *stack_page;
    // Next task in the queue.
    struct task *next;
};

// Next available process ID value.
unsigned int next_pid;

// Task that will be holding the kernel context.
struct task kernel_idle;

// The current running task.
struct task *current;

// The list of task ready to run.
struct task *ready;

static struct task* task_pop() {
    struct task *head = ready;
    ready = ready->next;
    head->next = NULL;
    return head;
}

static void task_put(struct task *task) {
    struct task *current;

    if (!task) {
        return;
    }

    if (!ready) {
        task->next = ready;
        ready = task;
        return;
    }

    if (task->priority > ready->priority) {
        task->next = ready->next;
        ready = task;
        return;
    } else {
        // Move to the end of the queue.
        current = ready;
        while (current->next && task->priority <= current->next->priority) {
            current = current->next;
        }
        task->next = current->next;
        current->next = task;
    }
}

// schedule_handler is the handler of the interruption that will lead to a
// context switch.
extern void schedule_handler(void);

void scheduler_init(void) {
    // Nothing in the ready task for now.
    ready = NULL;

    // Prepare PID.
    next_pid = 0;

    // Prepare the default kernel task.
    kernel_idle.pid = next_pid++;
    kernel_idle.priority = LOWEST_PRIORITY;
    current = &kernel_idle;

    // Hook the first IRQ (timer) to the interrupt handler and unmask the interrupt.
    interrupts_handle(INT_IRQ0, schedule_handler);
    irq_enable(MASK_IRQ0);
}

// schedule_switch performs the context switch between the <current> task and
// the <next> task. The function will be entered with the <current> task and
// will return in the <next> task.
extern void schedule_switch(struct task *current, struct task *next);

void schedule(void) {
    struct task *old, *next;

    // We're currently running the only task, nothing to do.
    if (!ready) {
        return;
    }

    // Pop out the first ready task.
    next = task_pop();

    // Put the former current task to the ready list, and update the current
    // one.
    task_put(current);

    // Keep a ref on the current task and set the new one.
    old = current;
    current = next;

    // Switch to the task.
    schedule_switch(old, current);
}

void scheduler_kthread_start(void (*start)(void), char priority) {
    uint16_t seg, *stack;
    struct task *new;
    struct hw_page *page;
    uint16_t fake_stack[7];

    // Allocate a stack.
    page = hw_page_alloc();
    seg = hw_page_seg(page);
    stack = hw_page_addr(page) + (PAGE_SZ / sizeof(uint16_t));

    // Prepare the future thread stack.
    fake_stack[0] = INTERRUPT_ENABLE_FLAG;   // Enable interrupt for this task.
    fake_stack[1] = KERNEL_DS;  // ES: kernel segment.
    fake_stack[2] = KERNEL_DS;  // DS: kernel segment.
    fake_stack[3] = 0x0;        // DI
    fake_stack[4] = 0x0;        // SI
    fake_stack[5] = 0x0;        // BP
    fake_stack[6] = (uint16_t)start;

    // Copy the stack to the right place.
    stack -= sizeof(fake_stack) / sizeof(uint16_t);
    ksegmemcpy(stack, seg, fake_stack, KERNEL_SS, sizeof(fake_stack));

    // Create a new task and add it to the list.
    new = malloc(sizeof(*new));
    new->stack_page = page;
    new->ss = seg;
    new->sp = stack;
    new->pid = next_pid++;
    new->priority = priority;
    task_put(new);
}
