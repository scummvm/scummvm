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

#include "common/system.h"
#include "graphics/cursorman.h"
#include "sci/graphics/celobj32.h"
#include "sci/graphics/cursor32.h"
#include "sci/graphics/frameout.h"

namespace Sci {

GfxCursor32::GfxCursor32() :
	_hideCount(0),
	_position(0, 0),
	_writeToVMAP(false) {
	CursorMan.showMouse(false);
}

void GfxCursor32::init(const Buffer &vmap) {
	_vmap = vmap;
	_vmapRegion.rect = Common::Rect(_vmap.screenWidth, _vmap.screenHeight);
	_vmapRegion.data = (byte *)_vmap.getPixels();
	_restrictedArea = _vmapRegion.rect;
}

GfxCursor32::~GfxCursor32() {
	CursorMan.showMouse(true);
	free(_cursor.data);
	free(_cursorBack.data);
	free(_drawBuff1.data);
	free(_drawBuff2.data);
	free(_savedVmapRegion.data);
}

void GfxCursor32::hide() {
	if (_hideCount++) {
		return;
	}

	if (!_cursorBack.rect.isEmpty()) {
		drawToHardware(_cursorBack);
	}
}

void GfxCursor32::revealCursor() {
	_cursorBack.rect = _cursor.rect;
	_cursorBack.rect.clip(_vmapRegion.rect);
	if (_cursorBack.rect.isEmpty()) {
		return;
	}

	readVideo(_cursorBack);
	_drawBuff1.rect = _cursor.rect;
	copy(_drawBuff1, _cursorBack);
	paint(_drawBuff1, _cursor);
	drawToHardware(_drawBuff1);
}

void GfxCursor32::paint(DrawRegion &target, const DrawRegion &source) {
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

	const int16 sourceStride = source.rect.width() - drawRectWidth;
	const int16 targetStride = target.rect.width() - drawRectWidth;

	for (int16 y = 0; y < drawRectHeight; ++y) {
		for (int16 x = 0; x < drawRectWidth; ++x) {
			if (*sourcePixel != skipColor) {
				*targetPixel = *sourcePixel;
			}
			++targetPixel;
			++sourcePixel;
		}
		sourcePixel += sourceStride;
		targetPixel += targetStride;
	}
}

void GfxCursor32::drawToHardware(const DrawRegion &source) {
	Common::Rect drawRect(source.rect);
	drawRect.clip(_vmapRegion.rect);
	const int16 sourceXOffset = drawRect.left - source.rect.left;
	const int16 sourceYOffset = drawRect.top - source.rect.top;
	byte *sourcePixel = source.data + (sourceYOffset * source.rect.width()) + sourceXOffset;

	g_system->copyRectToScreen(sourcePixel, source.rect.width(), drawRect.left, drawRect.top, drawRect.width(), drawRect.height());
}

void GfxCursor32::unhide() {
	if (_hideCount == 0 || --_hideCount) {
		return;
	}

	_cursor.rect.moveTo(_position.x - _hotSpot.x, _position.y - _hotSpot.y);
	revealCursor();
}

void GfxCursor32::show() {
	if (_hideCount) {
		_hideCount = 0;
		_cursor.rect.moveTo(_position.x - _hotSpot.x, _position.y - _hotSpot.y);
		revealCursor();
	}
}

void GfxCursor32::setRestrictedArea(const Common::Rect &rect) {
	_restrictedArea = rect;

	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;
	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	mulru(_restrictedArea, Ratio(screenWidth, scriptWidth), Ratio(screenHeight, scriptHeight), 0);

	if (_position.x < rect.left) {
		_position.x = rect.left;
	}
	if (_position.x >= rect.right) {
		_position.x = rect.right - 1;
	}
	if (_position.y < rect.top) {
		_position.y = rect.top;
	}
	if (_position.y >= rect.bottom) {
		_position.y = rect.bottom - 1;
	}

	g_system->warpMouse(_position.x, _position.y);
}

void GfxCursor32::clearRestrictedArea() {
	_restrictedArea = _vmapRegion.rect;
}

void GfxCursor32::setView(const GuiResourceId viewId, const int16 loopNo, const int16 celNo) {
	hide();

	_cursorInfo.resourceId = viewId;
	_cursorInfo.loopNo = loopNo;
	_cursorInfo.celNo = celNo;

	if (viewId != -1) {
		CelObjView view(viewId, loopNo, celNo);

		_hotSpot = view._displace;
		_width = view._width;
		_height = view._height;

		// SSCI never increased the size of cursors, but some of the cursors
		// in early SSCI games were designed for low-resolution screens and so
		// are kind of hard to pick out when running in high-resolution mode.
		// To address this, we make some slight adjustments to cursor display
		// in a couple of games.
		// GK1: All the cursors are increased in size since they all appear to
		//      be designed for low-res display.
		// PQ4: We only make the cursors bigger if they are above a set
		//      threshold size because inventory items usually have a
		//      high-resolution cursor representation.
		bool pixelDouble = false;
		if (g_sci->_gfxFrameout->_isHiRes &&
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

		Buffer target(_width, _height, _cursor.data);
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
		_cursorBack.rect.clip(_vmapRegion.rect);
		if (!_cursorBack.rect.isEmpty()) {
			readVideo(_cursorBack);
		}
	}

	_cursorBack.data = (byte *)realloc(_cursorBack.data, _width * _height);
	_drawBuff1.data = (byte *)realloc(_drawBuff1.data, _width * _height);
	_drawBuff2.data = (byte *)realloc(_drawBuff2.data, _width * _height * 4);
	_savedVmapRegion.data = (byte *)realloc(_savedVmapRegion.data, _width * _height);

	unhide();
}

void GfxCursor32::readVideo(DrawRegion &target) {
	if (g_sci->_gfxFrameout->_frameNowVisible) {
		copy(target, _vmapRegion);
	} else {
		// NOTE: SSCI would read the background for the cursor directly out of
		// video memory here, but as far as can be determined, this does not
		// seem to actually be necessary for proper cursor rendering
	}
}

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
	const int16 drawWidth = drawRect.width();
	const int16 drawHeight = drawRect.height();

	byte *targetPixel = target.data + ((drawRect.top - target.rect.top) * target.rect.width()) + (drawRect.left - target.rect.left);
	const byte *sourcePixel = source.data + (sourceYOffset * source.rect.width()) + sourceXOffset;

	const int16 sourceStride = source.rect.width();
	const int16 targetStride = target.rect.width();

	for (int y = 0; y < drawHeight; ++y) {
		memcpy(targetPixel, sourcePixel, drawWidth);
		targetPixel += targetStride;
		sourcePixel += sourceStride;
	}
}

void GfxCursor32::setPosition(const Common::Point &position) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;
	const int16 screenWidth = g_sci->_gfxFrameout->getCurrentBuffer().screenWidth;
	const int16 screenHeight = g_sci->_gfxFrameout->getCurrentBuffer().screenHeight;

	_position.x = (position.x * Ratio(screenWidth, scriptWidth)).toInt();
	_position.y = (position.y * Ratio(screenHeight, scriptHeight)).toInt();

	g_system->warpMouse(_position.x, _position.y);
}

void GfxCursor32::gonnaPaint(Common::Rect paintRect) {
	if (!_hideCount && !_writeToVMAP && !_cursorBack.rect.isEmpty()) {
		paintRect.left &= ~3;
		paintRect.right |= 3;
		if (_cursorBack.rect.intersects(paintRect)) {
			_writeToVMAP = true;
		}
	}
}

void GfxCursor32::paintStarting() {
	if (_writeToVMAP) {
		_savedVmapRegion.rect = _cursor.rect;
		copy(_savedVmapRegion, _vmapRegion);
		paint(_vmapRegion, _cursor);
	}
}

void GfxCursor32::donePainting() {
	if (_writeToVMAP) {
		copy(_vmapRegion, _savedVmapRegion);
		_savedVmapRegion.rect = Common::Rect();
		_writeToVMAP = false;
	}

	if (!_hideCount && !_cursorBack.rect.isEmpty()) {
		copy(_cursorBack, _vmapRegion);
	}
}

void GfxCursor32::deviceMoved(Common::Point &position) {
	if (position.x < _restrictedArea.left) {
		position.x = _restrictedArea.left;
	}
	if (position.x >= _restrictedArea.right) {
		position.x = _restrictedArea.right - 1;
	}
	if (position.y < _restrictedArea.top) {
		position.y = _restrictedArea.top;
	}
	if (position.y >= _restrictedArea.bottom) {
		position.y = _restrictedArea.bottom - 1;
	}

	_position = position;

	g_system->warpMouse(position.x, position.y);
	move();
}

void GfxCursor32::move() {
	if (_hideCount) {
		return;
	}

	// If it was off the screen, just show it and return
	_cursor.rect.moveTo(_position.x - _hotSpot.x, _position.y - _hotSpot.y);
	if (_cursorBack.rect.isEmpty()) {
		revealCursor();
		return;
	}

	// If we just moved entirely off screen, remove background & return
	if (!_cursor.rect.intersects(_vmapRegion.rect)) {
		drawToHardware(_cursorBack);
		return;
	}

	if (!_cursor.rect.intersects(_cursorBack.rect)) {
		_drawBuff1.rect = _cursor.rect;
		_drawBuff1.rect.clip(_vmapRegion.rect);
		readVideo(_drawBuff1);

		_drawBuff2.rect = _drawBuff1.rect;
		copy(_drawBuff2, _drawBuff1);

		paint(_drawBuff1, _cursor);
		drawToHardware(_drawBuff1);

		// erase
		drawToHardware(_cursorBack);

		_cursorBack.rect = _cursor.rect;
		_cursorBack.rect.clip(_vmapRegion.rect);
		copy(_cursorBack, _drawBuff2);
	} else {
		Common::Rect mergedRect(_cursorBack.rect);
		mergedRect.extend(_cursor.rect);
		mergedRect.clip(_vmapRegion.rect);

		_drawBuff2.rect = mergedRect;
		readVideo(_drawBuff2);

		copy(_drawBuff2, _cursorBack);

		_cursorBack.rect = _cursor.rect;
		_cursorBack.rect.clip(_vmapRegion.rect);
		copy(_cursorBack, _drawBuff2);

		paint(_drawBuff2, _cursor);
		drawToHardware(_drawBuff2);
	}
}
} // End of namespace Sci
