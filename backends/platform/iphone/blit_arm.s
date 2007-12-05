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
@ $URL$
@ $Id$
@
@ @author Robin Watts (robin@wss.co.uk)

	.text

	.global	_blitLandscapeScreenRect16bpp
	.global	_blitLandscapeScreenRect8bpp


_blitLandscapeScreenRect16bpp:
	@ r0 = dst
	@ r1 = src
	@ r2 = w
	@ r3 = h
	@ <> = _screenWidth
	@ <> = _screenHeight
	MOV	r12,r13
	STMFD	r13!,{r4-r11,r14}
	LDMFD	r12,{r12,r14}		@ r12 = _screenWidth
					@ r14 = _screenHeight
	ADD	r14,r14,r3		@ r14 = _screenHeight + h
	MVN	r11,#0
	MLA	r11,r3,r12,r11		@ r11= _screenWidth*h-1
	ADD	r12,r12,r12
xloop:
	SUBS	r4,r3,#5		@ r4 = y = h
	BLE	thin
yloop:
	LDRH	r5, [r1],r12		@ r5 = *src    src += _screenWidth
	LDRH	r6, [r1],r12		@ r6 = *src    src += _screenWidth
	LDRH	r7, [r1],r12		@ r7 = *src    src += _screenWidth
	LDRH	r8, [r1],r12		@ r8 = *src    src += _screenWidth
	LDRH	r9, [r1],r12		@ r9 = *src    src += _screenWidth
	LDRH	r10,[r1],r12		@ r10= *src    src += _screenWidth
	SUBS	r4,r4,#6
	STRH	r5, [r0],#2		@ *dst++ = r5
	STRH	r6, [r0],#2		@ *dst++ = r6
	STRH	r7, [r0],#2		@ *dst++ = r7
	STRH	r8, [r0],#2		@ *dst++ = r8
	STRH	r9, [r0],#2		@ *dst++ = r9
	STRH	r10,[r0],#2		@ *dst++ = r10
	BGT	yloop
thin:
	ADDS	r4,r4,#5
	BEQ	lineend
thin_loop:
	LDRH	r5,[r1],r12		@ r5 = *src    src += _screenWidth
	SUBS	r4,r4,#1
	STRH	r5,[r0],#2		@ *dst++ = r5
	BGT	thin_loop
lineend:
	SUB	r0,r0,r14,LSL #1	@ dst -= _screenHeight + h
	SUB	r1,r1,r11,LSL #1	@ src += 1-_screenWidth*h
	SUBS	r2,r2,#1
	BGT	xloop

	LDMFD	r13!,{r4-r11,PC}

_blitLandscapeScreenRect8bpp:
	@ r0 = dst
	@ r1 = src
	@ r2 = w
	@ r3 = h
	@ <> = _palette
	@ <> = _screenWidth
	@ <> = _screenHeight
	MOV	r12,r13
	STMFD	r13!,{r4-r11,r14}
	LDMFD	r12,{r11,r12,r14}	@ r11 = _palette
					@ r12 = _screenWidth
					@ r14 = _screenHeight
	ADD	r14,r14,r3		@ r14 = _screenHeight + h
	MVN	r6,#0
	MLA	r6,r3,r12,r6		@ r6 = _screenWidth*h-1
xloop8:
	MOV	r4,r3			@ r4 = y = h
	SUBS	r4,r3,#4		@ r4 = y = h
	BLE	thin8
yloop8:
	LDRB	r5, [r1],r12		@ r5 = *src    src += _screenWidth
	LDRB	r7, [r1],r12		@ r7 = *src    src += _screenWidth
	LDRB	r8, [r1],r12		@ r8 = *src    src += _screenWidth
	LDRB	r9, [r1],r12		@ r9 = *src    src += _screenWidth
	LDRB	r10,[r1],r12		@ r10= *src    src += _screenWidth
	ADD	r5, r5, r5
	ADD	r7, r7, r7
	ADD	r8, r8, r8
	ADD	r9, r9, r9
	ADD	r10,r10,r10
	LDRH	r5, [r11,r5]
	LDRH	r7, [r11,r7]
	LDRH	r8, [r11,r8]
	LDRH	r9, [r11,r9]
	LDRH	r10,[r11,r10]
	SUBS	r4,r4,#5
	STRH	r5, [r0],#2		@ *dst++ = r5
	STRH	r7, [r0],#2		@ *dst++ = r7
	STRH	r8, [r0],#2		@ *dst++ = r8
	STRH	r9, [r0],#2		@ *dst++ = r9
	STRH	r10,[r0],#2		@ *dst++ = r10
	BGT	yloop8
thin8:
	ADDS	r4,r4,#4
	BEQ	lineend8
thin_loop8:
	LDRB	r5,[r1],r12		@ r5 = *src    src += _screenWidth
	ADD	r5,r5,r5
	LDRH	r5,[r11,r5]
	SUBS	r4,r4,#1
	STRH	r5,[r0],#2		@ *dst++ = r5
	BGT	thin_loop8
lineend8:
	SUB	r0,r0,r14,LSL #1	@ dst -= _screenHeight + h
	SUB	r1,r1,r6		@ src += 1-_screenWidth*h
	SUBS	r2,r2,#1
	BGT	xloop8

	LDMFD	r13!,{r4-r11,PC}
