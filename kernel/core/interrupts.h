// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include <stdint.h>

// interrupts_setup installs the interruptions handlers, initializes the
// controller and enable interrupts. The kernel code segment is saved for
// use when installing an interrupt handler.
void interrupts_setup(uint16_t cs);

// interrupts_handler installs an interrupt handler for the provided
// interruption number.
void interrupts_handle(uint8_t index, void (*handler)(void));

#endif  // _INTERRUPTS_H_
