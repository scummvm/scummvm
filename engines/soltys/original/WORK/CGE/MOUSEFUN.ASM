EVT_MAX	=	256

	.model	small

	public	@MOUSE@NewMouseFun$qv


EVENT	struc
  M	dw	0
  X	dw	0
  Y	dw	0
  P	dw	0
EVENT	ends

	extrn	_EvtHead:word
	extrn	_EvtTail:word
	extrn	_Evt:EVENT:EVT_MAX
	extrn	@SpriteAt$qii:near

REMARK	=	0
L_DOWN	=	1
L_UP	=	2
R_DOWN	=	3
R_UP	=	4
ROLL	=	5

	.data
;	~~~~~~

over	db	0


	.code
;	~~~~~~

@MOUSE@NewMouseFun$qv	proc	far
	push	ds
	push	ax
	mov	ax,DGROUP
	mov	ds,ax
	pop	ax

;--- check/set overrun flag
	cmp	over,0
	jne	xit
	inc	over

;--- fix data format
	shr	cx,1		; horizontal position
;;--- currently unused:
;;	mov	bh,bl		; button status:
;;	shr	bh,1		;  - right in bh,
;;	and	bl,1		;  - left in bl

;--- fill current event entry with mask and position
	mov	bx,_EvtHead
	shl	bx,3		; * 8
	mov	_Evt[bx].M,ax	; event mask
	mov	_Evt[bx].X,cx	; column
	mov	_Evt[bx].Y,dx	; row
	push	bx

;--- take sprite at point
	push	dx		; row
	push	cx		; col
	call	@SpriteAt$qii
	pop	bx
	pop	bx
	pop	bx
	mov	_Evt[bx].P,ax	; SPRITE pointer

;--- update queue head pointer
	inc	byte ptr _EvtHead	; (255 + 1 == 0)

;--- reset overrun flag
	dec	over

;--- return form mouse user routine
xit:
	pop	ds
	ret
@MOUSE@NewMouseFun$qv	endp


	end
