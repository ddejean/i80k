// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _UART_C_
#define _UART_C_

#include <stddef.h>
#include <stdint.h>

// uart_initialize prepares the UART for sending and receiving.
void uart_initialize(void);

// uart_read reads <count> bytes of data from the wire and puts them into
// <buffer>.
int uart_read(uint8_t *buffer, size_t count);

// uart_write sends <count> bytes from <buffer> onto the wire.
int uart_write(uint8_t *buffer, size_t count);

#endif  // _UART_C_
