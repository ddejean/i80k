; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

bits 16

section .text

; void * kmemset(void *dst, int value, size_t count);
global _kmemset
_kmemset:
    push    bp
    mov     bp, sp
    ; Save nonscratch registers.
    push    di
    push    cx
    ; Set destination, value and count.
    mov     di, 4[bp]
    mov     ax, 6[bp]
    mov     cx, 8[bp]
    ; Trigger the copy.
    cld
    rep stosb
    ; Restore registers.
    pop     cx
    pop     dx
    ; Set the return value.
    mov     ax, 6[bp]
    pop     bp
    ret
