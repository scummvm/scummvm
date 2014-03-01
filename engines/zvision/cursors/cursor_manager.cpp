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

#include "zvision/cursors/cursor_manager.h"

#include "zvision/zvision.h"

#include "common/system.h"

#include "graphics/pixelformat.h"
#include "graphics/cursorman.h"


namespace ZVision {

const char *CursorManager::_cursorNames[NUM_CURSORS] = { "active", "arrow", "backward", "downarrow", "forward", "handpt", "handpu", "hdown", "hleft",
                                                         "hright", "hup", "idle", "leftarrow", "rightarrow", "suggest_surround", "suggest_tilt", "turnaround", "zuparrow" };

const char *CursorManager::_zgiCursorFileNames[NUM_CURSORS] = { "g0gbc011.zcr", "g0gac001.zcr", "g0gac021.zcr", "g0gac031.zcr", "g0gac041.zcr", "g0gac051.zcr", "g0gac061.zcr", "g0gac071.zcr", "g0gac081.zcr",
                                                                "g0gac091.zcr", "g0gac101.zcr", "g0gac011.zcr", "g0gac111.zcr", "g0gac121.zcr", "g0gac131.zcr", "g0gac141.zcr", "g0gac151.zcr", "g0gac161.zcr" };

const char *CursorManager::_zNemCursorFileNames[NUM_CURSORS] = { "00act", "arrow", "back", "down", "forw", "handpt", "handpu", "hdown", "hleft",
                                                                 "hright", "hup", "00idle", "left", "right", "ssurr", "stilt", "turn", "up" };


CursorManager::CursorManager(ZVision *engine, const Graphics::PixelFormat *pixelFormat)
		: _engine(engine),
		  _pixelFormat(pixelFormat),
		  _cursorIsPushed(false) {
	// WARNING: The index IDLE_CURSOR_INDEX is hardcoded. If you change the order of _cursorNames/_zgiCursorFileNames/_zNemCursorFileNames, you HAVE to change the index accordingly
	if (_engine->getGameId() == GID_NEMESIS) {
		Common::String name(Common::String::format("%sa.zcr", _zNemCursorFileNames[IDLE_CURSOR_INDEX]));
		_idleCursor = ZorkCursor(name);
	} else if (_engine->getGameId() == GID_GRANDINQUISITOR) {
		_idleCursor = ZorkCursor(_zgiCursorFileNames[IDLE_CURSOR_INDEX]);
	}
}

void CursorManager::initialize() {
	revertToIdle();
	CursorMan.showMouse(true);
}

void CursorManager::changeCursor(const Common::String &cursorName) {
	changeCursor(cursorName, _cursorIsPushed);
}

void CursorManager::changeCursor(const Common::String &cursorName, bool pushed) {
	if (_currentCursor.equals(cursorName) && _cursorIsPushed == pushed)
		return;

	if (_cursorIsPushed != pushed)
		_cursorIsPushed = pushed;

	if (cursorName == "idle" && !pushed) {
		CursorMan.replaceCursor(_idleCursor.getSurface(), _idleCursor.getWidth(), _idleCursor.getHeight(), _idleCursor.getHotspotX(), _idleCursor.getHotspotY(), _idleCursor.getKeyColor(), false, _pixelFormat);
		return;
	}

	for (int i = 0; i < NUM_CURSORS; ++i) {
		if (_engine->getGameId() == GID_NEMESIS) {
			if (cursorName.equals(_cursorNames[i])) {
				_currentCursor = cursorName;

				// ZNem uses a/b at the end of the file to signify not pushed/pushed respectively
				Common::String pushedFlag = pushed ? "b" : "a";
				Common::String name = Common::String::format("%s%s.zcr", _zNemCursorFileNames[i], pushedFlag.c_str());

				changeCursor(ZorkCursor(name));
				return;
			}
		} else if (_engine->getGameId() == GID_GRANDINQUISITOR) {
			if (cursorName.equals(_cursorNames[i])) {
				_currentCursor = cursorName;

				if (!pushed) {
					changeCursor(ZorkCursor(_zgiCursorFileNames[i]));
				} else {
					// ZGI flips not pushed/pushed between a/c and b/d
					// It flips the 4th character of the name
					char buffer[25];
					strcpy(buffer, _zgiCursorFileNames[i]);
					buffer[3] += 2;
					changeCursor(ZorkCursor(buffer));
				}
				return;
			}
		}
	}

	// If we get here, something went wrong
	warning("No cursor found for identifier %s", cursorName.c_str());
}

void CursorManager::changeCursor(const ZorkCursor &cursor) {
	CursorMan.replaceCursor(cursor.getSurface(), cursor.getWidth(), cursor.getHeight(), cursor.getHotspotX(), cursor.getHotspotY(), cursor.getKeyColor(), false, _pixelFormat);
}

void CursorManager::cursorDown(bool pushed) {
	if (_cursorIsPushed == pushed)
		return;

	_cursorIsPushed = pushed;
	changeCursor(_currentCursor, pushed);
}

void CursorManager::setLeftCursor() {
	changeCursor("leftarrow");
}

void CursorManager::setRightCursor() {
	changeCursor("rightarrow");
}

void CursorManager::setUpCursor() {
	changeCursor("zuparrow");
}

void CursorManager::setDownCursor() {
	changeCursor("downarrow");
}

void CursorManager::revertToIdle() {
	_currentCursor = "idle";
	if (!_cursorIsPushed)
		CursorMan.replaceCursor(_idleCursor.getSurface(), _idleCursor.getWidth(), _idleCursor.getHeight(), _idleCursor.getHotspotX(), _idleCursor.getHotspotY(), _idleCursor.getKeyColor(), false, _pixelFormat);
	else
		changeCursor(_currentCursor, _cursorIsPushed);
}

} // End of namespace ZVision
