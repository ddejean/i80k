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

// irq_enable enables interrupts on the lines provided in mask.
void irq_enable(uint8_t mask);

// irq_disable disables interrupts on the lines provided in mask.
void irq_disable(uint8_t mask);

// irq_ack acknoledge an interrupt to the interrupt controller.
void irq_ack(void);

#endif  // _INTERRUPTS_H_
