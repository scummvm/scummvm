	.model	small,c
	.code

	public  Video

VInt	equ	10h
SP_S	dw	?

Video	proc
	push	bx bp si di es
	xor	bx,bx		; video page #0
	mov	cs:SP_S,sp
	int	VInt
	mov	sp,cs:SP_S
	pop	es di si bp bx
	ret
Video	endp

        end
