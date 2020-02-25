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
#include <common/endian.h>
#include "common/system.h"
#include "common/rect.h"
#include "engines/util.h"
#include "dragons/dragons.h"
#include "dragons/scene.h"
#include "screen.h"

namespace Dragons {

Screen::Screen() {
	_pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 1, 10, 5, 0, 15);
	initGraphics(320, 200, &_pixelFormat);
	_backSurface = new Graphics::Surface();
	_backSurface->create(320, 200, _pixelFormat);
	_screenShakeOffset = 0;
}

void Screen::updateScreen() {
	g_system->copyRectToScreen((byte*)_backSurface->getBasePtr(0, 0), _backSurface->pitch, 0, 0, _backSurface->w, _backSurface->h);
	g_system->updateScreen();
}

Screen::~Screen() {
	_backSurface->free();
	delete _backSurface;
}

void Screen::copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY) {
	copyRectToSurface(srcSurface.getBasePtr(0, 0), srcSurface.pitch, srcSurface.w, 0, destX, destY, srcSurface.w, srcSurface.h, false, 255);
}

void Screen::copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect srcRect, bool flipX, uint8 alpha) {
	Common::Rect clipRect = clipRectToScreen( destX,  destY, srcRect);
	if (clipRect.width() == 0 || clipRect.height() == 0) {
		return;
	}

	if (destX < 0) {
		destX = 0;
	}
	if (destY < 0) {
		destY = 0;
	}

	copyRectToSurface(srcSurface.getBasePtr(clipRect.left, clipRect.top), srcSurface.pitch, srcSurface.w, clipRect.left, destX, destY, clipRect.width(), clipRect.height(), flipX, alpha);
}

void Screen::copyRectToSurface8bpp(const Graphics::Surface &srcSurface, byte *palette, int destX, int destY, const Common::Rect srcRect, bool flipX, uint8 alpha, uint16 scale) {
	if (scale != DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE) {
		drawScaledSprite(_backSurface, (byte *)srcSurface.getBasePtr(0, 0),
				srcRect.width(), srcRect.height(),
				destX, destY,
				srcRect.width() * scale / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE, srcRect.height() * scale / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE,
				palette, flipX, alpha);
		return;
	}
	Common::Rect clipRect = clipRectToScreen( destX,  destY, srcRect);
	if (clipRect.width() == 0 || clipRect.height() == 0) {
		return;
	}

	if (destX < 0) {
		destX = 0;
	}
	if (destY < 0) {
		destY = 0;
	}

	copyRectToSurface8bpp(srcSurface.getBasePtr(clipRect.left, clipRect.top), palette, srcSurface.pitch, srcSurface.w, clipRect.left, destX, destY, clipRect.width(), clipRect.height(), flipX, alpha);
}

/**
 * Fast RGB555 pixel blending
 * @param fg      The foreground color in uint16_t RGB565 format
 * @param bg      The background color in uint16_t RGB565 format
 * @param alpha   The alpha in range 0-255
 **/
uint16 alphaBlendRGB555( uint32 fg, uint32 bg, uint8 alpha ){
	alpha = ( alpha + 4 ) >> 3;
	bg = (bg | (bg << 16)) & 0b00000011111000000111110000011111;
	fg = (fg | (fg << 16)) & 0b00000011111000000111110000011111;
	uint32_t result = ((((fg - bg) * alpha) >> 5) + bg) & 0b00000011111000000111110000011111;
	return (uint16_t)((result >> 16) | result);
}

void Screen::copyRectToSurface(const void *buffer, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX, uint8 alpha) {
	assert(buffer);

	assert(destX >= 0 && destX < _backSurface->w);
	assert(destY >= 0 && destY < _backSurface->h);
	assert(height > 0 && destY + height <= _backSurface->h);
	assert(width > 0 && destX + width <= _backSurface->w);

	// Copy buffer data to internal buffer
	const byte *src = (const byte *)buffer;
	byte *dst = (byte *)_backSurface->getBasePtr(destX, destY);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int32 srcIdx = flipX ? srcWidth - (srcXOffset * 2) - j - 1 : j;
			if (src[srcIdx * 2] != 0 || src[srcIdx * 2 + 1] != 0) {
				if ((src[srcIdx * 2 + 1] & 0x80) == 0 || alpha == 255) {
					// only copy opaque pixels
					dst[j * 2] = src[srcIdx * 2];
					dst[j * 2 + 1] = src[srcIdx * 2 + 1];
				} else {
					WRITE_LE_UINT16(&dst[j * 2], alphaBlendRGB555(READ_LE_INT16(&src[srcIdx * 2]), READ_LE_INT16(&dst[j * 2]), alpha));
					// semi-transparent pixels.
				}
			}
		}
		src += srcPitch;
		dst += _backSurface->pitch;
	}
}

void Screen::copyRectToSurface8bpp(const void *buffer, byte* palette, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX, uint8 alpha) {
	assert(buffer);

	assert(destX >= 0 && destX < _backSurface->w);
	assert(destY >= 0 && destY < _backSurface->h);
	assert(height > 0 && destY + height <= _backSurface->h);
	assert(width > 0 && destX + width <= _backSurface->w);

	// Copy buffer data to internal buffer
	const byte *src = (const byte *)buffer;
	byte *dst = (byte *)_backSurface->getBasePtr(destX, destY);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int32 srcIdx = flipX ? srcWidth - (srcXOffset * 2) - j - 1 : j;
			uint16 c = READ_LE_UINT16(&palette[src[srcIdx] * 2]);
			if (c != 0) {
				if (!(c & 0x8000) || alpha == 255) {
					// only copy opaque pixels
					WRITE_LE_UINT16(&dst[j * 2], c & ~0x8000);
				} else {
					WRITE_LE_UINT16(&dst[j * 2], alphaBlendRGB555(c, READ_LE_INT16(&dst[j * 2]), alpha));
					// semi-transparent pixels.
				}
			}
		}
		src += srcPitch;
		dst += _backSurface->pitch;
	}
}

void Screen::drawScaledSprite(Graphics::Surface *destSurface, byte *source, int sourceWidth, int sourceHeight,
		int destX, int destY, int destWidth, int destHeight, byte *palette, bool flipX, uint8 alpha) {
	// Based on the GNAP engine scaling code
	// TODO Implement flipping
	const int xs = ((sourceWidth - 1) << 16) / destWidth;
	const int ys = ((sourceHeight -1) << 16) / destHeight;
	int clipX = 0, clipY = 0;
	const int destPitch = destSurface->pitch;
	if (destX < 0) {
		clipX = -destX;
		destX = 0;
		destWidth -= clipX;
	}
	if (destX + destWidth >= destSurface->w) {
		destWidth = destSurface->w - destX;
	}
	if (destY < 0) {
		clipY = -destY;
		destY = 0;
		destHeight -= clipY;
	}
	if (destY + destHeight >= destSurface->h) {
		destHeight = destSurface->h - destY;
	}
	if (destWidth < 0 || destHeight < 0)
		return;
	byte *dst = (byte *)destSurface->getBasePtr(destX, destY);
	int yi = ys * clipY;
	byte *hsrc = source + sourceWidth * ((yi + 0x8000) >> 16);
	for (int yc = 0; yc < destHeight; ++yc) {
		byte *wdst = flipX ? dst + (destWidth - 1) * 2 : dst;
		int xi = xs * clipX;
		byte *wsrc = hsrc + ((xi + 0x8000) >> 16);
		for (int xc = 0; xc < destWidth; ++xc) {
			byte colorIndex = *wsrc;
			uint16 c = READ_LE_UINT16(&palette[colorIndex * 2]);
			if (c != 0) {
				if (!(c & 0x8000) || alpha == 255) {
					// only copy opaque pixels
					WRITE_LE_UINT16(wdst, c & ~0x8000);
				} else {
					WRITE_LE_UINT16(wdst, alphaBlendRGB555(c, READ_LE_INT16(wdst), alpha));
					// semi-transparent pixels.
				}
			}
			wdst += (flipX ? -2 : 2);
			xi += xs;
			wsrc = hsrc + ((xi + 0x8000) >> 16);
		}
		dst += destPitch;
		yi += ys;
		hsrc = source + sourceWidth * ((yi + 0x8000) >> 16);
	}
}

Common::Rect Screen::clipRectToScreen(int destX, int destY, const Common::Rect rect) {
	return clipRectToRect(destX, destY, rect, Common::Rect(320, 200));
}

Common::Rect Screen::clipRectToRect(int destX, int destY, const Common::Rect rect, const Common::Rect containerRect) {
	int16 x, y, w, h;
	x = rect.left;
	y = rect.top;
	w = rect.width();
	h = rect.height();

	if (destX >= containerRect.width()) {
		w = 0;
	}

	if (destY >= containerRect.height()) {
		h = 0;
	}

	if (destX < 0) {
		w += destX;
		x += -destX;
	}

	if (destY < 0) {
		h += destY;
		y += -destY;
	}

	if (w > 0 && destX + w >= containerRect.width()) {
		w -= (destX + w) - containerRect.width();
	}

	if (h > 0 && destY + h >= containerRect.height()) {
		h -= (destY + h) - containerRect.height();
	}

	if (w < 0) {
		w = 0;
	}

	if (h < 0) {
		h = 0;
	}

	return Common::Rect(x, y, x + w, y + h);
}

void Screen::updatePaletteTransparency(uint16 paletteNum, uint16 startOffset, uint16 endOffset, bool isTransparent) {
	assert(paletteNum < DRAGONS_NUM_PALETTES);
	assert(startOffset < 256);
	assert(endOffset < 256);

	// TODO
	// this is needed for palette 0 for some reason.
//	DAT_80069638 = DAT_80069638 | 0x50000000;
//	DAT_8006965c = DAT_8006965c | 0x50000000;
//	DAT_80069680 = DAT_80069680 | 0x50000000;

	for (int i = startOffset; i <= endOffset; i++) {
		if (isTransparent) {
			_palettes[paletteNum][i * 2 + 1] |= 0x80;
		} else {
			_palettes[paletteNum][i * 2 + 1] &= ~0x80;
		}
	}
}

void Screen::loadPalette(uint16 paletteNum, byte *palette) {
	bool isTransPalette = (paletteNum & 0x8000);
	paletteNum &= ~0x8000;
	assert(paletteNum < DRAGONS_NUM_PALETTES);
	if (paletteNum == 0) {
		memcpy(&_palettes[paletteNum][0], palette, 512);
	} else {
		memcpy(&_palettes[paletteNum][0], palette, 512);
		if (paletteNum == 2 || paletteNum == 4 || paletteNum == 5) {
			_palettes[paletteNum][2] = 0;
			_palettes[paletteNum][3] = 0;
		}
		if (paletteNum == 1) {
			_palettes[paletteNum][2] = 1;
			_palettes[paletteNum][3] = 0;
		}
	}

	for (int i =1 ; i < 0x100; i++) {
		uint16 c = READ_LE_INT16(&_palettes[paletteNum][i * 2]);
		if ((c & ~0x8000) == 0) {
			if (!isTransPalette) {
				WRITE_LE_UINT16(&_palettes[paletteNum][i * 2], 0x8000);
			}
		} else {
			//TODO is this needed? see load_palette_into_frame_buffer()
//			c = (ushort)(((uint)c & 0x1f) << 10) | (ushort)(((uint)c & 0x7c00) >> 10) |
//					(c & 0x3e0) | (c & 0x8000);
		}
	}
	WRITE_LE_UINT16(&_palettes[paletteNum][0], 0);
}

void Screen::setPaletteRecord(uint16 paletteNum, uint16 offset, uint16 newValue) {
	assert(paletteNum < DRAGONS_NUM_PALETTES);
	assert(offset < 256);
	WRITE_LE_UINT16(&_palettes[paletteNum][offset * 2], newValue);
}

byte *Screen::getPalette(uint16 paletteNum) {
	assert(paletteNum < DRAGONS_NUM_PALETTES);
	return _palettes[paletteNum];
}

void Screen::clearScreen() {
	_backSurface->fillRect(Common::Rect(0,0, _backSurface->w, _backSurface->h), 0);
}

void Screen::drawRect(uint16 colour, Common::Rect rect, int id) {
	Common::Rect clippedRect = clipRectToScreen(0, 0, rect);
	//top
	_backSurface->drawLine(clippedRect.left, clippedRect.top, clippedRect.right, clippedRect.top, colour);
	//right
	_backSurface->drawLine(clippedRect.right, clippedRect.top, clippedRect.right, clippedRect.bottom, colour);
	//bottom
	_backSurface->drawLine(clippedRect.left, clippedRect.bottom, clippedRect.right, clippedRect.bottom, colour);
	//left
	_backSurface->drawLine(clippedRect.left, clippedRect.top, clippedRect.left, clippedRect.bottom, colour);

}

void Screen::setScreenShakeOffset(int16 newOffset) {
	_screenShakeOffset = newOffset;
}

void Screen::copyRectToSurface8bppWrappedY(const Graphics::Surface &srcSurface, byte *palette, int yOffset) {
	byte *dst = (byte *)_backSurface->getBasePtr(0, 0);
	for (int i = 0; i < 200; i++) {
		byte *src = (byte *)srcSurface.getPixels() + ((yOffset + i) % srcSurface.h) * srcSurface.pitch;
		for (int j = 0; j < 320; j++) {
			uint16 c = READ_LE_UINT16(&palette[src[j] * 2]);
			if (c != 0) {
					WRITE_LE_UINT16(&dst[j * 2], c & ~0x8000);
			}
		}
		dst += _backSurface->pitch;
	}
}

} // End of namespace Dragons
