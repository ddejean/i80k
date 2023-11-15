// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Provide the basic fonctions to output char and strings on the current binded
// stdout.

#include <stdint.h>
#include <string.h>

#include "board.h"
#include "utils/ringbuffer.h"

#ifdef BOARD_8088_REV2
#include "p8251.h"
#define uart_initialize p8251_initialize
#define uart_start_xmit p8251_start_xmit
#endif

#ifdef BOARD_8088_REV3
#include "pc16550.h"
#define uart_initialize pc16550_initialize
#define uart_start_xmit pc16550_start_xmit
#endif

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

void console_initialize(void) {
    ring_buffer_init(&rx_ring, rx_buf, sizeof(rx_buf));
    ring_buffer_init(&tx_ring, tx_buf, sizeof(tx_buf));
}

void console_bind_uart(void) {
    uart_initialize(&rx_ring, &tx_ring, 19200);
    if (!ring_buffer_is_empty(&tx_ring)) {
        uart_start_xmit();
    }
}

int console_putchar(int c) {
    if ((char)c == '\n') {
        ring_buffer_queue(&tx_ring, '\r');
    }
    ring_buffer_queue(&tx_ring, (const char)c);
    uart_start_xmit();
    return 1;
}

int console_puts(const char *s) {
    int len = 0;
    const char br[2] = {'\r', '\n'};

    len = strlen(s);
    ring_buffer_queue_arr(&tx_ring, s, len);
    ring_buffer_queue_arr(&tx_ring, br, sizeof(br));
    uart_start_xmit();
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