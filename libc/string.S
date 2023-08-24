// Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

// void * memset(void *dst, int value, size_t count);
.global memset
memset:
    push    %bp
    mov     %sp, %bp
    // Save nonscratch registers.
    push    %es
    push    %di
    // Prepare the %es register.
    mov     %ds,    %di
    mov     %di,    %es
    // Set destination, value and count.
    mov     4(%bp), %di
    mov     6(%bp), %ax
    mov     8(%bp), %cx
    // Trigger the copy.
    cld
    rep stosb
    // Set the return value.
    mov     4(%bp), %ax
    // Restore registers.
    pop     %di
    pop     %es
    pop     %bp
    // Restore base point and leave.
    ret

// data_copy copies the .text section of the kernel to the future location of
// the kernel in RAM.
.global memcpy
memcpy:
    push %bp
    mov  %sp, %bp
    // Save non scratch registers.
    push %di
    push %si
    push %es
    // Prepare the %es segment register.
    mov  %ds, %di
    mov  %di, %es
    // Copy
    mov  4(%bp), %di
    mov  6(%bp), %si
    mov  8(%bp), %cx
    cld
    rep movsb
    // Restore the context.
    pop %es
    pop %si
    pop %di
    pop %bp
    ret

// size_t strlen(const char* s);
.global strlen
strlen:
    push    %bp
    mov     %sp, %bp
    push    %di
    push    %es
    // Put the pointer in destination index.
    mov     4(%bp), %bx
    mov     %bx,    %di
    // Load the data segment into %es.
	mov     %ds,    %ax
	mov     %ax,    %es
    // Keep the origin for later.
	mov 	%di,	%bx
    // Search for '\0'.
	xorb	%al,	%al
	mov     $-1,    %cx
	repne   scasb
    // Compute length.
	mov     %di,    %ax
	sub 	%bx,	%ax
	dec     %ax
    // Restore state and return
    pop     %es
    pop     %di
    pop     %bp
    ret

// size_t strnlen(const char *s, size_t maxlen);
.global strnlen
strnlen:
    push    %bp
    mov     %sp, %bp
    push    %di
    push    %es
    // Put the pointer in destination index.
    mov     4(%bp), %bx
    mov     %bx,    %di
    // Load the data segment into %es.
	mov     %ds,    %ax
	mov     %ax,    %es
    // Search for '\0'.
	xorb	%al,	%al
	mov     6(%bp), %cx
	repne   scasb
    // Compute length.
	mov     %di,    %ax
	sub 	%bx,	%ax
	dec     %ax
    // Restore state and return
    pop     %es
    pop     %di
    pop     %bp
    ret