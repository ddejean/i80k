// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "scheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ctx.h"
#include "error.h"
#include "list.h"

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

void scheduler_initialize() {
    // Current kernel task runnning with a standard priority.
    current = calloc(1, sizeof(struct task));
    current->pid = next_pid++;
    current->parent = -1;
    current->state = RUNNING;
    current->prio = 0;
    list_initialize(&current->node);
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

struct task *scheduler_current(void) { return current; }

int scheduler_queue_new(struct task *t, int prio) {
    // Initialize the process structure.
    t->pid = next_pid++;
    t->parent = current->pid;
    t->prio = prio;
    t->state = READY;

    // Add the task to the ready list.
    task_put(&ready, t);

    return t->pid;
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
    // rusage is not used for now.
    (void)usage;

    if (pid < -1) {
        return ERR_NOT_SUPP;
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
