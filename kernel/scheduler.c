// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "scheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "board.h"
#include "cpu.h"
#include "ctx.h"
#include "error.h"
#include "list.h"

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
static pid_t next_pid;

// List or ready processes ordered by decreasing priority.
struct list_node ready = LIST_INITIAL_VALUE(ready);

// List of killed processes. Not ordered.
struct list_node zombies = LIST_INITIAL_VALUE(zombies);

// List of processes waiting for another to die.
struct list_node waiters = LIST_INITIAL_VALUE(waiters);

static inline struct task *task_get(struct list_node *list) {
    return list_remove_head_type(list, struct task, node);
}

static void task_put(struct list_node *list, struct task *task) {
    struct task *t = NULL;

    if (list_is_empty(list)) {
        list_add_before(list, &task->node);
        return;
    }

    // If the task we want to add has a lower priority than anything else, just
    // directly add it at the end of the list. It'll save a list traversal.
    t = list_peek_tail_type(list, struct task, node);
    if (t && t->prio >= task->prio) {
        list_add_after(&t->node, &task->node);
        return;
    }

    list_for_every_entry(list, t, struct task, node) {
        if (t->prio < task->prio) {
            list_add_before(&t->node, &task->node);
            return;
        }
    }

    // We should never reach here.
    assert(0);
}

struct task *task_find(struct list_node *list, pid_t pid) {
    struct task *t;
    list_for_every_entry(list, t, struct task, node) {
        if (t->pid == pid) {
            return t;
        }
    }
    return NULL;
}

struct task *task_find_child_of(struct list_node *list, pid_t pid) {
    struct task *t;
    list_for_every_entry(list, t, struct task, node) {
        if (t->parent == pid) {
            return t;
        }
    }
    return NULL;
}

// Kernel idle stack doesn't need to be a big one.
uint16_t _kernel_idle_stack[128];

// Kernel idle task, does nothing except pause waiting for interrupts.
int _kernel_idle() {
    while (1) {
        hlt();
    }
    // This function is never supposed to exit.
    return 0;
}

void scheduler_initialize() {
    // Current kernel task runnning with a standard priority.
    current = calloc(1, sizeof(struct task));
    current->pid = next_pid++;
    current->parent = -1;
    current->state = RUNNING;
    current->prio = 0;
    list_initialize(&current->node);

    // Kernel idle task, lowest priority, doing nothing.
    scheduler_start(_kernel_idle, _kernel_idle_stack,
                    sizeof(_kernel_idle_stack), -32767);
}

void schedule() {
    struct task *prev, *next;

    if (!current || list_is_empty(&ready)) {
        // Scheduler not initialized or the ready list is empty, nothing to do.
        return;
    }

    // Get the next process to run.
    next = task_get(&ready);

    // Switch processes internally.
    prev = current;
    current = next;

    // If the current process is still running, queue it to the ready list for
    // future run.
    if (prev->state == RUNNING) {
        prev->state = READY;
        task_put(&ready, prev);
    }

    // Switch to the next process.
    next->state = RUNNING;
    ctx_switch(&prev->ctx, &next->ctx);
}

void _kthread_bootstrap(int (*fn)(void)) {
    int status = fn();
    exit(status);
}

int scheduler_start(int (*fn)(void), void *stack, size_t sz, int prio) {
    struct task *new;
    struct bootstrap_stack *bst;

    if (sz < sizeof(struct bootstrap_stack)) {
        printf("scheduler: stack too small to start a process (%u bytes)\n",
               sz);
        return ERR_INVAL;
    }

    new = calloc(1, sizeof(*new));
    if (!new) {
        printf("scheduler: failed to allocate task\n");
        return ERR_NO_MEM;
    }

    // Put bootstrap values in the bootstrap stack.
    bst = (struct bootstrap_stack *)(((char *)stack) + sz - sizeof(*bst));
    bst->flags = INTERRUPT_ENABLE_FLAG;
    bst->es = KERNEL_DS;
    bst->ds = KERNEL_DS;
    bst->boostrap = _kthread_bootstrap;
    bst->kthread_start = fn;

    // Initialize the process structure.
    new->stack = stack;
    new->ctx.ss = KERNEL_SS;
    new->ctx.sp = bst;
    new->pid = next_pid++;
    new->parent = current->pid;
    new->prio = prio;
    new->state = READY;

    // Add the task to the ready list for later scheduling.
    task_put(&ready, new);

    return new->pid;
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

    // Wake-up any process waiting for this one to die.
    struct task *t, *tmp;
    list_for_every_entry_safe(&waiters, t, tmp, struct task, node) {
        pid_t pid = *(pid_t *)t->wait_state;
        // The waiter explicitely waits for this task.
        if (pid > 0 && current->pid == pid) {
            list_delete(&t->node);
            scheduler_wake_up(t);
            break;
        }
        // The waiter is a parent and waits for one of its children.
        if (pid == -1 && t->pid == current->parent) {
            list_delete(&t->node);
            scheduler_wake_up(t);
            break;
        }
    }

    schedule();
}

pid_t scheduler_getpid() {
    if (!current) {
        return ERR_NO_ENTRY;
    }
    return current->pid;
}

pid_t scheduler_wait(pid_t pid, int *wstatus, int options,
                     struct rusage *usage) {
    if (pid < -1) {
        return ERR_NOT_SUPP;
    }

    // rusage is not supported for now, just zero it out.
    if (usage) {
        memset(usage, 0, sizeof(*usage));
    }

    struct task *t = NULL;
    do {
        // Find the task we're waiting for.
        if (pid == -1) {
            t = task_find_child_of(&zombies, current->pid);
        } else {
            t = task_find(&zombies, pid);
        }
        // Task not found, and the call is non blocking.
        if (!t && (options & WNOHANG)) {
            // TODO: check the ready list.
            return 0;
        }
        // Task not found, block until (one of) the expected one(s) dies.
        if (!t) {
            scheduler_sleep_on(&waiters, &pid);
        }

    } while (!t);

    pid_t dead_pid = t->pid;
    if (wstatus) {
        *wstatus = (t->status & 0xff) << 8;
    }

    free(t->stack);
    free(t);
    return dead_pid;
}

void scheduler_sleep_on(struct list_node *queue, void *sleep_data) {
    if (!queue) {
        return;
    }
    current->state = WAITING;
    current->wait_state = sleep_data;
    list_add_before(queue, &current->node);
    schedule();
}

void scheduler_wake_up(struct task *task) {
    if (!task) {
        return;
    }
    task->state = READY;
    task_put(&ready, task);
}
