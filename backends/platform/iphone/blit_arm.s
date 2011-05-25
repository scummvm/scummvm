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

	.global	_blitLandscapeScreenRect16bpp
	.global	_blitLandscapeScreenRect8bpp


_blitLandscapeScreenRect16bpp:
	@ r0 = dst
	@ r1 = src
	@ r2 = w
	@ r3 = h
	@ <> = _screenWidth
	@ <> = _screenHeight
	mov		r12,r13
	stmfd	r13!,{r4-r11,r14}
	ldmfd	r12,{r12,r14}		@ r12 = _screenWidth
					@ r14 = _screenHeight
	add	r14,r14,r3		@ r14 = _screenHeight + h
	mvn	r11,#0
	mla	r11,r3,r12,r11		@ r11= _screenWidth*h-1
	add	r12,r12,r12
xloop:
	subs	r4,r3,#5		@ r4 = y = h
	ble		thin
yloop:
	ldrh	r5, [r1],r12		@ r5 = *src    src += _screenWidth
	ldrh	r6, [r1],r12		@ r6 = *src    src += _screenWidth
	ldrh	r7, [r1],r12		@ r7 = *src    src += _screenWidth
	ldrh	r8, [r1],r12		@ r8 = *src    src += _screenWidth
	ldrh	r9, [r1],r12		@ r9 = *src    src += _screenWidth
	ldrh	r10,[r1],r12		@ r10= *src    src += _screenWidth
	subs	r4,r4,#6
	strh	r5, [r0],#2		@ *dst++ = r5
	strh	r6, [r0],#2		@ *dst++ = r6
	strh	r7, [r0],#2		@ *dst++ = r7
	strh	r8, [r0],#2		@ *dst++ = r8
	strh	r9, [r0],#2		@ *dst++ = r9
	strh	r10,[r0],#2		@ *dst++ = r10
	bgt		yloop
thin:
	adds	r4,r4,#5
	beq		lineend
thin_loop:
	ldrh	r5,[r1],r12		@ r5 = *src    src += _screenWidth
	subs	r4,r4,#1
	strh	r5,[r0],#2		@ *dst++ = r5
	bgt	thin_loop
lineend:
	sub	r0,r0,r14,LSL #1	@ dst -= _screenHeight + h
	sub	r1,r1,r11,LSL #1	@ src += 1-_screenWidth*h
	subs	r2,r2,#1
	bgt	xloop

	ldmfd	r13!,{r4-r11,PC}

_blitLandscapeScreenRect8bpp:
	@ r0 = dst
	@ r1 = src
	@ r2 = w
	@ r3 = h
	@ <> = _palette
	@ <> = _screenWidth
	@ <> = _screenHeight
	mov		r12,r13
	stmfd	r13!,{r4-r11,r14}
	ldmfd	r12,{r11,r12,r14}	@ r11 = _palette
					@ r12 = _screenWidth
					@ r14 = _screenHeight
	add		r14,r14,r3		@ r14 = _screenHeight + h
	mvn		r6,#0
	mla		r6,r3,r12,r6		@ r6 = _screenWidth*h-1
xloop8:
	mov		r4,r3			@ r4 = y = h
	subs	r4,r3,#4		@ r4 = y = h
	ble		thin8
yloop8:
	ldrb	r5, [r1],r12		@ r5 = *src    src += _screenWidth
	ldrb	r7, [r1],r12		@ r7 = *src    src += _screenWidth
	ldrb	r8, [r1],r12		@ r8 = *src    src += _screenWidth
	ldrb	r9, [r1],r12		@ r9 = *src    src += _screenWidth
	ldrb	r10,[r1],r12		@ r10= *src    src += _screenWidth
	add	r5, r5, r5
	add	r7, r7, r7
	add	r8, r8, r8
	add	r9, r9, r9
	add	r10,r10,r10
	ldrh	r5, [r11,r5]
	ldrh	r7, [r11,r7]
	ldrh	r8, [r11,r8]
	ldrh	r9, [r11,r9]
	ldrh	r10,[r11,r10]
	subs	r4,r4,#5
	strh	r5, [r0],#2		@ *dst++ = r5
	strh	r7, [r0],#2		@ *dst++ = r7
	strh	r8, [r0],#2		@ *dst++ = r8
	strh	r9, [r0],#2		@ *dst++ = r9
	strh	r10,[r0],#2		@ *dst++ = r10
	bgt	yloop8
thin8:
	adds	r4,r4,#4
	beq		lineend8
thin_loop8:
	ldrb	r5,[r1],r12		@ r5 = *src    src += _screenWidth
	add	r5,r5,r5
	ldrh	r5,[r11,r5]
	subs	r4,r4,#1
	strh	r5,[r0],#2		@ *dst++ = r5
	bgt	thin_loop8
lineend8:
	sub	r0,r0,r14,LSL #1	@ dst -= _screenHeight + h
	sub	r1,r1,r6		@ src += 1-_screenWidth*h
	subs	r2,r2,#1
	bgt	xloop8

	ldmfd	r13!,{r4-r11,PC}
