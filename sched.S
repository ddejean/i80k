; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
;
; Contains the set of interrupt handlers called by the CPU from the IDT.

bits 16

section .text

; The function to call when the interruption context is saved.
extern _schedule

; Scheduling interrupt routing.
global _schedule_handler
_schedule_handler:
    ; Save current activity state.
    push ax

    mov ax,  0x20
    out 0,  al

    push bx
    push cx
    push dx
    push di
    push si
    push ds
    push es

    ; Call the C interrupt handler.
    call _schedule

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

; Context switch routine
global _schedule_switch
_schedule_switch:
    push    bp
    mov     bp, sp

    ; Save the current task context on the stack.
    push si
    push di
    push ds
    push es
    pushf

    ; Save the current stack segment and pointer.
    mov     bx,     [bp+4]      ; Current task
    mov     [bx],   sp
    mov     [bx+2], ss

    ; Set the new stack segment and pointer.
    mov     bx,     [bp+6]      ; Next task
    mov     sp,     [bx]
    mov     ss,     [bx+2]

    ; Restore the new task context from the stack.
    popf
    pop es
    pop ds
    pop di
    pop si

    ; Restore the base pointer
    pop bp

    ; Skip the return to the caller and jump directly to the "super" caller.
    ret         ; Return to the super caller
