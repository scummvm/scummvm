	.model	small

RATE	=	4



;**	extrn	_Scb			: SCBS
	extrn	_TimerProc		: near
	public	@ENGINE@NewTimer$qve

	.data

cntr1	db	RATE
flag	db	0

	.code

   ;
   ;	void interrupt ENGINE::NewTimer (...)
   ;
@ENGINE@NewTimer$qve	proc	far

	push	ax bx ds es
	mov	ax,DGROUP
	mov	ds,ax

; send E-O-I
	mov	al,20h
	out	020h,al
	sti				; enable interrupts

	cmp	word ptr DGROUP:_Scb.Siz,0
	jne	short count
	cmp	word ptr DGROUP:_Scb.Nxt,0
	je	short count

count:
	dec	cntr1
	jnz	xit
	mov	cntr1,RATE1

	push	cx dx
	call	_TimerProc
	pop	dx cx

	dec	cntr2
	jnz	xit
	mov	cntr2,RATE2

	mov	ax,40h
	mov	ds,ax
	inc	word ptr ds:[6Ch]
	jne	xit
	inc	word ptr ds:[6Eh]
xit:
	pop	es ds bx ax
	iret	
@ENGINE@NewTimer$qve	endp

	end
