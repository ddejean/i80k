// Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _P8254_H_
#define _P8254_H_

#include <stdint.h>

#include "timer.h"

// p8254_set_alarm sets an alarm on <timer> firing every <count> clocks.
void p8254_set_alarm(struct timer *t, uint16_t counter);

// p8254_set_freq configures <timer> as a square wave generator with a frequency
// of <freq>.
void p8254_set_freq(struct timer *t, uint32_t freq);

// p8254_read returns the current value of the <timer> counter.
uint16_t p8254_read(struct timer *t);

// p8254_ns_per_tick returns the number of ns spent during one timer increment.
uint32_t p8254_ns_per_tick(struct timer *t);

#endif  // _P8254_H_
