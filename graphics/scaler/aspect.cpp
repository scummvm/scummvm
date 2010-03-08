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

template<typename ColorMask, int scale>
static inline uint16 interpolate5(uint16 A, uint16 B) {
	uint16 r = (uint16)(((A & ColorMask::kRedBlueMask & 0xFF00) * scale + (B & ColorMask::kRedBlueMask & 0xFF00) * (5 - scale)) / 5);
	uint16 g = (uint16)(((A & ColorMask::kGreenMask) * scale + (B & ColorMask::kGreenMask) * (5 - scale)) / 5);
	uint16 b = (uint16)(((A & ColorMask::kRedBlueMask & 0x00FF) * scale + (B & ColorMask::kRedBlueMask & 0x00FF) * (5 - scale)) / 5);

	return (uint16)((r & ColorMask::kRedBlueMask & 0xFF00) | (g & ColorMask::kGreenMask) | (b & ColorMask::kRedBlueMask & 0x00FF));
}


template<typename ColorMask, int scale>
static inline void interpolate5Line(uint16 *dst, const uint16 *srcA, const uint16 *srcB, int width) {
	// Accurate but slightly slower code
	while (width--) {
		*dst++ = interpolate5<ColorMask, scale>(*srcA++, *srcB++);
	}
}
#endif

#if ASPECT_MODE == kFastAndNiceAspectMode

template<typename ColorMask, int scale>
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
			*d++ = interpolate32_3_1<ColorMask>(*sB++, *sA++);
		}
	} else {
		while (width--) {
			*d++ = interpolate32_1_1<ColorMask>(*sB++, *sA++);
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
template<typename ColorMask>
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
			interpolate5Line<ColorMask, 1>((uint16 *)dstPtr, (const uint16 *)(srcPtr - pitch), (const uint16 *)srcPtr, width);
			break;
		case 2:
			interpolate5Line<ColorMask, 2>((uint16 *)dstPtr, (const uint16 *)(srcPtr - pitch), (const uint16 *)srcPtr, width);
			break;
		case 3:
			interpolate5Line<ColorMask, 2>((uint16 *)dstPtr, (const uint16 *)srcPtr, (const uint16 *)(srcPtr - pitch), width);
			break;
		case 4:
			interpolate5Line<ColorMask, 1>((uint16 *)dstPtr, (const uint16 *)srcPtr, (const uint16 *)(srcPtr - pitch), width);
			break;
		}
#endif
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}

int stretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		return stretch200To240<Graphics::ColorMasks<565> >(buf, pitch, width, height, srcX, srcY, origSrcY);
	else // gBitFormat == 555
		return stretch200To240<Graphics::ColorMasks<555> >(buf, pitch, width, height, srcX, srcY, origSrcY);
}


void Normal1xAspect(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {

	extern int gBitFormat;

	const int redblueMasks[] = { 0x7C1F, 0xF81F };
	const int greenMasks[] = { 0x03E0, 0x07E0 };
	const int RBM = redblueMasks[gBitFormat == 565];
	const int GM = greenMasks[gBitFormat == 565];

	int i,j;
	unsigned int p1, p2;
	const uint8 *inbuf, *instart;
	uint8 *outbuf, *outstart;

#define RB(x) ((x & RBM)<<8)
#define G(x)  ((x & GM)<<3)

#define P20(x) (((x)>>2)-((x)>>4))
#define P40(x) (((x)>>1)-((x)>>3))
#define P60(x) (((x)>>1)+((x)>>3))
#define P80(x) (((x)>>1)+((x)>>2)+((x)>>4))

#define MAKEPIXEL(rb,g) ((((rb)>>8) & RBM | ((g)>>3) & GM))

	inbuf = (const uint8 *)srcPtr;
	outbuf = (uint8 *)dstPtr;
	height /= 5;

	// Various casts below go via (void *) to avoid warning. This is
	// safe as these are all even addresses.
	for (i = 0; i < height; i++) {
		instart = inbuf;
		outstart = outbuf;
		for (j=0; j < width; j++) {

			p1 = *(const uint16*)(const void *)inbuf; inbuf += srcPitch;
			*(uint16*)(void *)outbuf = p1; outbuf += dstPitch;

			p2 = *(const uint16*)(const void *)inbuf; inbuf += srcPitch;
			*(uint16*)(void *)outbuf = MAKEPIXEL(P20(RB(p1))+P80(RB(p2)),P20(G(p1))+P80(G(p2)));  outbuf += dstPitch;

			p1 = p2;
			p2 = *(const uint16*)(const void *)inbuf; inbuf += srcPitch;
			*(uint16*)(void *)outbuf = MAKEPIXEL(P40(RB(p1))+P60(RB(p2)),P40(G(p1))+P60(G(p2)));  outbuf += dstPitch;

			p1 = p2;
			p2 = *(const uint16*)(const void *)inbuf; inbuf += srcPitch;
			*(uint16*)(void *)outbuf = MAKEPIXEL(P60(RB(p1))+P40(RB(p2)),P60(G(p1))+P40(G(p2)));  outbuf += dstPitch;

			p1 = p2;
			p2 = *(const uint16*)(const void *)inbuf;
			*(uint16*)(void *)outbuf = MAKEPIXEL(P80(RB(p1))+P20(RB(p2)),P80(G(p1))+P20(G(p2)));  outbuf += dstPitch;

			*(uint16*)(void *)outbuf = p2;

			inbuf = inbuf - srcPitch*4 + sizeof(uint16);
			outbuf = outbuf - dstPitch*5 + sizeof(uint16);
		}
		inbuf = instart + srcPitch*5;
		outbuf = outstart + dstPitch*6;
	}
}


#ifdef USE_ARM_SCALER_ASM
extern "C" void Normal2xAspectMask(const uint8  *srcPtr,
                                         uint32  srcPitch,
                                         uint8  *dstPtr,
                                         uint32  dstPitch,
                                         int     width,
                                         int     height,
                                         uint32  mask);

/**
 * A 2x scaler which also does aspect ratio correction.
 * This is Normal2x combined with vertical stretching,
 * so it will scale a 320x200 surface to a 640x480 surface.
 */
void Normal2xAspect(const uint8  *srcPtr,
                          uint32  srcPitch,
                          uint8  *dstPtr,
                          uint32  dstPitch,
                          int     width,
                          int     height) {
	if (gBitFormat == 565) {
		Normal2xAspectMask(srcPtr,
		                   srcPitch,
		                   dstPtr,
		                   dstPitch,
		                   width,
		                   height,
		                   0x07e0F81F);
	} else {
		Normal2xAspectMask(srcPtr,
		                   srcPitch,
		                   dstPtr,
		                   dstPitch,
		                   width,
		                   height,
		                   0x03e07C1F);
	}
}

#endif	// USE_ARM_SCALER_ASM

