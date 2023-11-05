// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _PC16550_H_
#define _PC16550_H_

#include <stddef.h>
#include <stdint.h>

// pc16550_initialize prepares the UART for sending and receiving using buffers
// and interruptions.
void pc16550_initialize(uint16_t baud_rate);

// pc16550_read reads <count> bytes of data from the wire and puts them into
// <buffer>.
int pc16550_read(const char *buffer, const size_t count);

// pc16550_putchar sends <c> on the wire.
int pc16550_putchar(const char c);

// pc16550_write sends <count> bytes from <buffer> onto the wire.
int pc16550_write(const char *buffer, const size_t count);

#endif  // _PC16550_H_
