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
 * but WITHOUT ANY WARRANTY| without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "../../platform/atari/symbols.h"

	.global	SYM(asm_draw_4bpl_sprite)
	.global	SYM(asm_draw_8bpl_sprite)

	.text

| extern void asm_draw_4bpl_sprite(uint16 *dstBuffer, const uint16 *srcBuffer, const uint16 *srcMask,
|				   uint destX, uint destY, uint dstPitch, uint w, uint h);
|
SYM(asm_draw_4bpl_sprite):
	movem.l	d0-d7/a0-a2,-(sp)		| 11 longs

#ifdef __FASTCALL__
	move.l	a0,a2					| a2: dstBuffer
									| a1: srcBuffer
	move.l	(4+11*4,sp),a0			| a0: srcMask
									| d0.w: destX
									| d1.w: destY
	move.l	d2,d3					| d3.w: dstPitch
	ext.l	d3						| d3.l: dstPitch
	move.l	(8+11*4,sp),d6			| d6.w: w
	lsr.w	#4,d6					| d6.w: w/16
	move.l	(12+11*4,sp),d7			| d7.w: h
#else
	move.l	(4+11*4,sp),a2			| a2: dstBuffer
	move.l	(8+11*4,sp),a1			| a1: srcBuffer
	move.l	(12+11*4,sp),a0			| a0: srcMask
	move.l	(16+11*4,sp),d0			| d0.w: destX
	move.l	(20+11*4,sp),d1			| d1.w: destY
	move.l	(24+11*4,sp),d3			| d3.w: dstPitch
	ext.l	d3				| d3.l: dstPitch
	move.l	(28+11*4,sp),d6			| d6.w: w
	lsr.w	#4,d6				| d6.w: w/16
	move.l	(32+11*4,sp),d7			| d7.w: h
#endif

| Draws a 4 bitplane sprite at any position on screen.
| (c) 1999 Pieter van der Meer (EarX)
|
| INPUT: d0.w: x position of sprite on screen (left side)
|        d1.w: y position of sprite on screen (top side)
|        d6.w: number of 16pixel X blocks to do
|        d7.w: number of Y lines to to
|        a0: address of maskdata
|        a1: address of bitmapdata
|        a2: screen start address

	move.w  d0,d2				| / Calculate the
	andi.w  #0b111111110000,d0		| | number of bits
	sub.w   d0,d2				| \ to shift right.
	lsr.w   #1,d0				| / Add x-position to
	adda.w  d0,a2				| \ screenaddress.
	mulu.w  d3,d1				| / Add y-position to
	adda.l  d1,a2				| \ screenaddress.
	move.w  d6,d1				| / Prepare
	lsl.w   #3,d1				| | offset
	move.l  d3,d4				| | to next
	sub.w   d1,d4				| \ screenline.
	subq.w  #1,d7				| Adjust for dbra.
	subq.w  #1,d6				| Adjust for dbra.
	move.w  d6,d5				| Backup xloopcount in d5.w.
	moveq   #16,d1				| Size of two chunks.

sprite4_yloop:
	move.w  d5,d6				| Restore xloop counter.

sprite4_xloop:
	moveq   #0xffffffff,d0			| Prepare for maskshifting.
	move.w  (a0)+,d0			| Get 16pixel mask in d0.w.
	ror.l   d2,d0				| Shift it!
	and.w   d0,(a2)+			| Mask bitplane 0.
	and.w   d0,(a2)+			| Mask bitplane 1.
	and.w   d0,(a2)+			| Mask bitplane 2.
	and.w   d0,(a2)+			| Mask bitplane 3.
	swap    d0				| Get overspill in loword.
	and.w   d0,(a2)+			| Mask overspill bitplane 0.
	and.w   d0,(a2)+			| Mask overspill bitplane 1.
	and.w   d0,(a2)+			| Mask overspill bitplane 2.
	and.w   d0,(a2)+			| Mask overspill bitplane 3.
	suba.l  d1,a2				| Return to blockstart.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 0.
	swap    d0				| Get overspill in loword.
	or.w    d0,6(a2)			| Paint overspill bitplane 0.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 1.
	swap    d0				| Get overspill in loword.
	or.w    d0,6(a2)			| Paint overspill bitplane 1.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 2.
	swap    d0				| Get overspill in loword.
	or.w    d0,6(a2)			| Paint overspill bitplane 2.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 3.
	swap    d0				| Get overspill in loword.
	or.w    d0,6(a2)			| Paint overspill bitplane 3.

	dbra    d6,sprite4_xloop		| Loop until blocks done.

	adda.l  d4,a2				| Goto next screenline.
	dbra    d7,sprite4_yloop		| Loop until lines done.

	movem.l	(sp)+,d0-d7/a0-a2
	rts

| extern void asm_draw_8bpl_sprite(uint16 *dstBuffer, const uint16 *srcBuffer, const uint16 *srcMask,
|				   uint destX, uint destY, uint dstPitch, uint w, uint h);
|
SYM(asm_draw_8bpl_sprite):
	movem.l	d0-d7/a0-a2,-(sp)		| 11 longs

#ifdef __FASTCALL__
	move.l	a0,a2					| a2: dstBuffer
									| a1: srcBuffer
	move.l	(4+11*4,sp),a0			| a0: srcMask
									| d0.w: destX
									| d1.w: destY
	move.l	d2,d3					| d3.w: dstPitch
	ext.l	d3						| d3.l: dstPitch
	move.l	(8+11*4,sp),d6			| d6.w: w
	lsr.w	#4,d6					| d6.w: w/16
	move.l	(12+11*4,sp),d7			| d7.w: h
#else
	move.l	(4+11*4,sp),a2			| a2: dstBuffer
	move.l	(8+11*4,sp),a1			| a1: srcBuffer
	move.l	(12+11*4,sp),a0			| a0: srcMask
	move.l	(16+11*4,sp),d0			| d0.w: destX
	move.l	(20+11*4,sp),d1			| d1.w: destY
	move.l	(24+11*4,sp),d3			| d3.w: dstPitch
	ext.l	d3				| d3.l: dstPitch
	move.l	(28+11*4,sp),d6			| d6.w: w
	lsr.w	#4,d6				| d6.w: w/16
	move.l	(32+11*4,sp),d7			| d7.w: h
#endif

	move.w  d0,d2				| / Calculate the
	andi.w  #0b111111110000,d0		| | number of bits
	sub.w   d0,d2				| \ to shift right.
	adda.w  d0,a2				| Add x-position to screenaddress.
	mulu.w  d3,d1				| / Add y-position to
	adda.l  d1,a2				| \ screenaddress.
	move.w  d6,d1				| / Prepare
	lsl.w   #4,d1				| | offset
	move.l  d3,d4				| | to next
	sub.w   d1,d4				| \ screenline.
	subq.w  #1,d7				| Adjust for dbra.
	subq.w  #1,d6				| Adjust for dbra.
	move.w  d6,d5				| Backup xloopcount in d5.w.
	moveq   #32,d1				| Size of two chunks.

sprite8_yloop:
	move.w  d5,d6				| Restore xloop counter.

sprite8_xloop:
	moveq   #0xffffffff,d0			| Prepare for maskshifting.
	move.w  (a0)+,d0			| Get 16pixel mask in d0.w.
	ror.l   d2,d0				| Shift it!
	and.w   d0,(a2)+			| Mask bitplane 0.
	and.w   d0,(a2)+			| Mask bitplane 1.
	and.w   d0,(a2)+			| Mask bitplane 2.
	and.w   d0,(a2)+			| Mask bitplane 3.
	and.w   d0,(a2)+			| Mask bitplane 4.
	and.w   d0,(a2)+			| Mask bitplane 5.
	and.w   d0,(a2)+			| Mask bitplane 6.
	and.w   d0,(a2)+			| Mask bitplane 7.
	swap    d0				| Get overspill in loword.
	and.w   d0,(a2)+			| Mask overspill bitplane 0.
	and.w   d0,(a2)+			| Mask overspill bitplane 1.
	and.w   d0,(a2)+			| Mask overspill bitplane 2.
	and.w   d0,(a2)+			| Mask overspill bitplane 3.
	and.w   d0,(a2)+			| Mask overspill bitplane 4.
	and.w   d0,(a2)+			| Mask overspill bitplane 5.
	and.w   d0,(a2)+			| Mask overspill bitplane 6.
	and.w   d0,(a2)+			| Mask overspill bitplane 7.
	suba.l  d1,a2				| Return to blockstart.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 0.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 0.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 1.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 1.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 2.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 2.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 3.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 3.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 4.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 4.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 5.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 5.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 6.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 6.

	moveq   #0,d0				| Prepare for bitmapshifting.
	move.w  (a1)+,d0			| Get bitplaneword in d0.w.
	ror.l   d2,d0				| Shift it.
	or.w    d0,(a2)+			| Paint bitplane 7.
	swap    d0				| Get overspill in loword.
	or.w    d0,14(a2)			| Paint overspill bitplane 7.

	dbra    d6,sprite8_xloop		| Loop until blocks done.

	adda.l  d4,a2				| Goto next screenline.
	dbra    d7,sprite8_yloop		| Loop until lines done.

	movem.l	(sp)+,d0-d7/a0-a2
	rts


	.bss
	.even

save_pal:
	ds.l	256+16/2			| old colours (sized for falcon+ste palette)
save_video:
	ds.b	32+12+2				| old video regs (size of falcon regs)
