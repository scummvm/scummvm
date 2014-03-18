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
}

Screen::~Screen() {
	delete _drawQueue;
	delete _decompressQueue;
	_backSurface->free();
	delete _backSurface;
}

Graphics::Surface *Screen::allocSurface(int16 width, int16 height) {
	// TODO Use screen pixel format?
	Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, pixelFormat16);
	return surface; 
}

Graphics::Surface *Screen::allocSurface(SurfInfo &surfInfo) {
	return allocSurface(surfInfo._dimensions._width, surfInfo._dimensions._height);
}

bool Screen::isDisplayOn() {
	return _displayOn;
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
	// Unscaled, transparent
	// TODO
	debug("Screen::drawSurface10");
}

void Screen::drawSurface11(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Unscaled, non-transparent
	debug(1, "Screen::drawSurface11() destX: %d; destY: %d; srcRect: (%d, %d, %d, %d)", destX, destY, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	const int16 w = srcRect.width();
	const int16 h = srcRect.height();
	for (int16 yc = 0; yc < h; ++yc) {
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcRect.top + yc);
		byte *dst = (byte*)_backSurface->getBasePtr(destX, destY + yc);
		memcpy(dst, src, w * 2);
	}
}

void Screen::drawSurface20(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey) {
	// Scaled, transparent
	// TODO
	debug("Screen::drawSurface20");
}

void Screen::drawSurface21(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Scaled, non-transparent
	// TODO
	debug("Screen::drawSurface21");
}

} // End of namespace Illusions
