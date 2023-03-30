// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

// ksegmemcpy(void *dst, uint16_t dst_seg, void *src, uint16_t src_seg, size_t n)
.global ksegmemcpy
ksegmemcpy:
    push    %bp
    mov     %sp, %bp
    // Save registers.
    push    %ds
    push    %es
    push    %si
    push    %di
    push    %cx
    // Load copy parameters.
    les     4(%bp), %di
    lds     8(%bp), %si
    mov     12(%bp), %cx
    // Trigger the copy.
    cld
    rep movsb
    // Restore registers.
    pop     %cx
    pop     %di
    pop     %si
    pop     %es
    pop     %ds
    // Restore base point and leave.
    pop     %bp
    ret
