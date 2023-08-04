// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _IRQ_H_
#define _IRQ_H_

#include <stdint.h>

#include "board.h"

// External interruption identifiers.
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

// irq_setup() prepares the external interrupt controller and the CPU to be able
// to handle hardware interrupts.
void irq_setup(void);

// irq_enable() allows the CPU to receive the interrupt represented by <mask>.
void irq_enable(uint8_t mask);

// irq_enable() hides the interrupt by <mask>.
void irq_disable(uint8_t mask);

// irq_ack() acknowledges an external interrupt to the controler.
void irq_ack(void);

#endif  // _IRQ_H_
