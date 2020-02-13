@ ScummVM - Graphic Adventure Engine
@
@ ScummVM is the legal property of its developers, whose names
@ are too numerous to list here. Please refer to the COPYRIGHT
@ file distributed with this source distribution.
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
@ @author Robin Watts (robin@wss.co.uk)

	.text

	.global Normal2xARM


	@ Assumes dst is aligned (so did the C)
	@ Assumes 16bit (so did the C)
	.align 2
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
	ADDLT	r14,r14, #4
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
