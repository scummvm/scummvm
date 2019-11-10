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
#include "CEScaler.h"


/**
 * This filter (down)scales the source image horizontally by a factor of 1/2.
 * For example, a 320x200 image is scaled to 160x200.
 */
template<typename ColorMask>
void DownscaleHorizByHalfTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint16 *work;

	// Various casts below go via (void *) to avoid warning. This is
	// safe as these are all even addresses.
	while (height--) {
		work = (uint16 *)(void *)dstPtr;

		for (int i = 0; i < width; i += 2) {
			uint16 color1 = *(((const uint16 *)(const void *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)(const void *)srcPtr) + (i + 1));
			*work++ = interpolate32_1_1<ColorMask>(color1, color2);
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void DownscaleHorizByHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		DownscaleHorizByHalfTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		DownscaleHorizByHalfTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

/**
 * This filter (down)scales the source image horizontally by a factor of 3/4.
 * For example, a 320x200 image is scaled to 240x200.
 */
template<typename ColorMask>
void DownscaleHorizByThreeQuartersTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint16 *work;

	// Various casts below go via (void *) to avoid warning. This is
	// safe as these are all even addresses.
	while (height--) {
		work = (uint16 *)(void *)dstPtr;

		for (int i = 0; i < width; i += 4) {
			// Work with 4 pixels
			uint16 color1 = *(((const uint16 *)(const void *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)(const void *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)(const void *)srcPtr) + (i + 2));
			uint16 color4 = *(((const uint16 *)(const void *)srcPtr) + (i + 3));

			work[0] = interpolate32_3_1<ColorMask>(color1, color2);
			work[1] = interpolate32_1_1<ColorMask>(color2, color3);
			work[2] = interpolate32_3_1<ColorMask>(color4, color3);

			work += 3;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void DownscaleHorizByThreeQuarters(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		DownscaleHorizByThreeQuartersTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		DownscaleHorizByThreeQuartersTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

extern int gBitFormat;
#ifdef ARM
extern "C" {
	void SmartphoneLandscapeARM(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height, int mask);
}

void SmartphoneLandscape(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	// Rounding constants and masks used for different pixel formats
	static const int redbluegreenMasks[] = { 0x03E07C1F, 0x07E0F81F };
	const int maskUsed = (gBitFormat == 565);
	SmartphoneLandscapeARM(srcPtr, srcPitch, dstPtr, dstPitch, width, height, redbluegreenMasks[maskUsed]);
}

#else

template<typename ColorMask>
void SmartphoneLandscapeTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	int line = 0;

	assert((width % 16) == 0);

	while (height--) {
		uint16 *d = (uint16 *)dstPtr;

		const uint16 *s = (const uint16 *)srcPtr;
		for (int i = 0; i < width; i += 16) {
			// Downscale horizontally to 11/16.
			// See smartLandScale.s for an explanation of the scale pattern.
			*d++ = interpolate32_3_1<ColorMask>(s[0], s[1]);
			*d++ = interpolate32_1_1<ColorMask>(s[1], s[2]);
			*d++ = interpolate32_3_1<ColorMask>(s[3], s[2]);
			*d++ = interpolate32_1_1<ColorMask>(s[4], s[5]);
			*d++ = interpolate32_3_1<ColorMask>(s[6], s[7]);
			*d++ = interpolate32_1_1<ColorMask>(s[7], s[8]);
			*d++ = interpolate32_3_1<ColorMask>(s[9], s[8]);
			*d++ = interpolate32_1_1<ColorMask>(s[10], s[11]);
			*d++ = interpolate32_3_1<ColorMask>(s[12], s[13]);
			*d++ = interpolate32_1_1<ColorMask>(s[13], s[14]);
			*d++ = interpolate32_3_1<ColorMask>(s[15], s[14]);

			s += 16;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
		line++;

		// Skip every 8th row
		if (line == 7) {
			line = 0;
			srcPtr += srcPitch;
			height--;
		}
	}
}

void SmartphoneLandscape(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	if (gBitFormat == 565)
		SmartphoneLandscapeTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		SmartphoneLandscapeTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif
