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

class AtariGraphicsManager;
struct Screen;

// Global state consists of:
//	- palette (used for the overlay only atm)
//	- shape (surface, dimensions, hotspot, keycolor)
//	- visibility
// These always get updates by ScummVM, no need to differentiate between engines and the overlay.

struct Cursor {
	Cursor(AtariGraphicsManager *manager, Screen *screen)
		: _manager(manager)
		, _parentScreen(screen) {
	}

	void reset() {
		_positionChanged = true;
		_surfaceChanged = true;
		_visibilityChanged = false;

		_savedRect = Common::Rect();
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

	// position
	Common::Point getPosition() const {
		return Common::Point(_x, _y);
	}
	void setPosition(int x, int y) {
		if (_x == x && _y == y)
			return;

		_x = x;
		_y = y;

		_positionChanged = true;
	}
	void updatePosition(int deltaX, int deltaY);

	// surface
	void setSurface(const void *buf, int w, int h, int hotspotX, int hotspotY, uint32 keycolor);
	void setPalette(const byte *colors, uint start, uint num);
	void convertTo(const Graphics::PixelFormat &format);

	bool isVisible() const {
		return !_outOfScreen && _visible;
	}
	bool isChanged() const {
		return _positionChanged || _surfaceChanged || _visibilityChanged;
	}

	bool intersects(const Common::Rect &rect) const {
		return rect.intersects(_dstRect);
	}

	void flushBackground(const Graphics::Surface &srcSurface, const Common::Rect &rect);
	bool restoreBackground(const Graphics::Surface &srcSurface, bool force);
	bool draw(bool directRendering, bool force);

private:
	static byte _palette[256*3];

	AtariGraphicsManager *_manager;
	Screen *_parentScreen;

	bool _positionChanged = true;
	bool _surfaceChanged = true;
	bool _visibilityChanged = false;

	bool _visible = false;
	int _x = -1, _y = -1;
	bool _outOfScreen = true;
	Common::Rect _srcRect;
	Common::Rect _dstRect;

	Graphics::Surface _savedBackground;	// used by direct rendering
	Common::Rect _savedRect;

	// related to 'surface'
	const byte *_buf = nullptr;
	int _width;
	int _height;
	int _hotspotX;
	int _hotspotY;
	uint32 _keycolor;

	Graphics::Surface _surface;
	Graphics::Surface _surfaceMask;
	int _rShift, _gShift, _bShift;
	int _rMask, _gMask, _bMask;
};

#endif // BACKENDS_GRAPHICS_ATARI_CURSOR_H
