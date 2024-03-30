// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _TIMESPECT_H_
#define _TIMESPECT_H_

#include <sys/types.h>

struct timespec {
    time_t tv_sec;
    long long tv_nsec;
};

#endif  // _TIMESPECT_H_
