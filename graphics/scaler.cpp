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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/scaler/intern.h"
#include "graphics/scaler/scalebit.h"
#include "common/util.h"
#include "common/system.h"
#include "common/textconsole.h"

int gBitFormat = 565;

/** Lookup table for the DotMatrix scaler. */
uint16 g_dotmatrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/** Init the scaler subsystem. */
void InitScalers(uint32 BitFormat) {
	gBitFormat = BitFormat;

	// FIXME: The pixelformat should be param to this function, not the bitformat.
	// Until then, determine the pixelformat in other ways. Unfortunately,
	// calling OSystem::getOverlayFormat() here might not be safe on all ports.
	Graphics::PixelFormat format;
	if (gBitFormat == 555) {
		format = Graphics::createPixelFormat<555>();
	} else if (gBitFormat == 565) {
		format = Graphics::createPixelFormat<565>();
	} else {
		assert(g_system);
		format = g_system->getOverlayFormat();
	}

	// Build dotmatrix lookup table for the DotMatrix scaler.
	g_dotmatrix[0] = g_dotmatrix[10] = format.RGBToColor( 0, 63,  0);
	g_dotmatrix[1] = g_dotmatrix[11] = format.RGBToColor( 0,  0, 63);
	g_dotmatrix[2] = g_dotmatrix[ 8] = format.RGBToColor(63,  0,  0);
	g_dotmatrix[4] = g_dotmatrix[ 6] =
		g_dotmatrix[12] = g_dotmatrix[14] = format.RGBToColor(63, 63, 63);
}

void DestroyScalers() {
}


/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destination.
 */
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	// Spot the case when it can all be done in 1 hit
	if ((srcPitch == sizeof(uint16) * (uint)width) && (dstPitch == sizeof(uint16) * (uint)width)) {
		memcpy(dstPtr, srcPtr, sizeof(uint16) * width * height);
		return;
	}
	while (height--) {
		memcpy(dstPtr, srcPtr, sizeof(uint16) * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

#ifdef USE_SCALERS


#ifdef USE_ARM_SCALER_ASM
extern "C" void Normal2xARM(const uint8  *srcPtr,
                                  uint32  srcPitch,
                                  uint8  *dstPtr,
                                  uint32  dstPitch,
                                  int     width,
                                  int     height);

void Normal2x(const uint8  *srcPtr,
                    uint32  srcPitch,
                    uint8  *dstPtr,
                    uint32  dstPitch,
                    int     width,
                    int     height) {
	Normal2xARM(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#else
/**
 * Trivial nearest-neighbor 2x scaler.
 */
void Normal2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;

	assert(IS_ALIGNED(dstPtr, 4));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 4) {
			uint32 color = *(((const uint16 *)srcPtr) + i);

			color |= color << 16;

			*(uint32 *)(r) = color;
			*(uint32 *)(r + dstPitch) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
	}
}
#endif

/**
 * Trivial nearest-neighbor 3x scaler.
 */
void Normal3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	const uint32 dstPitch2 = dstPitch * 2;
	const uint32 dstPitch3 = dstPitch * 3;

	assert(IS_ALIGNED(dstPtr, 2));
	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 6) {
			uint16 color = *(((const uint16 *)srcPtr) + i);

			*(uint16 *)(r + 0) = color;
			*(uint16 *)(r + 2) = color;
			*(uint16 *)(r + 4) = color;
			*(uint16 *)(r + 0 + dstPitch) = color;
			*(uint16 *)(r + 2 + dstPitch) = color;
			*(uint16 *)(r + 4 + dstPitch) = color;
			*(uint16 *)(r + 0 + dstPitch2) = color;
			*(uint16 *)(r + 2 + dstPitch2) = color;
			*(uint16 *)(r + 4 + dstPitch2) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch3;
	}
}

/**
 * The Scale2x filter, also known as AdvMame2x.
 * See also http://scale2x.sourceforge.net
 */
void AdvMame2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	scale(2, dstPtr, dstPitch, srcPtr - srcPitch, srcPitch, 2, width, height);
}

/**
 * The Scale3x filter, also known as AdvMame3x.
 * See also http://scale2x.sourceforge.net
 */
void AdvMame3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	scale(3, dstPtr, dstPitch, srcPtr - srcPitch, srcPitch, 2, width, height);
}

template<typename ColorMask>
void TV2xTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height) {
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	while (height--) {
		for (int i = 0, j = 0; i < width; ++i, j += 2) {
			uint16 p1 = *(p + i);
			uint32 pi;

			pi = (((p1 & ColorMask::kRedBlueMask) * 7) >> 3) & ColorMask::kRedBlueMask;
			pi |= (((p1 & ColorMask::kGreenMask) * 7) >> 3) & ColorMask::kGreenMask;

			*(q + j) = p1;
			*(q + j + 1) = p1;
			*(q + j + nextlineDst) = (uint16)pi;
			*(q + j + nextlineDst + 1) = (uint16)pi;
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

void TV2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	if (gBitFormat == 565)
		TV2xTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		TV2xTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

static inline uint16 DOT_16(const uint16 *dotmatrix, uint16 c, int j, int i) {
	return c - ((c >> 2) & dotmatrix[((j & 3) << 2) + (i & 3)]);
}


// FIXME: This scaler doesn't quite work. Either it needs to know where on the
// screen it's drawing, or the dirty rects will have to be adjusted so that
// access to the dotmatrix array are made in a consistent way. (Doing that in
// a way that also works together with aspect-ratio correction is left as an
// exercise for the reader.)

void DotMatrix(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height) {

	const uint16 *dotmatrix = g_dotmatrix;

	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	for (int j = 0, jj = 0; j < height; ++j, jj += 2) {
		for (int i = 0, ii = 0; i < width; ++i, ii += 2) {
			uint16 c = *(p + i);
			*(q + ii) = DOT_16(dotmatrix, c, jj, ii);
			*(q + ii + 1) = DOT_16(dotmatrix, c, jj, ii + 1);
			*(q + ii + nextlineDst) = DOT_16(dotmatrix, c, jj + 1, ii);
			*(q + ii + nextlineDst + 1) = DOT_16(dotmatrix, c, jj + 1, ii + 1);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

#endif // #ifdef USE_SCALERS
