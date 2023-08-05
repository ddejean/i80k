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
#include "xmodem_server.h"

static void update(void);

// Kernel C entry point.
// cs is the code segment where the kernel runs provided by crt0.S.
void kernel(void) {
    // Prepares the interrupt system to allow the syscalls to be operational.
    interrupts_setup(KERNEL_CS);
    syscall_setup();

    // Initialize the UART in polling mode to enable early printf.
    uart_early_initialize(300);

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
    uart_initialize(300);

    // Initialize the clock system.
    clock_initialize();

    update();
}

void uart_tx_char(struct xmodem_server *xdm, uint8_t byte, void *cb_data) {
    (void)xdm;
    (void)cb_data;
    uart_write((const char *)&byte, sizeof(byte));
}

char received[2048];
int pos = 0;

static void update(void) {
    struct xmodem_server xdm;

    xmodem_server_init(&xdm, uart_tx_char, NULL);
    while (!xmodem_server_is_done(&xdm)) {
        char data;
        uint8_t resp[XMODEM_MAX_PACKET_SIZE];
        uint32_t block_nr;
        int rx_data_len;

        int len = uart_read(&data, sizeof(data));
        if (len == 1) {
            if (pos < 2048) {
                received[pos] = data;
                pos++;
            }
            xmodem_server_rx_byte(&xdm, data);
        }
        rx_data_len = xmodem_server_process(&xdm, resp, &block_nr, clock_now());
        if (rx_data_len > 0) {
            // Do something
        }
    }
    if (xmodem_server_get_state(&xdm) == XMODEM_STATE_FAILURE) {
        received[pos] = '\0';
        printk("Xmodem transfer failure, received:\r\n");
        printk("%s", received);
    }
}
