// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "uart.h"

#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "cpu.h"
#include "interrupts.h"
#include "ringbuffer.h"

#define P8251A_CMD (PORT_UART | 1)
#define P8251A_DATA (PORT_UART)

// P8251A configuration word.
#define MODE_SYNC 0
#define MODE_ASYNC_1 1
#define MODE_ASYNC_16 2
#define MODE_ASYNC_64 3

#define CHAR_5BITS 0
#define CHAR_6BITS (1 << 2)
#define CHAR_7BITS (2 << 2)
#define CHAR_8BITS (3 << 2)

#define PARITY_DISABLED 0
#define PARITY_ENABLED (1 << 4)
#define PARITY_ODD 0
#define PARITY_EVEN (1 << 5)

#define STOP_1BIT (1 << 6)
#define STOP_1HALFBIT (2 << 6)
#define STOP_2BITS (3 << 6)

// P8251A command word.
#define CMD_TX_DISABLE 0
#define CMD_TX_ENABLE 1
#define CMD_RX_DISABLE 0
#define CMD_RX_ENABLE (1 << 2)
#define CMD_RESET (1 << 6)
#define CMD_FORCE_RTS (1 << 7)

// P8251A status register.
#define STATUS_TXRDY 1
#define STATUS_RXRDY (1 << 1)
#define STATUS_TXEMPTY (1 << 2)
#define STATUS_PARITY_ERROR (1 << 3)
#define STATUS_OVERRUN_ERROR (1 << 4)
#define STATUS_FRAMING_ERROR (1 << 5)
#define STATUS_SYNDET (1 << 6)
#define STATUS_DATA_SET_RDY (1 << 7)

// Assembly interrupt handler for the UART.
extern void uart_int_handler(void);

// Size of the ring buffer.
#define RINGBUF_SIZE 256
// Pre-allocated ring buffer for the reception.
char rx_buf[RINGBUF_SIZE];
// RX ring buffer instance.
ring_buffer_t rx_ring;
// Pre-allocated ring buffer for emission.
char tx_buf[RINGBUF_SIZE];
// TX ring buffer instance.
ring_buffer_t tx_ring;

void uart_initialize(void) {
    // Prepare the reception ring buffer.
    ring_buffer_init(&rx_ring, rx_buf, sizeof(rx_buf));
    ring_buffer_init(&tx_ring, tx_buf, sizeof(tx_buf));
    // According to the datasheet, the chip might be in an unknown configuration
    // state after power up. Complete the worst case scenarion initialization
    // sequence and manually reset the chip to ensure we're in the right state.
    outb(P8251A_CMD, 0);
    outb(P8251A_CMD, 0);
    outb(P8251A_CMD, 0);
    outb(P8251A_CMD, CMD_RESET);
    // Configuration mode.
    outb(P8251A_CMD, MODE_ASYNC_64 | CHAR_8BITS | PARITY_DISABLED | STOP_1BIT);
    // Hook up the interrupt handler.
    interrupts_handle(INT_IRQ4, uart_int_handler);
    // Unmask the UART interrupt.
    irq_enable(MASK_IRQ4);
    // Enable RX and TX.
    outb(P8251A_CMD, CMD_RX_ENABLE);
}

void uart_handler(void) {
    uint8_t status, byte;

    // Get the status of the UART controller.
    status = inb(P8251A_CMD);

    if (status & STATUS_RXRDY) {
        byte = inb(P8251A_DATA);
        // Queue the byte into the reception buffer if there's any space left,
        // or drop it.
        if (!ring_buffer_is_full(&rx_ring)) {
            ring_buffer_queue(&rx_ring, byte);
        }
    }

    if (status & STATUS_TXRDY) {
        if (!ring_buffer_is_empty(&tx_ring)) {
            ring_buffer_dequeue(&tx_ring, &byte);
            outb(P8251A_DATA, byte);
        } else {
            outb(P8251A_CMD, CMD_RX_ENABLE);
        }
    }

    irq_ack();
}

int uart_read(uint8_t *buffer, size_t count) {
    size_t len = 0;

    // Mask the interrupts to avoid race conditions on the ring buffer.
    cli();
    // Read the data from the ring buffer.
    if (!ring_buffer_is_empty(&rx_ring)) {
        len = ring_buffer_dequeue_arr(&rx_ring, (char *)buffer, count);
    }
    // Re-enable interrupts.
    sti();

    return len;
}

int uart_write(uint8_t *buffer, size_t count) {
    // Mask the interrupts to avoid race conditions on the ring buffer.
    cli();
    // Put the data in the buffer.
    ring_buffer_queue_arr(&tx_ring, buffer, count);
    // Re-enable interrupts now.
    sti();
    // Enable TX.
    outb(P8251A_CMD, CMD_RX_ENABLE | CMD_TX_ENABLE);
    return count;
}

void _putchar(char c) { uart_write(&c, sizeof(c)); }
