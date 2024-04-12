// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "task.h"

// scheduler_initialize prepares the scheduler to manage threads and processes.
void scheduler_initialize(void);

// scheduler_queue_new queues a new task into the scheduler ready list with
// |prio| priority. Returns the PID of the newly started process.
int scheduler_queue_new(struct task *t, int prio);

// scheduler_exit quits the calling process and store the process return code
// |status|.
void scheduler_exit(int status);

// schedule stops the current process and runs the next one.
void schedule(void);

// scheduler_current returns the task currently runnning.
struct task *scheduler_current(void);

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
