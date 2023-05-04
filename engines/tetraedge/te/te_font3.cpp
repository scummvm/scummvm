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

// Since FreeType2 includes files, which contain forbidden symbols, we need to
// allow all symbols here.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_font3.h"
#include "tetraedge/te/te_core.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "common/unicode-bidi.h"

namespace Tetraedge {

TeFont3::TeFont3() {
}

TeFont3::~TeFont3() {
	unload();
}

Graphics::Font *TeFont3::getAtSize(uint size) {
	if (_fonts.contains(size))
		return _fonts.getVal(size);

	if (!_fontFile.isOpen())
		load(getAccessName());

	if (!_fontFile.isOpen())
		error("TeFont3::: Couldn't open font file %s.", getAccessName().c_str());

	_fontFile.seek(0);
	Graphics::Font *newFont = Graphics::loadTTFFont(_fontFile, size, Graphics::kTTFSizeModeCharacter, 0, Graphics::kTTFRenderModeNormal);
	if (!newFont) {
		error("TeFont3::: Couldn't load font %s at size %d.", _loadedPath.c_str(), size);
	}
	_fonts.setVal(size, newFont);
	return newFont;
}

bool TeFont3::load(const Common::String &path) {
	if (_loadedPath == path && _fontFile.isOpen())
		return true; // already open

	TeCore *core = g_engine->getCore();
	Common::FSNode node = core->findFile(path);
	return load(node);
}

bool TeFont3::load(const Common::FSNode &node) {
	const Common::String path = node.getPath();
	if (_loadedPath == path && _fontFile.isOpen())
		return true; // already open

	setAccessName(path);
	_loadedPath = path;

	if (!node.isReadable()) {
		warning("TeFont3::load: Can't read from %s", path.c_str());
		return false;
	}

	if (_fontFile.isOpen())
		_fontFile.close();

	if (!_fontFile.open(node)) {
		warning("TeFont3::load: can't open %s", path.c_str());
		return false;
	}
	return true;
}

void TeFont3::unload() {
	for (auto &entry : _fonts) {
		delete entry._value;
	}
	_fonts.clear();
	_fontFile.close();
}

} // end namespace Tetraedge
