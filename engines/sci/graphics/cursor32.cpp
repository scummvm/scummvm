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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/rational.h"        // for Rational, operator*
#include "common/system.h"          // for OSystem, g_system
#include "common/memstream.h"
#include "graphics/cursorman.h"     // for CursorMan
#include "graphics/maccursor.h"
#include "sci/graphics/celobj32.h"  // for CelObjView, CelInfo32, Ratio
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"  // for GfxFrameout

namespace Sci {

GfxCursor32::GfxCursor32() :
	_hideCount(0),
	_position(0, 0),
	_needsPaint(false) {
}

void GfxCursor32::init(const Buffer &outputBuffer) {
	_screen = outputBuffer;
	_screenRegion.rect = Common::Rect(_screen.w, _screen.h);
	_screenRegion.data = static_cast<byte *>(_screen.getPixels());
	_restrictedArea = _screenRegion.rect;
}

GfxCursor32::~GfxCursor32() {
	free(_cursor.data);
	free(_cursorBack.data);
	free(_scratch1.data);
	free(_scratch2.data);
	free(_savedScreenRegion.data);
}

void GfxCursor32::hide() {
	if (_hideCount++) {
		return;
	}

	g_system->showMouse(false);
	if (!_cursorBack.rect.isEmpty()) {
		drawToScreen(_cursorBack);
	}
}

void GfxCursor32::revealCursor() {
	_cursorBack.rect = _cursor.rect;
	_cursorBack.rect.clip(_screenRegion.rect);
	if (_cursorBack.rect.isEmpty()) {
		return;
	}

	copyFromScreen(_cursorBack);
	_scratch1.rect = _cursor.rect;
	copy<false>(_scratch1, _cursorBack);
	copy<true>(_scratch1, _cursor);
	drawToScreen(_scratch1);
}

template <bool SKIP>
void GfxCursor32::copy(DrawRegion &target, const DrawRegion &source) {
	if (source.rect.isEmpty()) {
		return;
	}

	Common::Rect drawRect(source.rect);
	drawRect.clip(target.rect);
	if (drawRect.isEmpty()) {
		return;
	}

	const int16 sourceXOffset = drawRect.left - source.rect.left;
	const int16 sourceYOffset = drawRect.top - source.rect.top;
	const int16 drawRectWidth = drawRect.width();
	const int16 drawRectHeight = drawRect.height();

	byte *targetPixel = target.data + ((drawRect.top - target.rect.top) * target.rect.width()) + (drawRect.left - target.rect.left);
	const byte *sourcePixel = source.data + (sourceYOffset * source.rect.width()) + sourceXOffset;
	const uint8 skipColor = source.skipColor;

	int16 sourceStride = source.rect.width();
	int16 targetStride = target.rect.width();
	if (SKIP) {
		sourceStride -= drawRectWidth;
		targetStride -= drawRectWidth;
	}

	for (int16 y = 0; y < drawRectHeight; ++y) {
		if (SKIP) {
			for (int16 x = 0; x < drawRectWidth; ++x) {
				if (*sourcePixel != skipColor) {
					*targetPixel = *sourcePixel;
				}
				++targetPixel;
				++sourcePixel;
			}
		} else {
			memcpy(targetPixel, sourcePixel, drawRectWidth);
		}
		targetPixel += targetStride;
		sourcePixel += sourceStride;
	}
}

void GfxCursor32::drawToScreen(const DrawRegion &source) {
	Common::Rect drawRect(source.rect);
	drawRect.clip(_screenRegion.rect);
	const int16 sourceXOffset = drawRect.left - source.rect.left;
	const int16 sourceYOffset = drawRect.top - source.rect.top;
	byte *sourcePixel = source.data + (sourceYOffset * source.rect.width()) + sourceXOffset;

	g_system->copyRectToScreen(sourcePixel, source.rect.width(), drawRect.left, drawRect.top, drawRect.width(), drawRect.height());
}

void GfxCursor32::unhide() {
	if (_hideCount == 0 || --_hideCount) {
		return;
	}

	g_system->showMouse(true);
	_cursor.rect.moveTo(_position.x - _hotSpot.x, _position.y - _hotSpot.y);
	revealCursor();
}

void GfxCursor32::show() {
	if (_hideCount) {
		g_system->showMouse(true);
		_hideCount = 0;
		_cursor.rect.moveTo(_position.x - _hotSpot.x, _position.y - _hotSpot.y);
		revealCursor();
	}
}

void GfxCursor32::setRestrictedArea(const Common::Rect &rect) {
	_restrictedArea = rect;

	const int16 screenWidth = g_sci->_gfxFrameout->getScreenWidth();
	const int16 screenHeight = g_sci->_gfxFrameout->getScreenHeight();
	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	mulru(_restrictedArea, Ratio(screenWidth, scriptWidth), Ratio(screenHeight, scriptHeight), 0);

	bool restricted = false;

	if (_position.x < rect.left) {
		_position.x = rect.left;
		restricted = true;
	}
	if (_position.x >= rect.right) {
		_position.x = rect.right - 1;
		restricted = true;
	}
	if (_position.y < rect.top) {
		_position.y = rect.top;
		restricted = true;
	}
	if (_position.y >= rect.bottom) {
		_position.y = rect.bottom - 1;
		restricted = true;
	}

	if (restricted) {
		g_system->warpMouse(_position.x, _position.y);
	}
}

void GfxCursor32::clearRestrictedArea() {
	_restrictedArea = _screenRegion.rect;
}

void GfxCursor32::setView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo) {
	hide();

	_cursorInfo.resourceId = viewId;
	_cursorInfo.loopNo = loopNo;
	_cursorInfo.celNo = celNo;

	if (viewId != -1) {
		CelObjView view(viewId, loopNo, celNo);

		_hotSpot = view._origin;
		_width = view._width;
		_height = view._height;

		// SSCI never increased the size of cursors, but some of the cursors
		// in early SCI32 games were designed for low-resolution display mode
		// and so are kind of hard to pick out when running in high-resolution
		// mode.
		// To address this, we make some slight adjustments to cursor display
		// in these early games:
		// GK1: All the cursors are increased in size since they all appear to
		//      be designed for low-res display.
		// PQ4: We only make the cursors bigger if they are above a set
		//      threshold size because inventory items usually have a
		//      high-resolution cursor representation.
		bool pixelDouble = false;
		if (g_sci->_gfxFrameout->isHiRes() &&
			(g_sci->getGameId() == GID_GK1 ||
			(g_sci->getGameId() == GID_PQ4 && _width <= 22 && _height <= 22))) {

			_width *= 2;
			_height *= 2;
			_hotSpot.x *= 2;
			_hotSpot.y *= 2;
			pixelDouble = true;
		}

		_cursor.data = (byte *)realloc(_cursor.data, _width * _height);
		_cursor.rect = Common::Rect(_width, _height);
		memset(_cursor.data, 255, _width * _height);
		_cursor.skipColor = 255;

		Buffer target;
		target.init(_width, _height, _width, _cursor.data, Graphics::PixelFormat::createFormatCLUT8());
		if (pixelDouble) {
			view.draw(target, _cursor.rect, Common::Point(0, 0), false, 2, 2);
		} else {
			view.draw(target, _cursor.rect, Common::Point(0, 0), false);
		}
	} else {
		_hotSpot = Common::Point(0, 0);
		_width = _height = 1;
		_cursor.data = (byte *)realloc(_cursor.data, _width * _height);
		_cursor.rect = Common::Rect(_width, _height);
		*_cursor.data = _cursor.skipColor;
		_cursorBack.rect = _cursor.rect;
		_cursorBack.rect.clip(_screenRegion.rect);
		if (!_cursorBack.rect.isEmpty()) {
			copyFromScreen(_cursorBack);
		}
	}

	_cursorBack.data = (byte *)realloc(_cursorBack.data, _width * _height);
	memset(_cursorBack.data, 0, _width * _height);
	_scratch1.data = (byte *)realloc(_scratch1.data, _width * _height);
	_scratch2.data = (byte *)realloc(_scratch2.data, _width * _height * 4);
	_savedScreenRegion.data = (byte *)realloc(_savedScreenRegion.data, _width * _height);

	unhide();
}

void GfxCursor32::copyFromScreen(DrawRegion &target) {
	// In SSCI, mouse events were received via hardware interrupt, so there was
	// a separate branch here that would read from VRAM instead of from the
	// game's back buffer when a mouse event was received while the back buffer
	// was being updated. In ScummVM, mouse events are polled, which means it is
	// not possible to receive a mouse event during a back buffer update, so the
	// code responsible for handling that is removed.
	copy<false>(target, _screenRegion);
}

void GfxCursor32::setPosition(const Common::Point &position) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
	const int16 screenWidth = g_sci->_gfxFrameout->getScreenWidth();
	const int16 screenHeight = g_sci->_gfxFrameout->getScreenHeight();

	Common::Point newPosition;
	newPosition.x = (position.x * Ratio(screenWidth, scriptWidth)).toInt();
	newPosition.y = (position.y * Ratio(screenHeight, scriptHeight)).toInt();

	if (!deviceMoved(newPosition)) {
		g_system->warpMouse(newPosition.x, newPosition.y);
	}
}

void GfxCursor32::gonnaPaint(Common::Rect paintRect) {
	if (!_hideCount && !_needsPaint && !_cursorBack.rect.isEmpty()) {
		paintRect.left &= ~3;
		paintRect.right |= 3;
		if (_cursorBack.rect.intersects(paintRect)) {
			_needsPaint = true;
		}
	}
}

void GfxCursor32::paintStarting() {
	if (_needsPaint) {
		_savedScreenRegion.rect = _cursor.rect;
		copy<false>(_savedScreenRegion, _screenRegion);
		copy<true>(_screenRegion, _cursor);
	}
}

void GfxCursor32::donePainting() {
	if (_needsPaint) {
		copy<false>(_screenRegion, _savedScreenRegion);
		_savedScreenRegion.rect = Common::Rect();
		_needsPaint = false;
	}

	if (!_hideCount && !_cursorBack.rect.isEmpty()) {
		copy<false>(_cursorBack, _screenRegion);
	}
}

bool GfxCursor32::deviceMoved(Common::Point &position) {
	bool restricted = false;

	if (position.x < _restrictedArea.left) {
		position.x = _restrictedArea.left;
		restricted = true;
	}
	if (position.x >= _restrictedArea.right) {
		position.x = _restrictedArea.right - 1;
		restricted = true;
	}
	if (position.y < _restrictedArea.top) {
		position.y = _restrictedArea.top;
		restricted = true;
	}
	if (position.y >= _restrictedArea.bottom) {
		position.y = _restrictedArea.bottom - 1;
		restricted = true;
	}

	if (restricted) {
		g_system->warpMouse(position.x, position.y);
	}

	if (_position != position) {
		_position = position;
		move();
	}

	return restricted;
}

void GfxCursor32::move() {
	if (_hideCount) {
		return;
	}

	// Cursor moved onto the screen after being offscreen
	_cursor.rect.moveTo(_position.x - _hotSpot.x, _position.y - _hotSpot.y);
	if (_cursorBack.rect.isEmpty()) {
		revealCursor();
		return;
	}

	// Cursor moved offscreen
	if (!_cursor.rect.intersects(_screenRegion.rect)) {
		drawToScreen(_cursorBack);
		return;
	}

	if (!_cursor.rect.intersects(_cursorBack.rect)) {
		// Cursor moved to a completely different part of the screen
		_scratch1.rect = _cursor.rect;
		_scratch1.rect.clip(_screenRegion.rect);
		copyFromScreen(_scratch1);

		_scratch2.rect = _scratch1.rect;
		copy<false>(_scratch2, _scratch1);

		copy<true>(_scratch1, _cursor);
		drawToScreen(_scratch1);

		drawToScreen(_cursorBack);

		_cursorBack.rect = _cursor.rect;
		_cursorBack.rect.clip(_screenRegion.rect);
		copy<false>(_cursorBack, _scratch2);
	} else {
		// Cursor moved, but still overlaps the previous cursor location
		Common::Rect mergedRect(_cursorBack.rect);
		mergedRect.extend(_cursor.rect);
		mergedRect.clip(_screenRegion.rect);

		_scratch2.rect = mergedRect;
		copyFromScreen(_scratch2);

		copy<false>(_scratch2, _cursorBack);

		_cursorBack.rect = _cursor.rect;
		_cursorBack.rect.clip(_screenRegion.rect);
		copy<false>(_cursorBack, _scratch2);

		copy<true>(_scratch2, _cursor);
		drawToScreen(_scratch2);
	}
}

} // End of namespace Sci
