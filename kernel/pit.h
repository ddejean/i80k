// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _PIT_H_
#define _PIT_H_

#include <stdint.h>

#define PIT_TIMER0 ((timer_t)0)
#define PIT_TIMER1 ((timer_t)1)
#define PIT_TIMER2 ((timer_t)2)
typedef uint8_t timer_t;

// pit_set_alarm sets an alarm on <timer> firing every <count> clocks.
void pit_set_alarm(timer_t timer, uint16_t counter);

// pit_freq_gen configures <timer> as a square wave generator with a frequency
// of PIT_FREQ/<divider>.
void pit_freq_gen(timer_t timer, uint32_t freq);

#endif  // _PIT_H_
