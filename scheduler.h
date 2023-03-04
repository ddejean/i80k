// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#define DEFAULT_PRIORITY 0
#define LOWEST_PRIORITY (-128)

// scheduler_initialize prepares the scheduler and create the first task that
// will be the first unscheduled task.
void scheduler_init(void);

// scheduler_kthread_start creates a kernel thread with its own stack starting
// with function <start>.
void scheduler_kthread_start(void (*start)(void), char priority);

// schedule does a switch to the next available task in the queue.
void schedule(void);

#endif  // _SCHEDULER_H_
