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

// size_t strlen(const char* s);
.global strlen
strlen:
    push    %bp
    mov     %sp, %bp
    push    %di
    // Search for '\0'
    mov     4(%bp), %dx
    mov     %dx, %di
    mov     $0xffff, %cx
    xor     %ax, %ax
    repnz   scasb
    // Get the distance between the begining of the string and the '\0'.
    sub     %dx, %di
    lea     -1(%di), %ax
    // Restore state and return
    pop     %di
    pop     %bp
    ret
