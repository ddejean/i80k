// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

// void * memset(void *dst, int value, size_t count);
.global memset
memset:
    push    %bp
    mov     %sp, %bp
    // Save nonscratch registers.
    push    %di
    push    %cx
    // Set destination, value and count.
    mov     4(%bp), %di
    mov     6(%bp), %ax
    mov     8(%bp), %cx
    // Trigger the copy.
    cld
    rep stosb
    // Restore registers.
    pop     %cx
    pop     %dx
    // Set the return value.
    mov     4(%bp), %ax
    pop     %bp
    // Restore base point and leave.
    ret
