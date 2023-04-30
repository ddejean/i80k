// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Definitions specific to the board on which the kernel runs.

#ifndef _BOARD_H_
#define _BOARD_H_

// Kernel code segment used during bootstrap.
// The start code is supposed to run at address xxxx:0400 but the ROM is
// physically located at address F000:8000. The segment is calculated to
// include the offset until the ROM is relocated.
// CS = 0xF000 + (0x8000 - 0x400) >> 4
#define KERNEL_CS 0xF7C0

// Kernel segments, keep in sync with crt0.S
#define KERNEL_DS 0x0000
#define KERNEL_SS 0x0000

// Lower address of the kernel stack.
#define KERNEL_STACK_LOW 0xF000

// IRQ offset in the interrupt descriptor table.
#define IDT_IRQ_OFFSET 32

// IO ports
#define PORT_PIC 0x20
#define PORT_PIT 0x40
#define PORT_DEBUG 0x80
#define PORT_UART 0x3F8

// PIT clock frequency: 1.25Mhz
#define PIT_FREQ 1250000U

// Symbols created by the linker to help manipulating the binary sections.
// Note: guarded by a macro as this file is also included in .S files.
#ifndef __ASSEMBLER__
extern char _text_start[];
extern char _text_end[];
extern char _data_start[];
extern char _data_end[];
extern char _bss_start[];
extern char _bss_end[];
#endif

#endif  // _BOARD_H_
