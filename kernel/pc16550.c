// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "pc16550.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"
#include "utils/ringbuffer.h"

#define PC16550_RX_TX_BUF (PORT_UART)
#define PC16550_INT_ENABLE (PORT_UART + 1)
#define PC16550_INT_ID (PORT_UART + 2)
#define PC16550_FIFO_CTRL (PORT_UART + 2)
#define PC16550_LINE_CTRL (PORT_UART + 3)
#define PC16550_MODEM_CTRL (PORT_UART + 4)
#define PC16550_LINE_STATUS (PORT_UART + 5)
#define PC16550_MODEM_STATUS (PORT_UART + 6)

#define PC16550_DIV_LSB (PORT_UART)
#define PC16550_DIV_MSB (PORT_UART + 1)

#define LINE_CTRL_5BITS 0
#define LINE_CTRL_6BITS 1
#define LINE_CTRL_7BITS 2
#define LINE_CTRL_8BITS 3
#define LINE_CTRL_1HALF_OR_2_STOPS (1 << 2)
#define LINE_CTRL_PARITY_ENABLE (1 << 3)
#define LINE_CTRL_PARITY_EVEN (1 << 4)
#define LINE_CTRL_STICK_PARITY (1 << 5)
#define LINE_CTRL_BREAK_CTRL (1 << 6)
#define LINE_CTRL_DLAB (1 << 7)

#define LINE_STATUS_DATA_READY 1
#define LINE_STATUS_OVERRUN_ERR (1 << 1)
#define LINE_STATUS_PARITY_ERR (1 << 2)
#define LINE_STATUS_FRAME_ERR (1 << 3)
#define LINE_STATUS_BREAK_INT (1 << 4)
#define LINE_STATUS_THRE (1 << 5)
#define LINE_STATUS_TEMT (1 << 6)

#define FIFO_CTRL_RX_TX_ENABLE 1
#define FIFO_CTRL_RX_CLEAR (1 << 1)
#define FIFO_CTRL_TX_CLEAR (1 << 2)
#define FIFO_CTRL_TRIGGER_1B (0)
#define FIFO_CTRL_TRIGGER_4B (1 << 6)
#define FIFO_CTRL_TRIGGER_8B (2 << 6)
#define FIFO_CTRL_TRIGGER_14B (3 << 6)

typedef enum intr {
    MODEM_STATUS = 0,
    NO_INTERRUPT = 0x1,
    TX_EMPTY = 0x2,
    RX_DATA = 0x4,
    RX_LINE_STATUS = 0x6,
    RX_FIFO_TIMEOUT = 0xc,
} intr_t;

#define INTR_STATUS_VALUE(reg) ((intr_t)(reg & 0xf))

#define INTR_ENABLE_RX_RDY 1
#define INTR_ENABLE_TX_RDY (1 << 1)
#define INTR_ENABLE_RX_LINE_STATUS (1 << 2)
#define INTR_ENABLE_MODEM_STATUS (1 << 3)

#define MODEM_CTRL_DTR 1
#define MODEM_CTRL_RTS (1 << 1)
#define MODEM_CTRL_OUT1 (1 << 2)
#define MODEM_CTRL_OUT2 (1 << 3)
#define MODEM_CTRL_LOOPBACK (1 << 4)

#define MODEM_STATUS_DCTS 1
#define MODEM_STATUS_DDSR (1 << 1)
#define MODEM_STATUS_TERI (1 << 2)
#define MODEM_STATUS_DDCD (1 << 3)
#define MODEM_STATUS_CTS (1 << 4)
#define MODEM_STATUS_DSR (1 << 5)
#define MODEM_STATUS_RI (1 << 6)
#define MODEM_STATUS_DCD (1 << 6)

// Assembly interrupt handler for the UART.
extern void uart_int_handler(void);

// Size of the ring buffer.
#define RINGBUF_SIZE 512
// Pre-allocated ring buffer for the reception.
static char rx_buf[RINGBUF_SIZE];
// RX ring buffer instance.
static ring_buffer_t rx_ring;
// Pre-allocated ring buffer for emission.
static char tx_buf[RINGBUF_SIZE];
// TX ring buffer instance.
static ring_buffer_t tx_ring;

static void pc16550_set_baud_rate(uint16_t baud_rate) {
    uint8_t lcr;

    // Set the Divisor Latch Bit to be able to set the divisor.
    lcr = inb(PC16550_LINE_CTRL);
    lcr |= LINE_CTRL_DLAB;
    outb(PC16550_LINE_CTRL, lcr);

    // The divisor is computed as follow: UART_FREQ / (16 * baud_rate).
    uint32_t div = (UART_FREQ >> 4) / (uint32_t)baud_rate;
    // Divisor LSB
    outb(PC16550_DIV_LSB, div & 0xff);
    // Divisor MSB
    outb(PC16550_DIV_MSB, (div >> 8) & 0xff);

    lcr &= ~LINE_CTRL_DLAB;
    outb(PC16550_LINE_CTRL, lcr);
}

void pc16550_initialize(uint16_t baud_rate) {
    ring_buffer_init(&rx_ring, rx_buf, sizeof(rx_buf));
    ring_buffer_init(&tx_ring, tx_buf, sizeof(tx_buf));

    // Enable FIFO mode with a trigger at 8 bytes in the queue.
    outb(PC16550_FIFO_CTRL, FIFO_CTRL_RX_TX_ENABLE | FIFO_CTRL_TRIGGER_8B);
    // Use 8 bits per word, no parity, one stop bit.
    outb(PC16550_LINE_CTRL, LINE_CTRL_8BITS);
    // Enable interrupts.
    outb(PC16550_INT_ENABLE, INTR_ENABLE_RX_RDY | INTR_ENABLE_RX_LINE_STATUS);

    pc16550_set_baud_rate(baud_rate);

    // Hook up the interrupt handler.
    interrupts_handle(INT_IRQ4, uart_int_handler);
    // Unmask the UART interrupt.
    irq_enable(MASK_IRQ4);

    printf(
        "UART: mode: buffered, baudrate: %u, ring buffers size: %d bytes, "
        "using IRQ4\n",
        baud_rate, RINGBUF_SIZE);
}

void uart_handler(void) {
    uint8_t status;
    char data;
    intr_t intr = INTR_STATUS_VALUE(inb(PC16550_INT_ID));

    switch (intr) {
        case RX_DATA:
        case RX_FIFO_TIMEOUT:
            status = inb(PC16550_LINE_STATUS);
            while (status & LINE_STATUS_DATA_READY) {
                char data = (char)inb(PC16550_RX_TX_BUF);
                ring_buffer_queue(&rx_ring, data);
                status = inb(PC16550_LINE_STATUS);
            }
            break;

        case RX_LINE_STATUS:
            // TODO: handle errors.
            (void)inb(PC16550_LINE_STATUS);
            break;

        case TX_EMPTY:
            if (!ring_buffer_is_empty(&tx_ring)) {
                ring_buffer_dequeue(&tx_ring, &data);
                outb(PC16550_RX_TX_BUF, data);
            } else {
                // Disable TX_EMPTY interrupt since there's nothing to transmit.
                outb(PC16550_INT_ENABLE,
                     INTR_ENABLE_RX_RDY | INTR_ENABLE_RX_LINE_STATUS);
            }
            break;

        case MODEM_STATUS:
            (void)inb(PC16550_MODEM_STATUS);
            break;

        case NO_INTERRUPT:
        default:
            break;
    }

    // Acknoledge the interrupt controller.
    irq_ack();
}

int pc16550_read(const char *buffer, const size_t count) {
    size_t len = 0;

    // Read the data from the ring buffer.
    if (!ring_buffer_is_empty(&rx_ring)) {
        len = ring_buffer_dequeue_arr(&rx_ring, (char *)buffer, count);
    }

    return len;
}

int pc16550_putchar(const char c) {
    ring_buffer_queue(&tx_ring, (char)c);
    outb(PC16550_INT_ENABLE,
         INTR_ENABLE_RX_RDY | INTR_ENABLE_TX_RDY | INTR_ENABLE_RX_LINE_STATUS);
    return c;
}

int pc16550_write(const char *buffer, const size_t count) {
    // Put the data in the buffer.
    ring_buffer_queue_arr(&tx_ring, buffer, count);
    // Enable TX.
    outb(PC16550_INT_ENABLE,
         INTR_ENABLE_RX_RDY | INTR_ENABLE_TX_RDY | INTR_ENABLE_RX_LINE_STATUS);
    return count;
}
