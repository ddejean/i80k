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

#define PC16550_TX_FIFO_SZ 16
#define PC16550_RX_FIFO_TRIG 14

#define PC16550_BUFR (PORT_UART)
#define PC16550_IER (PORT_UART + 1)
#define PC16550_ISR (PORT_UART + 2)
#define PC16550_FCR (PORT_UART + 2)
#define PC16550_LINE_CTRL (PORT_UART + 3)
#define PC16550_MCR (PORT_UART + 4)
#define PC16550_LSR (PORT_UART + 5)
#define PC16550_MSR (PORT_UART + 6)

#define PC16550_DIV_LSB (PORT_UART)
#define PC16550_DIV_MSB (PORT_UART + 1)

#define LCR_5BITS 0
#define LCR_6BITS 1
#define LCR_7BITS 2
#define LCR_8BITS 3
#define LCR_1HALF_OR_2_STOPS (1 << 2)
#define LCR_PARITY_ENABLE (1 << 3)
#define LCR_PARITY_EVEN (1 << 4)
#define LCR_STICK_PARITY (1 << 5)
#define LCR_BREAK_CTRL (1 << 6)
#define LCR_DLAB (1 << 7)

#define LSR_DATA_READY 1
#define LSR_OVERRUN_ERR (1 << 1)
#define LSR_PARITY_ERR (1 << 2)
#define LSR_FRAME_ERR (1 << 3)
#define LSR_BREAK_INT (1 << 4)
#define LSR_THRE (1 << 5)
#define LSR_TEMT (1 << 6)

#define FCR_RX_TX_ENABLE 1
#define FCR_RX_CLEAR (1 << 1)
#define FCR_TX_CLEAR (1 << 2)
#define FCR_TRIGGER_1B (0)
#define FCR_TRIGGER_4B (1 << 6)
#define FCR_TRIGGER_8B (2 << 6)
#define FCR_TRIGGER_14B (3 << 6)

typedef enum intr {
    MODEM_STATUS = 0,
    TX_EMPTY = 0x1,
    RX_DATA = 0x2,
    RX_LINE_STATUS = 0x3,
    RX_FIFO_TIMEOUT = 0x6,
} intr_t;

#define ISR_NO_INTERRUPT 1
#define ISR_INT_ID(reg) ((intr_t)((reg >> 1) & 0x7))

#define IER_RX_RDY 1
#define IER_TX_RDY (1 << 1)
#define IER_RX_LINE_STATUS (1 << 2)
#define IER_MODEM_STATUS (1 << 3)

#define MCR_DTR 1
#define MCR_RTS (1 << 1)
#define MCR_OUT1 (1 << 2)
#define MCR_OUT2 (1 << 3)
#define MCR_LOOPBACK (1 << 4)

#define MSR_DCTS 1
#define MSR_DDSR (1 << 1)
#define MSR_TERI (1 << 2)
#define MSR_DDCD (1 << 3)
#define MSR_CTS (1 << 4)
#define MSR_DSR (1 << 5)
#define MSR_RI (1 << 6)
#define MSR_DCD (1 << 6)

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
    lcr |= LCR_DLAB;
    outb(PC16550_LINE_CTRL, lcr);

    // The divisor is computed as follow: UART_FREQ / (16 * baud_rate).
    uint32_t div = (UART_FREQ >> 4) / (uint32_t)baud_rate;
    // Divisor LSB
    outb(PC16550_DIV_LSB, div & 0xff);
    // Divisor MSB
    outb(PC16550_DIV_MSB, (div >> 8) & 0xff);

    lcr &= ~LCR_DLAB;
    outb(PC16550_LINE_CTRL, lcr);
}

static inline void pc16550_enable_tx_int(void) {
    uint8_t ier = inb(PC16550_IER);
    ier |= IER_TX_RDY;
    outb(PC16550_IER, ier);
}

static inline void pc16550_disable_tx_int(void) {
    uint8_t ier = inb(PC16550_IER);
    ier &= ~IER_TX_RDY;
    outb(PC16550_IER, ier);
}

void pc16550_initialize(uint16_t baud_rate) {
    ring_buffer_init(&rx_ring, rx_buf, sizeof(rx_buf));
    ring_buffer_init(&tx_ring, tx_buf, sizeof(tx_buf));

    // Enable FIFO mode with a trigger at 8 bytes in the queue.
    outb(PC16550_FCR,
         FCR_RX_TX_ENABLE | FCR_RX_CLEAR | FCR_TX_CLEAR | FCR_TRIGGER_14B);
    // Use 8 bits per word, no parity, one stop bit.
    outb(PC16550_LINE_CTRL, LCR_8BITS);
    // Enable interrupts.
    outb(PC16550_IER, IER_RX_RDY | IER_RX_LINE_STATUS);
    // Set clock dividor register.
    pc16550_set_baud_rate(baud_rate);
    // Ensure RTS is low.
    outb(PC16550_MCR, MCR_RTS);

    // Hook up the interrupt handler.
    interrupts_handle(INT_IRQ4, uart_int_handler);
    // Unmask the UART interrupt.
    irq_enable(MASK_IRQ4);

    printf(
        "UART: baudrate: %u, ring buffers size: %d bytes, "
        "using IRQ4\n",
        baud_rate, RINGBUF_SIZE);
}

void uart_handler(void) {
    uint8_t isr, status;
    char data;

    isr = inb(PC16550_ISR);
    while (!(isr & ISR_NO_INTERRUPT)) {
        switch (ISR_INT_ID(isr)) {
            case RX_DATA:
                // The queue has reached the trigger so we exactly know how many
                // bytes we can pull.
                for (int i = 0; i < PC16550_RX_FIFO_TRIG; i++) {
                    char data = (char)inb(PC16550_BUFR);
                    ring_buffer_queue(&rx_ring, data);
                }
                break;

            case RX_FIFO_TIMEOUT:
                // FIFO timeout implies we don't know how many bytes are
                // available. Pull everything we can checking the status
                // register each time.
                status = inb(PC16550_LSR);
                while (status & LSR_DATA_READY) {
                    char data = (char)inb(PC16550_BUFR);
                    ring_buffer_queue(&rx_ring, data);
                    status = inb(PC16550_LSR);
                }
                break;

            case RX_LINE_STATUS:
                // TODO: handle errors.
                (void)inb(PC16550_LSR);
                break;

            case TX_EMPTY:
                // The FIFO is empty, we can put at most 16 bytes inside.
                if (!ring_buffer_is_empty(&tx_ring)) {
                    for (int i = 0; i < PC16550_TX_FIFO_SZ &&
                                    !ring_buffer_is_empty(&tx_ring);
                         i++) {
                        ring_buffer_dequeue(&tx_ring, &data);
                        outb(PC16550_BUFR, data);
                    }
                } else {
                    // Disable TX_EMPTY interrupt since there's nothing to
                    // transmit.
                    pc16550_disable_tx_int();
                }
                break;

            case MODEM_STATUS:
                (void)inb(PC16550_MSR);
                break;
        }
        isr = inb(PC16550_ISR);
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
    pc16550_enable_tx_int();
    return c;
}

int pc16550_write(const char *buffer, const size_t count) {
    // Put the data in the buffer.
    ring_buffer_queue_arr(&tx_ring, buffer, count);
    // Enable TX.
    pc16550_enable_tx_int();
    return count;
}
