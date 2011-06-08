		.model	small,c

;----------------------------------------------------------------------------

public		GetTime

timseg		equ	40h
timer		equ	6Ch


		.code

;  dword GetTime (void);
;  Compute hh, mm from system timer (ax:dx)

GetTime		proc
		mov	ax,timseg
		mov	es,ax
		mov	ax,es:timer
		mov	dx,es:timer+2
		mov	cx,1556h
		div	cx		; dx:ax/cx == count of 1/12 h
		mov	bx,dx		; bx := remainder
		mov	cx,5
		mul	cx		; ax := minutes (0, 5, 10, ...)
		xchg	ax,bx		; save in bx, rem to ax
		mov	cx,5
		mul	cx		; remainder * 5 ...
		mov	cx,1555h
		div	cx		; ... and / 1555
		add	bx,ax		; whole count of minutes
		mov	ax,dx		; remainder to get sec pulse
		mov	cl,91
		div	cl
		xchg	ax,bx
		xor	dx,dx
		mov	cx,60
		div	cx		; hours into ax, minutes into dx
		test	bx,1
		jz	return
		or	dx,8000h
return:		ret
GetTime		endp

		end
