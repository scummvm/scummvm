;	MAXROWS - screen length

	.model	small,c
	.code

	extrn	Video:near
	public	MaxColumns

;	int	MaxColumns(void);

MaxColumns proc
	mov	ah,0Fh		; get current video mode
	call	Video		; BIOS video service
	xchg	ah,al		; answer in ah
	xor	ah,ah		; wipe unused bits
	ret
MaxColumns endp

	end
