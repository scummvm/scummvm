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

#include "illusions/illusions.h"
#include "illusions/screen.h"
#include "illusions/graphics.h"
#include "illusions/spritedrawqueue.h"
#include "illusions/spritedecompressqueue.h"

namespace Illusions {

// Screen

Screen::Screen(IllusionsEngine *vm)
	: _vm(vm), _colorKey2(0) {
	_displayOn = true;
	_backSurface = allocSurface(640, 480);
	_decompressQueue = new SpriteDecompressQueue();
	_drawQueue = new SpriteDrawQueue(this);
	_colorKey1 = 0xF800 | 0x1F;
}

Screen::~Screen() {
	delete _drawQueue;
	delete _decompressQueue;
	_backSurface->free();
	delete _backSurface;
}

Graphics::Surface *Screen::allocSurface(int16 width, int16 height) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, _vm->_system->getScreenFormat());
	return surface; 
}

Graphics::Surface *Screen::allocSurface(SurfInfo &surfInfo) {
	return allocSurface(surfInfo._dimensions._width, surfInfo._dimensions._height);
}

bool Screen::isDisplayOn() {
	return _displayOn;
}

void Screen::setDisplayOn(bool isOn) {
	_displayOn = isOn;
	// TODO Clear screen when off
}

uint16 Screen::getColorKey2() {
	return _colorKey2;
}

void Screen::updateSprites() {
	_decompressQueue->decompressAll();
	// NOTE Skipped doShiftBrightness and related as it seems to be unused
	_drawQueue->drawAll();
	if (!_displayOn) // TODO Check if a video is playing then don't do it
		_backSurface->fillRect(Common::Rect(_backSurface->w, _backSurface->h), 0);
	g_system->copyRectToScreen((byte*)_backSurface->getBasePtr(0, 0), _backSurface->pitch, 0, 0, _backSurface->w, _backSurface->h);
}

void Screen::drawSurface10(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey) {
	// Unscaled
	// TODO
	//debug("Screen::drawSurface10");
}

void Screen::drawSurface11(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Unscaled
	//debug("Screen::drawSurface11() destX: %d; destY: %d; srcRect: (%d, %d, %d, %d)", destX, destY, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	const int16 w = srcRect.width();
	const int16 h = srcRect.height();
	for (int16 yc = 0; yc < h; ++yc) {
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcRect.top + yc);
		byte *dst = (byte*)_backSurface->getBasePtr(destX, destY + yc);
		//memcpy(dst, src, w * 2);
		for (int16 xc = 0; xc < w; ++xc) {
			uint16 pixel = READ_LE_UINT16(src);
			if (pixel != _colorKey1)
				WRITE_LE_UINT16(dst, pixel);
			src += 2;
			dst += 2;
		}
	}
}

void Screen::drawSurface20(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey) {
	// Scaled
	// TODO
	//debug("Screen::drawSurface20");
}

static uint16 average(const uint16 a, const uint16 b) {
	byte r1, g1, b1, r2, g2, b2;
	g_system->getScreenFormat().colorToRGB(a, r1, g1, b1);
	g_system->getScreenFormat().colorToRGB(b, r2, g2, b2);
	return g_system->getScreenFormat().RGBToColor((r1 + r1 + r2) / 3, (g1 + g1 + g2) / 3, (b1 + b1 + b2) / 3);
}

void Screen::drawSurface21(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Scaled
	const int dstWidth = dstRect.width(), dstHeight = dstRect.height();
	const int srcWidth = srcRect.width(), srcHeight = srcRect.height();
	const int errYStart = srcHeight / dstHeight;
	const int errYIncr = srcHeight % dstHeight;
	const int midY = dstHeight / 2;
	const int errXStart = srcWidth / dstWidth;
	const int errXIncr = srcWidth % dstWidth;
	const int midX = dstWidth / 2;
	int h = dstHeight, errY = 0, skipY, srcY = srcRect.top;
	byte *dst = (byte*)_backSurface->getBasePtr(dstRect.left, dstRect.top);
	skipY = (dstHeight < srcHeight) ? 0 : dstHeight / (2*srcHeight) + 1;
	h -= skipY;
	while (h-- > 0) {
		int w = dstWidth, errX = 0, skipX;
		skipX = (dstWidth < srcWidth) ? 0 : dstWidth / (2*srcWidth) + 1;
		w -= skipX;
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcY);
		byte *dstRow = dst; 
		while (w-- > 0) {
			uint16 pixel = READ_LE_UINT16(src);
			if (pixel != _colorKey1) {
				if (errX >= midX) {
					uint16 npixel = READ_LE_UINT16(src + 2);
					if (npixel == _colorKey1)
						npixel = READ_LE_UINT16(dstRow);
					pixel = average(pixel, npixel);
				}
				WRITE_LE_UINT16(dstRow, pixel);
			}
			dstRow += 2;
			src += 2 * errXStart;
			errX += errXIncr;
			if (errX >= dstWidth) {
				errX -= dstWidth;
				src += 2;
			}
		}
		while (skipX-- > 0) {
			uint16 pixel = READ_LE_UINT16(src);
			if (pixel != _colorKey1)
				WRITE_LE_UINT16(dstRow, pixel);
			src += 2;
			dstRow += 2;
		}
		dst += _backSurface->pitch;
		srcY += errYStart;
		errY += errYIncr;
		if (errY >= dstHeight) {
			errY -= dstHeight;
			++srcY;
		}
	}

}

} // End of namespace Illusions
