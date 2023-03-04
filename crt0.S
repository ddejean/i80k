; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

bits 16

section .text

; _kernel C symbol has to be declared as coming from another object file.
extern _kernel

; _start is the begining of the ROM, it has to be declared in a special way
; to ensure the linker sees it.
global _start
..start:
    ; Set the stack segment and pointer at the end of the RAM.
    mov ax, 0x0      ; Stack segment: first segment where kernel is located.
    mov ss, ax
    mov ax, 0x0      ; Stack: stored at the end of the first segment
    mov sp, ax
    mov ax, 0x0      ; Data: first memory segment.
    mov ds, ax
    mov es, ax
    ; Call the C space
    push cs          ; Provide the code segment to the C world.
    call _kernel
    add sp, 0x2
    ; Should never be reached, but in case hang forever.
    cli
    hlt
