// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "cpu.h"
#include "devices.h"
#include "include/uart.h"
#include "interrupts.h"
#include "irq.h"
#include "pit.h"
#include "ringbuffer.h"

#define PC16550_TX_FIFO_SZ 16
#define PC16550_RX_FIFO_TRIG 14

#define PC16550_BUFR(dev) (dev->port)
#define PC16550_IER(dev) (dev->port + 1)
#define PC16550_ISR(dev) (dev->port + 2)
#define PC16550_FCR(dev) (dev->port + 2)
#define PC16550_LINE_CTRL(dev) (dev->port + 3)
#define PC16550_MCR(dev) (dev->port + 4)
#define PC16550_LSR(dev) (dev->port + 5)
#define PC16550_MSR(dev) (dev->port + 6)

#define PC16550_DIV_LSB(dev) (dev->port)
#define PC16550_DIV_MSB(dev) (dev->port + 1)

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

// I/O device used by the driver.
static struct io_device *uart;
// RX ring buffer instance.
static ring_buffer_t *rx_ring;
// TX ring buffer instance.
static ring_buffer_t *tx_ring;

static void pc16550_set_baud_rate(uint16_t baud_rate) {
    uint8_t lcr;

    // Set the Divisor Latch Bit to be able to set the divisor.
    lcr = inb(PC16550_LINE_CTRL(uart));
    lcr |= LCR_DLAB;
    outb(PC16550_LINE_CTRL(uart), lcr);

    // The divisor is computed as follow: UART_FREQ / (16 * baud_rate).
    uint32_t div = (uart->u.uart.freq >> 4) / (uint32_t)baud_rate;
    // Divisor LSB
    outb(PC16550_DIV_LSB(uart), div & 0xff);
    // Divisor MSB
    outb(PC16550_DIV_MSB(uart), (div >> 8) & 0xff);

    lcr &= ~LCR_DLAB;
    outb(PC16550_LINE_CTRL(uart), lcr);
}

static inline void pc16550_enable_tx_int(void) {}

static inline void pc16550_disable_tx_int(void) {
    uint8_t ier = inb(PC16550_IER(uart));
    ier &= ~IER_TX_RDY;
    outb(PC16550_IER(uart), ier);
}

void uart_initialize(ring_buffer_t *rxq, ring_buffer_t *txq,
                     uint16_t baud_rate) {
    rx_ring = rxq;
    tx_ring = txq;
    // Obtain the I/O device.
    uart = board_get_io_dev(IO_DEV_UART);
    // Enable FIFO mode with a trigger at 8 bytes in the queue.
    outb(PC16550_FCR(uart),
         FCR_RX_TX_ENABLE | FCR_RX_CLEAR | FCR_TX_CLEAR | FCR_TRIGGER_14B);
    // Use 8 bits per word, no parity, one stop bit.
    outb(PC16550_LINE_CTRL(uart), LCR_8BITS);
    // Enable interrupts.
    outb(PC16550_IER(uart), IER_RX_RDY | IER_RX_LINE_STATUS);
    // Set clock dividor register.
    pc16550_set_baud_rate(baud_rate);
    // Ensure RTS is low.
    outb(PC16550_MCR(uart), MCR_RTS);

    // Hook up the interrupt handler.
    interrupts_handle(IRQ_TO_INTERRUPT(uart->irq), uart_int_handler);
    // Unmask the UART interrupt.
    irq_enable(uart->irq);

    printf("UART: baudrate: %u, using IRQ %d\n", baud_rate, uart->irq);
}

void uart_handler(void) {
    uint8_t isr, status;
    char data;

    isr = inb(PC16550_ISR(uart));
    while (!(isr & ISR_NO_INTERRUPT)) {
        switch (ISR_INT_ID(isr)) {
            case RX_DATA:
                // The queue has reached the trigger so we exactly know how many
                // bytes we can pull.
                for (int i = 0; i < PC16550_RX_FIFO_TRIG; i++) {
                    char data = (char)inb(PC16550_BUFR(uart));
                    ring_buffer_queue(rx_ring, data);
                }
                break;

            case RX_FIFO_TIMEOUT:
                // FIFO timeout implies we don't know how many bytes are
                // available. Pull everything we can checking the status
                // register each time.
                status = inb(PC16550_LSR(uart));
                while (status & LSR_DATA_READY) {
                    char data = (char)inb(PC16550_BUFR(uart));
                    ring_buffer_queue(rx_ring, data);
                    status = inb(PC16550_LSR(uart));
                }
                break;

            case RX_LINE_STATUS:
                // TODO: handle errors.
                (void)inb(PC16550_LSR(uart));
                break;

            case TX_EMPTY:
                // The FIFO is empty, we can put at most 16 bytes inside.
                if (!ring_buffer_is_empty(tx_ring)) {
                    for (int i = 0; i < PC16550_TX_FIFO_SZ &&
                                    !ring_buffer_is_empty(tx_ring);
                         i++) {
                        ring_buffer_dequeue(tx_ring, &data);
                        outb(PC16550_BUFR(uart), data);
                    }
                } else {
                    // Disable TX_EMPTY interrupt since there's nothing to
                    // transmit.
                    pc16550_disable_tx_int();
                }
                break;

            case MODEM_STATUS:
                (void)inb(PC16550_MSR(uart));
                break;
        }
        isr = inb(PC16550_ISR(uart));
    }

    // Acknoledge the interrupt controller.
    irq_ack(uart->irq);
}

void uart_start_xmit(void) {
    uint8_t ier = inb(PC16550_IER(uart));
    // Enable the transmit interruption.
    ier |= IER_TX_RDY;
    outb(PC16550_IER(uart), ier);
}
