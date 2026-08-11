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

	if (!_fontFile)
		load(getAccessName());

	if (!_fontFile)
		error("TeFont3::: Couldn't open font file %s.", getAccessName().toString(Common::Path::kNativeSeparator).c_str());

	_fontFile->seek(0);
	Graphics::Font *newFont = Graphics::loadTTFFont(_fontFile.get(), DisposeAfterUse::NO, size, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeNormal);
	if (!newFont) {
		error("TeFont3::: Couldn't load font %s at size %d.", _loadedPath.toString(Common::Path::kNativeSeparator).c_str(), size);
	}
	_fonts.setVal(size, newFont);
	return newFont;
}

bool TeFont3::load(const Common::Path &path) {
	if (_loadedPath == path && _fontFile)
		return true; // already open

	TeCore *core = g_engine->getCore();
	TetraedgeFSNode node = core->findFile(path);
	return load(node);
}

bool TeFont3::load(const TetraedgeFSNode &node) {
	const Common::Path fontPath = node.getPath();
	if (_loadedPath == fontPath && _fontFile)
		return true; // already open

	setAccessName(fontPath);
	_loadedPath = fontPath;

	if (!node.exists()) {
		warning("TeFont3::load: Can't find %s", node.toString().c_str());
		return false;
	}

	_fontFile.reset(node.createReadStream());
	if (!_fontFile) {
		warning("TeFont3::load: can't open %s", node.toString().c_str());
		return false;
	}
	return true;
}

void TeFont3::unload() {
	for (auto &entry : _fonts) {
		delete entry._value;
	}
	_fonts.clear();
	_fontFile.reset();
}

} // end namespace Tetraedge
