
	.ALIGN
	.ARM

	.GLOBAL		cpu_speed
	.GLOBAL		mmu_change

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
        swi             #0x02
        ldmia   r13!, {r0-r12,lr}
        bx              lr
        nop
