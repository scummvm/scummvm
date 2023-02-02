/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <nds.h>

#include "backends/platform/ds/background.h"
#include "backends/platform/ds/blitters.h"

#include "graphics/surface.h"

namespace DS {

static BgSize getBgSize(uint16 width, uint16 height, bool isRGB, bool swScale, uint16 &realPitch, uint16 &realHeight) {
	if (swScale) {
		isRGB = true;
		width = (width * 4) / 5;
	}

	BgSize size;
	if (width > 512 && !isRGB) {
		size = BgSize_B8_1024x512;
		realPitch = 1024;
		realHeight = 512;
	} else if (height > 512 && !isRGB) {
		size = BgSize_B8_512x1024;
		realPitch = 512;
		realHeight = 1024;
	} else if (height > 256) {
		if (isRGB) {
			size = BgSize_B16_512x512;
			realPitch = 1024;
		} else {
			size = BgSize_B8_512x512;
			realPitch = 512;
		}
		realHeight = 512;
	} else if (width > 256) {
		if (isRGB) {
			size = BgSize_B16_512x256;
			realPitch = 1024;
		} else {
			size = BgSize_B8_512x256;
			realPitch = 512;
		}
		realHeight = 256;
	} else if (width > 128 || height > 128) {
		if (isRGB) {
			size = BgSize_B16_256x256;
			realPitch = 512;
		} else {
			size = BgSize_B8_256x256;
			realPitch = 256;
		}
		realHeight = 256;
	} else {
		if (isRGB) {
			size = BgSize_B16_128x128;
			realPitch = 256;
		} else {
			size = BgSize_B8_128x128;
			realPitch = 128;
		}
		realHeight = 128;
	}
	return size;
}

size_t Background::getRequiredVRAM(uint16 width, uint16 height, bool isRGB, bool swScale) {
	uint16 realPitch, realHeight;
	/* BgSize size = */ getBgSize(width, height, isRGB, swScale, realPitch, realHeight);
	return realPitch * realHeight;
}

Background::Background(Graphics::Surface *surface, int layer, bool isSub, int mapBase, bool swScale) :
	_bg(-1), _visible(true), _swScale(swScale),
	_scaleX(1 << 8), _scaleY(1 << 8), _scrollX(0), _scrollY(0),
	_surface(surface), _pitch(0) {

	assert(_surface);

	uint16 realPitch, realHeight;
	bool isRGB = !_surface->format.isCLUT8();
	BgType type = (isRGB || swScale) ? BgType_Bmp16 : BgType_Bmp8;
	BgSize size = getBgSize(_surface->w, _surface->h, isRGB, swScale, realPitch, realHeight);

	if (isSub) {
		_bg = bgInitSub(layer, type, size, mapBase, 0);
	} else {
		_bg = bgInit(layer, type, size, mapBase, 0);
	}

	_pitch = realPitch;
}

static void dmaBlit(uint16 *dst, const uint dstPitch, const uint16 *src, const uint srcPitch,
					const uint w, const uint h, const uint bytesPerPixel) {
	if (dstPitch == srcPitch && ((w * bytesPerPixel) == dstPitch)) {
		dmaCopy(src, dst, dstPitch * h);
		return;
	}

	// The DS video RAM doesn't support 8-bit writes because Nintendo wanted
	// to save a few pennies/euro cents on the hardware.

	uint row = w * bytesPerPixel;

	for (uint dy = 0; dy < h; dy += 2) {
		const u16 *src1 = src;
		src += (srcPitch >> 1);
		DC_FlushRange(src1, row << 1);

		const u16 *src2 = src;
		src += (srcPitch >> 1);
		DC_FlushRange(src2, row << 1);

		u16 *dest1 = dst;
		dst += (dstPitch >> 1);
		DC_FlushRange(dest1, row << 1);

		u16 *dest2 = dst;
		dst += (dstPitch >> 1);
		DC_FlushRange(dest2, row << 1);

		dmaCopyHalfWordsAsynch(2, src1, dest1, row);
		dmaCopyHalfWordsAsynch(3, src2, dest2, row);

		while (dmaBusy(2) || dmaBusy(3));
	}
}

void Background::update() {
	if (_bg < 0)
		return;

	u16 *dst = bgGetGfxPtr(_bg);
	if (_swScale) {
		if (_surface->format == Graphics::PixelFormat::createFormatCLUT8()) {
			Rescale_320x256xPAL8_To_256x256x1555(
				dst, (const u8 *)_surface->getPixels(), _pitch / 2, _surface->pitch, BG_PALETTE, _surface->h);
		} else {
			Rescale_320x256x1555_To_256x256x1555(
				dst, (const u16 *)_surface->getPixels(), _pitch / 2, _surface->pitch / 2);
		}
	} else {
		dmaBlit(dst, _pitch, (const u16 *)_surface->getPixels(), _surface->pitch, _surface->w, _surface->h, _surface->format.bytesPerPixel);
	}
}

void Background::reset() {
	if (_bg < 0)
		return;

	u16 *dst = bgGetGfxPtr(_bg);
	dmaFillHalfWords(0, dst, _pitch * _surface->h);
}

void Background::show() {
	if (_bg >= 0)
		bgShow(_bg);
	_visible = true;
}

void Background::hide() {
	if (_bg >= 0)
		bgHide(_bg);
	_visible = false;
}

void Background::setScalef(int32 sx, int32 sy) {
	if (_bg < 0 || (_scaleX == sx && _scaleY == sy))
			return;

	bgSetScale(_bg, _swScale ? 256 : sx, sy);
	_scaleX = sx;
	_scaleY = sy;
}

void Background::setScrollf(int32 x, int32 y) {
	if (_bg < 0 || (_scrollX == x && _scrollY == y))
			return;

	bgSetScrollf(_bg, x, y);
	_scrollX = x;
	_scrollY = y;
}

Common::Point Background::realToScaled(int16 x, int16 y) const {
	x = CLIP<int16>(((x * _scaleX) + _scrollX) >> 8, 0, _surface->w  - 1);
	y = CLIP<int16>(((y * _scaleY) + _scrollY) >> 8, 0, _surface->h - 1);
	return Common::Point(x, y);
}

Common::Point Background::scaledToReal(int16 x, int16 y) const {
	x = ((x << 8) - _scrollX) / _scaleX;
	y = ((y << 8) - _scrollY) / _scaleY;
	return Common::Point(x, y);
}

TiledBackground::TiledBackground(const unsigned int *tiles, size_t tilesLen, const unsigned short *map, size_t mapLen, int layer, bool isSub, int mapBase, int tileBase) :
	_tiles(tiles), _tilesLen(tilesLen), _map(map), _mapLen(mapLen),
	_bg(-1), _visible(true) {

	if (isSub) {
		_bg = bgInitSub(layer, BgType_Text8bpp, BgSize_T_256x256, mapBase, tileBase);
	} else {
		_bg = bgInit(layer, BgType_Text8bpp, BgSize_T_256x256, mapBase, tileBase);
	}
}

void TiledBackground::update() {
	if (_bg < 0)
		return;

	dmaCopy(_tiles, bgGetGfxPtr(_bg), _tilesLen);
	dmaCopy(_map, bgGetMapPtr(_bg), _mapLen);
}

void TiledBackground::reset() {
	if (_bg < 0)
		return;

	dmaFillHalfWords(0, bgGetMapPtr(_bg), _mapLen);
	dmaFillHalfWords(0, bgGetGfxPtr(_bg), _tilesLen);
}

void TiledBackground::show() {
	if (_bg >= 0)
		bgShow(_bg);
	_visible = true;
}

void TiledBackground::hide() {
	if (_bg >= 0)
		bgHide(_bg);
	_visible = false;
}

} // End of namespace DS
