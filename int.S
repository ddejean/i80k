; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
;
; Contains the set of interrupt handlers called by the CPU from the IDT.

bits 16

section .text

; int_handler <asm_handler_label> <C_handler label> is a macro helper to
; declare an interrupt handler that calls the C world.
%macro int_handler 2
    ; External symbol to call when the current context is saved.
    extern _%2

    ; Interrupt handler itself
    global _%1
    _%1:
        ; Save current activity state.
        push ax
        push bx
        push cx
        push dx
        push di
        push si
        ; Call the C interrupt handler.
        call _%2
        ; Restore current activity state.
        pop si
        pop di
        pop dx
        pop cx
        pop bx
        pop ax
        iret
%endmacro

; Handler for IRQ0
int_handler irq0, handler_irq0
int_handler irq1, handler_irq1
