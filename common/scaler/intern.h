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

template<int bitFormat>
struct ColorMasks {
};

template<>
struct ColorMasks<565> {
	enum {
		highBits = 0xF7DEF7DE,
		lowBits = 0x08210821,
		qhighBits = 0xE79CE79C,
		qlowBits = 0x18631863,
		redblueMask = 0xF81F,
		greenMask = 0x07E0
	};
};

template<>
struct ColorMasks<555> {
	enum {
		highBits = 0x7BDE7BDE,
		lowBits = 0x04210421,
		qhighBits = 0x739C739C,
		qlowBits = 0x0C630C63,
		redblueMask = 0x7C1F,
		greenMask = 0x03E0
	};
};

#define highBits	ColorMasks<bitFormat>::highBits
#define lowBits		ColorMasks<bitFormat>::lowBits
#define qhighBits	ColorMasks<bitFormat>::qhighBits
#define qlowBits	ColorMasks<bitFormat>::qlowBits
#define redblueMask	ColorMasks<bitFormat>::redblueMask
#define greenMask	ColorMasks<bitFormat>::greenMask


/**
 * Interpolate two 16 bit pixel pairs at once with equal weights 1.
 * In particular, A and B can contain two pixels/each in the upper
 * and lower halves.
 */
template<int bitFormat>
static inline uint32 INTERPOLATE(uint32 A, uint32 B) {

	return (((A & highBits) >> 1) + ((B & highBits) >> 1) + (A & B & lowBits));
}

/**
 * Interpolate four 16 bit pixel pairs at once with equal weights 1.
 * In particular, A and B can contain two pixels/each in the upper
 * and lower halves.
 */
template<int bitFormat>
static inline uint32 Q_INTERPOLATE(uint32 A, uint32 B, uint32 C, uint32 D) {
	register uint32 x = ((A & qhighBits) >> 2) + ((B & qhighBits) >> 2) + ((C & qhighBits) >> 2) + ((D & qhighBits) >> 2);
	register uint32 y = ((A & qlowBits) + (B & qlowBits) + (C & qlowBits) + (D & qlowBits)) >> 2;

	y &= qlowBits;
	return x + y;
}


/**
 * Interpolate two 16 bit pixels with the weights specified in the template
 * parameters. Used by the hq scaler family.
 */
template<int bitFormat, int w1, int w2>
static inline uint16 interpolate16_2(uint16 p1, uint16 p2) {
	return ((((p1 & redblueMask) * w1 + (p2 & redblueMask) * w2) / (w1 + w2)) & redblueMask) |
	       ((((p1 & greenMask) * w1 + (p2 & greenMask) * w2) / (w1 + w2)) & greenMask);
}

/**
 * Interpolate three 16 bit pixels with the weights specified in the template
 * parameters. Used by the hq scaler family.
 */
template<int bitFormat, int w1, int w2, int w3>
static inline uint16 interpolate16_3(uint16 p1, uint16 p2, uint16 p3) {
	return ((((p1 & redblueMask) * w1 + (p2 & redblueMask) * w2 + (p3 & redblueMask) * w3) / (w1 + w2 + w3)) & redblueMask) |
		   ((((p1 & greenMask) * w1 + (p2 & greenMask) * w2 + (p3 & greenMask) * w3) / (w1 + w2 + w3)) & greenMask);
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
	
	int diff;
	int mask;
	
	diff = ((yuv1 & Ymask) - (yuv2 & Ymask));
	mask = diff >> 31; // -1 if value < 0, 0 otherwise 
	diff = (diff ^ mask) - mask; //-1: ~value + 1; 0: value 
	if (diff > trY) return true;

	diff = ((yuv1 & Umask) - (yuv2 & Umask));
	mask = diff >> 31; // -1 if value < 0, 0 otherwise 
	diff = (diff ^ mask) - mask; //-1: ~value + 1; 0: value 
	if (diff > trU) return true;

	diff = ((yuv1 & Vmask) - (yuv2 & Vmask));
	mask = diff >> 31; // -1 if value < 0, 0 otherwise 
	diff = (diff ^ mask) - mask; //-1: ~value + 1; 0: value 
	if (diff > trV) return true;

	return false;
/*
	return
	  ( ( ABS((yuv1 & Ymask) - (yuv2 & Ymask)) > trY ) ||
	    ( ABS((yuv1 & Umask) - (yuv2 & Umask)) > trU ) ||
	    ( ABS((yuv1 & Vmask) - (yuv2 & Vmask)) > trV ) );
*/
}

/**
 * 16bit RGB to YUV conversion table. This table is setup by InitLUT().
 * Used by the hq scaler family.
 */
extern int   RGBtoYUV[65536];

/** Auxiliary macro to simplify creating those template function wrappers. */
#define MAKE_WRAPPER(FUNC) \
	void FUNC(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) { \
		if (gBitFormat == 565) \
			FUNC ## Template<565>(srcPtr, srcPitch, dstPtr, dstPitch, width, height); \
		else \
			FUNC ## Template<555>(srcPtr, srcPitch, dstPtr, dstPitch, width, height); \
	}

/** Specifies the currently active 16bit pixel format, 555 or 565. */
extern int gBitFormat;

#endif
