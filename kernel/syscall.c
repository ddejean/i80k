// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "syscall.h"

#include <stdint.h>

#include "board.h"
#include "console.h"
#include "interrupts.h"
#include "heap.h"

// Syscall interrupts handlers provided by syscall_handlers.S.
extern void int21_handler(void);
extern void int80_handler(void);

void syscall_setup(void) {
    interrupts_handle(0x21, int21_handler);
    interrupts_handle(0x80, int80_handler);
}

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

uint16_t syscall_int80(uint16_t nr, uint16_t arg0, uint16_t arg1, uint16_t arg2, uint16_t arg3, uint16_t arg4, uint16_t arg5) {
    uint16_t ret = (uint16_t)-1;

    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    (void)arg5;

    switch (nr) {
        case 0x0c: // brk
            ret = (uint16_t)heap_brk((void*)arg0);
            break;
    }
    return ret;
}