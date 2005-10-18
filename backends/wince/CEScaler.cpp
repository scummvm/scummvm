/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */
#include "common/scaler/intern.h"
#include "common/stdafx.h"
#include "CEScaler.h"

int redblueMasks[] = { 0x7C1F, 0xF81F };
int greenMasks[] = { 0x03E0, 0x07E0 };

static int maskUsed;

void initCEScaler(void) {
	if (gBitFormat == 555)
		maskUsed = 0;
	else
		maskUsed = 1;
}

static inline uint16 CEinterpolate16_4(uint16 p1, uint16 p2, uint16 p3, uint16 p4)
{
        return ((((p1 & redblueMasks[maskUsed]) + (p2 & redblueMasks[maskUsed]) + (p3 & redblueMasks[maskUsed]) + (p4 & redblueMasks[maskUsed])) / 4) & redblueMasks[maskUsed]) |
               ((((p1 & greenMasks[maskUsed]) + (p2 & greenMasks[maskUsed]) + (p3 & greenMasks[maskUsed]) + (p4 & greenMasks[maskUsed])) / 4) & greenMasks[maskUsed]);
}

static inline uint16 CEinterpolate16_2(uint16 p1, int w1, uint16 p2, int w2) {
        return ((((p1 & redblueMasks[maskUsed]) * w1 + (p2 & redblueMasks[maskUsed]) * w2) / (w1 + w2)) & redblueMasks[maskUsed]) |
               ((((p1 & greenMasks[maskUsed]) * w1 + (p2 & greenMasks[maskUsed]) * w2) / (w1 + w2)) & greenMasks[maskUsed]);
}

void PocketPCPortrait(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint8 *work;
	int i;

	while (height--) {
		i = 0;
		work = dstPtr;

		for (int i=0; i<width; i+=4) {
			// Work with 4 pixels
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + 2));
			uint16 color4 = *(((const uint16 *)srcPtr) + (i + 3));

			*(((uint16 *)work) + 0) = CEinterpolate16_2(color1, 3, color2, 1);
			*(((uint16 *)work) + 1) = CEinterpolate16_2(color2, 1, color3, 1);
			*(((uint16 *)work) + 2) = CEinterpolate16_2(color3, 1, color4, 3);

			work += 3 * sizeof(uint16);
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void PocketPCHalf(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint8 *work;
	int i;
	uint16 srcPitch16 = (uint16)(srcPitch / sizeof(uint16));

	while ((height-=2) >= 0) {
		i = 0;
		work = dstPtr;

		for (int i=0; i<width; i+=2) {
			// Another lame filter attempt :)
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + srcPitch16));
			uint16 color4 = *(((const uint16 *)srcPtr) + (i + srcPitch16 + 1));
			*(((uint16 *)work) + 0) = CEinterpolate16_4(color1, color2, color3, color4);

			work += sizeof(uint16);
		}
		srcPtr += 2 * srcPitch;
		dstPtr += dstPitch;
	}
}


void PocketPCHalfZoom(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint8 *work;
	int i;
	uint16 srcPitch16 = (uint16)(srcPitch / sizeof(uint16));

	if (!height)
		return;

	while (height--) {
		i = 0;
		work = dstPtr;

		for (int i=0; i<width; i+=2) {
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			*(((uint16 *)work) + 0) = CEinterpolate16_2(color1, 1, color2, 1);

			work += sizeof(uint16);
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

//#ifdef WIN32_PLATFORM_WFSP
void SmartphoneLandscape(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	uint8 *work;
	int i;
	int line = 0;

	while (height--) {
		i = 0;
		work = dstPtr;

		for (int i=0; i<width; i+=3) {
			// Filter 2/3
			uint16 color1 = *(((const uint16 *)srcPtr) + i);
			uint16 color2 = *(((const uint16 *)srcPtr) + (i + 1));
			uint16 color3 = *(((const uint16 *)srcPtr) + (i + 2));

			*(((uint16 *)work) + 0) = CEinterpolate16_2(color1, 3, color2, 1);
			*(((uint16 *)work) + 1) = CEinterpolate16_2(color2, 1, color3, 1);

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
//#endif

