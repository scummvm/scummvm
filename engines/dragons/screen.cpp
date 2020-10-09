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
#include "common/endian.h"
#include "common/system.h"
#include "common/rect.h"
#include "engines/util.h"
#include "dragons/dragons.h"
#include "dragons/scene.h"
#include "dragons/screen.h"

namespace Dragons {

Screen::Screen() {
	//TODO add support for more video modes like RGB565 and RGBA555
	_pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	initGraphics(320, 200, &_pixelFormat);
	_backSurface = new Graphics::Surface();
	_backSurface->create(320, 200, _pixelFormat);
	_screenShakeOffset = Common::Point();
}

void Screen::updateScreen() {
	if (_screenShakeOffset.x != 0 || _screenShakeOffset.y != 0) {
		g_system->fillScreen(0); //TODO this is meant for 8bit screens. we should use system shake here.
	}
	Common::Rect clipRect = clipRectToScreen(_screenShakeOffset.x,  _screenShakeOffset.y, Common::Rect(_backSurface->w, _backSurface->h));
	g_system->copyRectToScreen((byte*)_backSurface->getBasePtr(clipRect.left, clipRect.top),
			_backSurface->pitch,
			_screenShakeOffset.x < 0 ? 0 : _screenShakeOffset.x, _screenShakeOffset.y < 0 ? 0 : _screenShakeOffset.y,
			clipRect.width(), clipRect.height());
//	if (_screenShakeOffset < 0) {
//		_backSurface->fillRect(Common::Rect(0, _backSurface->h + _screenShakeOffset - 1, _backSurface->w - 1, _backSurface->h - 1), 0);
//	}
//	if (_screenShakeOffset > 0) {
//		_backSurface->fillRect(Common::Rect(0, 0, _backSurface->w - 1, _screenShakeOffset - 1), 0);
//	}
	g_system->updateScreen();
}

Screen::~Screen() {
	_backSurface->free();
	delete _backSurface;
}

void Screen::copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY) {
	copyRectToSurface(srcSurface.getBasePtr(0, 0), srcSurface.pitch, srcSurface.w, 0, destX, destY, srcSurface.w, srcSurface.h, false, NONE);
}

void Screen::copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect srcRect, bool flipX, AlphaBlendMode alpha) {
	Common::Rect clipRect = clipRectToScreen(destX,  destY, srcRect);
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

void Screen::copyRectToSurface8bpp(const Graphics::Surface &srcSurface, const byte *palette, int destX, int destY, const Common::Rect srcRect, bool flipX, AlphaBlendMode alpha, uint16 scale) {
	if (scale != DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE) {
		drawScaledSprite(_backSurface, (const byte *)srcSurface.getBasePtr(0, 0),
				srcRect.width(), srcRect.height(),
				destX, destY,
				srcRect.width() * scale / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE, srcRect.height() * scale / DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE,
				palette, flipX, alpha);
		return;
	}
	Common::Rect clipRect = clipRectToScreen(destX,  destY, srcRect);
	if (clipRect.width() == 0 || clipRect.height() == 0) {
		return;
	}

	if (destX < 0) {
		destX = 0;
	}
	if (destY < 0) {
		destY = 0;
	}
	//TODO should we have different methods for alpha modes?
	copyRectToSurface8bpp(srcSurface.getBasePtr(clipRect.left, clipRect.top), palette, srcSurface.pitch, srcSurface.w, clipRect.left, destX, destY, clipRect.width(), clipRect.height(), flipX, alpha);
}

/**
 * Fast RGB555 pixel blending
 * @param fg      The foreground color in uint16 RGB565 format
 * @param bg      The background color in uint16 RGB565 format
 * @param alpha   The alpha in range 0-255
 **/
uint16 alphaBlendRGB555(uint32 fg, uint32 bg, uint8 alpha){
	alpha = (alpha + 4) >> 3;
	bg = (bg | (bg << 16)) & 0x3e07c1f;
	fg = (fg | (fg << 16)) & 0x3e07c1f;
	uint32 result = ((((fg - bg) * alpha) >> 5) + bg) & 0x3e07c1f;
	return (uint16)((result >> 16) | result);
}

uint16 alphaBlendAdditiveRGB555(uint32 fg, uint32 bg){
	bg = (bg | (bg << 16)) & 0x3e07c1f;
	fg = (fg | (fg << 16)) & 0x3e07c1f;

	uint32 result = bg + fg;
	//clip r g b values to 565.
	if (result & (0x3f << 26)) {
		result &= 0x1fffff;
		result |= 0x3E00000;
	}

	if (result & 0x1F8000) {
		result &= 0x3e07fff;
		result |= 0x7C00;
	}

	if (result & 0x3E0) {
		result &= 0x3e07c1f;
		result |= 0x1f;
	}
	return (uint16)((result >> 16) | result);
}

void Screen::copyRectToSurface(const void *buffer, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX, AlphaBlendMode alpha) {
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
				if ((src[srcIdx * 2 + 1] & 0x80) == 0 || alpha == NONE) {
					// only copy opaque pixels
					dst[j * 2] = src[srcIdx * 2];
					dst[j * 2 + 1] = src[srcIdx * 2 + 1];
				} else {
					WRITE_LE_UINT16(&dst[j * 2], alphaBlendRGB555(READ_LE_INT16(&src[srcIdx * 2]), READ_LE_INT16(&dst[j * 2]), 128));
					// semi-transparent pixels.
				}
			}
		}
		src += srcPitch;
		dst += _backSurface->pitch;
	}
}

void Screen::copyRectToSurface8bpp(const void *buffer, const byte* palette, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX, AlphaBlendMode alpha) {
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
				if (!(c & 0x8000) || alpha == NONE) {
					// only copy opaque pixels
					WRITE_LE_UINT16(&dst[j * 2], c & ~0x8000);
				} else {
					// semi-transparent pixels.
					WRITE_LE_UINT16(&dst[j * 2], alpha == NORMAL
						? alphaBlendRGB555(c & 0x7fff, READ_LE_INT16(&dst[j * 2]) & 0x7fff, 128)
						: alphaBlendAdditiveRGB555(c & 0x7fff, READ_LE_INT16(&dst[j * 2]) & 0x7fff));
				}
			}
		}
		src += srcPitch;
		dst += _backSurface->pitch;
	}
}

void Screen::drawScaledSprite(Graphics::Surface *destSurface, const byte *source, int sourceWidth, int sourceHeight,
		int destX, int destY, int destWidth, int destHeight, const byte *palette, bool flipX, AlphaBlendMode alpha) {
	//TODO this logic is pretty messy. It should probably be re-written. It is trying to scale, clip, flip and blend at once.

	// Based on the GNAP engine scaling code
	if (destWidth == 0 || destHeight == 0) {
		return;
	}
	const int xs = ((sourceWidth - 1) << 16) / destWidth;
	const int ys = ((sourceHeight -1) << 16) / destHeight;
	int clipX = 0, clipY = 0;
	const int destPitch = destSurface->pitch;
	if (destX < 0) {
		clipX = -destX;
		destX = 0;
		destWidth -= clipX;
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
	const byte *hsrc = source + sourceWidth * ((yi + 0x8000) >> 16);
	for (int yc = 0; yc < destHeight; ++yc) {
		byte *wdst = flipX ? dst + (destWidth - 1) * 2 : dst;
		int16 currX = flipX ? destX + (destWidth - 1) : destX;
		int xi = flipX ? xs : xs * clipX;
		const byte *wsrc = hsrc + ((xi + 0x8000) >> 16);
		for (int xc = 0; xc < destWidth; ++xc) {
			if (currX >= 0 && currX < destSurface->w) {
				byte colorIndex = *wsrc;
				uint16 c = READ_LE_UINT16(&palette[colorIndex * 2]);
				if (c != 0) {
					if (!(c & 0x8000u) || alpha == NONE) {
						// only copy opaque pixels
						WRITE_LE_UINT16(wdst, c & ~0x8000);
					} else {
						WRITE_LE_UINT16(wdst, alphaBlendRGB555(c & 0x7fffu, READ_LE_UINT16(wdst) & 0x7fffu, 128));
						// semi-transparent pixels.
					}
				}
			}
			currX += (flipX ? -1 : 1);
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

	if (paletteNum == 0) {
		// set all layers to pixel addition blending (100% back + 100% sprite)
		DragonsEngine *vm = getEngine();
		vm->_scene->setLayerAlphaMode(0, ADDITIVE);
		vm->_scene->setLayerAlphaMode(1, ADDITIVE);
		vm->_scene->setLayerAlphaMode(2, ADDITIVE);
	}

	for (int i = startOffset; i <= endOffset; i++) {
		if (isTransparent) {
			_palettes[paletteNum][i * 2 + 1] |= 0x80;
		} else {
			_palettes[paletteNum][i * 2 + 1] &= ~0x80;
		}
	}
}

void Screen::loadPalette(uint16 paletteNum, const byte *palette) {
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
//			c = (uint16)(((uint)c & 0x1f) << 10) | (uint16)(((uint)c & 0x7c00) >> 10) |
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
	_backSurface->fillRect(Common::Rect(0, 0, _backSurface->w, _backSurface->h), 0);
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

void Screen::setScreenShakeOffset(int16 x, int16 y) {
	_screenShakeOffset.x = x;
	_screenShakeOffset.y = y;
}

void Screen::copyRectToSurface8bppWrappedY(const Graphics::Surface &srcSurface, const byte *palette, int yOffset) {
	byte *dst = (byte *)_backSurface->getBasePtr(0, 0);
	for (int i = 0; i < DRAGONS_SCREEN_HEIGHT; i++) {
		const byte *src = (const byte *)srcSurface.getPixels() + ((yOffset + i) % srcSurface.h) * srcSurface.pitch;
		for (int j = 0; j < DRAGONS_SCREEN_WIDTH; j++) {
			uint16 c = READ_LE_UINT16(&palette[src[j] * 2]);
			if (c != 0) {
					WRITE_LE_UINT16(&dst[j * 2], c & ~0x8000);
			}
		}
		dst += _backSurface->pitch;
	}
}

void Screen::copyRectToSurface8bppWrappedX(const Graphics::Surface &srcSurface, const byte *palette, Common::Rect srcRect,
										   AlphaBlendMode alpha) {
	// Copy buffer data to internal buffer
	const byte *src = (const byte *)srcSurface.getBasePtr(0, 0);
	int width = srcSurface.w > DRAGONS_SCREEN_WIDTH ? DRAGONS_SCREEN_WIDTH : srcSurface.w;
	int height = srcRect.height();

	byte *dst = (byte *)_backSurface->getBasePtr(0, 0);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int32 srcIdx = (i + srcRect.top) * srcSurface.w + ((j + srcRect.left) % srcSurface.w);
			uint16 c = READ_LE_UINT16(&palette[src[srcIdx] * 2]);
			if (c != 0) {
				if (!(c & 0x8000) || alpha == NONE) {
					// only copy opaque pixels
					WRITE_LE_UINT16(&dst[j * 2], c & ~0x8000);
				} else {
					WRITE_LE_UINT16(&dst[j * 2], alpha == NORMAL ? alphaBlendRGB555(c, READ_LE_INT16(&dst[j * 2]), 128) : alphaBlendAdditiveRGB555(c, READ_LE_INT16(&dst[j * 2])));
					// semi-transparent pixels.
				}
			}
		}
		dst += _backSurface->pitch;
	}
}

int16 Screen::addFlatQuad(int16 x0, int16 y0, int16 x1, int16 y1, int16 x3, int16 y3, int16 x2, int16 y2, uint16 colour,
						 int16 priorityLayer, uint16 flags) {

	assert(x0 == x2 && x1 == x3 && y0 == y1 && y2 == y3); //make sure this is a rectangle

	for (int i = 0; i < DRAGONS_NUM_FLAT_QUADS; i++) {
		if (!(_flatQuads[i].flags & 1u)) {
			_flatQuads[i].flags = flags | 1u;
			_flatQuads[i].points[0].x = x0;
			_flatQuads[i].points[0].y = y0;
			_flatQuads[i].points[1].x = x1;
			_flatQuads[i].points[1].y = y1;
			_flatQuads[i].points[2].x = x2;
			_flatQuads[i].points[2].y = y2;
			_flatQuads[i].points[3].x = x3;
			_flatQuads[i].points[3].y = y3;
			_flatQuads[i].colour = colour;
			_flatQuads[i].priorityLayer = priorityLayer;
			return i;
		}
	}

	return -1;
}

void Screen::drawFlatQuads(uint16 priorityLayer) {
	for (int i = 0; i < DRAGONS_NUM_FLAT_QUADS; i++) {
		if (_flatQuads[i].flags & 1u && _flatQuads[i].priorityLayer == priorityLayer) {
			//TODO need to support semitrans mode.
			//TODO check if we need to support non-rectangular quads.
			fillRect(_flatQuads[i].colour, Common::Rect(_flatQuads[i].points[0].x, _flatQuads[i].points[0].y, _flatQuads[i].points[3].x + 1, _flatQuads[i].points[3].y + 1));
		}
	}
}

void Screen::fillRect(uint16 colour, Common::Rect rect) {
	_backSurface->fillRect(rect, colour);
}

void Screen::clearAllFlatQuads() {
	for (int i = 0; i < DRAGONS_NUM_FLAT_QUADS; i++) {
		_flatQuads[i].flags = 0;
	}
}

FlatQuad *Screen::getFlatQuad(uint16 quadId) {
	assert(quadId < DRAGONS_NUM_FLAT_QUADS);
	return &_flatQuads[quadId];
}

} // End of namespace Dragons
