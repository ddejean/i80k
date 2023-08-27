// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "syscall.h"

#include <stdint.h>

#include "board.h"
#include "console.h"
#include "heap.h"
#include "interrupts.h"

// Syscall interrupts handlers provided by syscall_handlers.S.
extern void int21_handler(void);
extern void int80_handler(void);

void syscall_setup(void) {
    interrupts_handle(0x21, int21_handler);
    interrupts_handle(0x80, int80_handler);
}

int syscall_int21(uint16_t ax, uint16_t dx) {
    int ret = -1;
    uint8_t ah = ax >> 8;
    switch (ah) {
        case 0x01:
            ret = console_getchar();
            break;
        case 0x02:
            console_putchar((int)dx);
            break;
        case 0x09:
            ret = console_puts((const char*)dx);
            break;
    }
    return ret;
}

int syscall_int80(uint16_t nr, uint16_t arg0) {
    int ret = -1;

    switch (nr) {
        case 0x0c:  // brk
            ret = (int)heap_brk((void*)arg0);
            break;
    }
    return ret;
}