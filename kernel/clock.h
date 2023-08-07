// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CLOCK_H_
#define _CLOCK_H_

// clock_initialize prepares the clock system: sets the initial time counter and
// hooks the interruption.
void clock_initialize(void);

// clock_now returns the current time in ms since the initialization.
unsigned long clock_now(void);

enum wait {
    POLL_WAIT,  // Wait implemented using polling.
    BLOCK_WAIT  // Wait implemented by blocking the process.
};
typedef enum wait wait_t;

// clock_wait stops the current thread/process until <delay> (in ms) is expired.
// <type> defines the wait behavior.
int clock_wait(unsigned long delay, wait_t type);

#endif  // _CLOCK_H_
