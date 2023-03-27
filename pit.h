// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _PIT_H_
#define _PIT_H_

#include <stdint.h>

enum timer { PIT_TIMER0 = 0, PIT_TIMER1 = 1, PIT_TIMER2 = 2 };
typedef enum timer timer_t;

// pit_set_alarm sets an alarm on <timer> firing every <count> clocks.
void pit_set_alarm(timer_t timer, uint16_t counter);

#endif  // _PIT_H_
