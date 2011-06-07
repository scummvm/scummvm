;	MAXROWS - screen length

	.model	small,c
	.code

	extrn	Video:near
	public	MaxRows

;	int	MaxRows	(void);

MaxRows	proc
	mov	dl,24		; last row on std screen
	xor	bx,bx		; valid request in BH
	mov	ax,1130h	; get EGA's last row #
	call	Video		; BIOS video service
	xor	dh,dh		; one-byte answer
	inc	dx		; # of rows = last+1
	mov	ax,dx		; return value
	ret
MaxRows	endp

	end
