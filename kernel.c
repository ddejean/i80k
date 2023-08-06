// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>

#include "board.h"
#include "clock.h"
#include "cpu.h"
#include "debug.h"
#include "heap.h"
#include "interrupts.h"
#include "irq.h"
#include "syscall.h"
#include "uart.h"

// Kernel C entry point.
// cs is the code segment where the kernel runs provided by crt0.S.
void kernel(void) {
    // Prepares the interrupt system to allow the syscalls to be operational.
    interrupts_setup(KERNEL_CS);
    syscall_setup();

    // Initialize the UART in polling mode to enable early printf.
    uart_early_initialize(19200);

    printk("Kernel loaded:\r\n");
    printk("  .text: %04x[%04x:%04x], %d bytes\r\n", KERNEL_CS, _text_start,
           _text_end, _text_end - _text_start);
    printk("  .data: %04x[%04x:%04x], %d bytes\r\n", KERNEL_DS, _data_start,
           _data_end, _data_end - _data_start);
    printk("  .bss:  %04x[%04x:%04x], %d bytes\r\n", KERNEL_DS, _bss_start,
           _bss_end, _bss_end - _bss_start);

    // Initiliaze the heap to alloc future allocations.
    heap_initialize(_bss_end, (void *)KERNEL_STACK_LOW);

    // Setup the interruption controller.
    irq_setup();
    sti();

    // Setup the UART as soon as possible.
    uart_initialize(19200);

    // Initialize the clock system.
    clock_initialize();

    // Kernel idle task.
    int i = 0;
    while (1) {
        printk("%d\r", i++);
        clock_wait(1000, POLL_WAIT);
    }
}
