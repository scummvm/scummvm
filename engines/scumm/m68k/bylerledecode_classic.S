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

#include "bylerledecode.inc"

	.globl	SYM(ByleRLEDecode_Classic)

	add_var	color,1+3	// byte color; (align stack)


	.text

| void ByleRLEDecode_Classic(
|	BaseCostumeRenderer::ByleRLEData *pcompData,
|	const byte _scaleX, /* unused */
|	const byte _scaleY, /* unused */
|	const int _height,
|	const int pitch,
|	const int _numStrips,
|	const byte *_srcPtr,
|	const byte *_shadowTable,
|	const uint16 *_palette /* unused */);
SYM(ByleRLEDecode_Classic):
// TODO: __FASTCALL__
	link	a6,#-sizeof_local_vars		| a6: frame pointer
	movem.l	d2-d7/a2-a5,-(sp)

	// registers taken so far:
	// - d7(zero/shadow scratch), d6(len), d5(height), d4(maskbit), d3(pitch), d2(_numStrips)
	// - a0(src), a1(dst), a2(mask), a3(destPtr), a4(_shadowTable), a5(pcompData), a6(frame pointer)

	// BaseCostumeRenderer::ByleRLEData &compData = *pcompData;
	move.l	(arg_pcompData,a6),a5		| a5: pcompData

	// const byte *src = _srcPtr;
	move.l	(arg_srcPtr,a6),a0		| a0: src

	// uint16 len = compData.repLen;
	clr.w	d6
	move.b	(compData.repLen,a5),d6		| d6.w: len
	// uint16 color = compData.repColor;
	move.b	(compData.repColor,a5),(color,a6)

	// byte *dst = compData.destPtr;
	move.l	(compData.destPtr,a5),a3	| a3: compData.destPtr
	move.l	a3,a1				| a1: dst
	// uint16 height = _height;
	move.w	(arg_height,a6),d5		| d5.w: height
	// byte maskbit = revBitMask(compData.x & 7);
	// #define revBitMask(x) (0x80 >> (x))
	move.l	(compData.x,a5),d0
	move.b	d0,d4
	not.b	d4
	and.b	#0b00000111,d4			| d4.b: bit to test
	// const byte *mask = compData.maskPtr + compData.x / 8;
	asr.l	#3,d0
	move.l	(compData.maskPtr,a5),a2
	add.l	d0,a2				| a2: mask

	move.l	(arg_pitch,a6),d3		| d3.w: pitch (used as long, too)
	move.l	(arg_numStrips,a6),d2		| d2.w: _numStrips (used as long, too)
	move.l	(arg_shadowTable,a6),a4		| a4: _shadowTable
	clr.l	d7				| d7.l: used for shadow table indexing

	// if (len) {
	// 	--len;
	// 	goto StartPos;
	// }
	// len   --len action
	// 0     -1    jump into main_loop
	// 1      0    jump into main_loop = jump into len_loop (0x)
	// 2      1    jump into len_loop (1x)
	// 3      2    jump into len_loop (2x)
	subq.w	#1,d6
	bgt.b	.Llen_loop
.Lmain_loop:
	// do {
	// len = *src++;
	// color = len >> compData.shr;
	// len &= compData.mask;
	// if (!len)
	// 	len = *src++;
	clr.w	d6				| d6.w: clear the high byte
	move.b	(a0)+,d0
	move.b	d0,d6
	move.b	(compData.shr,a5),d1
	lsr.b	d1,d0				| d0.b: color (only care about zero/nonzero)
	move.b	d0,(color,a6)
	and.b	(compData.mask,a5),d6		| d6.w: len
	bne.b	.Llen_loop
	move.b	(a0)+,d6
.Llen_loop:
	// do {
	// 	batch = height < len ? height : len;
	move.w	d5,d1				| d1.w: batch
	cmp.w	d6,d1
	blt.b	1f
	move.w	d6,d1
1:	// 	len -= batch;
	// 	height -= batch;
	sub.w	d1,d6
	sub.w	d1,d5
	// 	if (color) {
	tst.b	(color,a6)
	beq.b	2f

	subq.w	#1,d1				| dbra
.Lbatch_loop_shadow:
	// 		do {
	// 			if (!(*mask & maskbit))
	btst	d4,(a2)
	bne.b	1f
	// 				*dst = _shadowTable[*dst];
	move.b	(a1),d7
	move.b	(a4,d7.l),(a1)
1:	// 			dst  += pitch;
	// 			mask += _numStrips;
	add.l	d3,a1
	add.l	d2,a2
	// 		} while (--batch);
	dbra	d1,.Lbatch_loop_shadow
	bra.b	3f

	// 	} else {
2:	// 		dst  += batch * pitch;
	move.w	d1,d0
	mulu.w	d3,d0
	add.l	d0,a1
	// 		mask += batch * _numStrips;
	mulu.w	d2,d1
	add.l	d1,a2
	// 	}

3:	// 	if (height == 0) {
	tst.w	d5
	bne.b	5f
	// 		if (--compData.skipWidth == 0)
	subq.w	#1,(compData.skipWidth,a5)
	bne.b	4f
	// 			return;
	movem.l	(sp)+,d2-d7/a2-a5
	unlk	a6
	rts

4:	// 		height = _height;
	move.w	(arg_height,a6),d5		| d5.w: height
	move.l	(compData.scaleXStep,a5),d1	| d1.l: preload scaleXStep
	// 		compData.x += compData.scaleXStep;
	move.l	(compData.x,a5),d0
	add.l	d1,d0
	move.l	d0,(compData.x,a5)		| d0.l: compData.x
	move.b	d0,d4
	// 		maskbit = revBitMask(compData.x & 7);
	not.b	d4
	and.b	#0b00000111,d4			| d4.b: bit to test
	// 		destPtr += compData.scaleXStep;
	add.l	d1,a3
	// 		dst = destPtr;
	move.l	a3,a1				| a1: dst
	// 		mask = compData.maskPtr + compData.x / 8;
	asr.l	#3,d0
	move.l	(compData.maskPtr,a5),a2
	add.l	d0,a2				| a2: mask
	// 	}
5:	// StartPos:
	// } while (len > 0);
	tst.w	d6
	bne.b	.Llen_loop
	// } while (true);
	bra.w	.Lmain_loop
