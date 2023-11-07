// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _PC16550_H_
#define _PC16550_H_

#include <stddef.h>
#include <stdint.h>

#include "utils/ringbuffer.h"

// pc16550_initialize prepares the UART for sending and receiving using buffers
// and interruptions.
void pc16550_initialize(ring_buffer_t *rx_ring, ring_buffer_t *tx_ring,
                        uint16_t baud_rate);

// pc16550_start_xmit notifies the UART driver there's data in the tx queue.
void pc16550_start_xmit(void);

#endif  // _PC16550_H_
