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

extern bool g_unalignedPitch;

void Cursor::update(const Graphics::Surface &screen, bool isModified) {
	if (!_buf) {
		outOfScreen = true;
		return;
	}

	if (!visible || !isModified)
		return;

	srcRect = Common::Rect(_width, _height);

	dstRect = Common::Rect(
		_x - _hotspotX,	// left
		_y - _hotspotY,	// top
		_x - _hotspotX + _width,	// right
		_y - _hotspotY + _height);	// bottom

	outOfScreen = !screen.clip(srcRect, dstRect);

	assert(srcRect.width() == dstRect.width());
	assert(srcRect.height() == dstRect.height());
}

void Cursor::updatePosition(int deltaX, int deltaY, const Graphics::Surface &screen) {
	_x += deltaX;
	_y += deltaY;

	if (_x < 0)
		_x = 0;
	else if (_x >= screen.w)
		_x = screen.w - 1;

	if (_y < 0)
		_y = 0;
	else if (_y >= screen.h)
		_y = screen.h - 1;
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
}

void Cursor::convertTo(const Graphics::PixelFormat &format) {
	const int cursorWidth = (srcRect.width() + 15) & (-16);
	const int cursorHeight = _height;
	const bool isCLUT8 = format.isCLUT8();

	if (surface.w != cursorWidth || surface.h != cursorHeight || surface.format != format) {
		if (!isCLUT8 && surface.format != format) {
			_rShift = format.rLoss - format.rShift;
			_gShift = format.gLoss - format.gShift;
			_bShift = format.bLoss - format.bShift;

			_rMask = format.rMax() << format.rShift;
			_gMask = format.gMax() << format.gShift;
			_bMask = format.bMax() << format.bShift;
		}

		surface.create(cursorWidth, cursorHeight, format);

		const bool old_unalignedPitch = g_unalignedPitch;
		g_unalignedPitch = true;
		surfaceMask.create(surface.w / 8, surface.h, format);	// 1 bpl
		g_unalignedPitch = old_unalignedPitch;
	}

	const int srcRectWidth = srcRect.width();

	const byte *src = _buf + srcRect.left;
	byte *dst = (byte *)surface.getPixels();
	uint16 *dstMask = (uint16 *)surfaceMask.getPixels();
	const int srcPadding = _width - srcRectWidth;
	const int dstPadding = surface.w - srcRectWidth;

	for (int j = 0; j < cursorHeight; ++j) {
		for (int i = 0; i < srcRectWidth; ++i) {
			const uint32 color = *src++;
			const uint16 bit = 1 << (15 - (i % 16));

			if (color != _keycolor) {
				if (!isCLUT8) {
					// Convert CLUT8 to RGB332/RGB121 palette
					*dst++ = ((palette[color*3 + 0] >> _rShift) & _rMask)
						   | ((palette[color*3 + 1] >> _gShift) & _gMask)
						   | ((palette[color*3 + 2] >> _bShift) & _bMask);
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
