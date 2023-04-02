// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _UART_C_
#define _UART_C_

#include <stddef.h>
#include <stdint.h>

// uart_early_initialize prepares the UART for sending using a polling method,
// suitable for early boot when the interrupt controller and the memory
// management are not ready yet.
void uart_early_initialize(void);

// uart_initialize prepares the UART for sending and receiving using buffers
// and interruptions.
void uart_initialize(void);

// uart_read reads <count> bytes of data from the wire and puts them into
// <buffer>.
int uart_read(const char *buffer, const size_t count);

// uart_write sends <count> bytes from <buffer> onto the wire.
int uart_write(const char *buffer, const size_t count);

#endif  // _UART_C_
