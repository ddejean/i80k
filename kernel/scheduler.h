// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stddef.h>
#include <sys/types.h>

// scheduler_initialize prepares the scheduler to manage threads and processes.
void scheduler_initialize(void);

// scheduler_kthread_start starts a kernel thread running |fn| with a stack of
// size |sz|.
int scheduler_kthread_start(int (*fn)(void), size_t sz);

// scheduler_exit quits the calling process and store the process return code
// |status|.
void scheduler_exit(int status);

// schedule stops the current process and runs the next one.
void schedule(void);

// scheduler_getpid returns the process ID of the process currently running.
pid_t scheduler_getpid();

#endif  // _SCHEDULER_H_
