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

// memcmp()
.global		memcmp
memcmp:
    push    %bp
    mov     %sp, %bp
	pushw	%si
	pushw	%di
	pushw	%es
	movw	%ds,	%si
	movw	%si,	%es
    mov     4(%bp), %di
    mov     6(%bp), %si
    mov     8(%bp), %cx
	movw	%cx,	%dx
	shrw	$1,	%cx
	repe	cmpsw
	jne	.L1
	testb	$1,	%dl
	jz	.L2
	cmpsb
	jne	.L3
.L2:
	xorw	%ax,	%ax
	popw	%es
	popw	%di
	popw	%si
	pop     %bp
    ret
.L3:
	movb	-1(%si),	%al
	xorb	%ah,	%ah
	movb	-1(%di),	%dl
	xorb	%dh,	%dh
	subw	%dx,	%ax
	popw	%es
	popw	%di
	popw	%si
	pop     %bp
    ret
.L1:
	movw	-2(%si),	%ax
	movw	-2(%di),	%dx
	cmpb	%al,	%dl
	je	.L4
	xorb	%ah,	%ah
	xorb	%dh,	%dh
	subw	%dx,	%ax
	popw	%es
	popw	%di
	popw	%si
	pop     %bp
    ret
.L4:
	movb	%ah,	%al
	movb	%dh,	%dl
	xorb	%ah,	%ah
	xorb	%dh,	%dh
	subw	%dx,	%ax
	popw	%es
	popw	%di
	popw	%si
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

// int strcmp(const char *s1, const char *s2)
.global strcmp
strcmp:
    push    %bp
    mov     %sp, %bp
    push    %si
    mov     4(%bp), %bx
    mov     %bx, %si
    mov     6(%bp), %bx
    dec     %bx
.loop:
    lodsb
    inc     %bx
    testb   %al, %al
    jz      .quit
    cmpb    (%bx), %al
    je      .loop
.quit:
    mov     (%bx), %dl
    xorb    %dh, %dh
    xorb    %ah, %ah
    sub     %dx, %ax
    pop     %si
    pop     %bp
    ret
