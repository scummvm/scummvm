; is this VGA board?



VIDEO	=	10h


	.model	small

	public	@IsVga$qv


	.code

@IsVga$qv	proc	near

		mov	ax,1A00h
		int	VIDEO
		cmp	al,1Ah
		mov	ax,1
		je	xit
		dec	al
xit:		ret
		endp



		end
