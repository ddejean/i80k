// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Definitions specific to the board on which the kernel runs.

#ifndef _BOARD_H_
#define _BOARD_H_

// IRQ offset in the interrupt descriptor table.
#define IDT_IRQ_OFFSET 32

// IO ports
#define PORT_PIC 0x0
#define PORT_DEBUG 0x2000

#endif  // _BOARD_H_
