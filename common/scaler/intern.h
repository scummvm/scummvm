/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#ifndef COMMON_SCALER_INTERN_H
#define COMMON_SCALER_INTERN_H

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/scaler.h"
#include "common/util.h"


static const uint32 redblueMask_565 = 0xF81F;
static const uint32 greenMask_565 = 0x07E0;
static const uint32 redblueMask_555 = 0x7C1F;
static const uint32 greenMask_555 = 0x03E0;


extern uint32 colorMask;
extern uint32 lowPixelMask;
extern uint32 qcolorMask;
extern uint32 qlowpixelMask;

extern int gBitFormat;


/**
 * Interpolate two 16 bit pixels with the weights specified in the template
 * parameters. Used by the hq scaler family.
 */
template<int bitFormat, int w1, int w2>
static inline uint16 interpolate16_2(uint16 p1, uint16 p2) {
	if (bitFormat == 565)
		return ((((p1 & redblueMask_565) * w1 + (p2 & redblueMask_565) * w2) / (w1 + w2)) & redblueMask_565) |
		       ((((p1 & greenMask_565) * w1 + (p2 & greenMask_565) * w2) / (w1 + w2)) & greenMask_565);
	else // bitFormat == 555
		return ((((p1 & redblueMask_555) * w1 + (p2 & redblueMask_555) * w2) / (w1 + w2)) & redblueMask_555) |
		       ((((p1 & greenMask_555) * w1 + (p2 & greenMask_555) * w2) / (w1 + w2)) & greenMask_555);
}

/**
 * Interpolate three 16 bit pixels with the weights specified in the template
 * parameters. Used by the hq scaler family.
 */
template<int bitFormat, int w1, int w2, int w3>
static inline uint16 interpolate16_3(uint16 p1, uint16 p2, uint16 p3) {
	if (bitFormat == 565)
		return ((((p1 & redblueMask_565) * w1 + (p2 & redblueMask_565) * w2 + (p3 & redblueMask_565) * w3) / (w1 + w2 + w3)) & redblueMask_565) |
		       ((((p1 & greenMask_565) * w1 + (p2 & greenMask_565) * w2 + (p3 & greenMask_565) * w3) / (w1 + w2 + w3)) & greenMask_565);
	else // bitFormat == 555
		return ((((p1 & redblueMask_555) * w1 + (p2 & redblueMask_555) * w2 + (p3 & redblueMask_555) * w3) / (w1 + w2 + w3)) & redblueMask_555) |
		       ((((p1 & greenMask_555) * w1 + (p2 & greenMask_555) * w2 + (p3 & greenMask_555) * w3) / (w1 + w2 + w3)) & greenMask_555);
}


/**
 * Compare two YUV values (encoded 8-8-8) and check if they differ by more than
 * a certain hard coded threshold. Used by the hq scaler family.
 */
static inline bool diffYUV(int yuv1, int yuv2) {
	static const int Ymask = 0x00FF0000;
	static const int Umask = 0x0000FF00;
	static const int Vmask = 0x000000FF;
	static const int trY   = 0x00300000;
	static const int trU   = 0x00000700;
	static const int trV   = 0x00000006;

	return
	  ( ( ABS((yuv1 & Ymask) - (yuv2 & Ymask)) > trY ) ||
	    ( ABS((yuv1 & Umask) - (yuv2 & Umask)) > trU ) ||
	    ( ABS((yuv1 & Vmask) - (yuv2 & Vmask)) > trV ) );
}

/**
 * 16bit RGB to YUV conversion table. This table is setup by InitLUT().
 * Used by the hq scaler family.
 */
extern int   RGBtoYUV[65536];

#endif
