// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _IRQ_H_
#define _IRQ_H_

#include <stdint.h>

#include "board.h"

// p8259a_initialize prepares the external interrupt controller and the CPU to
// be able to handle hardware interrupts.
void p8259a_initialize(const struct pic *p);

// p8259a_irq_enable allows the CPU to receive the interrupt number <irq>.
void p8259a_irq_enable(const struct pic *p, int irq);

// p8259a_irq_enable hides the interrupt number <irq>.
void p8259a_irq_disable(const struct pic *p, int irq);

// p8259a_irq_ack acknowledges an external interrupt to the controler.
void p8259a_irq_ack(const struct pic *p);

#endif  // _IRQ_H_
