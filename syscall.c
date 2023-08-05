// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "syscall.h"

#include <stdint.h>

#include "board.h"
#include "console.h"
#include "interrupts.h"

// Syscall interrupts handlers provided by syscall_handlers.S.
extern void int21_handler(void);

void syscall_setup(void) { interrupts_handle(0x21, int21_handler); }

void syscall_int21(uint16_t ax, uint16_t ds, uint16_t dx) {
    uint8_t ah = ax >> 8;
    switch (ah) {
        case 0x02:
            console_putchar((int)dx);
            break;
        case 0x09:
            if (ds == KERNEL_DS) {
                console_puts((const char*)dx);
            }
            break;
    }
}