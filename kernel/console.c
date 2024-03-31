// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>
#include <string.h>

#include "board.h"
#include "ringbuffer.h"
#include "uart.h"

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
// Tells if the UART is binded to the console.
static int binded;

static inline void console_start_xmit(void) {
    if (binded) {
        uart_start_xmit();
    }
}

void console_initialize(void) {
    ring_buffer_init(&rx_ring, rx_buf, sizeof(rx_buf));
    ring_buffer_init(&tx_ring, tx_buf, sizeof(tx_buf));
    binded = 0;
}

void console_bind_uart(void) {
    uart_initialize(&rx_ring, &tx_ring, 38400);
    binded = 1;
    if (!ring_buffer_is_empty(&tx_ring)) {
        console_start_xmit();
    }
}

int console_putchar(int c) {
    if ((char)c == '\n') {
        ring_buffer_queue(&tx_ring, '\r');
    }
    ring_buffer_queue(&tx_ring, (const char)c);
    console_start_xmit();
    return 1;
}

int console_puts(const char *s) {
    int len = 0;
    const char br[2] = {'\r', '\n'};

    len = strlen(s);
    ring_buffer_queue_arr(&tx_ring, s, len);
    ring_buffer_queue_arr(&tx_ring, br, sizeof(br));
    console_start_xmit();
    return len + sizeof(br);
}

int console_getchar(void) {
    char c;

    if (ring_buffer_is_empty(&rx_ring)) {
        return -1;
    }
    ring_buffer_dequeue(&rx_ring, &c);
    return (int)c & 0xff;
}