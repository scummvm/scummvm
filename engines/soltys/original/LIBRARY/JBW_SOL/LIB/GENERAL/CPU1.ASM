; CPU type check



_8086	=	0
_80186	=	1
_80286	=	2
_80386	=	3
_80486	=	4


DOS	=	21h


	.model	small

	public	@Cpu$qv


	.data

oldInt6	dw	2 dup (?)


	.code

@Cpu$qv	proc	near

; trap interrupt #6
	push	es
	mov	ax,3506h	; get vector
	int	DOS
	mov	oldInt6[0],bx
	mov	oldInt6[2],es
	pop	es

	push	ds
	mov	ax,2506h	; set vector
	push	cs
	pop	ds
	mov	dx,offset newInt6
	int	DOS
	pop	ds

	xor	ax,ax		; 0 = 8086/88
	push	sp		; 8086 increments SP before pushing!
	pop	dx		; take pushed value
	cmp	dx,sp		; same as SP?
	jne	short xit	; no: exit now with answer = 0

; try to execute specific opcodes
	mov	ax,-1		; not 8086 and below 80186 ??

	.186
	shl	dx,5		; >= 80186
	mov	ax,1		; passed above test

	.286P
	smsw	dx		; >= 80286
	inc	ax		; passed above test

	.386P
	mov	edx,cr0		; >= 80386
	inc	ax		; passed above test

	.486
	xadd	dx,dx		; >= 80486
	inc	ax		; passed above test

	.8086

; restore original int #6 vector

xit:	push	ax		; preserve return value
	push	ds
	mov	ax,2506h	; set vector
	lds	dx,dword ptr oldInt6
	int	DOS
	pop	ds

	pop	ax		; return value

	ret
	endp



newInt6	proc	far
	pop	dx			; take IP
	mov	dx,offset cs:xit	; change IP
	push	dx			; put updated IP
	iret
	endp



	end
