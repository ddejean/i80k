// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "board.h"
#include "cpu.h"
#include "firmware.h"
#include "heap.h"
#include "interrupts.h"
#include "libc/malloc.h"

extern void irq0(void);
extern void irq1(void);

void handler_irq0() {
    outb(PORT_DEBUG, 1);
    // Ack interrupt
    outb(PORT_PIC, 0x20);
}

void handler_irq1() {
    outb(PORT_DEBUG, 2);
    // Ack interrupt
    outb(PORT_PIC, 0x20);
}

// Kernel C entry point.
// cs is the code segment where the kernel runs provided by crt0.S.
void kernel(uint16_t cs) {
    uint8_t i, j;

    // Prepare .data and .bss sections to ensure data are correctly located and
    // initialized.
    firmware_data_setup();
    firmware_bss_setup();

    // Initiliaze the heap to alloc future allocations.
    heap_initialize(firmware_data_end(), (void*)KERNEL_STACK_LOW);

    interrupts_setup(cs);
    interrupts_handle(INT_IRQ0, irq0);
    interrupts_handle(INT_IRQ1, irq1);
    irq_enable(MASK_IRQ0 | MASK_IRQ1);
    sti();

    // Wait for interrupt and clear the debug port after a while.
    while (1) {
        hlt();
        for (i = 0; i < 255; i++) {
            for (j = 0; j < 255; j++)
                ;
        }
        outb(PORT_DEBUG, 0);
    }
}
