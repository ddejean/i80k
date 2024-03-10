// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "timer.h"

#ifndef _DELAY_H_
#define _DELAY_H_

// delay_calibrate calibrates the delay loop using the timer |t|.
void delay_calibrate(struct timer *t);

// udelay waits for |delay| microseconds.
void udelay(unsigned long delay);

// mdelay waits for |delay| milliseconds.
void mdelay(unsigned long delay);

#endif  // _DELAY_H_
