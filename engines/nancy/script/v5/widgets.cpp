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

#include "common/events.h"
#include "common/system.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/luastate.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/ref.h"
#include "engines/nancy/script/scenecontext.h"

#include "engines/nancy/script/v5/scriptinput.h"
#include "engines/nancy/script/v5/widgets.h"

namespace Nancy {
namespace Script {

// Nancy26 names its cursors with strings ("MenuHot")
static bool readCursorValue(lua_State *L, int index, uint &cursorType, bool &isHot) {
	if (lua_type(L, index) == LUA_TSTRING) {
		const int type = g_nancy->_cursor->getCursorTypeByName(lua_tostring(L, index));
		if (type < 0) {
			return false;
		}

		cursorType = type;
		isHot = false;
		return true;
	}

	return testCursor(L, index, cursorType, isHot);
}

// HotspotRecord

void HotspotRecord::readCursorField(int valueIndex, Cursor &cursor) {
	cursor.isSet = readCursorValue(getLua(), valueIndex, cursor.type, cursor.isHot);
}

void HotspotRecord::pushCursorField(const Cursor &cursor) {
	if (cursor.isSet) {
		pushCursor(getLua(), cursor.type, cursor.isHot);
	} else {
		lua_pushnil(getLua());
	}
}

void HotspotRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "onScreen")) {
		_onScreen = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "scene", _sceneName);
	readField(L, tableIndex, "frame", _frame);
	readField(L, tableIndex, "stream", _streamName);
	readField(L, tableIndex, "watchLeft", _watchesLeft);
	readField(L, tableIndex, "watchRight", _watchesRight);

	static const char *const kCursorNames[] = { "cursor", "startCursor", "downCursor", "dragOnCursor", "dragOffCursor" };
	Cursor *const cursors[] = { &_cursor, &_startCursor, &_downCursor, &_dragOnCursor, &_dragOffCursor };
	for (uint i = 0; i < ARRAYSIZE(kCursorNames); ++i) {
		if (pushField(L, tableIndex, kCursorNames[i])) {
			readCursorField(-1, *cursors[i]);
			lua_pop(L, 1);
		}
	}

	Common::String refName;
	if (readField(L, tableIndex, "ref", refName) && !refName.empty()) {
		_ref = getContext().loadRef(refName);
	}

	if (pushField(L, tableIndex, "refColor")) {
		_refColor = checkColor(L, -1);
		_hasRefColor = true;
		lua_pop(L, 1);
	}

	if (pushField(L, tableIndex, "source")) {
		_source = checkRect(L, -1);
		lua_pop(L, 1);
	}
}

bool HotspotRecord::hitTest(const ScriptPoint &point) const {
	double x = point.x;
	double y = point.y;
	getTransform().inverse().transformPoint(x, y);

	int refX = (int)floor(x);
	int refY = (int)floor(y);
	if (!_ref) {
		return _onScreen.contains(refX, refY);
	}

	if (!_source.isEmpty() || !_onScreen.isEmpty()) {
		if (!_onScreen.contains(refX, refY)) {
			return false;
		}

		refX += _source.left - _onScreen.left;
		refY += _source.top - _onScreen.top;
	}

	uint32 pixel;
	if (!_ref->getPixel(refX, refY, pixel)) {
		return false;
	}

	return _hasRefColor ? pixel == _refColor : (pixel >> 24) != 0;
}

void HotspotRecord::setMouseOver(bool isOver) {
	if (isOver == _isMouseOver) {
		return;
	}

	_isMouseOver = isOver;
	callFunction(isOver ? "OnMouseOver" : "OnMouseLeave");
}

bool HotspotRecord::pressRegion(int button) {
	if (!(button == 0 ? _watchesLeft : _watchesRight)) {
		return false;
	}

	_pressedButton = button;
	callFunction(button == 0 ? "OnLeftDown" : "OnRightDown");

	return true;
}

// A button that comes up over the hotspot clicks it; anywhere else, the
// hotspot only lets go of the mouse
void HotspotRecord::releaseRegion(int button) {
	if (_pressedButton != button) {
		return;
	}

	_pressedButton = -1;

	if (!_isMouseOver) {
		callFunction("OnLoseMouseFocus");

		return;
	}

	callFunction(button == 0 ? "OnLeftUp" : "OnRightUp");
	fire();
}

void HotspotRecord::fire() {
	if (!_sceneName.empty()) {
		getContext().changeScene(_streamName, _sceneName, (uint16)_frame);
	}

	finish();
}

bool HotspotRecord::getRegionCursor(uint &cursorType, bool &isHot) const {
	const Cursor *cursor = &_cursor;
	if (_pressedButton >= 0) {
		cursor = _isMouseOver ? &_downCursor : &_dragOffCursor;
	} else if (g_nancy->_cursor->isHoldingItem() && _startCursor.isSet) {
		cursor = &_startCursor;
	}

	if (!cursor->isSet) {
		cursor = &_cursor;
	}

	if (!cursor->isSet) {
		return false;
	}

	cursorType = cursor->type;
	isHot = cursor->isHot;
	return true;
}

bool HotspotRecord::prepareReset() {
	_pressedButton = -1;
	return true;
}

bool HotspotRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _onScreen);
	} else if (key == "scene") {
		lua_pushstring(L, _sceneName.c_str());
	} else if (key == "stream") {
		lua_pushstring(L, _streamName.c_str());
	} else if (key == "frame") {
		lua_pushinteger(L, _frame);
	} else if (key == "watchLeft") {
		lua_pushboolean(L, _watchesLeft);
	} else if (key == "watchRight") {
		lua_pushboolean(L, _watchesRight);
	} else if (key == "down") {
		lua_pushboolean(L, isDown());
	} else if (key == "mouseOver") {
		lua_pushboolean(L, _isMouseOver);
	} else if (key == "cursor") {
		pushCursorField(_cursor);
	} else if (key == "startCursor") {
		pushCursorField(_startCursor);
	} else if (key == "downCursor") {
		pushCursorField(_downCursor);
	} else if (key == "dragOnCursor") {
		pushCursorField(_dragOnCursor);
	} else if (key == "dragOffCursor") {
		pushCursorField(_dragOffCursor);
	} else if (key == "refColor" && _hasRefColor) {
		pushColor(L, _refColor);
	} else if (key == "source") {
		pushRect(L, _source);
	} else {
		return false;
	}

	return true;
}

bool HotspotRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_onScreen = checkRect(L, 3);
	} else if (key == "scene") {
		_sceneName = luaL_checkstring(L, 3);
	} else if (key == "stream") {
		_streamName = luaL_checkstring(L, 3);
	} else if (key == "frame") {
		_frame = luaL_checkint(L, 3);
	} else if (key == "watchLeft") {
		_watchesLeft = lua_toboolean(L, 3) != 0;
	} else if (key == "watchRight") {
		_watchesRight = lua_toboolean(L, 3) != 0;
	} else if (key == "cursor") {
		readCursorField(3, _cursor);
	} else if (key == "startCursor") {
		readCursorField(3, _startCursor);
	} else if (key == "downCursor") {
		readCursorField(3, _downCursor);
	} else if (key == "dragOnCursor") {
		readCursorField(3, _dragOnCursor);
	} else if (key == "dragOffCursor") {
		readCursorField(3, _dragOffCursor);
	} else if (key == "ref") {
		_ref = lua_isnil(L, 3) ? nullptr : getContext().loadRef(luaL_checkstring(L, 3));
	} else if (key == "refColor") {
		_hasRefColor = !lua_isnil(L, 3);
		if (_hasRefColor) {
			_refColor = checkColor(L, 3);
		}
	} else if (key == "source") {
		_source = checkRect(L, 3);
	} else {
		return false;
	}

	return true;
}

// SinkRecord

void SinkRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	// The whole viewport, unless the table says otherwise
	const VIEW *viewport = GetEngineData(VIEW);
	if (viewport) {
		_onScreen = ScriptRect(0, 0, viewport->screenPosition.width(), viewport->screenPosition.height());
	}

	if (pushField(L, tableIndex, "onScreen")) {
		_onScreen = checkRect(L, -1);
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "handleKeys", _handlesKeys);
	readField(L, tableIndex, "handleMouse", _handlesMouse);

	if (pushField(L, tableIndex, "cursor")) {
		_hasCursor = readCursorValue(L, -1, _cursorType, _cursorIsHot);
		lua_pop(L, 1);
	}
}

bool SinkRecord::hitTest(const ScriptPoint &point) const {
	double x = point.x;
	double y = point.y;
	getTransform().inverse().transformPoint(x, y);
	return _onScreen.contains((int)floor(x), (int)floor(y));
}

bool SinkRecord::getRegionCursor(uint &cursorType, bool &isHot) const {
	if (!_hasCursor) {
		return false;
	}

	cursorType = _cursorType;
	isHot = _cursorIsHot;
	return true;
}

bool SinkRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _onScreen);
	} else if (key == "handleKeys") {
		lua_pushboolean(L, _handlesKeys);
	} else if (key == "handleMouse") {
		lua_pushboolean(L, _handlesMouse);
	} else if (key == "cursor") {
		if (_hasCursor) {
			pushCursor(L, _cursorType, _cursorIsHot);
		} else {
			lua_pushnil(L);
		}
	} else {
		return false;
	}

	return true;
}

bool SinkRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_onScreen = checkRect(L, 3);
	} else if (key == "handleKeys") {
		_handlesKeys = lua_toboolean(L, 3) != 0;
	} else if (key == "handleMouse") {
		_handlesMouse = lua_toboolean(L, 3) != 0;
	} else if (key == "cursor") {
		_hasCursor = readCursorValue(L, 3, _cursorType, _cursorIsHot);
	} else {
		return false;
	}

	return true;
}

} // End of namespace Script
} // End of namespace Nancy
