		.model	small,c



CPU86		EQU	0
CPU186		EQU	1
CPU286		EQU	2
CPU386SX	EQU	3
CPU386DX	EQU	4
CPU486		EQU	5


		.code


;----------------------------------------------------------------
; enum CPU { CPU86, CPU186, CPU286, CPU386SX, CPU386DX, CPU486 };
;
; CPU CpuType (void);
;----------------------------------------------------------------
CpuType		PROC
		pushf

		xor	ax, ax
		push	ax
		popf
		pushf
		pop	ax
		and	ax, 0f000h
		cmp	ax, 0f000h
		jne	DCPU1

		push	cx
		mov	ax, 0ffffh
		mov	cl, 021h
		shl	ax, cl
		pop	cx
		mov	ax, CPU186
		jnz	DCExit
		mov	ax, CPU86
		jmp	short DCExit

DCPU1:		mov	ax, 07000h
		push	ax
		popf
		pushf
		pop	ax
		and	ax, 07000h
		mov	ax, CPU286
		jz	DCExit

; 386/486 checking code by Dave M. Walker
		.386P
		mov	eax, cr0
		mov	ebx, eax
		or	al, 10h
		mov	cr0, eax
		mov	eax, cr0
		mov	cr0, ebx
		test	al, 10h
		mov	ax, CPU386SX
		jz	short DCExit

		mov	ecx, esp
		pushfd
		pop	ebx
		and	esp, NOT 3
		mov	eax, ebx
		xor	eax, 40000h
		push	eax
		popfd
		pushfd
		pop	eax
		push	ebx
		popfd
		mov	esp, ecx
		cmp	eax, ebx
		mov	ax, CPU386DX
		je	short DCExit

		mov	ax, CPU486
DCExit: 	.8086
		popf
		ret
CpuType		ENDP


		end
