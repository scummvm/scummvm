	.ALIGN
@	.ARM

	.GLOBAL cpu_speed
	.GLOBAL mmu_change
	.GLOBAL gp_enableIRQ
	.GLOBAL gp_disableIRQ
	.GLOBAL gp_installSWIIRQ
	.GLOBAL gp_removeSWIIRQ

cpu_speed:
	stmdb	r13!, {r0-r12,lr}
	stmdb	sp!, {r0, r1, r2}
	mov		r0, sp
	swi		#0x0d
	add		sp, sp, #12
	ldmia	r13!, {r0-r12,lr}
	bx		lr
	nop

mmu_change:
	stmdb   r13!, {r0-r12,lr}
	swi     #0x02
	ldmia   r13!, {r0-r12,lr}
	bx      lr
	nop

gp_enableIRQ:
	stmdb   r13!, {r0,lr}
	mrs     r0, CPSR
	bic     r0, r0, #0x80
	msr     CPSR, r0
	ldmia   r13!, {r0,pc}
	bx      lr

gp_disableIRQ:
	stmdb   r13!, {r0,lr}
	MRS     r0, CPSR
	ORR     r0, r0, #0xc0
	MSR     CPSR, r0
	ldmia   r13!, {r0,pc}
	bx      lr

gp_installSWIIRQ:
	stmdb   r13!, {r14}
	swi     0x9
	ldmia   r13!, {pc}
	bx      lr

gp_removeSWIIRQ:
	stmdb   r13!, {r14}
	swi     0xa
	ldmia   r13!, {pc}
	bx      lr


@ ******** ASMFastSolidBlit(unsigned char *src4, unsigned char *dst4, int nbx, int nby, int height2, int trans, int coul) ********

	.ALIGN
	.GLOBAL ASMFastSolidBlit
	.TYPE   ASMFastSolidBlit, function
	.CODE 32

@r0 = src4
@r1 = dst4
@r2 = nbx
@r3 = nby

@r4 = height2
@r5 = trans
@r6 = coul
@r7 = tmp
@r8 = tmpnby
@r9 = tmp2

_bx7:
	ldrb		r8,[r0,+r3]		@lecture 1er pixel
	subs		r7,r3,#1
	bmi		_sauty2

_sauty2:					@ecriture r8
	TEQ		r8,r5
	STRNEB	r8,[r1,+r7]
	SUB		r0,r0,r4
	SUB		r1,r1,#240
	SUBS		r2,r2,#1
	BPL		_bx7

        ldmfd		r13!,{r4-r8}
	add		sp,sp,#8
	bx		lr

ASMFastSolidBlit:

	sub		sp,sp,#12
        stmfd		r13!,{r4-r9}
        ldr		r4,[r13,#36]
        ldr		r5,[r13,#40]
        ldr		r6,[r13,#44]

_bx8:
	ldrb		r9,[r0,+r3]		@lecture 1er pixel
	subs		r8,r3,#1
	bmi		_sauty2

_by8:
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3
	LDRB		r7,[r0,+r8]
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1
	BMI		_sauty3b
	LDRB		r9,[r0,+r8]
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUBS		r8,r8,#1

	BPL		_by8

_sauty3:					@ecriture pour r9
	TEQ		r9,r5
	STRNEB	r6,[r1,+r8]
	SUB		r0,r0,r4
	SUB		r1,r1,#240
	SUBS		r2,r2,#1
	BPL		_bx8

        ldmfd		r13!,{r4-r9}
	add		sp,sp,#12
	bx		lr

_sauty3b:					@ecriture pour r7
	TEQ		r7,r5
	STRNEB	r6,[r1,+r8]
	SUB		r0,r0,r4
	SUB		r1,r1,#240
	SUBS		r2,r2,#1
	BPL		_bx8

        ldmfd		r13!,{r4-r9}
	add		sp,sp,#12
	bx		lr


@ ******** ASMSaveBitmap(unsigned char *src4, unsigned char *dst, int nbx, int nby, int height2) ********

	.ALIGN
	.GLOBAL ASMSaveBitmap
	.TYPE   ASMSaveBitmap, function
	.CODE 32

@r0 = src4
@r1 = dst + 1
@r2 = nbx
@r3 = nby

@r7 = height2
@r8 = tmp
@r9 = tmpnby
@r10 = dst4

ASMSaveBitmap:

	sub		sp,sp,#4
        stmfd		r13!,{r7-r10}
        LDR		r7,[r13,#20]

_bx6:
	MLA		r10,r2,r7,r1
	MOV		r9,r3

_by6:
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]
	BMI		_fincol6
	LDRB		r8,[r0,+r9]
	SUBS		r9,r9,#1
	STRB		r8,[r10,+r9]

	BPL		_by6

_fincol6:
	SUB		r0,r0,#240
	SUBS		r2,r2,#1
	BPL		_bx6

        ldmfd		r13!,{r7-r10}
	add		sp,sp,#4
	bx		lr


	.ALIGN
	.GLOBAL gp_clearFramebuffer
	.TYPE   gp_clearFramebuffer, function
	.CODE 32
gp_clearFramebuffer:
        @ r0 = framebuffer
        @ r1 = color
        orr r1,r1,r1,lsl #16   @hi halfword = lo halfword  
        mov r2,#38400
clear:  str  r1,[r0],#4
        subs r2,r2,#1
        bne clear
        bx lr


@ ******** ASMFastClear(unsigned char *dst4, int nbx, int nby) ********

	.ALIGN
	.GLOBAL ASMFastClear
	.TYPE   ASMFastClear, function
	.CODE 32

@r0 = dst4
@r1 = nbx
@r2 = nby

@r3 = #0
@r4 = tmpnby

@optimis?pour h=20

ASMFastClear:

	str		r4,[sp,#-4]!
	MOV		r3,#0

_bx9:
	MOV		r4,r2

_by9:
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1
	BMI		_sauty4
	STRB		r3,[r0,+r4]
	SUBS		r4,r4,#1

	BPL		_by9

_sauty4:
	SUB		r0,r0,#240
	SUBS		r1,r1,#1
	BPL		_bx9

	ldr		r4,[sp],#4
	bx		lr
