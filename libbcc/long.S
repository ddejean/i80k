; This file contains the BCC compiler helper functions */
; (C) Copyright Bruce Evans */
; Support for long arithmetic on little-endian (normal) longs
; __laddl.o __landl.o __lcmpl.o __lcoml.o __ldecl.o __ldivl.o __ldivul.o
; __leorl.o __lincl.o __lmodl.o __lmodul.o __lmull.o __lnegl.o __lorl.o
; __lsll.o __lsrl.o __lsrul.o __lsubl.o __ltstl.o

bits 16

section .text

; Function laddl

global	laddl
global	laddul
laddl:
laddul:
	add	ax,[di]
	adc	bx,2[di]
	ret


; Function landl
; landl.s

global	landl
global	landul
landl:
landul:
	and	ax,[di]
	and	bx,2[di]
	ret


; Function lcmpl */
; lcmpl.s
; lcmpl, lcmpul don`t preserve bx

global	lcmpl
global	lcmpul

lcmpl:
lcmpul:
	sub	bx,2[di]
	je	LCMP_NOT_SURE
	ret

LCMP_NOT_SURE:
	cmp	ax,[di]
	jb	LCMP_B_AND_LT
	jge	LCMP_EXIT

	inc	bx
LCMP_EXIT:
	ret

LCMP_B_AND_LT:
	dec	bx
	ret


; Function lcoml */
; lcoml.s

global	lcoml
global	lcomul

lcoml:
lcomul:
	not	ax
	not	bx
	ret


; Function ldecl
; ldecl.s

global	ldecl
global	ldecul

ldecl:
ldecul:
	cmp [bx], word 0
	je	LDEC_BOTH
	dec word [bx]
	ret



LDEC_BOTH:
	dec	word [bx]
	dec	word 2[bx]
	ret


; Function ldivl
; ldivl.s
; bx:ax / 2(di):(di), quotient bx:ax, remainder di:cx, dx not preserved

extern	ldivmod
global	ldivl

ldivl:
	mov	cx,[di]
	mov	di,2[di]
	call	ldivmod
	xchg	ax,cx
	xchg	bx,di
	ret


; Function ldivul */
; ldivul.s
; unsigned bx:ax / 2(di):(di), quotient bx:ax,remainder di:cx, dx not preserved

extern	ludivmod
global	ldivul

ldivul:
	mov	cx,[di]
	mov	di,2[di]
	call	ludivmod
	xchg	ax,cx
	xchg	bx,di
	ret


; Function leorl */
; leorl.s

global	leorl
global	leorul
leorl:
leorul:
	xor	ax,[di]
	xor	bx,2[di]
	ret


; Function lincl */
global	lincl
global	lincul

lincl:
lincul:
	inc	word [bx]
	je	LINC_HIGH_WORD
	ret

LINC_HIGH_WORD:
	inc	word 2[bx]
	ret


; Function lmodl
; lmodl.s
; bx:ax % 2(di):(di), remainder bx:ax, quotient di:cx, dx not preserved

extern	ldivmod
global	lmodl

lmodl:
	mov	cx,[di]
	mov	di,2[di]
	call	ldivmod
	ret


; Function lmodul */
; lmodul.s
; unsigned bx:ax / 2(di):(di), remainder bx:ax,quotient di:cx, dx not preserved

extern	ludivmod
global	lmodul

lmodul:
	mov	cx,[di]
	mov	di,2[di]
	call	ludivmod
	ret


; Function lmull */
; lmull.s
; lmull, lmulul don`t preserve cx, dx

global	lmull
global	lmulul
lmull:
lmulul:
	mov	cx,ax
	mul	word 2[di]
	xchg	ax,bx
	mul	word [di]
	add	bx,ax
	mov	ax, [di]
	mul	cx
	add	bx,dx
	ret


; Function lnegl */
; lnegl.s

global	lnegl
global	lnegul
lnegl:
lnegul:
	neg	bx
	neg	ax
	sbb	bx,0
	ret


; Function lorl
; lorl.s

global	lorl
global	lorul

lorl:
lorul:
	or	ax,[di]
	or	bx,2[di]
	ret


; Function lsll
; lsll.s
; lsll, lslul don`t preserve cx

global	lsll
global	lslul

lsll:
lslul:
	mov	cx,di
	jcxz	LSL_EXIT
	cmp	cx, 32
	jae	LSL_ZERO
LSL_LOOP:
	shl	ax, 1
	rcl	bx, 1
	loop	LSL_LOOP
LSL_EXIT:
	ret

LSL_ZERO:
	xor	ax,ax
	mov	bx,ax
	ret


; Function lsrl */
; lsrl.s
; lsrl doesn`t preserve cx

global	lsrl
lsrl:
	mov	cx, di
	jcxz	LSR_EXIT
	cmp	cx, 32
	jae	LSR_SIGNBIT
LSR_LOOP:
	sar	bx, 1
	rcr	ax, 1
	loop	LSR_LOOP
LSR_EXIT:
	ret

LSR_SIGNBIT:
	mov	cx, 32
	jmp	LSR_LOOP


; Function lsrul */
; lsrul.s
; lsrul doesn`t preserve cx

global	lsrul
lsrul:
	mov	cx,di
	jcxz	LSRU_EXIT
	cmp	cx, 32
	jae	LSRU_ZERO
LSRU_LOOP:
	shr	bx, 1
	rcr	ax, 1
	loop	LSRU_LOOP
LSRU_EXIT:
	ret

LSRU_ZERO:
	xor	ax,ax
	mov	bx,ax
	ret

; Function lsubl */
; lsubl.s

global	lsubl
global	lsubul
lsubl:
lsubul:
	sub	ax,[di]
	sbb	bx,2[di]
	ret

; Function ltstl
; ltstl.s
; ltstl, ltstul don`t preserve bx

global	ltstl
global	ltstul

ltstl:
ltstul:
	test	bx,bx
	je	LTST_NOT_SURE
	ret

LTST_NOT_SURE:
	test	ax,ax
	js	LTST_FIX_SIGN
	ret

LTST_FIX_SIGN:
	inc	bx
	ret
