/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef SCALER_H
#define SCALER_H

extern int Init_2xSaI (uint32 BitFormat);
extern void _2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr,
						uint32 dstPitch, int width, int height);
extern void Super2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void SuperEagle(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void AdvMame2x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void Normal1x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void Normal2x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void Normal3x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void TV2x(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
extern void DotMatrix(uint8 *srcPtr, uint32 srcPitch, uint8 *null,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);


enum {
	GFX_NORMAL = 0,
	GFX_DOUBLESIZE = 1,
	GFX_TRIPLESIZE = 2,
	GFX_2XSAI = 3,
	GFX_SUPER2XSAI = 4,
	GFX_SUPEREAGLE = 5,
	GFX_ADVMAME2X = 6,
	GFX_TV2X = 7,
	GFX_DOTMATRIX = 8
};


#endif
