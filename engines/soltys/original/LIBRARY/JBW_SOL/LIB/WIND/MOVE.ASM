	.model	small,c
	.code
;----------------------------------------------------------------------------
        public  Move

; moves memory block (up to 65535 bytes)
; using far pointers
;
; void Move (far * dst, far * src, unsigned len);
;

;----------------------------------------------------------------------------
Move	proc	dst:far ptr byte,src:far ptr byte,len:word
;--- save some registers
	push	ds
	push	si
	push	di
;--- take parameters
	mov	cx,len			; block length
	lds	si,src			; source addr
	les	di,dst			; destination address
;--- compare addresses, set direction
	cld
	xor	bx,bx
	cmp	si,di
	mov	ax,ds
	mov	dx,es
	sbb	ax,dx
	jnc	mov2
	std
	inc	bx
	add	si,cx
	add	di,cx
	cmpsw				; decrement si, di by 2
;--- just move!
mov2:	shr	cx,1			; length in words
	je	mov1
	rep movsw			; move string as words
mov1:	jnc	exit
	add	si,bx
	add	di,bx
	movsb				; mov odd byte
;--- restore some regs
exit:	cld
	pop	di
	pop	si
	pop	ds
	ret
Move	endp

;----------------------------------------------------------------------------

        end
