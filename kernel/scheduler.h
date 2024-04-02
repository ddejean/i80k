// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ctx.h"
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
    pid_t pid;
    // Parent process ID.
    pid_t parent;
    // Process priority.
    int prio;
    // Process stack.
    void *stack;

    // Process state.
    enum task_state state;
    // Process return value.
    int status;
    // Private data use for wait condition.
    void *wait_state;
};

// scheduler_initialize prepares the scheduler to manage threads and processes.
void scheduler_initialize(void);

// scheduler_start starts a thread running |fn| with a stack of
// size |sz|.
int scheduler_start(int (*fn)(void), void *stack, size_t sz, int prio);

// scheduler_exit quits the calling process and store the process return code
// |status|.
void scheduler_exit(int status);

// schedule stops the current process and runs the next one.
void schedule(void);

// scheduler_getpid returns the process ID of the process currently running.
pid_t scheduler_getpid();

// scheduler_waitid waits for the a process.
pid_t scheduler_wait(pid_t id, int *wstatus, int options, struct rusage *usage);

// scheduler_sleep_on puts the current process to waiting state, put it in
// |queue| and sets |sleep_data| as its wait_state.
void scheduler_sleep_on(struct list_node *queue, void *sleep_data);

// scheduler_wake_up puts |task| in the ready list for later scheduling.
void scheduler_wake_up(struct task *task);

#endif  // _SCHEDULER_H_
