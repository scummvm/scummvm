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

#include "common/textconsole.h"

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/script/v5/overlaycombiner.h"

namespace Nancy {
namespace Script {

static void readRequiredRect(lua_State *L, int tableIndex, const char *name, ScriptRect &rect) {
	if (!pushField(L, tableIndex, name)) {
		luaL_error(L, "OverlayCombiner ARs must have an '%s' field", name);
	}

	rect = checkRect(L, -1);
	lua_pop(L, 1);
}

static void readRequiredName(lua_State *L, int tableIndex, const char *name, Common::String &value) {
	if (!readField(L, tableIndex, name, value)) {
		luaL_error(L, "OverlayCombiner ARs must have an '%s' field", name);
	}
}

void OverlayCombinerRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	readRequiredRect(L, tableIndex, "rgbSource", _rgbSource);
	readRequiredRect(L, tableIndex, "alphaSource", _alphaSource);
	readRequiredRect(L, tableIndex, "onScreen", _dest);
	readRequiredName(L, tableIndex, "rgbOvl", _rgbImageName);
	readField(L, tableIndex, "rgbTree", _rgbTreeName);
	readRequiredName(L, tableIndex, "alphaOvl", _alphaImageName);
	readField(L, tableIndex, "alphaTree", _alphaTreeName);

	if (pushField(L, tableIndex, "modColor")) {
		_modColor = checkColor(L, -1) | 0xFF000000;
		lua_pop(L, 1);
	}

	readField(L, tableIndex, "invertAlpha", _isAlphaInverted);

	_graphic.init();
	_graphic.setVisible(false);
	onZChanged();
	_graphic.registerGraphics();
}

// The images are only kept loaded while they are being combined
void OverlayCombinerRecord::combine() {
	_isCombined = true;
	_hasPicture = false;
	_isPlacementStale = true;

	if (_rgbSource.width() != _alphaSource.width() || _rgbSource.height() != _alphaSource.height()) {
		warning("OverlayCombiner: Sources are not the same size, RGB: %s, Alpha: %s", _rgbImageName.c_str(), _alphaImageName.c_str());
		return;
	}

	Common::SharedPtr<Graphics::ManagedSurface> rgbCached;
	Common::SharedPtr<Graphics::ManagedSurface> alphaCached;
	Graphics::ManagedSurface rgbImage;
	Graphics::ManagedSurface alphaImage;
	if (!loadCachedImage(_rgbImageName, _rgbTreeName, rgbCached, rgbImage) ||
			!loadCachedImage(_alphaImageName, _alphaTreeName, alphaCached, alphaImage)) {
		warning("OverlayCombiner: Could not load %s or %s", _rgbImageName.c_str(), _alphaImageName.c_str());
		return;
	}

	const Common::Rect rgbSource = _rgbSource.toRect();
	const Common::Rect alphaSource = _alphaSource.toRect();
	if (!Common::Rect(rgbImage.w, rgbImage.h).contains(rgbSource) || !Common::Rect(alphaImage.w, alphaImage.h).contains(alphaSource)) {
		warning("OverlayCombiner: Either %s or %s were too large to lock for reading", _rgbImageName.c_str(), _alphaImageName.c_str());
		return;
	}

	_image.create(rgbSource.width(), rgbSource.height(), g_nancy->_graphics->getTransparentPixelFormat());
	for (int y = 0; y < _image.h; ++y) {
		for (int x = 0; x < _image.w; ++x) {
			byte alpha, red, green, blue;
			rgbImage.format.colorToARGB(rgbImage.getPixel(rgbSource.left + x, rgbSource.top + y), alpha, red, green, blue);

			byte maskAlpha, maskRed, maskGreen, maskBlue;
			alphaImage.format.colorToARGB(alphaImage.getPixel(alphaSource.left + x, alphaSource.top + y), maskAlpha, maskRed, maskGreen, maskBlue);
			if (_isAlphaInverted) {
				maskAlpha = 255 - maskAlpha;
			}

			_image.setPixel(x, y, _image.format.ARGBToColor(alpha * maskAlpha >> 8, red, green, blue));
		}
	}

	_hasPicture = true;
}

void OverlayCombinerRecord::place() {
	_isPlacementStale = false;
	_isDrawn = false;
	if (!_hasPicture) {
		return;
	}

	if (_dest.width() != _image.w || _dest.height() != _image.h) {
		warning("OverlayCombiner: Source and dest are not the same size");
		return;
	}

	placePicture(_graphic, _image, Common::Rect(_image.w, _image.h), _dest, getTransform(), takeModColor(_modColor));
	_isDrawn = true;
}

void OverlayCombinerRecord::onTransformChanged() {
	_isPlacementStale = true;
}

void OverlayCombinerRecord::onZChanged() {
	_graphic.setScriptZ(getZ(), getContext());
}

void OverlayCombinerRecord::registerGraphics() {
	_graphic.registerGraphics();
}

void OverlayCombinerRecord::updateGraphics() {
	if (isShown()) {
		if (!_isCombined) {
			combine();
		}

		if (_isPlacementStale || hasAlphaChanged()) {
			place();
		}
	}

	_graphic.setClipRect(getScreenClip());
	_graphic.setVisible(isShown() && _isDrawn, false);
}

bool OverlayCombinerRecord::getField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "rgbOvl") {
		lua_pushstring(L, _rgbImageName.c_str());
	} else if (key == "rgbTree") {
		lua_pushstring(L, _rgbTreeName.c_str());
	} else if (key == "rgbSource") {
		pushRect(L, _rgbSource);
	} else if (key == "alphaOvl") {
		lua_pushstring(L, _alphaImageName.c_str());
	} else if (key == "alphaTree") {
		lua_pushstring(L, _alphaTreeName.c_str());
	} else if (key == "alphaSource") {
		pushRect(L, _alphaSource);
	} else if (key == "onScreen") {
		pushRect(L, _dest);
	} else if (key == "invertAlpha") {
		lua_pushboolean(L, _isAlphaInverted);
	} else if (key == "modColor") {
		pushColor(L, _modColor);
	} else {
		return false;
	}

	return true;
}

// Changing the images or their source parts combines them again. Changing
// where or how the picture is shown places it again.
bool OverlayCombinerRecord::setField(const Common::String &key) {
	lua_State *L = getLua();

	if (key == "rgbOvl") {
		_rgbImageName = luaL_checkstring(L, 3);
	} else if (key == "rgbTree") {
		_rgbTreeName = luaL_checkstring(L, 3);
	} else if (key == "rgbSource") {
		_rgbSource = checkRect(L, 3);
	} else if (key == "alphaOvl") {
		_alphaImageName = luaL_checkstring(L, 3);
	} else if (key == "alphaTree") {
		_alphaTreeName = luaL_checkstring(L, 3);
	} else if (key == "alphaSource") {
		_alphaSource = checkRect(L, 3);
	} else if (key == "invertAlpha") {
		_isAlphaInverted = lua_toboolean(L, 3) != 0;
	} else if (key == "onScreen") {
		_dest = checkRect(L, 3);
		_isPlacementStale = true;
		return true;
	} else if (key == "modColor") {
		_modColor = checkColor(L, 3) | 0xFF000000;
		_isPlacementStale = true;
		return true;
	} else {
		return false;
	}

	_isCombined = false;
	return true;
}

} // End of namespace Script
} // End of namespace Nancy
