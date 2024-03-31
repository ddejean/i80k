// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <sys/types.h>

union sigval {
    int sival_int;    // Integer signal value.
    void *sival_ptr;  // Pointer signal value.
};

#define SI_USER 1     // Sent by a user. kill(), abort(), etc.
#define SI_QUEUE 2    // Sent by sigqueue().
#define SI_TIMER 3    // Sent by expiration of a timer_settime() timer.
#define SI_ASYNCIO 4  // Indicates completion of asycnhronous IO.
#define SI_MESGQ 5    // Indicates arrival of a message at an empty queue.

typedef struct {
    pid_t si_pid;           // Process id of the child.
    int si_signo;           // Signal number.
    int si_code;            // Cause of the signal.
    union sigval si_value;  // Signal value.
} siginfo_t;

#endif  // _SIGNAL_H_
