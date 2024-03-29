// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "clock.h"
#include "console.h"
#include "cpu.h"
#include "debug.h"
#include "driver.h"
#include "fs.h"
#include "heap.h"
#include "scheduler.h"
#include "syscall.h"
#include "unistd.h"
#include "update.h"

int task0(void) {
    printf("%s: pid=%d\n", __func__, getpid());
    for (int i = 0; i < 128; i++) {
        printf("task0: %d\n", i++);
        hlt();
    }
    return 0;
}

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
    // Prepares the scheduler to manage thread and processes.
    scheduler_initialize();

    printf("Kernel loaded:\n");
    printf("  .text: %04x[%p:%p], %d bytes\n", KERNEL_CS, _text_start,
           _text_end, _text_end - _text_start);
    printf("  .data: %04x[%p:%p], %d bytes\n", KERNEL_DS, _data_start,
           _data_end, _data_end - _data_start);
    printf("  .bss:  %04x[%p:%p], %d bytes\n", KERNEL_DS, _bss_start, _bss_end,
           _bss_end - _bss_start);

    // Enable interrupts.
    sti();

    // Binds the console to the board UART.
    console_bind_uart();

    // Initialize the clock system.
    clock_initialize();

    // Probe devices and instantiate the drivers.
    driver_probes();

    cli();
    sti();

    cli();
    scheduler_kthread_start(task0, 2046);
    sti();

    int i = 0;
    while (1) {
        printf("kernel: %d - %llu ms\n", i, clock_now());
        i++;
        hlt();
    }
}
