// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _P8251_H_
#define _P8251_H_

#include <stddef.h>
#include <stdint.h>

// uart_initialize prepares the UART for sending and receiving using buffers
// and interruptions.
void p8251_initialize(uint16_t baud_rate);

// uart_read reads <count> bytes of data from the wire and puts them into
// <buffer>.
int p8251_read(const char *buffer, const size_t count);

// uart_putchar sends <c> on the wire.
int p8251_putchar(const char c);

// uart_write sends <count> bytes from <buffer> onto the wire.
int p8251_write(const char *buffer, const size_t count);

#endif  // _P8251_H_
