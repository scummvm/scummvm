/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "image/cicn.h"
#include "image/pict.h"

#include "pegasus/cursor.h"
#include "pegasus/graphics.h"
#include "pegasus/pegasus.h"

namespace Pegasus {

Cursor::Cursor() {
	_cursorObscured = false;
	_index = -1;
	startIdling();
}

Cursor::~Cursor() {
	for (uint32 i = 0; i < _info.size(); i++) {
		if (_info[i].surface) {
			_info[i].surface->free();
			delete _info[i].surface;
		}
		delete[] _info[i].palette;
	}

	stopIdling();
}

void Cursor::addCursorFrames(uint16 id) {
	Common::SeekableReadStream *cursStream = g_vm->_resFork->getResource(MKTAG('C', 'u', 'r', 's'), id);
	if (!cursStream)
		error("Could not load cursor frames set %d", id);

	uint16 frameCount = cursStream->readUint16BE();
	for (uint16 i = 0; i < frameCount; i++) {
		CursorInfo info;
		info.tag = cursStream->readUint16BE();
		info.hotspot.x = cursStream->readUint16BE();
		info.hotspot.y = cursStream->readUint16BE();
		info.surface = nullptr;
		info.palette = nullptr;
		info.colorCount = 0;
		_info.push_back(info);
	}

	delete cursStream;

	setCurrentFrameIndex(0);
}

void Cursor::setCurrentFrameIndex(int32 index) {
	if (_index != index) {
		_index = index;
		if (index != -1) {
			loadCursorImage(_info[index]);

			if (_info[index].surface->format.bytesPerPixel == 1) {
				CursorMan.replaceCursorPalette(_info[index].palette, 0, _info[index].colorCount);
				CursorMan.replaceCursor(*_info[index].surface, _info[index].hotspot.x, _info[index].hotspot.y, 0);
			} else {
				CursorMan.replaceCursor(*_info[index].surface, _info[index].hotspot.x, _info[index].hotspot.y, _info[index].surface->format.RGBToColor(0xFF, 0xFF, 0xFF), false);
			}
		}
	}
}

int32 Cursor::getCurrentFrameIndex() const {
	return _index;
}

void Cursor::show() {
	if (!isVisible())
		CursorMan.showMouse(true);

	_cursorObscured = false;
}

void Cursor::hide() {
	CursorMan.showMouse(false);
	setCurrentFrameIndex(0);
}

void Cursor::hideUntilMoved() {
	if (!_cursorObscured) {
		hide();
		_cursorObscured = true;
	}
}

void Cursor::useIdleTime() {
	if (g_system->getEventManager()->getMousePos() != _cursorLocation) {
		_cursorLocation = g_system->getEventManager()->getMousePos();
		if (_index != -1 && _cursorObscured)
			show();
	}
}

void Cursor::getCursorLocation(Common::Point &pt) const {
	pt = _cursorLocation;
}

bool Cursor::isVisible() {
	return CursorMan.isVisible();
}

void Cursor::loadCursorImage(CursorInfo &cursorInfo) {
	if (cursorInfo.surface)
		return;

	if (g_vm->isDVD()) {
		// The DVD version has some higher color PICT images for its cursors
		Common::SeekableReadStream *pictStream = g_vm->_resFork->getResource(MKTAG('P', 'I', 'C', 'T'), cursorInfo.tag + 1000);

		if (pictStream) {
			Image::PICTDecoder pict;
			if (!pict.loadStream(*pictStream))
				error("Failed to decode cursor PICT %d", cursorInfo.tag + 1000);

			cursorInfo.surface = pict.getSurface()->convertTo(g_system->getScreenFormat());
			delete pictStream;
			return;
		}
	}

	cursorInfo.surface = new Graphics::Surface();

	// The CD version uses (only) lower color cicn images for its cursors
	Common::SeekableReadStream *cicnStream = g_vm->_resFork->getResource(MKTAG('c', 'i', 'c', 'n'), cursorInfo.tag);

	if (cicnStream) {
		Image::CicnDecoder cicn;
		if (!cicn.loadStream(*cicnStream))
			error("Failed to decode cursor cicn %d", cursorInfo.tag);

		cursorInfo.surface = cicn.getSurface()->convertTo(g_system->getScreenFormat(), cicn.getPalette(), cicn.getPaletteColorCount());
		delete cicnStream;
		return;
	}

	error("Failed to find color icon %d", cursorInfo.tag);
}

} // End of namespace Pegasus
