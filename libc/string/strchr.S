// Copyright (C) 2024 - Damien Dejean <dam.dejean@gmail.com>

.code16

.section .text

.global		strchr
strchr:
    push    %bp
    movw    %sp, %bp
	pushw	%si
    mov     4(%bp), %si
    movb    6(%bp), %ah
.strchr.loop:
	lodsb
	cmp	    %al, %ah
	jz	    .strchr.quit
	testb	%al, %al
	jnz	    .strchr.loop

	xorw	%ax,	%ax
	popw	%si
	popw    %bp
    ret
.strchr.quit:
	movw	%si,	%ax
	decw	%ax
	popw	%si
	popw    %bp
    ret
