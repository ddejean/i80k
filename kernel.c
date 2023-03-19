// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stdint.h>
#include <stdio.h>

#include "board.h"
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

    // Initiliaze the heap to alloc future allocations.
    heap_initialize(firmware_data_end(), (void*)KERNEL_STACK_LOW);

    // Prepare the memory pages allocator.
    hw_alloc_init(1, 7);

    // Setup the interruption controller.
    interrupts_setup(cs);

    // Setup the UART as soon as possible.
    uart_initialize();
    sti();

    printf("Kernel booting...\r\n");

    // Prepare the scheduler.
    printf("Starting scheduler\r\n");
    // scheduler_init();

    // Start a first kthread.
    // scheduler_kthread_start(task0, DEFAULT_PRIORITY);
    // scheduler_kthread_start(task1, DEFAULT_PRIORITY);
    // scheduler_kthread_start(task2, DEFAULT_PRIORITY);

    // Kernel idle task.
    while (1) {
        int i, len;
        char buffer[8];
        for (i = 0; i < 255; i++) {
            DEBUG(i);
            len = uart_read(buffer, sizeof(buffer));
            if (len > 0) {
                uart_write(buffer, len);
            }
        }
    }
}
