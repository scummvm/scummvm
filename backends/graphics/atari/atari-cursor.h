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

#ifndef BACKENDS_GRAPHICS_ATARI_CURSOR_H
#define BACKENDS_GRAPHICS_ATARI_CURSOR_H

#include "common/rect.h"
#include "common/scummsys.h"
#include "graphics/surface.h"

struct Cursor {
	void update(const Graphics::Surface &screen, bool isModified);

	bool visible = false;

	// position
	Common::Point getPosition() const {
		return Common::Point(_x, _y);
	}
	void setPosition(int x, int y) {
		_x = x;
		_y = y;
	}
	void updatePosition(int deltaX, int deltaY, const Graphics::Surface &screen);
	void swap() {
		const int tmpX = _oldX;
		const int tmpY = _oldY;

		_oldX = _x;
		_oldY = _y;

		_x = tmpX;
		_y = tmpY;
	}

	// surface
	void setSurface(const void *buf, int w, int h, int hotspotX, int hotspotY, uint32 keycolor);
	void convertTo(const Graphics::PixelFormat &format);
	Graphics::Surface surface;
	Graphics::Surface surfaceMask;

	// rects (valid only if !outOfScreen)
	bool isClipped() const {
		return outOfScreen ? false : _width != srcRect.width();
	}
	bool outOfScreen = true;
	Common::Rect srcRect;
	Common::Rect dstRect;

	// palette (only used for the overlay)
	byte palette[256*3] = {};

private:
	int _x = -1, _y = -1;
	int _oldX = -1, _oldY = -1;

	// related to 'surface'
	const byte *_buf = nullptr;
	int _width;
	int _height;
	int _hotspotX;
	int _hotspotY;
	uint32 _keycolor;

	int _rShift, _gShift, _bShift;
	int _rMask, _gMask, _bMask;
};

#endif // BACKENDS_GRAPHICS_ATARI_CURSOR_H
