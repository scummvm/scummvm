		.model	small


MAP_XCNT	=	40
MAP_ZCNT	=	20


CLUSTER	struc
  A	db	?
  B	db	?
CLUSTER	ends


BAR	struc
  Vert	db	?
  Horz	db	?
BAR	ends



		public	_Target,@Find1Way$qv

		extrn	_Barriers	: BAR
		extrn	_Now		: word
		extrn	@CLUSTER@Map	: byte
		extrn	_FindLevel	: word
		extrn	_Trace		: CLUSTER

		.data


_Target		dw	?
level		dw	0

tab		db	-1,  0
		db	 1,  0
		db	 0, -1
		db	 0,  1
tablen		=	$-tab

		.code



;-----------------------------------------------


chkbar		proc	near
		push	bx
		mov	bx,_Now
		add	bx,bx
		cmp	ch,_Barriers[bx].Horz
		je	cbx
		cmp	cl,_Barriers[bx].Vert
cbx:		pop	bx
		ret
chkbar		endp



;	row in CH, col in CL

@Find1Way$qv	proc	near
		push	bx cx dx si	; preserve registers
		mov	dx,cx
; take cell number
		mov	al,MAP_XCNT
		mul	ch
		add	al,cl
		adc	ah,0
		mov	si,ax
   ;	  if (c == Here) return TRUE;
		cmp	cx,_Target
		je	ok
   ;	  if (level >= FindLevel) return FALSE;	// nesting limit
		mov	ax,level
		cmp	ax,_FindLevel
		jge	nok
   ;	  if (c.Protected()) return FALSE;
		call	chkbar		; look for barriers
		je	nok
		cmp	@CLUSTER@Map[si],0
		jne	nok


   ;	  for (i = 0; i < ArrayCount(inc); i ++)
		xor	bx,bx
   more:
		add	cl,tab[bx]
		js	nxdir
		cmp	cl,MAP_XCNT
		jae	nxdir
		add	ch,tab[bx+1]
		js	nxdir
		cmp	ch,MAP_ZCNT
		jae	nxdir
		inc	level
		inc	@CLUSTER@Map[si]
		call	@Find1Way$qv	; find next step
		dec	@CLUSTER@Map[si]
		dec	level
		or	ax,ax
		je	next
   ;		      _Trace[level] = c;
		mov	bx,level
		add	bx,bx		; CLUSTER is 2 byte long
		mov	_Trace[bx].A,dl
		mov	_Trace[bx].B,dh

ok:		mov	ax,1
xit:            pop	si dx cx bx
		ret


   ;	      while (! c1.Protected());
   next:	call	chkbar		; look for barriers
		je	nxdir
		push	si
		mov	al,MAP_XCNT
		mul	ch
		add	al,cl
		adc	ah,0
		mov	si,ax
		cmp	@CLUSTER@Map[si],0
		pop	si
		je	more

   nxdir:	mov	cx,dx
		add	bx,2
		cmp	bx,tablen
		jb	more
nok:		xor	ax,ax
		clc
		jz	xit		; always jmp

@Find1Way$qv	endp



		end
