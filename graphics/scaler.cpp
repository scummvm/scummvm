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
	g_dotmatrix[0] = g_dotmatrix[10] = format.RGBToColor(0, 63, 0);
	g_dotmatrix[1] = g_dotmatrix[11] = format.RGBToColor(0, 0, 63);
	g_dotmatrix[2] = g_dotmatrix[8] = format.RGBToColor(63, 0, 0);
	g_dotmatrix[4] = g_dotmatrix[6] =
		g_dotmatrix[12] = g_dotmatrix[14] = format.RGBToColor(63, 63, 63);
}

void DestroyScalers(){
}

#ifdef USE_SCALERS

#define interpolate_1_1		interpolate16_1_1<ColorMask>
#define interpolate_1_1_1_1	interpolate16_1_1_1_1<ColorMask>

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
