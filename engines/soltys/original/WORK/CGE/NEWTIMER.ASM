	.model	small

RATE1	=	300
RATE2	=	4



SCBS	STRUC
	Adr	dd	?
	Siz	dw	?
	Nxt	dw	?
SCBS	ENDS

	extrn	_Scb			: SCBS
	extrn	_TimerProc		: near
	public	@ENGINE@NewTimer$qve

	.data

cntr1	dw	RATE1
cntr2	dw	RATE2
flag	db	0

	.code

   ;	
   ;	void interrupt ENGINE::NewTimer (...)
   ;	
@ENGINE@NewTimer$qve	proc	far

	push	ax bx ds es
	mov	ax,DGROUP
	mov	ds,ax

	cmp	word ptr DGROUP:_Scb.Siz,0
	je	short done

	  les	bx,dword ptr _Scb.Adr	; take pointer
	  mov	al,es:[bx]		; get data byte
	  xor	flag,0FFh		; flip flag
	  jz	odd			; odd action
	  shr	al,2
	  out	42h,al			; play even!
	  jnz	done			; always jump
odd:
	  inc	bx			; advance pointer
	  add	al,es:[bx]		; add adjacent
	  rcr	al,1			; average
	  shr	al,2	
	  out	42h,al			; play odd!
	  mov	word ptr _Scb.Adr,bx	; update pointer
	  dec	word ptr _Scb.Siz	; decrement size
done:
; send E-O-I
	mov	al,20h
	out	020h,al
	sti				; enable interrupts

	cmp	word ptr DGROUP:_Scb.Siz,0
	jne	short count
	cmp	word ptr DGROUP:_Scb.Nxt,0
	je	short count

	mov	bx,_Scb.Nxt
	mov	ax,[bx+0]
	mov	word ptr _Scb+0,ax
	mov	ax,[bx+2]
	mov	word ptr _Scb+2,ax
	mov	ax,[bx+4]
	mov	word ptr _Scb+4,ax
	mov	ax,[bx+6]
	mov	word ptr _Scb+6,ax
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
