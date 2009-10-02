@ ScummVM Scumm Interpreter
@ Copyright (C) 2009 The ScummVM project
@
@ This program is free software@ you can redistribute it and/or
@ modify it under the terms of the GNU General Public License
@ as published by the Free Software Foundation@ either version 2
@ of the License, or (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY@ without even the implied warranty of
@ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@ GNU General Public License for more details.
@
@ You should have received a copy of the GNU General Public License
@ along with this program@ if not, write to the Free Software
@ Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
@
@ $URL$
@ $Id$
@
@ @author Robin Watts (robin@wss.co.uk)

	.text

	.global Normal2xARM
	.global Normal2xAspectMask


	@ Assumes dst is aligned (so did the C)
	@ Assumes 16bit (so did the C)
Normal2xARM:
	@ r0 = src
	@ r1 = srcPitch
	@ r2 = dst
	@ r3 = dstPitch
	@ r4 = w
	@ r5 = h
	STMFD	r13!,{r4-r11,r14}
	LDR	r4, [r13,#4*9]		@ r4 = w
	LDR	r5, [r13,#4*10]		@ r5 = h
	ADD	r12,r2, r3
	SUB	r1, r1, r4, LSL #1
	SUB	r6, r3, r4, LSL #2
	ADD	r3, r3, r6
yloop:
	SUBS	r14,r4, #4
	BLT	thin
xloop:
	LDRH	r6, [r0], #2
	LDRH	r7, [r0], #2
	LDRH	r8, [r0], #2
	LDRH	r9, [r0], #2
	ORR	r6, r6, r6, LSL #16
	ORR	r7, r7, r7, LSL #16
	ORR	r8, r8, r8, LSL #16
	ORR	r9, r9, r9, LSL #16
	STMIA	r2!, {r6-r9}
	STMIA	r12!,{r6-r9}
	SUBS	r14,r14,#4
	BGE	xloop
	ADDS	r14,r14,#4
	BNE	thin
	ADD	r0, r0, r1
	ADD	r2, r2, r3
	ADD	r12,r12,r3
	SUBS	r5, r5, #1
	BGT	yloop

	LDMFD	r13!,{r4-r11,PC}
thin:
	LDRH	r6, [r0], #2
	ORR	r6, r6, r6, LSL #16
	STR	r6, [r2], #4
	STR	r6, [r12],#4
	SUBS	r14,r14,#1
	BGT	thin
	ADD	r0, r0, r1
	ADD	r2, r2, r3
	ADD	r12,r12,r3
	SUBS	r5, r5, #1
	BGT	yloop

	LDMFD	r13!,{r4-r11,PC}


	@ Horrid filter calculations
	@ AAAAAAAAAAAABBBBBBBBBBBBCCCCCCCCCCCCDDDDDDDDDDDDEEEEEEEEEEEE
	@ <-+-><-+-><-+-><-+-><-+-><-+-><-+-><-+-><-+-><-+-><-+-><-+->
	@ Ideal:  A,A,(A*2+B*3)/5,B,(B*4+C)/5,C,C,(C+D*4)/5,D,(D*3+E*2)/5,E,E
	@ Actual: A,A,(A*3+B*5)/8,B,(B*7+C)/8,C,C,(C+D*7)/8,D,(D*5+E*3)/8,E,E

	@ Assumes dst is aligned (so did the C)
	@ Assumes 16bit (so did the C)
Normal2xAspectMask:
	@ r0 = src
	@ r1 = srcPitch
	@ r2 = dst
	@ r3 = dstPitch
	@ r4 = w
	@ r5 = h
	@ r12= mask
	STMFD	r13!,{r4-r11,r14}
	LDR	r4, [r13,#4*9]		@ r4 = w
	LDR	r5, [r13,#4*10]		@ r5 = h
	LDR	r12,[r13,#4*11]		@ r12= mask
	MOV	r11,#2
	SUB	r11,r11,r1, LSL #2	@ r11= 2-srcPitch*4
	MOV	r14,#4
	SUB	r14,r14,r3, LSL #3
	SUB	r14,r14,r3, LSL #1
	SUB	r14,r14,r3		@ r14 = 4-dstPitch*11
yloop_aspect:
xloop_aspect:
	LDRH	r6, [r0], r1		@ r6 = A
	LDRH	r7, [r0], r1		@ r7 = B
	LDRH	r8, [r0], r1		@ r8 = C
	LDRH	r9, [r0], r1		@ r9 = D
	LDRH	r10,[r0], r11		@ r10= E
	ORR	r6, r6, r6, LSL #16	@ r6 = output 0, 1
	ORR	r7, r7, r7, LSL #16	@ r7 = output 3
	ORR	r8, r8, r8, LSL #16	@ r8 = output 5,6
	ORR	r9, r9, r9, LSL #16	@ r9 = output 8
	ORR	r10,r10,r10,LSL #16	@ r10= output 10, 11
	STR	r6, [r2], r3		@ output 0 (A)
	STR	r6, [r2], r3		@ output 1 (A)
	AND	r6, r6, r12		@ r6 = A split
	ADD	r6, r6, r6, LSL #1	@ r6 = A*3
	STR	r7, [r2, r3]		@ output 3 (B)
	AND	r7, r7, r12		@ r7 = B split
	ADD	r6, r6, r7		@ r6 = A*3 + B
	ADD	r6, r6, r7, LSL #2	@ r6 = A*3 + B*5
	AND	r6, r12,r6, LSR #3	@ r6 = (A*3 + B*5)>>3
	ORR	r6, r6, r6, ROR #16	@ r6 = output 2
	STR	r6, [r2], r3, LSL #1	@ output 2 (A*3+B*5)>>3
	RSB	r7, r7, r7, LSL #3	@ r7 = B*7
	AND	r6, r8, r12		@ r6 = C split
	ADD	r7, r7, r6		@ r7 = B*7+C
	AND	r7, r12,r7, LSR #3	@ r7 = (B*7 + C)>>3
	ORR	r7, r7, r7, ROR #16	@ r7 = output 4
	STR	r7, [r2], r3		@ output 4 (B*7+C)>>3
	STR	r8, [r2], r3		@ output 5 (C)
	STR	r8, [r2], r3		@ output 6 (C)
	STR	r9, [r2, r3]		@ output 8 (D)
	AND	r9, r9, r12		@ r9 = D split
	RSB	r7, r9, r9, LSL #3	@ r7 = D*7
	ADD	r6, r6, r7		@ r6 = C+D*7
	AND	r6, r12,r6, LSR #3	@ r6 = (C + D*7)>>3
	ORR	r6, r6, r6, ROR #16	@ r6 = output 7
	STR	r6, [r2], r3, LSL #1	@ output 7 (C+D*7)>>3
	ADD	r9, r9, r9, LSL #2	@ r9 = D*5
	AND	r6, r10,r12		@ r6 = E split
	ADD	r9, r9, r6		@ r9 = D*5+E
	ADD	r9, r9, r6, LSL #1	@ r9 = D*5+E*3
	AND	r9, r12,r9, LSR #3	@ r9 = (D*5 + E*3)>>3
	ORR	r9, r9, r9, ROR #16	@ r9 = output 9
	STR	r9, [r2], r3		@ output 9 (D*5+E*3)>>3
	STR	r10,[r2], r3		@ output 10 (E)
	STR	r10,[r2], r14		@ output 11 (E)
	SUBS	r4, r4, #1
	BGT	xloop_aspect
	LDR	r4, [r13,#4*9]		@ r4 = w
	ADD	r0, r0, r1, LSL #2
	ADD	r0, r0, r1
	SUB	r0, r0, r4, LSL #1
	ADD	r2, r2, r3, LSL #3
	ADD	r2, r2, r3, LSL #2
	SUB	r2, r2, r4, LSL #2
	SUBS	r5, r5, #5
	BGT	yloop_aspect

	LDMFD	r13!,{r4-r11,PC}
