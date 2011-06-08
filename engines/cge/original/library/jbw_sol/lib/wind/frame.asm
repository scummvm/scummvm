		.model	small,c

TRUE		equ		1
FALSE		equ		0


		.data

Lft		dw	-1
Top		dw	-1
Wid		dw	80
Hig		dw	25
Explode		dw	FALSE
Shadow		dw	FALSE
FrDes		db	"ÚÄ¿"
		db	"³ ³"
		db	"ÀÄÙ"
		db	"ÉÍ»"
		db	"º º"
		db	"ÈÍ¼"


	.code
;----------------------------------------------------------------------------
	extrn	Attr:word
	extrn	ScrAdr:near


adrsiz	equ	2
llen	equ	80
lsiz	equ	2*llen
NORMAL	equ	07h

;----------------------------------------------------------------------------

;void ScFram (int col, int row, int wid, int hig);

ScFram	proc	col:word,row:word,wid:word,hig:word
;--- save some registers
	push	es
	push	si
	push	di
;--- screen address
sa:	mov	ax,row
	push	ax
	mov	ax,col
	push	ax
	call	ScrAdr
	add	sp,4
	mov	es,dx
	mov	di,ax
;--- shadow
	mov	dx,Shadow
	or	dx,dx
	je	mf
	mov	dh,NORMAL
;--- make frame
mf:	mov	ax,Attr
	call	frm
;--- restore some regs
	pop	di
	pop	si
	pop     es
	ret
ScFram	endp


;--- frame routine
frm	proc
	cld
; upper line
	lea	si,ds:FrDes
	push	dx
	xor	dx,dx
	call	drwl
	pop	dx
; body lines
	add	si,3
	mov	cx,hig
	or	cx,cx
	jz	lastl
body:	call	drwl
	loop	body
; lower line
lastl:	add	si,3
	call	drwl
;--- shadow line
	or	dx,dx
	je	re
	add	di,4
	mov	cx,wid
	add	cx,2
	xchg	ax,dx
Shad:	mov	al,es:[di]
	stosw
	loop	Shad
	xchg	ax,dx
re:	ret

;--- draw horiz. line
drwl:	push	di
; 1st character
	lodsb				; char from design
	stosw
; body
	push	cx
	mov	cx,wid
	lodsb
	or	cx,cx
	je	popc
	rep stosw
popc:	pop	cx
; last character
	lodsb				; char from design
	sub	si,3
	stosw
	or	dx,dx
	je	ldrr
; shadow on right side
	xchg	ax,dx
	mov	al,es:[di]
	stosw
	mov	al,es:[di]
	stosw
	xchg	ax,dx
; restore regs & ret
ldrr:	pop	di
	add	di,lsiz
	ret
frm	endp

;----------------------------------------------------------------------------

	public	Explode, Shadow
	public	FrDes
	public	Lft, Top, Wid, Hig
	public  ScFram


	end
