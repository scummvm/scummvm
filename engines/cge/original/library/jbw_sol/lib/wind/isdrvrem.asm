		.model	small

		public	@IsDrvRemote

		.code

; int _fastcall IsDrvRemote (int drv)

@IsDrvRemote	proc
		mov	bx,ax
		mov	ax,4409h
		int	21h
		xor	ax,ax
		test	dh,10h		; remote flag
		jz	idl_xit
		inc	ax
idl_xit:	ret
@IsDrvRemote	endp

		end
