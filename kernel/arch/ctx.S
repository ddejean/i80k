// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

.global ctx_switch
ctx_switch:
    push    %bp
    mov     %sp, %bp

    // Save all non-scratch registers on the |prev| stack.
    push    %si
    push    %di
    push    %ds
    push    %es
    pushf

    // Get the context structs.
    mov     4(%bp), %di
    mov     6(%bp), %si

    // Save stack to the |prev| context.
    mov     %sp, (%di)
    mov     %ss, 2(%di)

    // Restore stack from the |next| context.
    mov     (%si), %sp
    mov     2(%si), %ss

    // Restore all non scratch registers from the |next| stack.
    popf
    pop     %es
    pop     %ds
    pop     %di
    pop     %si

    pop     %bp
    ret