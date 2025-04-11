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
//#include "backends/platform/atari/atari-debug.h"

class AtariGraphicsManager;
struct Screen;

// Global state consists of:
//	- palette (used for the overlay only atm)
//	- shape (surface, dimensions, hotspot, keycolor)
//	- visibility
// These always get updates by ScummVM, no need to differentiate between engines and the overlay.

struct Cursor {
	Cursor(const AtariGraphicsManager *manager, const Screen *screen, int x, int y);

	void reset(const Graphics::Surface *boundingSurf, int xOffset) {
		_boundingSurf = boundingSurf;
		_xOffset = xOffset;

		_positionChanged = true;
		_surfaceChanged = true;
		_visibilityChanged = false;

		_savedRect = _previousSrcRect = _alignedDstRect = Common::Rect();
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
		//atari_debug("Cursor::setPosition: %d, %d", x, y);

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
	void convertSurfaceTo(const Graphics::PixelFormat &format);

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
	void restoreBackground();

	static byte _palette[256*3];

	const AtariGraphicsManager *_manager;
	const Screen *_parentScreen;
	const Graphics::Surface *_boundingSurf;
	int _xOffset = 0;

	bool _positionChanged = true;
	bool _surfaceChanged = true;
	bool _visibilityChanged = false;

	bool _visible = false;
	int _x;
	int _y;
	bool _outOfScreen = true;
	Common::Rect _srcRect;
	Common::Rect _dstRect;

	Graphics::Surface _savedBackground;
	Common::Rect _savedRect;
	Common::Rect _previousSrcRect;
	Common::Rect _alignedDstRect;

	// related to 'surface'
	const byte *_buf = nullptr;
	int _width;
	int _height;
	int _hotspotX;
	int _hotspotY;
	uint32 _keycolor;

	// TODO: make all surface-related variables and functions static, similar to _palette/
	// but there's a catch: we still need _surfaceChanged instantiated and convertTo may
	// be called when clipping changes. Perhaps Cursor should be a singleton and those
	// flags moved to Screen...
	Graphics::Surface _surface;
	Graphics::Surface _surfaceMask;
	int _rShift, _gShift, _bShift;
	int _rMask, _gMask, _bMask;
};

#endif // BACKENDS_GRAPHICS_ATARI_CURSOR_H
