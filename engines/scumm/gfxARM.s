@ ScummVM Scumm Interpreter
@ Copyright (C) 2007 The ScummVM project
@
@ This program is free software@ you can redistribute it and/or
@ modify it under the terms of the GNU General Public License
@ as published by the Free Software Foundation@ either version 2
@ of the License, or (at your option) any later version.
@
@ This program is distributed in the hope that it will be useful,
@ but WITHOUT ANY WARRANTY; without even the implied warranty of
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

	.global	asmDrawStripToScreen
	.global	asmCopy8Col
	
	@ ARM implementation of asmDrawStripToScreen.
	@
	@ C prototype would be:
	@
	@ extern "C" void asmDrawStripToScreen(int         height,
	@                                      int         width,
	@                                      byte const *text,
	@                                      byte const *src,
	@                                      byte       *dst,
	@                                      int         vsPitch,
	@                                      int         vsScreenWidth,
	@                                      int         textSurfacePitch);
	@
	@ In addition, we assume that text, src and dst are all word (4 byte)
	@ aligned. This is the same assumption that the old 'inline' version
	@ made.
asmDrawStripToScreen:
	@ r0 = height
	@ r1 = width
	@ r2 = text
	@ r3 = src
	MOV	r12,r13
	STMFD	r13!,{r4-r7,r9-r11,R14}
	LDMIA	r12,{r4,r5,r6,r7}
	@ r4 = dst
	@ r5 = vsPitch
	@ r6 = vmScreenWidth
	@ r7 = textSurfacePitch

	CMP	r0,#0			@ If height<=0
	MOVLE	r0,#1			@    height=1
	CMP	r1,#4			@ If width<4
	BLT	end			@    return

	@ Width &= ~4 ? What''s that about then? Width &= ~3 I could have
	@ understood...
	BIC	r1,r1,#4

	SUB	r5,r5,r1		@ vsPitch          -= width
	SUB	r6,r6,r1		@ vmScreenWidth    -= width
	SUB	r7,r7,r1		@ textSurfacePitch -= width
	MOV	r10,#253
	ORR	r10,r10,r10,LSL #8
	ORR	r10,r10,r10,LSL #16	@ r10 = mask
yLoop:
	MOV	r14,r1			@ r14 = width
xLoop:
	LDR	r12,[r2],#4		@ r12 = [text]
	LDR	r11,[r3],#4		@ r11 = [src]
	CMP	r12,r10
	BNE	singleByteCompare
	SUBS	r14,r14,#4
	STR	r11,[r4], #4		@ r4 = [dst]
	BGT	xLoop

	ADD	r2,r2,r7		@ text += textSurfacePitch
	ADD	r3,r3,r5		@ src  += vsPitch
	ADD	r4,r4,r6		@ dst  += vmScreenWidth
	SUBS	r0,r0,#1
	BGT	yLoop
	LDMFD	r13!,{r4-r7,r9-r11,PC}

singleByteCompare:
	MOV	r9,r12,LSR #24		@ r9 = 1st byte of [text]
	CMP	r9,r10,LSR #24		@ if (r9 == mask)
	MOVEQ	r9,r11,LSR #24		@     r9 = 1st byte of [src]
	ORR	r12,r9,r12,LSL #8	@ r12 = combine r9 and r12

	MOV	r9,r12,LSR #24		@ r9 = 1st byte of [text]
	CMP	r9,r10,LSR #24		@ if (r9 == mask)
	MOVEQ	r9,r11,LSR #24		@     r9 = 1st byte of [src]
	ORR	r12,r9,r12,LSL #8	@ r12 = combine r9 and r12

	MOV	r9,r12,LSR #24		@ r9 = 1st byte of [text]
	CMP	r9,r10,LSR #24		@ if (r9 == mask)
	MOVEQ	r9,r11,LSR #24		@     r9 = 1st byte of [src]
	ORR	r12,r9,r12,LSL #8	@ r12 = combine r9 and r12

	MOV	r9,r12,LSR #24		@ r9 = 1st byte of [text]
	CMP	r9,r10,LSR #24		@ if (r9 == mask)
	MOVEQ	r9,r11,LSR #24		@     r9 = 1st byte of [src]
	ORR	r12,r9,r12,LSL #8	@ r12 = combine r9 and r12

	STR	r12,[r4],#4
	SUBS	r14,r14,#4
	BGT	xLoop

	ADD	r2,r2,r7		@ text += textSurfacePitch
	ADD	r3,r3,r5		@ src  += vsPitch
	ADD	r4,r4,r6		@ dst  += vmScreenWidth
	SUBS	r0,r0,#1
	BGT	yLoop
end:
	LDMFD	r13!,{r4-r7,r9-r11,PC}
	
	@ ARM implementation of asmCopy8Col
	@
	@ C prototype would be:
	@
	@ extern "C" void asmCopy8Col(byte       *dst,
	@                             int         dstPitch,
	@                             const byte *src,
	@                             int         height);
	@
	@ In addition, we assume that src and dst are both word (4 byte)
	@ aligned. This is the same assumption that the old 'inline' version
	@ made.
asmCopy8Col:
	@ r0 = dst
	@ r1 = dstPitch
	@ r2 = src
	@ r3 = height
	STMFD	r13!,{r14}
	SUB	r1,r1,#4

	TST	r3,#1
	ADDNE   r3,r3,#1
	BNE	roll2
yLoop2:
	LDR	r12,[r2],#4
	LDR	r14,[r2],r1
	STR	r12,[r0],#4
	STR	r14,[r0],r1
roll2:
	LDR	r12,[r2],#4
	LDR	r14,[r2],r1
	SUBS	r3,r3,#2
	STR	r12,[r0],#4
	STR	r14,[r0],r1
	BNE	yLoop2

	LDMFD	r13!,{PC}	
