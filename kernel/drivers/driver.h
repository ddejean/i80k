// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdbool.h>

#ifndef _DRIVER_H_
#define _DRIVER_H_

// Describes a driver.
struct driver {
    // Name of the driver.
    const char *name;
    // Driver's probe entry point.
    bool (*probe)(void);
};

#define concat(a, b) __ex_concat(a, b)
#define __ex_concat(a, b) a##b

#define DRIVER(name_, probe_)                    \
    const struct driver concat(__driver_, name_) \
        __attribute__((section(".drivers"))) = { \
            .name = #name_,                      \
            .probe = probe_,                     \
    }

// driver_init go through all the available drivers and triggers probing.
void driver_probes(void);

#endif  // _DRIVER_H_
