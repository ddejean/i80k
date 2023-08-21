// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "uart.h"

#include <stddef.h>
#include <stdint.h>

#include "board.h"
#include "cpu.h"
#include "debug.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"
#include "utils/ringbuffer.h"

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
#define CMD_FORCE_RTS (1 << 5)
#define CMD_RESET (1 << 6)

// P8251A status register.
#define STATUS_TXRDY 1
#define STATUS_RXRDY (1 << 1)
#define STATUS_TXEMPTY (1 << 2)
#define STATUS_PARITY_ERROR (1 << 3)
#define STATUS_OVERRUN_ERROR (1 << 4)
#define STATUS_FRAMING_ERROR (1 << 5)
#define STATUS_SYNDET (1 << 6)
#define STATUS_DATA_SET_RDY (1 << 7)

// Operating mode of the UART.
typedef enum uart_mode {
    NONE,     // UART driver is uninitialized
    POLLED,   // UART driver is initialized to work in polling mode.
    BUFFERED  // UART driver is initialized to working with buffers and
              // interrupts.
} mode_t;

// Operating mode of the UART driver.
static mode_t mode = NONE;

// Assembly interrupt handler for the UART.
extern void uart_int_handler(void);

// Size of the ring buffer.
#define RINGBUF_SIZE 256
// Pre-allocated ring buffer for the reception.
static char rx_buf[RINGBUF_SIZE];
// RX ring buffer instance.
static ring_buffer_t rx_ring;
// Pre-allocated ring buffer for emission.
static char tx_buf[RINGBUF_SIZE];
// TX ring buffer instance.
static ring_buffer_t tx_ring;
// Current commands in the UART.
static uint8_t cmd;

static void p8251a_init(void) {
    // According to the datasheet, the chip might be in an unknown configuration
    // state after power up. Complete the worst case scenarion initialization
    // sequence and manually reset the chip to ensure we're in the right state.
    outb(P8251A_CMD, 0);
    outb(P8251A_CMD, 0);
    outb(P8251A_CMD, 0);
    outb(P8251A_CMD, CMD_RESET);
    // Configuration mode.
    outb(P8251A_CMD, MODE_ASYNC_1 | CHAR_8BITS | PARITY_DISABLED | STOP_1BIT);
}

static inline void p8251a_cmd(uint8_t command) {
    cmd = command;
    outb(P8251A_CMD, cmd);
}

void uart_early_initialize(uint16_t baud_rate) {
    // Configure the PIT to generate a frequency that matches the desired baud
    // rate.
    pit_freq_gen(PIT_TIMER2, baud_rate);
    // Configure the UART controller.
    p8251a_init();
    // The driver is initialized in polling mode.
    mode = POLLED;
    // Only enable transmission.
    p8251a_cmd(CMD_TX_ENABLE);
}

void uart_initialize(uint16_t baud_rate) {
    // Prepare the reception ring buffer.
    ring_buffer_init(&rx_ring, rx_buf, sizeof(rx_buf));
    ring_buffer_init(&tx_ring, tx_buf, sizeof(tx_buf));
    // The driver is configured in buffered mode with interruptions.
    mode = BUFFERED;
    // Configure the PIT to provide the frequency matching the desired baud
    // rate.
    pit_freq_gen(PIT_TIMER2, baud_rate);
    // Configure the P8251A.
    p8251a_init();
    // Hook up the interrupt handler.
    interrupts_handle(INT_IRQ4, uart_int_handler);
    // Unmask the UART interrupt.
    irq_enable(MASK_IRQ4);
    // Enable RX only (TX will be enabled when bytes are ready to send).
    p8251a_cmd(CMD_RX_ENABLE | CMD_FORCE_RTS);

    // Print only after the UART is correctly initialized.
    printk(
        "UART: mode: buffered, baudrate: %u, ring buffers size: %d bytes, "
        "using IRQ4\r\n",
        baud_rate, RINGBUF_SIZE);
}

void uart_handler(void) {
    uint8_t status, byte;

    // Get the status of the UART controller.
    status = inb(P8251A_CMD);

    // Disable RTS so that we won't receive bytes while handling the interrupt.
    p8251a_cmd(cmd & ~CMD_FORCE_RTS);

    if (status & STATUS_RXRDY) {
        byte = inb(P8251A_DATA);
        // Queue the byte into the reception buffer if there's any space left,
        // or drop it.
        ring_buffer_queue(&rx_ring, byte);
    }

    if (status & STATUS_TXRDY) {
        if (ring_buffer_dequeue(&tx_ring, (char *)&byte)) {
            outb(P8251A_DATA, byte);
        } else {
            // No more data to send, disable TX.
            cmd = CMD_RX_ENABLE;
        }
    }

    // Re-enable RTS so that we can now receive bytes.
    p8251a_cmd(cmd | CMD_FORCE_RTS);

    // Acknoledge the interrupt controller.
    irq_ack();
}

int uart_read(const char *buffer, const size_t count) {
    size_t len = 0;

    if (mode != BUFFERED) {
        // Reception is disabled when not in buffered mode.
        return -1;
    }

    // Read the data from the ring buffer.
    if (!ring_buffer_is_empty(&rx_ring)) {
        len = ring_buffer_dequeue_arr(&rx_ring, (char *)buffer, count);
    }

    return len;
}

static inline void uart_write_char(const char c) {
    char status;
    // Wait for the queue to be empty before sending a byte.
    do {
        status = inb(P8251A_CMD);
    } while (!(status & STATUS_TXRDY));
    // Send the byte of data.
    outb(P8251A_DATA, c);
}

int uart_write(const char *buffer, const size_t count) {
    switch (mode) {
        case POLLED:
            for (size_t i = 0; i < count; i++) {
                uart_write_char(buffer[i]);
            }
            return count;

        case BUFFERED:
            // Put the data in the buffer.
            ring_buffer_queue_arr(&tx_ring, buffer, count);
            // Enable TX.
            p8251a_cmd(cmd | CMD_TX_ENABLE);
            return count;

        case NONE:
        default:
            // Driver is not initialized, nothing to do.
            return -1;
    }
}
