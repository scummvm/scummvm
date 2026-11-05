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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/scenecontext.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/script/v5/textrecord.h"

namespace Nancy {
namespace Script {

void BitmapTextRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (pushField(L, tableIndex, "onScreen")) {
		_dest = checkRect(L, -1);
		lua_pop(L, 1);
	}

	_initialDest = _dest;
	readField(L, tableIndex, "useAutotext", _usesAutotext);
	readField(L, tableIndex, "overrideFontColor", _overrideFontColor);
	readField(L, tableIndex, "renderStartIdx", _renderStart);
	readField(L, tableIndex, "renderLen", _renderLength);

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1);
		lua_pop(L, 1);
	}

	Common::String text;
	if (!readField(L, tableIndex, "text", text)) {
		luaL_error(L, "Text AR requires field %s", "text");
	}

	setText(text);

	_graphic.init();
	_graphic.setVisible(false);
	applyDest();
	onZChanged();
	_graphic.registerGraphics();
}

// The transform moves the text, like the scrolling lists of the UI do with
// theirs. The letters don't get cut off at the area of the text.
void BitmapTextRecord::applyDest() {
	const Graphics::ManagedSurface &surface = _graphic._drawSurface;
	Common::Rect position = getTransformedDest(_dest, getTransform(), MAX<int>(surface.w, 1), MAX<int>(surface.h, 1));
	position.translate(_inkOffset.x, _inkOffset.y);
	_graphic.moveTo(position);
	_graphic.setNeedsRedraw(true);
}

void BitmapTextRecord::setText(const Common::String &text) {
	_textKey = text;
	_text = text;
	if (_usesAutotext && !findText(text, _text)) {
		_text = "Missing text: " + text;
	}

	_isDirty = true;
}

void BitmapTextRecord::draw() {
	_layout.layout(_text, _dest.width(), _dest.height(), _overrideFontColor);

	// The picture goes around the letters, wherever they are
	Common::Rect area = _layout.getInkArea();
	if (area.isEmpty()) {
		area = Common::Rect(1, 1);
	}

	Graphics::ManagedSurface &surface = _graphic._drawSurface;
	if (surface.w != area.width() || surface.h != area.height() || !surface.getPixels()) {
		surface.create(area.width(), area.height(), g_nancy->_graphics->getTransparentPixelFormat());
	}

	surface.clear(0);
	_layout.draw(surface, Common::Point(-area.left, -area.top), takeModColor(_modColor), _renderStart, _renderLength);
	_inkOffset = Common::Point(area.left, area.top);
	applyDest();
}

void BitmapTextRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void BitmapTextRecord::registerGraphics() {
	_graphic.registerGraphics();
}

void BitmapTextRecord::updateGraphics() {
	_graphic.setClipRect(getScreenClip());
	_graphic.setVisible(isShown());

	if (_isDirty || hasAlphaChanged()) {
		_isDirty = false;
		draw();
	}
}

bool BitmapTextRecord::prepareReset() {
	_dest = _initialDest;
	applyDest();
	return true;
}

bool BitmapTextRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "text") {
		lua_pushstring(L, _textKey.c_str());
	} else if (key == "useAutotext") {
		lua_pushboolean(L, _usesAutotext);
	} else if (key == "overrideFontColor") {
		if (_overrideFontColor >= 0) {
			lua_pushinteger(L, _overrideFontColor);
		} else {
			lua_pushnil(L);
		}
	} else if (key == "modColor") {
		pushColor(L, _modColor);
	} else if (key == "renderStartIdx") {
		lua_pushinteger(L, _renderStart);
	} else if (key == "renderLen") {
		lua_pushinteger(L, _renderLength);
	} else if (key == "bounds") {
		// Where the letters are on the screen
		if (_isDirty) {
			_layout.layout(_text, _dest.width(), _dest.height(), _overrideFontColor);
		}

		ScriptRect bounds = _layout.getBounds();
		bounds.translate(_dest.left, _dest.top);
		pushRect(L, bounds);
	} else {
		return false;
	}

	return true;
}

bool BitmapTextRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "onScreen") {
		_dest = checkRect(L, 3);
		applyDest();
	} else if (key == "text") {
		setText(luaL_checkstring(L, 3));
	} else if (key == "useAutotext") {
		_usesAutotext = lua_toboolean(L, 3) != 0;
		setText(_textKey);
	} else if (key == "overrideFontColor") {
		_overrideFontColor = lua_isnil(L, 3) ? -1 : luaL_checkint(L, 3);
	} else if (key == "modColor") {
		_modColor = checkColor(L, 3);
	} else if (key == "renderStartIdx") {
		_renderStart = luaL_checkint(L, 3);
	} else if (key == "renderLen") {
		_renderLength = luaL_checkint(L, 3);
	} else {
		return false;
	}

	_isDirty = true;
	return true;
}

} // End of namespace Script
} // End of namespace Nancy
