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

	.globl	SYM(ByleRLEDecode_Scaled_Mode3)

	add_var	len,2		// uint16 len;
	add_var	height,2	// uint16 height;
	add_var	scaleTableX,4	// &compData.scaleTable[compData.scaleXIndex]
	add_var	scaleTableY,4	// &compData.scaleTable[compData.scaleYIndex]
	add_var	shadowMask,1+3	// 0xFF = draw shadows (new column), 0x00 = skip shadows (duplicate column) (align stack)

	.text

| void ByleRLEDecode_Scaled_Mode3(
|	BaseCostumeRenderer::ByleRLEData *pcompData,
|	const byte _scaleX,
|	const byte _scaleY,
|	const int _height,
|	const int pitch,
|	const int _numStrips,
|	const byte *_srcPtr,
|	const byte *_shadowTable,
|	const uint16 *_palette);
SYM(ByleRLEDecode_Scaled_Mode3):
// TODO: __FASTCALL__
	link	a6,#-sizeof_local_vars		| a6: frame pointer
	movem.l	d2-d7/a2-a5,-(sp)

	// registers taken so far:
	// - d7(pcolor), d6(color), d5(_scaleY), d4(maskbit), d3(pitch), d2(_numStrips)
	// - a0(src), a1(dst), a2(mask), a3(scaleTableY), a4(_shadowTable), a5(pcompData), a6(frame pointer)

	// BaseCostumeRenderer::ByleRLEData &compData = *pcompData;
	move.l	(arg_pcompData,a6),a5		| a5: pcompData

	// const byte *src = _srcPtr;
	move.l	(arg_srcPtr,a6),a0		| a0: src

	// uint16 len = compData.repLen;
	clr.w	(len,a6)
	move.b	(compData.repLen,a5),(len+1,a6)
	// uint16 color = compData.repColor;
	clr.l	d6
	move.b	(compData.repColor,a5),d6	| d6.b: color (used as long, too)
	clr.l	d7				| used as long, too

	// byte *dst = compData.destPtr;
	move.l	(compData.destPtr,a5),a1	| a1: dst
	// int lastColumnX = -1;
	st.b	(shadowMask,a6)
	// uint16 height = _height;
	move.w	(arg_height,a6),(height,a6)
	// int scaleIndexY = compData.scaleYIndex;
	move.l	(compData.scaleXIndex,a5),d0
	lea	([compData.scaleTable,a5],d0.l),a3
	move.l	a3,(scaleTableX,a6)
	move.l	(compData.scaleYIndex,a5),d0
	lea	([compData.scaleTable,a5],d0.l),a3	| a3: scaleTableY
	move.l	a3,(scaleTableY,a6)
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

	move.b	(arg_scaleY,a6),d5		| d5.b: _scaleY
	move.l	(arg_pitch,a6),d3		| d3.w: pitch (used as long, too)
	move.l	(arg_numStrips,a6),d2		| d2.w: _numStrips (used as long, too)
	move.l	(arg_shadowTable,a6),a4		| a4: _shadowTable

	// if (len) {
	// 	--len;
	// 	goto StartPos;
	// }
	// len   --len action
	// 0     -1    jump into main_loop
	// 1      0    jump into main_loop = jump into len_loop (0x)
	// 2      1    jump into len_loop (1x)
	// 3      2    jump into len_loop (2x)
	subq.w	#1,(len,a6)
	bgt.b	.Lpcolor_setup
.Lmain_loop:
	// do {
	// len = *src++;
	// color = len >> compData.shr;
	// len &= compData.mask;
	// if (!len)
	// 	len = *src++;
	clr.w	d7
	move.b	(a0)+,d7
	move.w	d7,d6				| d6.w: clear the highest bit ("shadow_on" bit cleared)
	move.b	(compData.shr,a5),d0
	lsr.b	d0,d6				| d6.b: color (used as long, too)
	and.b	(compData.mask,a5),d7
	bne.b	1f
	move.b	(a0)+,d7
1:	move.w	d7,(len,a6)

.Lpcolor_setup:
	tst.b	d6
	beq.b	.Llen_loop
	move.w	([arg_palette,a6],d6.l*2),d7	| d7.w: pcolor = _palette[color] (used as long, too)
	cmp.w	#8,d7
	bhs.b	.Llen_loop
	lsl.w	#8,d7				| d7.w: if pcolor < 8: pcolor << 8
	or.w	#0x8000,d6			| d6.w: "shadow_on" bit set
	and.b	(shadowMask,a6),d6		| 0xFF: no-op; 0x00: clears color, suppresses writes
.Llen_loop:
	// do {
	// 	batch = height < len ? height : len;
	move.w	(len,a6),d0
	move.w	(height,a6),d1			| d1.w: batch
	cmp.w	d0,d1
	blt.b	1f
	move.w	d0,d1
1:	// 	len -= batch;
	// 	height -= batch;
	sub.w	d1,(len,a6)
	sub.w	d1,(height,a6)

	subq.w	#1,d1				| dbra
	tst.w	d6
	bmi.b	.Lbatch_loop_shadow
.Lbatch_loop:
	// 		do {
	// 			if (compData.scaleTable[scaleIndexY++] < _scaleY)
	cmp.b	(a3)+,d5
	bls.b	2f
	// 				if (color) {
	tst.b	d6
	beq.b	1f
	// 					if (!(*mask & maskbit))
	btst	d4,(a2)
	bne.b	1f
	// 						*dst = _palette[color];
	move.b	d7,(a1)
	// 				}
1:	// 				dst  += pitch;
	// 				mask += _numStrips;
	add.l	d3,a1
	add.l	d2,a2
	// 			}
2:	// 		} while (--batch);
	dbra	d1,.Lbatch_loop
	bra.b	3f

.Lbatch_loop_shadow:
	// 		do {
	// 			if (compData.scaleTable[scaleIndexY++] < _scaleY) {
	cmp.b	(a3)+,d5
	bls.b	2f
	// 				if (color) {
	tst.b	d6
	beq.b	1f
	// 					if (!(*mask & maskbit)) {
	btst	d4,(a2)
	bne.b	1f
	// 						uint16 pcolor = _palette[color];
	// 						pcolor = (pcolor << 8) + *dst;
	// 						*dst = _shadowTable[pcolor];
	move.b	(a1),d7
	move.b	(a4,d7.l),(a1)
	// 					}
	// 				}
1:	// 				dst  += pitch;
	// 				mask += _numStrips;
	add.l	d3,a1
	add.l	d2,a2
	// 			}
2:	// 		} while (--batch);
	dbra	d1,.Lbatch_loop_shadow

3:	// 	if (height == 0) {
	tst.w	(height,a6)
	bne.b	6f
	// 		if (--compData.skipWidth == 0)
	subq.w	#1,(compData.skipWidth,a5)
	bne.b	4f
	// 			return;
	movem.l	(sp)+,d2-d7/a2-a5
	unlk	a6
	rts

4:	// 		height = _height;
	move.w	(arg_height,a6),(height,a6)
	// 		scaleIndexY = compData.scaleYIndex;
	move.l	(scaleTableY,a6),a3
	// 		lastColumnX = compData.x;
	sf.b	(shadowMask,a6)			| assume duplicate column (0x00)

	move.l	(compData.scaleXStep,a5),d1	| d1.l: preload scaleXStep

	// 		dst = compData.destPtr; (moved from the below)
	move.l	(compData.destPtr,a5),a1	| a1: dst
	// 		if (compData.scaleTable[compData.scaleXIndex] < _scaleX) {
	move.b	(arg_scaleX,a6),d0
	cmp.b	([scaleTableX,a6]),d0
	bls.b	5f

	st.b	(shadowMask,a6)			| new column (0xFF)

	// 			compData.x += compData.scaleXStep;
	move.l	(compData.x,a5),d4
	add.l	d1,d4
	move.l	d4,(compData.x,a5)
	// 			maskbit = revBitMask(compData.x & 7);
	not.b	d4
	and.b	#0b00000111,d4			| d4.b: bit to test
	// 			compData.destPtr += compData.scaleXStep;
	add.l	d1,a1
	move.l	a1,(compData.destPtr,a5)
	// 		}
5:	// 		compData.scaleXIndex += compData.scaleXStep;
	add.l	d1,(scaleTableX,a6)
	// 		mask = compData.maskPtr + compData.x / 8;
	move.l	(compData.x,a5),d0
	asr.l	#3,d0
	move.l	(compData.maskPtr,a5),a2
	add.l	d0,a2				| a2: mask
	// 	}
6:	// StartPos:
	// } while (len > 0);
	tst.w	(len,a6)
	bne.w	.Llen_loop
	// } while (true);
	bra.w	.Lmain_loop
