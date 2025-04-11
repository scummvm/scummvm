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

#include "atari-cursor.h"

#include <cassert>

#include "atari-graphics.h"
#include "atari-screen.h"
//#include "backends/platform/atari/atari-debug.h"

byte Cursor::_palette[256*3] = {};

Cursor::Cursor(const AtariGraphicsManager *manager, const Screen *screen, int x, int y)
		: _manager(manager)
		, _parentScreen(screen)
		, _boundingSurf(screen->offsettedSurf)
		, _x(x)
		, _y(y) {
}

void Cursor::update() {
	if (!_buf) {
		_outOfScreen = true;
		_savedRect = _alignedDstRect = Common::Rect();
		return;
	}

	if (!_visible || (!_positionChanged && !_surfaceChanged))
		return;

	_srcRect = Common::Rect(_width, _height);

	_dstRect = Common::Rect(
		_x - _hotspotX,	// left
		_y - _hotspotY,	// top
		_x - _hotspotX + _width,	// right
		_y - _hotspotY + _height);	// bottom

	_outOfScreen = !_boundingSurf->clip(_srcRect, _dstRect);

	if (!_outOfScreen) {
		assert(_srcRect.width() == _dstRect.width());
		assert(_srcRect.height() == _dstRect.height());

		const int dstBitsPerPixel = _manager->getBitsPerPixel(_parentScreen->offsettedSurf->format);

		// non-direct rendering never uses 4bpp but maybe in the future ...
		_savedRect = _manager->alignRect(
			_dstRect.left * dstBitsPerPixel / 8,	// fake 4bpp by 8bpp's x/2
			_dstRect.top,
			_dstRect.right * dstBitsPerPixel / 8,	// fake 4bpp by 8bpp's width/2
			_dstRect.bottom);

		// this is used only in flushBackground()
		_alignedDstRect = _manager->alignRect(
			_dstRect.left + _xOffset,
			_dstRect.top,
			_dstRect.right + _xOffset,
			_dstRect.bottom);
	}
}

void Cursor::updatePosition(int deltaX, int deltaY) {
	//atari_debug("Cursor::updatePosition: %d, %d", deltaX, deltaX);

	if (deltaX == 0 && deltaY == 0)
		return;

	_x += deltaX;
	_y += deltaY;

	if (_x < 0)
		_x = 0;
	else if (_x >= _boundingSurf->w)
		_x = _boundingSurf->w - 1;

	if (_y < 0)
		_y = 0;
	else if (_y >= _boundingSurf->h)
		_y = _boundingSurf->h - 1;

	_positionChanged = true;
}

void Cursor::setSurface(const void *buf, int w, int h, int hotspotX, int hotspotY, uint32 keycolor) {
	if (w == 0 || h == 0 || buf == nullptr) {
		_buf = nullptr;
		return;
	}

	_buf = (const byte *)buf;
	_width = w;
	_height = h;
	_hotspotX = hotspotX;
	_hotspotY = hotspotY;
	_keycolor = keycolor;

	_surfaceChanged = true;
}

void Cursor::setPalette(const byte *colors, uint start, uint num) {
	memcpy(&_palette[start * 3], colors, num * 3);

	_surfaceChanged = true;
}

void Cursor::convertSurfaceTo(const Graphics::PixelFormat &format) {
	const int cursorWidth = (_srcRect.width() + 15) & (-16);
	const int cursorHeight = _height;
	const bool isCLUT8 = format.isCLUT8();

	if (_surface.w != cursorWidth || _surface.h != cursorHeight || _surface.format != format) {
		if (!isCLUT8 && _surface.format != format) {
			_rShift = format.rLoss - format.rShift;
			_gShift = format.gLoss - format.gShift;
			_bShift = format.bLoss - format.bShift;

			_rMask = format.rMax() << format.rShift;
			_gMask = format.gMax() << format.gShift;
			_bMask = format.bMax() << format.bShift;
		}

		_surface.create(cursorWidth, cursorHeight, format);

		extern bool g_unalignedPitch;
		const bool old_unalignedPitch = g_unalignedPitch;
		g_unalignedPitch = true;
		_surfaceMask.create(_surface.w / 8, _surface.h, format);	// 1 bpl
		g_unalignedPitch = old_unalignedPitch;
	}

	const int srcRectWidth = _srcRect.width();

	const byte *src = _buf + _srcRect.left;
	byte *dst = (byte *)_surface.getPixels();
	uint16 *dstMask = (uint16 *)_surfaceMask.getPixels();
	const int srcPadding = _width - srcRectWidth;
	const int dstPadding = _surface.w - srcRectWidth;

	for (int j = 0; j < cursorHeight; ++j) {
		for (int i = 0; i < srcRectWidth; ++i) {
			const uint32 color = *src++;
			const uint16 bit = 1 << (15 - (i % 16));

			if (color != _keycolor) {
				if (!isCLUT8) {
					// Convert CLUT8 to RGB332/RGB121 palette
					*dst++ = ((_palette[color*3 + 0] >> _rShift) & _rMask)
						   | ((_palette[color*3 + 1] >> _gShift) & _gMask)
						   | ((_palette[color*3 + 2] >> _bShift) & _bMask);
				} else {
					*dst++ = color;
				}

				// clear bit
				*dstMask &= ~bit;
			} else {
				*dst++ = 0x00;

				// set bit
				*dstMask |= bit;
			}

			if (bit == 0x0001)
				dstMask++;
		}

		src += srcPadding;

		if (dstPadding) {
			memset(dst, 0x00, dstPadding);
			dst += dstPadding;

			*dstMask |= ((1 << dstPadding) - 1);
			dstMask++;
		}
	}
}

Common::Rect Cursor::flushBackground(const Common::Rect &alignedRect, bool directRendering) {
	if (_savedRect.isEmpty())
		return _savedRect;

	if (!alignedRect.isEmpty() && alignedRect.contains(_alignedDstRect)) {
		// better would be _visibilityChanged but update() ignores it
		_positionChanged = true;

		_savedRect = Common::Rect();
	} else if (alignedRect.isEmpty() || alignedRect.intersects(_alignedDstRect)) {
		// better would be _visibilityChanged but update() ignores it
		_positionChanged = true;

		if (directRendering)
			restoreBackground();
		else
			return _alignedDstRect;
	}

	return Common::Rect();
}

void Cursor::saveBackground() {
	if (_savedRect.isEmpty())
		return;

	// as this is used only for direct rendering, we don't need to worry about offsettedSurf
	// having different dimensions than the source surface
	Graphics::Surface &dstSurface = *_parentScreen->offsettedSurf;

	//atari_debug("Cursor::saveBackground: %d %d %d %d", _savedRect.left, _savedRect.top, _savedRect.width(), _savedRect.height());

	// save native pixels (i.e. bitplanes)
	if (_savedBackground.w != _savedRect.width()
		|| _savedBackground.h != _savedRect.height()
		|| _savedBackground.format != dstSurface.format) {
		_savedBackground.create(_savedRect.width(), _savedRect.height(), dstSurface.format);
	}

	_savedBackground.copyRectToSurface(dstSurface, 0, 0, _savedRect);
}

void Cursor::draw() {
	Graphics::Surface &dstSurface = *_parentScreen->offsettedSurf;
	const int dstBitsPerPixel     = _manager->getBitsPerPixel(dstSurface.format);

	//atari_debug("Cursor::draw: %d %d %d %d", _dstRect.left, _dstRect.top, _dstRect.width(), _dstRect.height());

	if (_surfaceChanged || _srcRect != _previousSrcRect) {
		_previousSrcRect = _srcRect;

		// TODO: some sort of in-place C2P directly into convertSurfaceTo() ...
		convertSurfaceTo(dstSurface.format);
		{
			// c2p in-place (will do nothing on regular Surface::copyRectToSurface)
			Graphics::Surface surf;
			surf.init(
				_surface.w,
				_surface.h,
				_surface.pitch * dstBitsPerPixel / 8,	// 4bpp is not byte per pixel anymore
				_surface.getPixels(),
				_surface.format);
			_manager->copyRectToSurface(
				surf, _surface,
				0, 0,
				Common::Rect(_surface.w, _surface.h));
		}
	}

	// don't use _srcRect.right as 'x2' as this must be aligned first
	// (_surface.w is recalculated thanks to convertSurfaceTo())
	_manager->drawMaskedSprite(
		dstSurface,
		_surface, _surfaceMask,
		_dstRect.left + _xOffset, _dstRect.top,
		Common::Rect(0, _srcRect.top, _surface.w, _srcRect.bottom));

	_visibilityChanged = _positionChanged = _surfaceChanged = false;
}

void Cursor::restoreBackground() {
	if (_savedRect.isEmpty())
		return;

	assert(_savedBackground.getPixels());

	//atari_debug("Cursor::restoreBackground: %d %d %d %d", _savedRect.left, _savedRect.top, _savedRect.width(), _savedRect.height());

	// as this is used only for direct rendering, we don't need to worry about offsettedSurf
	// having different dimensions than the source surface
	Graphics::Surface &dstSurface = *_parentScreen->offsettedSurf;

	// restore native pixels (i.e. bitplanes)
	dstSurface.copyRectToSurface(
		_savedBackground,
		_savedRect.left, _savedRect.top,
		Common::Rect(_savedBackground.w, _savedBackground.h));

	_savedRect = Common::Rect();
}
