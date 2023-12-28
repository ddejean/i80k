// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _IRQ_H_
#define _IRQ_H_

#include <stdint.h>

#include "board.h"

// External interruption identifiers.
#define INT_IRQ0 (IDT_IRQ_OFFSET)
#define INT_IRQ1 (IDT_IRQ_OFFSET + 1)
#define INT_IRQ2 (IDT_IRQ_OFFSET + 2)
#define INT_IRQ3 (IDT_IRQ_OFFSET + 3)
#define INT_IRQ4 (IDT_IRQ_OFFSET + 4)
#define INT_IRQ5 (IDT_IRQ_OFFSET + 5)
#define INT_IRQ6 (IDT_IRQ_OFFSET + 6)
#define INT_IRQ7 (IDT_IRQ_OFFSET + 7)

// Values defined to mask an unmask IRQs
#define MASK_IRQ0 0
#define MASK_IRQ1 1
#define MASK_IRQ2 2
#define MASK_IRQ3 3
#define MASK_IRQ4 4
#define MASK_IRQ5 5
#define MASK_IRQ6 6
#define MASK_IRQ7 7

// irq_setup() prepares the external interrupt controller and the CPU to be able
// to handle hardware interrupts.
void irq_setup(void);

// irq_enable() allows the CPU to receive the interrupt number <irq>.
void irq_enable(int irq);

// irq_enable() hides the interrupt number <irq>.
void irq_disable(int irq);

// irq_ack() acknowledges an external interrupt to the controler.
void irq_ack(void);

#endif  // _IRQ_H_
