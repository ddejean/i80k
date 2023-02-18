// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

// outb emits <data> on io address <port>.
void outb(uint16_t port, uint8_t data);

// inb retries a byte from io address <port>.
uint8_t inb(uint16_t port);

// cli clears the interrupts flag which disables the interruptions.
void cli(void);

// sti sets the interrupts flag which enables the interruptions.
void sti(void);

// hlt pauses the processor until an interruption.
void hlt(void);

#endif  // _CPU_H_
