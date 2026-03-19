/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

| kate: tab-width 8; replace-tabs off; hl Motorola 68k (VASM/Devpac);

#include "../../backends/platform/atari/symbols.h"

	.globl	SYM(asmDrawStripToScreen)


	.text

| void asmDrawStripToScreen(
|     const int height,      4+9*4(sp)
|     const int width,       4+9*4+4(sp)
|     const uint32 *src32,   4+9*4+8(sp)
|     uint32 *dst32,         4+9*4+12(sp)
|     const int vsPitch,     4+9*4+16(sp)
|     const uint32 *text32,  4+9*4+20(sp)
|     const int textPitch    4+9*4+24(sp)
| );
SYM(asmDrawStripToScreen):
// TODO: __FASTCALL__
	movem.l	d2-d7/a2-a4,-(sp)		| 9 × 4 = 36 bytes
	move.l	(4+9*4+8,sp),a0			| a0: src32
	move.l	(4+9*4+12,sp),a1		| a1: dst32
	move.l	(4+9*4+20,sp),a2		| a2: text32

	move.l	(4+9*4+16,sp),d0		| vsPitch
	lsl.l	#2,d0				|
	move.l	d0,a3				| a3: vsPitch in bytes

	move.l	(4+9*4+24,sp),d0		| textPitch
	lsl.l	#2,d0				|
	move.l	d0,a4				| a4: textPitch in bytes

	move.l	(4+9*4+4,sp),d5			| width
	lsr.l	#2,d5				|
	beq.b	3f
	subq.l	#1,d5				| d5.w: width/4 - 1 for dbra

	// for (int h = height * m; h > 0; --h) {
	move.l	(4+9*4,sp),d7
	beq.b	3f

	move.w	d5,-(sp)
	move.l	#0x7F7F7F7F,d3
	move.l	#0x80808080,d4
	move.l	#0xFDFDFDFD,d5
	subq.l	#1,d7				| d7.w: height - 1 for dbra

1:	//	for (int w = width * m; w > 0; w -= 4) {
	move.w	(sp),d6
2:	//		uint32 temp = *text32++;
	move.l	(a2)+,d0
	//		uint32 mask = temp ^ CHARSET_MASK_TRANSPARENCY_32;
	move.l	d0,d1
	eor.l	d5,d1
	//		mask = (((mask & 0x7f7f7f7f) + 0x7f7f7f7f) | mask) & 0x80808080;
	move.l	d1,d2
	and.l	d3,d2
	add.l	d3,d2
	or.l	d1,d2
	and.l	d4,d2
	//		mask = ((mask >> 7) + 0x7f7f7f7f) ^ 0x80808080;
	lsr.l	#7,d2
	add.l	d3,d2
	eor.l	d4,d2
	//		*dst32++ = ((temp ^ *src32++) & mask) ^ temp;
	move.l	(a0)+,d1
	eor.l	d0,d1
	and.l	d2,d1
	eor.l	d0,d1
	move.l	d1,(a1)+
	//	}
	dbra	d6,2b
	//	src32 += vsPitch;
	add.l	a3,a0
	//	text32 += textPitch;
	add.l	a4,a2
	// }
	dbra	d7,1b

	addq.l	#2,sp				| pop d5.w

3:	movem.l	(sp)+,d2-d7/a2-a4
	rts
