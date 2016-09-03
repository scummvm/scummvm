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

#include "common/scummsys.h"

#include "zvision/graphics/cursors/cursor_manager.h"

#include "zvision/zvision.h"

#include "common/system.h"

#include "graphics/pixelformat.h"
#include "graphics/cursorman.h"

namespace ZVision {

const char *CursorManager::_cursorNames[NUM_CURSORS] = { "active", "arrow", "backward", "downarrow", "forward", "handpt", "handpu", "hdown", "hleft",
                                                         "hright", "hup", "idle", "leftarrow", "rightarrow", "suggest_surround", "suggest_tilt", "turnaround", "zuparrow"
                                                       };

const char *CursorManager::_zgiCursorFileNames[NUM_CURSORS] = { "g0gbc011.zcr", "g0gac011.zcr", "g0gac021.zcr", "g0gac031.zcr", "g0gac041.zcr", "g0gac051.zcr", "g0gac061.zcr", "g0gac071.zcr", "g0gac081.zcr",
                                                                "g0gac091.zcr", "g0gac101.zcr", "g0gac011.zcr", "g0gac111.zcr", "g0gac121.zcr", "g0gac131.zcr", "g0gac141.zcr", "g0gac151.zcr", "g0gac161.zcr"
                                                              };

const char *CursorManager::_zNemCursorFileNames[NUM_CURSORS] = { "00act", "arrow", "back", "down", "forw", "handpt", "handpu", "hdown", "hleft",
                                                                 "hright", "hup", "00idle", "left", "right", "ssurr", "stilt", "turn", "up"
                                                               };

CursorManager::CursorManager(ZVision *engine, const Graphics::PixelFormat pixelFormat)
	: _engine(engine),
	  _pixelFormat(pixelFormat),
	  _cursorIsPushed(false),
	  _item(0),
	  _lastitem(0),
	  _currentCursor(CursorIndex_Idle) {
	for (int i = 0; i < NUM_CURSORS; i++) {
		if (_engine->getGameId() == GID_NEMESIS) {
			Common::String name;
			if (i == 1) {
				// Cursors "arrowa.zcr" and "arrowb.zcr" are missing
				_cursors[i][0] = _cursors[i][1] = ZorkCursor();
				continue;
			}
			name = Common::String::format("%sa.zcr", _zNemCursorFileNames[i]);
			_cursors[i][0] = ZorkCursor(_engine, name); // Up cursor
			name = Common::String::format("%sb.zcr", _zNemCursorFileNames[i]);
			_cursors[i][1] = ZorkCursor(_engine, name); // Down cursor
		} else if (_engine->getGameId() == GID_GRANDINQUISITOR) {
			_cursors[i][0] = ZorkCursor(_engine, _zgiCursorFileNames[i]); // Up cursor
			char buffer[25];
			memset(buffer, 0, 25);
			strncpy(buffer, _zgiCursorFileNames[i], 24);
			buffer[3] += 2;
			_cursors[i][1] = ZorkCursor(_engine, buffer); // Down cursor
		}
	}
}

void CursorManager::setItemID(int id) {
	if (id != _item) {
		if (id) {
			Common::String file;
			if (_engine->getGameId() == GID_NEMESIS) {
				file = Common::String::format("%2.2d%s%c.zcr", id, "idle", 'a');
				_cursors[NUM_CURSORS][0] = ZorkCursor(_engine, file);
				file = Common::String::format("%2.2d%s%c.zcr", id, "idle", 'b');
				_cursors[NUM_CURSORS][1] = ZorkCursor(_engine, file);
				file = Common::String::format("%2.2d%s%c.zcr", id, "act", 'a');
				_cursors[NUM_CURSORS + 1][0] = ZorkCursor(_engine, file);
				file = Common::String::format("%2.2d%s%c.zcr", id, "act", 'b');
				_cursors[NUM_CURSORS + 1][0] = ZorkCursor(_engine, file);
			} else if (_engine->getGameId() == GID_GRANDINQUISITOR) {
				file = Common::String::format("g0b%cc%2.2x1.zcr", 'a' , id);
				_cursors[NUM_CURSORS][0] = ZorkCursor(_engine, file);
				file = Common::String::format("g0b%cc%2.2x1.zcr", 'c' , id);
				_cursors[NUM_CURSORS][1] = ZorkCursor(_engine, file);
				file = Common::String::format("g0b%cc%2.2x1.zcr", 'b' , id);
				_cursors[NUM_CURSORS + 1][0] = ZorkCursor(_engine, file);
				file = Common::String::format("g0b%cc%2.2x1.zcr", 'd' , id);
				_cursors[NUM_CURSORS + 1][1] = ZorkCursor(_engine, file);
			} else
				return;
		}
		_item = id;
		changeCursor(CursorIndex_Idle);
	}
}

void CursorManager::initialize() {
	changeCursor(_cursors[CursorIndex_Idle][_cursorIsPushed]);
	showMouse(true);
}

void CursorManager::changeCursor(const ZorkCursor &cursor) {
	CursorMan.replaceCursor(cursor.getSurface(), cursor.getWidth(), cursor.getHeight(), cursor.getHotspotX(), cursor.getHotspotY(), cursor.getKeyColor(), false, &_pixelFormat);
}

void CursorManager::cursorDown(bool pushed) {
	if (_cursorIsPushed == pushed)
		return;

	_cursorIsPushed = pushed;

	changeCursor(_cursors[_currentCursor][_cursorIsPushed]);
}

void CursorManager::changeCursor(int id) {
	if (_item && (id == CursorIndex_Active ||
	              id == CursorIndex_Idle ||
	              id == CursorIndex_HandPu)) {
		if (id == CursorIndex_Idle) {
			id = CursorIndex_ItemIdle;
		} else {
			id = CursorIndex_ItemAct;
		}
	}

	if (_currentCursor != id || ((id == CursorIndex_ItemAct || id == CursorIndex_ItemIdle) && _lastitem != _item)) {
		_currentCursor = id;
		_lastitem = _item;
		changeCursor(_cursors[_currentCursor][_cursorIsPushed]);
	}
}

int CursorManager::getCursorId(const Common::String &name) {
	for (int i = 0; i < NUM_CURSORS; i++) {
		if (name.equals(_cursorNames[i])) {
			return i;
		}
	}

	return CursorIndex_Idle;
}

void CursorManager::showMouse(bool vis) {
	CursorMan.showMouse(vis);
}

} // End of namespace ZVision
