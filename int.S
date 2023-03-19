; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
;
; Contains the various of interrupt handlers called by the CPU from the IDT.

bits 16

section .text

; The function to call when the context is saved.
extern _uart_handler

; Macro that defines an IRQ handler.
global _uart_int_handler
_uart_int_handler:
    ; Save current activity state.
    push ax
    push bx
    push cx
    push dx
    push di
    push si
    push ds
    push es
    ; Call the C interrupt handler.
    call _uart_handler
    ; Restore current activity state.
    pop es
    pop ds
    pop si
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    iret
