// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _KTHREAD_H_
#define _KTHREAD_H_

#include <stddef.h>

// kthread_initialize starts the required kernel threads for it to function.
void kthread_initialize(void);

// kthread_start allocates a stack and starts kernel process.
int kthread_start(int (*fn)(void), size_t sz, int prio);

#endif  // _KTHREAD_H_
