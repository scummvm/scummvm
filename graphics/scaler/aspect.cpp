/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/scaler/intern.h"
#include "graphics/scaler/aspect.h"


#define	kVeryFastAndUglyAspectMode	0	// No interpolation at all, but super-fast
#define	kFastAndNiceAspectMode		1	// Quite good quality with good speed
#define	kSlowAndPerfectAspectMode	2	// Accurate but slow code

#define ASPECT_MODE	kFastAndNiceAspectMode


#if ASPECT_MODE == kSlowAndPerfectAspectMode

template<int bitFormat, int scale>
static inline uint16 interpolate5(uint16 A, uint16 B) {
	uint16 r = (uint16)(((A & redblueMask & 0xFF00) * scale + (B & redblueMask & 0xFF00) * (5 - scale)) / 5);
	uint16 g = (uint16)(((A & greenMask) * scale + (B & greenMask) * (5 - scale)) / 5);
	uint16 b = (uint16)(((A & redblueMask & 0x00FF) * scale + (B & redblueMask & 0x00FF) * (5 - scale)) / 5);

	return (uint16)((r & redblueMask & 0xFF00) | (g & greenMask) | (b & redblueMask & 0x00FF));
}


template<int bitFormat, int scale>
static inline void interpolate5Line(uint16 *dst, const uint16 *srcA, const uint16 *srcB, int width) {
	// Accurate but slightly slower code
	while (width--) {
		*dst++ = interpolate5<bitFormat, scale>(*srcA++, *srcB++);
	}
}
#endif

#if ASPECT_MODE == kFastAndNiceAspectMode

template<int bitFormat, int scale>
static inline void interpolate5Line(uint16 *dst, const uint16 *srcA, const uint16 *srcB, int width) {
	// For efficiency reasons we blit two pixels at a time, so it is important
	// that makeRectStretchable() guarantees that the width is even and that
	// the rect starts on a well-aligned address. (Even where unaligned memory
	// access is allowed there may be a speed penalty for it.)

	// These asserts are disabled for maximal speed; but I leave them in here
	// in case  other people want to test if the memory alignment (to an
	// address divisible by 4) is really working properly.
	//assert(((int)dst & 3) == 0);
	//assert(((int)srcA & 3) == 0);
	//assert(((int)srcB & 3) == 0);
	//assert((width & 1) == 0);

	width /= 2;
	const uint32 *sA = (const uint32 *)srcA;
	const uint32 *sB = (const uint32 *)srcB;
	uint32 *d = (uint32 *)dst;
	if (scale == 1) {
		while (width--) {
			*d++ = interpolate32_3_1<bitFormat>(*sB++, *sA++);
		}
	} else {
		while (width--) {
			*d++ = interpolate32_1_1<bitFormat>(*sB++, *sA++);
		}
	}
}
#endif

void makeRectStretchable(int &x, int &y, int &w, int &h) {
#if ASPECT_MODE != kVeryFastAndUglyAspectMode
	int m = real2Aspect(y) % 6;

	// Ensure that the rect will start on a line that won't have its
	// colours changed by the stretching function.
	if (m != 0 && m != 5) {
		y -= m;
		h += m;
	}

  #if ASPECT_MODE == kFastAndNiceAspectMode
	// Force x to be even, to ensure aligned memory access (this assumes
	// that each line starts at an even memory location, but that should
	// be the case on every target anyway).
	if (x & 1) {
		x--;
		w++;
	}

	// Finally force the width to be even, since we blit 2 pixels at a time.
	// While this means we may sometimes blit one column more than necessary,
	// this should actually be faster than having the check for the
	if (w & 1)
		w++;
  #endif
#endif
}

/**
 * Stretch a 16bpp image vertically by factor 1.2. Used to correct the
 * aspect-ratio in games using 320x200 pixel graphics with non-qudratic
 * pixels. Applying this method effectively turns that into 320x240, which
 * provides the correct aspect-ratio on modern displays.
 *
 * The image would normally have occupied y coordinates origSrcY through
 * origSrcY + height - 1.
 *
 * However, we have already placed it at srcY - the aspect-corrected y
 * coordinate - to allow in-place stretching.
 *
 * Therefore, the source image now occupies Y coordinates srcY through
 * srcY + height - 1, and it should be stretched to Y coordinates srcY
 * through real2Aspect(srcY + height - 1).
 */
template<int bitFormat>
int stretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	int maxDstY = real2Aspect(origSrcY + height - 1);
	int y;
	const uint8 *startSrcPtr = buf + srcX * 2 + (srcY - origSrcY) * pitch;
	uint8 *dstPtr = buf + srcX * 2 + maxDstY * pitch;

	for (y = maxDstY; y >= srcY; y--) {
		const uint8 *srcPtr = startSrcPtr + aspect2Real(y) * pitch;

#if ASPECT_MODE == kVeryFastAndUglyAspectMode
		if (srcPtr == dstPtr)
			break;
		memcpy(dstPtr, srcPtr, width * 2);
#else
		// Bilinear filter
		switch (y % 6) {
		case 0:
		case 5:
			if (srcPtr != dstPtr)
				memcpy(dstPtr, srcPtr, width * 2);
			break;
		case 1:
			interpolate5Line<bitFormat, 1>((uint16 *)dstPtr, (const uint16 *)(srcPtr - pitch), (const uint16 *)srcPtr, width);
			break;
		case 2:
			interpolate5Line<bitFormat, 2>((uint16 *)dstPtr, (const uint16 *)(srcPtr - pitch), (const uint16 *)srcPtr, width);
			break;
		case 3:
			interpolate5Line<bitFormat, 2>((uint16 *)dstPtr, (const uint16 *)srcPtr, (const uint16 *)(srcPtr - pitch), width);
			break;
		case 4:
			interpolate5Line<bitFormat, 1>((uint16 *)dstPtr, (const uint16 *)srcPtr, (const uint16 *)(srcPtr - pitch), width);
			break;
		}
#endif
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}

int stretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	if (gBitFormat == 565)
		return stretch200To240<565>(buf, pitch, width, height, srcX, srcY, origSrcY);
	else // gBitFormat == 555
		return stretch200To240<555>(buf, pitch, width, height, srcX, srcY, origSrcY);
}

