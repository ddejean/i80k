// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include <stdint.h>

// Interruptions indentifiers.
#define INT_IRQ0 32
#define INT_IRQ1 33
#define INT_IRQ2 34
#define INT_IRQ3 35
#define INT_IRQ4 36
#define INT_IRQ5 37
#define INT_IRQ6 38
#define INT_IRQ7 39

// Values defined to mask an unmask IRQs
#define MASK_IRQ0 (1 << 0)
#define MASK_IRQ1 (1 << 1)
#define MASK_IRQ2 (1 << 2)
#define MASK_IRQ3 (1 << 3)
#define MASK_IRQ4 (1 << 4)
#define MASK_IRQ5 (1 << 5)
#define MASK_IRQ6 (1 << 6)
#define MASK_IRQ7 (1 << 7)
#define MASK_ALL 0xff

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
