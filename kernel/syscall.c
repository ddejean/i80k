// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "syscall.h"

#include <stdint.h>
#include <stdio.h>

#include "board.h"
#include "console.h"
#include "cpu.h"
#include "heap.h"
#include "interrupts.h"
#include "scheduler.h"

// Syscall interrupts handlers provided by syscall_handlers.S.
extern void int3_handler(void);
extern void int21_handler(void);
extern void int80_handler(void);

void syscall_setup(void) {
    interrupts_handle(0x3, KERNEL_CS, int3_handler);
    interrupts_handle(0x21, KERNEL_CS, int21_handler);
    interrupts_handle(0x80, KERNEL_CS, int80_handler);
}

void breakpoint(uint16_t flags, uint16_t cs, uint16_t ip, uint16_t ax,
                uint16_t bx, uint16_t cx, uint16_t dx, uint16_t si, uint16_t di,
                uint16_t ds, uint16_t es, uint16_t ss, uint16_t sp) {
    printf("Exception: breakpoint\n");

    // Print registers.
    printf("Registers:\n");
    printf("  CS: %04x  IP: %04x  Flags: %02x  SS: %04x  SP: %04x\n", cs, ip,
           flags & 0xff, ss, sp);
    printf("  AX: %04x  BX: %04x  CX: %04x  DX: %04x\n", ax, bx, cx, dx);
    printf("  SI: %04x  DI: %04x  DS: %04x  ES: %04x\n", si, di, ds, es);

    // Print the stack.
    uint16_t *stack = (uint16_t *)sp;
    printf("-------- STACK --------\n");
    printf("0x%04x [%04x]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x] [ra:offset]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x] [ra:segment]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x]\n", (uint16_t)stack, *stack);
    stack++;
    printf("0x%04x [%04x]\n", (uint16_t)stack, *stack);
    stack++;

    // Hang forever but keep interrupts enabled to be able to print messages.
    sti();
    while (1) {
        hlt();
    }
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
            ret = console_puts((const char *)dx);
            break;
    }
    return ret;
}

int syscall_int80(uint16_t nr, uint16_t arg0) {
    int ret = -1;

    switch (nr) {
        case 0x27:
            ret = scheduler_getpid();
            break;
        case 0x0c:  // brk
            ret = (int)heap_brk((void *)arg0);
            break;
        case 0x3c:
            scheduler_exit((int)arg0);
            break;
    }
    return ret;
}