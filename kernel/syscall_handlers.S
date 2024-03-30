// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Syscall handlers implementation

.code16

.section .text

.extern breakpoint
.global int3_handler
int3_handler:
    mov %sp, %bp
    // Push directly accessible registers on the stack.
    push %sp
    push %ss
    push %es
    push %ds
    push %di
    push %si
    push %dx
    push %cx
    push %bx
    push %ax
    // Get other registers from the stack and push them for the call.
    mov (%bp), %ax  // IP
    push %ax
    mov 2(%bp), %ax // CS
    push %ax
    mov 4(%bp), %ax // Flags
    push %ax
    call breakpoint
    add $26, %sp
    iret

.extern syscall_int21
.global int21_handler
int21_handler:
    push %dx
    push %ax
    call syscall_int21
    add $4, %sp
    iret

.extern syscall_int80
.global int80_handler
int80_handler:
    push %si
    push %dx
    push %cx
    push %bx
    push %ax
    call syscall_int80
    add $10, %sp
    iret
