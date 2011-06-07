		.model	small

		public	@IsDrvFixed

		.code

;	int _fastcall IsDrvFixed (int drv)

@IsDrvFixed	proc
		mov	bx,ax
		mov	ax,4408h
		int	21h
		cmp	ax,2
		jb	idf_xit
		mov	ax,-1
idf_xit:	ret
@IsDrvFixed	endp

		end
