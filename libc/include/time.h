// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _TIME_H_
#define _TIME_H_

#include <sys/timespec.h>
#include <sys/types.h>

enum clockid {
    CLOCK_REALTIME,
    CLOCK_MONOTONIC,
    CLOCK_BOOTTIME,
    CLOCK_PROCESS_CPUTIME_ID,
};
typedef enum clockid clockid_t;

#define TIMER_ABSTIME 4

int clock_gettime(clockid_t clockid, struct timespec *tp);

int clock_nanosleep(clockid_t clockid, int flags,
                    const struct timespec *request, struct timespec *remain);

#endif  // _TIME_H_
