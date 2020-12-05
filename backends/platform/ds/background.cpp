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

#include <nds.h>

#include "backends/platform/ds/background.h"

namespace DS {

BgSize getBgSize(int width, int height, bool isRGB, int &realPitch) {
	BgSize size;
	if (width > 512 && !isRGB) {
		size = BgSize_B8_1024x512;
		realPitch = 1024;
	} else if (height > 512 && !isRGB) {
		size = BgSize_B8_512x1024;
		realPitch = 512;
	} else if (height > 256) {
		if (isRGB) {
			size = BgSize_B16_512x512;
			realPitch = 1024;
		} else {
			size = BgSize_B8_512x512;
			realPitch = 512;
		}
	} else if (width > 256) {
		if (isRGB) {
			size = BgSize_B16_512x256;
			realPitch = 1024;
		} else {
			size = BgSize_B8_512x256;
			realPitch = 512;
		}
	} else if (width > 128 || height > 128) {
		if (isRGB) {
			size = BgSize_B16_256x256;
			realPitch = 512;
		} else {
			size = BgSize_B8_256x256;
			realPitch = 256;
		}
	} else {
		if (isRGB) {
			size = BgSize_B16_128x128;
			realPitch = 256;
		} else {
			size = BgSize_B8_128x128;
			realPitch = 128;
		}
	}
	return size;
}

void Background::create(uint16 width, uint16 height, bool isRGB, int layer, bool isSub, int mapBase) {
	const Graphics::PixelFormat f = isRGB ? Graphics::PixelFormat(2, 5, 5, 5, 1, 0, 5, 10, 15) : Graphics::PixelFormat::createFormatCLUT8();
	Surface::create(width, height, f);

	BgType type = isRGB ? BgType_Bmp16 : BgType_Bmp8;
	BgSize size = getBgSize(width, height, isRGB, _realPitch);

	if (isSub) {
		_bg = bgInitSub(layer, type, size, mapBase, 0);
	} else {
		_bg = bgInit(layer, type, size, mapBase, 0);
	}
}

void Background::update() {
	u16 *src = (u16 *)getPixels();
	u16 *dst = bgGetGfxPtr(_bg);
	dmaCopy(src, dst, _realPitch * h);
}

void Background::reset() {
	u16 *dst = bgGetGfxPtr(_bg);
	dmaFillHalfWords(0, dst, _realPitch * h);
}

void Background::show() {
	bgShow(_bg);
	_visible = true;
}

void Background::hide() {
	bgHide(_bg);
	_visible = false;
}

} // End of namespace DS
