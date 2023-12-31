// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _PIT_H_
#define _PIT_H_

#include <stdint.h>

#include "devices.h"

// pit_initialize prepares the PIT for counter configuration.
void pit_initialize(void);

// pit_set_alarm sets an alarm on <timer> firing every <count> clocks.
void pit_set_alarm(struct io_device *dev, uint16_t counter);

// pit_freq_gen configures <timer> as a square wave generator with a frequency
// of <freq>.
void pit_freq_gen(struct io_device *dev, uint32_t freq);

// pit_read returns the current value of the <timer> counter.
uint16_t pit_read(struct io_device *dev);

// pit_ns_per_tick returns the number of ns spent during one timer increment.
uint32_t pit_ns_per_tick(struct io_device *dev);

#endif  // _PIT_H_
