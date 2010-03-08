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
#include "CEScaler.h"

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
	uint8 *work;
	int line = 0;

	while (height--) {
		work = dstPtr;

		for (int i = 0; i < width; i += 3) {
			// Filter 2/3
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + 2));

			*(((uint16 *)work) + 0) = interpolate32_3_1<ColorMask>(color1, color2);
			*(((uint16 *)work) + 1) = interpolate32_3_1<ColorMask>(color3, color2);

			work += 2 * sizeof(uint16);
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
		line++;
		if (line == 7) {
			line = 0;
			srcPtr += srcPitch;
			height--;
		}
	}
}

void SmartphoneLandscape(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	extern int gBitFormat;
	if (gBitFormat == 565)
		SmartphoneLandscapeTemplate<Graphics::ColorMasks<565> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		SmartphoneLandscapeTemplate<Graphics::ColorMasks<555> >(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif

