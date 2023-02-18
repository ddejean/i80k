// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Definitions specific to the board on which the kernel runs.

#ifndef _BOARD_H_
#define _BOARD_H_

// Kernel segments, keep in sync with crt0.S
#define KERNEL_CS 0xF000
#define KERNEL_DS 0x0000

// Lower address of the kernel stack.
#define KERNEL_STACK_LOW 0xF000

// IRQ offset in the interrupt descriptor table.
#define IDT_IRQ_OFFSET 32

// IO ports
#define PORT_PIC 0x0
#define PORT_DEBUG 0x2000

#endif  // _BOARD_H_
