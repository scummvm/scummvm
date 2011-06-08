_TEXT	segment	byte public 'CODE'
;----------------------------------------------------------------------------
        extrn	_ScrAdr:near
	extrn	_Attr:word

        public  _SayMemD


adrsiz	equ	2
llen	equ	80
lsiz	equ	2*llen

	assume	cs:_TEXT

; parameters: x, y, *str, len
;----------------------------------------------------------------------------
_SayMemD proc	near
	push	bp
	mov	bp,sp
;--- save some registers
	push	es
	push	si
	push	di
;--- screen address
	mov	ax,[bp+adrsiz+4]	; 1st row #
	push	ax
	mov	ax,[bp+adrsiz+2]	; 1st col #
	push	ax
	call	_ScrAdr
	add	sp,4
	mov	es,dx
	mov	di,ax
;--- string address
	mov	si,[bp+adrsiz+6]	; string addr
;--- display
	cld
	mov	ax,_Attr
	mov	cx,[bp+adrsiz+8]	; text length
	or	cx,cx
	je	ss_e
dsp_s:	lodsb
	stosw
	loop	dsp_s
;--- restore some regs
ss_e:	pop     di
        pop     si
        pop     es
	pop	bp
	ret
_SayMemD endp

;----------------------------------------------------------------------------
_TEXT	ends

        end
