/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "graphics/scaler.h"
#include "graphics/scaler/intern.h"

template<int bitFormat>
uint16 quadBlockInterpolate(const uint8* src, uint32 srcPitch) {
	uint16 colorx1y1 = *(((const uint16*)src));
	uint16 colorx2y1 = *(((const uint16*)src) + 1);

	uint16 colorx1y2 = *(((const uint16*)(src + srcPitch)));
	uint16 colorx2y2 = *(((const uint16*)(src + srcPitch)) + 1);

	return interpolate32_1_1_1_1<bitFormat>(colorx1y1, colorx2y1, colorx1y2, colorx2y2);
}

template<int bitFormat>
void createThumbnail_2(const uint8* src, uint32 srcPitch, uint8* dstPtr, uint32 dstPitch, int width, int height) {
	assert(width % 2 == 0);
	assert(height % 2 == 0);
	for (int y = 0; y < height; y += 2) {
		for (int x = 0; x < width; x += 2, dstPtr += 2) {
			*((uint16*)dstPtr) = quadBlockInterpolate<bitFormat>(src + 2 * x, srcPitch);
		}
		dstPtr += (dstPitch - 2 * width / 2);
		src += 2 * srcPitch;
	}
}

template<int bitFormat>
void createThumbnail_4(const uint8* src, uint32 srcPitch, uint8* dstPtr, uint32 dstPitch, int width, int height) {
	assert(width % 4 == 0);
	assert(height % 4 == 0);
	for (int y = 0; y < height; y += 4) {
		for (int x = 0; x < width; x += 4, dstPtr += 2) {
			uint16 upleft = quadBlockInterpolate<bitFormat>(src + 2 * x, srcPitch);
			uint16 upright = quadBlockInterpolate<bitFormat>(src + 2 * (x + 2), srcPitch);
			uint16 downleft = quadBlockInterpolate<bitFormat>(src + srcPitch * 2 + 2 * x, srcPitch);
			uint16 downright = quadBlockInterpolate<bitFormat>(src + srcPitch * 2 + 2 * (x + 2), srcPitch);

			*((uint16*)dstPtr) = interpolate32_1_1_1_1<bitFormat>(upleft, upright, downleft, downright);
		}
		dstPtr += (dstPitch - 2 * width / 4);
		src += 4 * srcPitch;
	}
}

void createThumbnail(const uint8* src, uint32 srcPitch, uint8* dstPtr, uint32 dstPitch, int width, int height) {
	// only 1/2 and 1/4 downscale supported
	if (width != 320 && width != 640)
		return;

	int downScaleMode = (width == 320) ? 2 : 4;

	if (downScaleMode == 2) {
		if (gBitFormat == 565)
			createThumbnail_2<565>(src, srcPitch, dstPtr, dstPitch, width, height);
		else if (gBitFormat == 555)
			createThumbnail_2<555>(src, srcPitch, dstPtr, dstPitch, width, height);
	} else if (downScaleMode == 4) {
		if (gBitFormat == 565)
			createThumbnail_4<565>(src, srcPitch, dstPtr, dstPitch, width, height);
		else if (gBitFormat == 555)
			createThumbnail_4<555>(src, srcPitch, dstPtr, dstPitch, width, height);
	}
}


/**
 * Copies the current screen contents to a new surface, using RGB565 format.
 * WARNING: surf->free() must be called by the user to avoid leaking.
 *
 * @param surf		the surfce to store the data in it
 */
static bool grabScreen565(Graphics::Surface *surf) {
	Graphics::Surface screen;
	if (!g_system->grabRawScreen(&screen))
		return false;

	assert(screen.bytesPerPixel == 1 && screen.pixels != 0);

	byte palette[256 * 4];
	g_system->grabPalette(&palette[0], 0, 256);

	surf->create(screen.w, screen.h, 2);

	for (uint y = 0; y < screen.h; ++y) {
		for (uint x = 0; x < screen.w; ++x) {
			byte r, g, b;
			r = palette[((uint8*)screen.pixels)[y * screen.pitch + x] * 4];
			g = palette[((uint8*)screen.pixels)[y * screen.pitch + x] * 4 + 1];
			b = palette[((uint8*)screen.pixels)[y * screen.pitch + x] * 4 + 2];

			((uint16*)surf->pixels)[y * surf->w + x] = (((r >> 3) & 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | ((b >> 3) & 0x1F);
		}
	}

	screen.free();
	return true;
}

bool createThumbnailFromScreen(Graphics::Surface* surf) {
	assert(surf);

	int screenWidth = g_system->getWidth();
	int screenHeight = g_system->getHeight();

	Graphics::Surface screen;

	if (!grabScreen565(&screen))
		return false;

	uint16 width = screenWidth;

	if (screenWidth < 320) {
		// Special case to handle MM NES (uses a screen width of 256)
		width = 320;

		// center MM NES screen
		Graphics::Surface newscreen;
		newscreen.create(width, screen.h, screen.bytesPerPixel);

		uint8 *dst = (uint8*)newscreen.getBasePtr((320 - screenWidth) / 2, 0);
		uint8 *src = (uint8*)screen.getBasePtr(0, 0);
		uint16 height = screen.h;

		while (height--) {
			memcpy(dst, src, screen.pitch);
			dst += newscreen.pitch;
			src += screen.pitch;
		}

		screen.free();
		screen = newscreen;
	} else if (screenWidth == 720) {
		// Special case to handle Hercules mode
		width = 640;
		screenHeight = 400;

		// cut off menu and so on..
		Graphics::Surface newscreen;
		newscreen.create(width, 400, screen.bytesPerPixel);

		uint8 *dst = (uint8*)newscreen.getBasePtr(0, (400 - 240) / 2);
		uint8 *src = (uint8*)screen.getBasePtr(41, 28);

		for (int y = 0; y < 240; ++y) {
			memcpy(dst, src, 640 * screen.bytesPerPixel);
			dst += newscreen.pitch;
			src += screen.pitch;
		}

		screen.free();
		screen = newscreen;
	}

	uint16 newHeight = !(screenHeight % 240) ? kThumbnailHeight2 : kThumbnailHeight1;

	int gBitFormatBackUp = gBitFormat;
	gBitFormat = 565;
	surf->create(kThumbnailWidth, newHeight, sizeof(uint16));
	createThumbnail((const uint8*)screen.pixels, width * sizeof(uint16), (uint8*)surf->pixels, surf->pitch, width, screenHeight);
	gBitFormat = gBitFormatBackUp;

	screen.free();

	return true;
}
