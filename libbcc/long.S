; Copyright (C) 2023 - Damien Dejean <dam.dejean@gmail.com>

bits 16

section .text

global	lcmpl
lcmpl:
global	lcmpul
lcmpul:
	sub	bx, 2[di]
	je	lcmp_not_sure
	ret

lcmp_not_sure:
	cmp	ax,[di]
	jb	lcmp_b_and_lt
	jge	lcmp_exit
	inc	bx

lcmp_exit:
	ret

lcmp_b_and_lt:
	dec	bx
	ret
