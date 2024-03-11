// Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

#include <stdint.h>

struct pic;

struct pic {
    // I/O port of the interrupt controller.
    uint16_t port;
    // IRQ this controller is mapped to, -1 if master.
    int irq;
    // IRQ base of this interrupt controller.
    int irq_base;
    // Number of IRQ managed by this controller.
    int irq_max;
    // Offset the controller should use in the interrupt table.
    uint8_t idt_offset;
    // Slave interrupt controller wired to this one.
    const struct pic *slave;

    // IRQ controller operations.
    void (*initialize)(const struct pic *p);
    void (*irq_enable)(const struct pic *p, int irq);
    void (*irq_disable)(const struct pic *p, int irq);
    void (*irq_ack)(const struct pic *p);
};

// interrupts_initialize installs the interruptions handlers, initializes the
// controller and enable interrupts.
void interrupts_initialize(void);

// interrupts_handler installs an interrupt handler for the provided
// interruption number.
void interrupts_handle(int index, uint16_t seg, void (*handler)(void));

// interrupts_from_irq converts and IRQ number to an interrupt number.
int interrupts_from_irq(int irq);

// irq_enable unmasks the IRQ |irq|.
void irq_enable(int irq);

// irq_disable unmasks the IRQ |irq|.
void irq_disable(int irq);

// irq_disable unmasks the IRQ |irq|.
void irq_ack(int irq);

#endif  // _INTERRUPTS_H_
