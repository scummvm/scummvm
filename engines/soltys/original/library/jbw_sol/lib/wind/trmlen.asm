	.model	small,c
	.code

	public	MemTrmLen

;----------------------------------------------------------------------------



MemTrmLen	proc	mem:near ptr byte,len:word
		push	di es
		push	ds
		pop	es		; load data segment
		mov	di,mem		; address of string
		mov	cx,len          ; length of string
		add	di,cx
		dec	di		; last char at len-1
		mov	al,' '		; look for not space
		std			; scan from last to first
		repe scasb		; go!
		je	mtl_xit		; not found - return 0
		inc	cx		; found - fix result
mtl_xit:	mov	ax,cx		; return value in AX
		pop	es di
		ret
MemTrmLen	endp

;----------------------------------------------------------------------------

		end
