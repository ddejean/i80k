// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdint.h>
#include <time.h>

// clk_initialize prepares the clock system: sets the initial time counter and
// hooks the interruption.
void clk_initialize(void);

int clk_gettime(clockid_t clockid, struct timespec *tp);

int clk_nanosleep(clockid_t clockid, int flags, const struct timespec *request,
                  struct timespec *remain);

#endif  // _CLOCK_H_
