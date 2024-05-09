// Copyright (C) 2023-2024 - Damien Dejean <dam.dejean@gmail.com>

#include <stdio.h>

#include "board.h"
#include "clk.h"
#include "console.h"
#include "cpu.h"
#include "driver.h"
#include "heap.h"
#include "kthread.h"
#include "mem.h"
#include "scheduler.h"
#include "syscall.h"

// Kernel C entry point.
// cs is the code segment where the kernel runs provided by crt0.S.
void kernel(void) {
    // Declare the devices present on the board.
    board_initialize();
    // Initiliaze the heap to alloc future allocations.
    heap_initialize(_bss_end, (void *)KERNEL_STACK_LOW);
    // Prepare the console to be able to print traces.
    console_initialize();
    // Prepares the interrupt system to allow the syscalls to be operational.
    syscall_setup();
    // Prepares the memory subsystem to manage the entire board memory.
    mem_initialize();
    // Prepares the scheduler to manage thread and processes.
    scheduler_initialize();
    // Start minimal kernel threads.
    kthread_initialize();

    printf("Kernel loaded:\n");
    printf("  .text: %04x[%p:%p], %u bytes\n", KERNEL_CS, _text_start,
           _text_end, _text_end - _text_start);
    printf("  .data: %04x[%p:%p], %u bytes\n", KERNEL_DS, _data_start,
           _data_end, _data_end - _data_start);
    printf("  .bss:  %04x[%p:%p], %u bytes\n", KERNEL_DS, _bss_start, _bss_end,
           _bss_end - _bss_start);

    // Enable interrupts.
    sti();

    // Binds the console to the board UART.
    console_bind_uart();

    // Initialize the clock system.
    clk_initialize();

    // Probe devices and instantiate the drivers.
    driver_probes();

    printf("Kernel booted!\n");

    while (1) {
        hlt();
    }
}
