// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"
#include "ringbuffer.h"
#include "uart.h"

#define P8251A_CMD(dev) (dev->port | 1)
#define P8251A_DATA(dev) (dev->port)

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

// Assembly interrupt handler for the UART.
extern void uart_int_handler(void);

// I/O device used by the driver.
static struct io_device *uart;
// RX ring buffer instance.
static ring_buffer_t *rx_ring;
// TX ring buffer instance.
static ring_buffer_t *tx_ring;
// Current commands in the UART.
static uint8_t cmd;

static inline void p8251a_cmd(uint8_t command) {
    cmd = command;
    outb(P8251A_CMD(uart), cmd);
}

void uart_initialize(ring_buffer_t *rxq, ring_buffer_t *txq,
                     uint16_t baud_rate) {
    rx_ring = rxq;
    tx_ring = txq;
    // Configure the PIT to provide the frequency matching the desired baud
    // rate.
    struct io_device *timer2 = board_get_io_dev(IO_DEV_PIT_TIMER2);
    pit_freq_gen(timer2, baud_rate);
    // Obtain the UART device.
    uart = board_get_io_dev(IO_DEV_UART);
    // According to the datasheet, the chip might be in an unknown configuration
    // state after power up. Complete the worst case scenarion initialization
    // sequence and manually reset the chip to ensure we're in the right state.
    outb(P8251A_CMD(uart), 0);
    outb(P8251A_CMD(uart), 0);
    outb(P8251A_CMD(uart), 0);
    outb(P8251A_CMD(uart), CMD_RESET);
    // Configuration mode.
    outb(P8251A_CMD(uart),
         MODE_ASYNC_1 | CHAR_8BITS | PARITY_DISABLED | STOP_1BIT);
    // Hook up the interrupt handler.
    interrupts_handle(IRQ_TO_INTERRUPT(uart->u.uart.irq), uart_int_handler);
    // Unmask the UART interrupt.
    irq_enable(uart->u.uart.irq);
    // Enable RX only (TX will be enabled when bytes are ready to send).
    p8251a_cmd(CMD_RX_ENABLE | CMD_FORCE_RTS);

    // Print only after the UART is correctly initialized.
    printf("UART: baudrate: %u, using IRQ %d\n", baud_rate, uart->u.uart.irq);
}

void uart_handler(void) {
    uint8_t status, byte;

    // Disable RTS so that we won't receive bytes while handling the interrupt.
    p8251a_cmd(cmd & ~CMD_FORCE_RTS);

    // Get the status of the UART controller.
    status = inb(P8251A_CMD(uart));

    if (status & STATUS_RXRDY) {
        byte = inb(P8251A_DATA(uart));
        // Queue the byte into the reception buffer if there's any space left,
        // or drop it.
        ring_buffer_queue(rx_ring, byte);
    }

    if (status & STATUS_TXRDY) {
        if (ring_buffer_dequeue(tx_ring, (char *)&byte)) {
            outb(P8251A_DATA(uart), byte);
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

void uart_start_xmit(void) { p8251a_cmd(cmd | CMD_TX_ENABLE); }
