// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "clock.h"
#include "cpu.h"
#include "debug.h"
#include "firmware.h"
#include "heap.h"
#include "hwalloc.h"
#include "interrupts.h"
#include "scheduler.h"
#include "uart.h"

void task0(void) {
    uint8_t i = 0;

    while (1) {
        DEBUG(i++);
    }
}

void task1(void) {
    uint8_t i = 0xFF;

    while (1) {
        DEBUG(i--);
    }
}

void task2(void) {
    uint8_t i = 0xAA;

    while (1) {
        DEBUG(i);
        i = i ^ 0xFF;
    }
}

// Kernel C entry point.
// cs is the code segment where the kernel runs provided by crt0.S.
void kernel(uint16_t cs) {
    // Prepare .data and .bss sections to ensure data are correctly located and
    // initialized.
    firmware_data_setup();
    firmware_bss_setup();

    // Initialize the UART in polling mode to enable early printf.
    uart_early_initialize();

    printf("Kernel booting...\r\n");
    printf("  Code segment: %04X\r\n", cs);
    printf("  Data segment: %04X\r\n", KERNEL_DS);
    printf("  Stack segment: %04X\r\n", KERNEL_SS);

    // Initiliaze the heap to alloc future allocations.
    heap_initialize(firmware_data_end(), (void*)KERNEL_STACK_LOW);

    // Prepare the memory pages allocator.
    hw_alloc_init(1, 14);

    // Setup the interruption controller.
    interrupts_setup(cs);
    sti();

    // Setup the UART as soon as possible.
    uart_initialize();

    // Initialize the clock system.
    clock_initialize();

    // Prepare the scheduler.
    // printf("Scheduler starting.\r\n");
    // scheduler_init();

    // Start a first kthread.
    // scheduler_kthread_start(task0, DEFAULT_PRIORITY);
    // scheduler_kthread_start(task1, DEFAULT_PRIORITY);
    // scheduler_kthread_start(task2, DEFAULT_PRIORITY);

    // Kernel idle task.
    int i = 0;
    while (1) {
        printf("%d\r\n", i++);
        clock_wait(1000, POLL_WAIT);
    }
}
