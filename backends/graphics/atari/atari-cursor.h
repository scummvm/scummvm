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

#include "graphics/surface.h"

class AtariSurface;

// Global state consists of:
//	- palette (used for the overlay only atm)
//	- shape (surface, dimensions, hotspot, keycolor)
//	- visibility
// These always get updates by ScummVM, no need to differentiate between engines and the overlay.

struct Cursor {
	~Cursor();

	void reset(AtariSurface* screenSurf, const Graphics::Surface *boundingSurf) {
		_screenSurf = screenSurf;
		_boundingSurf = boundingSurf;

		_positionChanged = true;
		_surfaceChanged = true;
		_visibilityChanged = false;

		_savedRect = _alignedDstRect = Common::Rect();
	}

	// updates outOfScreen OR srcRect/dstRect (only if visible/needed)
	void update();

	// visibility
	bool setVisible(bool visible) {
		if (_visible == visible) {
			return _visible;
		}

		bool last = _visible;

		_visible = visible;

		_visibilityChanged = true;

		return last;
	}
	void setSurfaceChanged() {
		_surfaceChanged = true;
	}

	// position
	Common::Point getPosition() const {
		return Common::Point(_x, _y);
	}
	void setPosition(int x, int y) {
		//atari_debug("Cursor::setPosition: %d, %d", x, y);

		if (_x == x && _y == y)
			return;

		_x = x;
		_y = y;

		_positionChanged = true;
	}
	void updatePosition(int deltaX, int deltaY);

	// surface
	static void setSurface(const void *buf, int w, int h, int hotspotX, int hotspotY, uint32 keycolor);
	static void setPalette(const byte *colors, uint start, uint num);

	bool isVisible() const {
		return !_outOfScreen && _visible;
	}
	bool isChanged() const {
		return _positionChanged || _surfaceChanged || _visibilityChanged;
	}

	Common::Rect flushBackground(const Common::Rect &alignedRect, bool directRendering);
	void saveBackground();
	void draw();

private:
	static void convertSurfaceTo(const Graphics::PixelFormat &format);
	void restoreBackground();

	AtariSurface *_screenSurf;
	const Graphics::Surface *_boundingSurf = nullptr;

	bool _positionChanged = false;
	bool _surfaceChanged = false;
	bool _visibilityChanged = false;

	bool _visible = false;
	int _x = 0;
	int _y = 0;
	bool _outOfScreen = true;
	Common::Rect _srcRect;
	Common::Rect _dstRect;

	Graphics::Surface _savedBackground;
	Common::Rect _savedRect;
	Common::Rect _alignedDstRect;

	// related to 'surface'
	static bool _globalSurfaceChanged;

	static byte _palette[256*3];

	static const byte *_buf;
	static int _width;
	static int _height;
	static int _hotspotX;
	static int _hotspotY;
	static uint32 _keycolor;

	static Graphics::Surface _surface;
	static Graphics::Surface _surfaceMask;
};

#endif // BACKENDS_GRAPHICS_ATARI_CURSOR_H
