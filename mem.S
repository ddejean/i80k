; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

bits 16

section .text

; ksegmemcpy(void *dst, uint16_t dst_seg, void *src, uint16_t src_seg, size_t n)
global _ksegmemcpy
_ksegmemcpy:
    push    bp
    mov     bp, sp
    ; Save registers.
    push    ds
    push    es
    push    si
    push    di
    push    cx
    ; Load copy parameters.
    les     di, [bp+4]
    lds     si, [bp+8]
    mov     cx, [bp+12]
    ; Trigger the copy.
    cld
    rep movsb
    ; Restore registers.
    pop     cx
    pop     di
    pop     si
    pop     es
    pop     ds
    ; Restore base point and leave.
    pop     bp
    ret
