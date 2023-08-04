// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

#include "syscall.h"

#include "interrupts.h"

// Syscall interrupts handlers provided by syscall_handlers.S.
extern void int21_handler(void);

void syscall_setup(void) {
    interrupts_handle(0x21, int21_handler);
}