// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "interrupts.h"

#include <stdint.h>
#include <string.h>

#include "devices.h"
#include "fmem.h"

#define IDT_ENTRIES 256

// Interrupt Descriptor Table start at address 0x00000 with 256 entries.
// The location of the table is handled by the linker script (kernel.lds).
void __far* idt[IDT_ENTRIES] __attribute__((section(".idt")));

// Interrupt controller(s).
static const struct pic* p;

void interrupts_initialize(void) {
    const struct device* dev;

    // Clear the IDT.
    memset(idt, 0, sizeof(idt));

    dev = board_get_by_name("pic");
    if (!dev) {
        // No interrupt controller, nothing else to do.
        return;
    }
    p = dev->config;
    if (!p) {
        // No configuration specified, nothing to do.
        return;
    }

    // Initialize the pic cascade.
    p->initialize(p);
    if (p->slave) {
        p->slave->initialize(p->slave);
    }
}

void interrupts_handle(int index, uint16_t seg, void (*handler)(void)) {
    if (index < 0 || index >= IDT_ENTRIES) {
        // Invalid index.
        return;
    }
    idt[index] = fmem_void_fptr(seg, handler);
}

int interrupts_from_irq(int irq) {
    if (irq >= p->irq_base && irq < p->irq_max) {
        return p->idt_offset + irq;
    }
    if (p->slave && irq >= p->slave->irq_base && irq < p->slave->irq_max) {
        return p->slave->idt_offset + (irq - p->slave->irq_base);
    }
    return -1;
}

void irq_enable(int irq) {
    if (irq < 0) {
        return;
    }
    if (irq < p->irq_max) {
        p->irq_enable(p, irq);
    } else if (p->slave && irq < p->slave->irq_max) {
        p->slave->irq_enable(p, irq - p->slave->irq_base);
    }
}

void irq_disable(int irq) {
    if (irq < 0) {
        return;
    }
    if (irq < p->irq_max) {
        p->irq_disable(p, irq);
    } else if (p->slave && irq < p->slave->irq_max) {
        p->slave->irq_disable(p, irq - p->slave->irq_base);
    }
}

void irq_ack(int irq) {
    if (irq < 0) {
        return;
    }
    if (p->slave && irq >= p->slave->irq_base) {
        p->irq_ack(p->slave);
    }
    p->irq_ack(p);
}
