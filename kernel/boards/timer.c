// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include "timer.h"

#include <stddef.h>
#include <stdint.h>

#include "devices.h"

struct timer *timer_get(const char *name) {
    const struct device *dev = board_get_by_name(name);
    if (!dev) {
        return NULL;
    }
    return (struct timer *)dev->config;
}

uint32_t timer_ns_per_tick(struct timer *t) { return t->ns_per_tick(t); }

uint16_t timer_read(struct timer *t) { return t->read(t); }

void timer_set_alarm(struct timer *t, uint16_t count) {
    t->set_alarm(t, count);
}

void timer_set_freq(struct timer *t, uint32_t freq) { t->set_freq(t, freq); }
