// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdint.h>

// clock_initialize prepares the clock system: sets the initial time counter and
// hooks the interruption.
void clock_initialize(void);

// clock_now returns the current time in ms since the initialization.
uint64_t clock_now(void);

#endif  // _CLOCK_H_
