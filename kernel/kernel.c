// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "clock.h"
#include "console.h"
#include "cpu.h"
#include "heap.h"
#include "interrupts.h"
#include "irq.h"
#include "syscall.h"

// Kernel C entry point.
// cs is the code segment where the kernel runs provided by crt0.S.
void kernel(void) {
    // Initiliaze the heap to alloc future allocations.
    heap_initialize(_bss_end, (void *)KERNEL_STACK_LOW);
    // Prepare the console to be able to print traces.
    console_initialize();
    // Prepares the interrupt system to allow the syscalls to be operational.
    interrupts_setup(KERNEL_CS);
    syscall_setup();

    printf("Kernel loaded:\n");
    printf("  .text: %04x[%p:%p], %d bytes\n", KERNEL_CS, _text_start,
           _text_end, _text_end - _text_start);
    printf("  .data: %04x[%p:%p], %d bytes\n", KERNEL_DS, _data_start,
           _data_end, _data_end - _data_start);
    printf("  .bss:  %04x[%p:%p], %d bytes\n", KERNEL_DS, _bss_start, _bss_end,
           _bss_end - _bss_start);

    // Setup the interruption controller.
    irq_setup();
    sti();

    // Binds the console to the board UART.
    console_bind_uart();

    // Initialize the clock system.
    clock_initialize();

    // Kernel idle task.
    while (1) {
        int c;

        c = getchar();
        if (c < 0) {
            // clock_wait(100, POLL_WAIT);
        } else {
            printf("%c", (char)c);
        }
    }
}
