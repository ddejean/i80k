// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>
//
// Contains the set of interrupt handlers called by the CPU from the IDT.

.code16

.section .text

// The function to call when the interruption context is saved.
.extern schedule

// Scheduling interrupt routing.
.global schedule_handler
schedule_handler:
    // Save current activity state.
    push %ax

    mov $0x20, %ax
    outb %al, $0x0

    push %bx
    push %cx
    push %dx
    push %di
    push %si
    push %ds
    push %es

    // Call the C interrupt handler.
    call schedule

    // Restore current activity state.
    pop %es
    pop %ds
    pop %si
    pop %di
    pop %dx
    pop %cx
    pop %bx
    pop %ax
    iret

// Context switch routine
.global schedule_switch
schedule_switch:
    push    %bp
    mov     %sp, %bp

    // Save the current task context on the stack.
    push %si
    push %di
    push %ds
    push %es
    pushf

    // Save the current stack segment and pointer.
    mov     4(%bp), %bx      // Current task
    mov     %sp, (%bx)
    mov     %ss, 2(%bx)

    // Set the new stack segment and pointer.
    mov   6(%bp), %bx          // Next task
    mov   (%bx),  %sp     
    mov   2(%bx), %ss     

    // Restore the new task context from the stack.
    popf
    pop %es
    pop %ds
    pop %di
    pop %si

    // Restore the base pointer
    pop %bp

    // Skip the return to the caller and jump directly to the "super" caller.
    ret         // Return to the super caller
