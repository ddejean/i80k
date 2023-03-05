; This file contains the BCC compiler helper functions */
; (C) Copyright Bruce Evans */
; Support for integer arithmetic
; __idiv.o __idivu.o __imod.o __imodu.o __imul.o __isl.o __isr.o __isru.o

bits 16

section .text

; Function idiv */
; idiv.s
; idiv_ doesn`t preserve dx (returns remainder in it)

global idiv_

idiv_:
	cwd
	idiv	bx
	ret


; Function idivu
; idivu.s
; idiv_u doesn`t preserve dx (returns remainder in it)

global idiv_u
idiv_u:
	xor	dx,dx
	div	bx
	ret


; Function imod
; imod.s
; imod doesn`t preserve dx (returns quotient in it)

global imod
imod:
	cwd
	idiv	bx
	mov	ax,dx
	ret


; Function imodu
; imodu.s
; imodu doesn`t preserve dx (returns quotient in it)

global imodu
imodu:
	xor	dx,dx
	div	bx
	mov	ax,dx		; instruction queue full so xchg slower
	ret


; Function imul
; imul.s
; imul_, imul_u don`t preserve dx

global imul_
global imul_u
imul_:
imul_u:
	imul	bx
	ret


; Function isl
; isl.s
; isl, islu don`t preserve cl

global isl
global islu
isl:
islu:
	mov	cl,bl
	shl	ax,cl
	ret


; Function isr
; isr.s
; isr doesn`t preserve cl

global isr
isr:
	mov	cl,bl
	sar	ax,cl
	ret


; Function isru */
; isru.s
; isru doesn`t preserve cl

global isru
isru:
	mov	cl,bl
	shr	ax,cl
	ret
