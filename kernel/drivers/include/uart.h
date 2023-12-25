// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _UART_H_
#define _UART_H_

#include <stddef.h>
#include <stdint.h>

#include "ringbuffer.h"

// uart_initialize prepares the UART for sending and receiving using buffers
// and interruptions.
void uart_initialize(ring_buffer_t *rx_ring, ring_buffer_t *tx_ring,
                     uint16_t baud_rate);

// pc16550_start_xmit notifies the UART driver there's data in the tx queue.
void uart_start_xmit(void);

#endif  // _P8251_H_
