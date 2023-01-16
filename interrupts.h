// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include "stdint.h"

#define INT_IRQ0 (1 << 0)
#define INT_IRQ1 (1 << 1)
#define INT_IRQ2 (1 << 2)
#define INT_IRQ3 (1 << 3)
#define INT_IRQ4 (1 << 4)
#define INT_IRQ5 (1 << 5)
#define INT_IRQ6 (1 << 6)
#define INT_IRQ7 (1 << 7)
#define INT_ALL  0xff

// interrupts_setup installs the interruptions handlers, initializes the
// controller and enable interrupts.
void interrupts_setup(void);

// interrupts_enable enables interrupts on the lines provided in mask.
void interrupts_enable(uint8_t mask);

// interrupts_disable disables interrupts on the lines provided in mask.
void interrupts_disable(uint8_t mask);

#endif // _INTERRUPTS_H_