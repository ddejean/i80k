; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

bits 16

section .text

; void * memset(void *dst, int value, size_t count);
global _memset
_memset:
    push    bp
    mov     bp, sp
    ; Save nonscratch registers.
    push    di
    push    cx
    ; Set destination, value and count.
    mov     di, [bp+4]
    mov     ax, [bp+6]
    mov     cx, [bp+8]
    ; Trigger the copy.
    cld
    rep stosb
    ; Restore registers.
    pop     cx
    pop     dx
    ; Set the return value.
    mov     ax, [bp+4]
    pop     bp
    ; Restore base point and leave.
    ret
