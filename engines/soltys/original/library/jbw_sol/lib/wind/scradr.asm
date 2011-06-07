	.model	small,c
	.code
;----------------------------------------------------------------------------

adrsiz	equ	2
llen	equ	80
lsiz	equ	2*llen

IsMono	proc
	push	es
	mov	ax,40H
	mov	es,ax
	xor	ax,ax
	cmp	byte ptr es:[49H],07H	; Mono?
	jne	popes
	inc	ax
popes:	pop	es
	ret
IsMono	endp

;----------------------------------------------------------------------------

ScrAdr	proc	x:word,y:word
;--- video segment
	call	IsMono
	mov	dx,0B000H		; Mono
	or	ax,ax
	jne	so
	mov	dx,0B800H		; Color
;--- screen offset
so:	mov	ax,y			; Row #
	mov	ah,lsiz
	mul	ah
	add     ax,x			; Col #
	add     ax,x			; Col #
        ret
ScrAdr	endp

;----------------------------------------------------------------------------

public	IsMono
public	ScrAdr

        end
