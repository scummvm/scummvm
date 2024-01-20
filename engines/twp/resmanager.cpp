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

#include "common/config-manager.h"
#include "common/str.h"
#include "image/png.h"
#include "twp/resmanager.h"
#include "twp/gfx.h"
#include "twp/twp.h"
#include "twp/ggpack.h"

namespace Twp {

Common::String getKey(const char *path) {
	int len = strlen(path);
	Common::String p(path);
	size_t i = p.findLastOf(".");
	p = p.substr(0, i);
	if ((len > 4) && scumm_strnicmp(p.c_str() + 4, "_en", 3) == 0) {
		Common::String lang = ConfMan.get("language");
		Common::String filename(path, len - 3);
		const char *ext = path + i;
		return Common::String::format("%s_%s%s", filename.c_str(), lang.c_str(), ext);
	}
	return path;
}

void ResManager::loadTexture(const Common::String &name) {
	debug("Load texture %s", name.c_str());
	GGPackEntryReader r;
	if(!r.open(g_engine->_pack, name)) {
		error("Texture %s not found", name.c_str());
	}
	Image::PNGDecoder d;
	d.loadStream(r);
	const Graphics::Surface *surface = d.getSurface();
	if(!surface) {
		error("PNG %s not loaded, please check USE_PNG flag", name.c_str());
	}

	_textures[name].load(*surface);
}

Texture *ResManager::texture(const Common::String &name) {
	Common::String key = getKey(name.c_str());
	if (!_textures.contains(key)) {
		loadTexture(key.c_str());
	}
	return &_textures[key];
}

void ResManager::loadSpriteSheet(const Common::String &name) {
	GGPackEntryReader r;
	r.open(g_engine->_pack, name + ".json");

	// read all contents
	Common::Array<char> data(r.size());
	r.read(data.data(), r.size());

	Common::String s(data.data(), r.size());
	_spriteSheets[name].parseSpriteSheet(s);
}

void ResManager::loadFont(const Common::String &name) {
	if (name == "sayline") {
		debug("Load font %s", name.c_str());
		Common::String resName = ConfMan.getBool("retroFonts") ? "FontRetroSheet": "FontModernSheet";
		_fontModernSheet.load(resName);
		_fonts[name] = &_fontModernSheet;
	} else if (name == "C64Font") {
		debug("Load font %s", name.c_str());
		_fontC64TermSheet.load("FontC64TermSheet");
		_fonts[name] = &_fontC64TermSheet;
	} else {
		BmFont* font = new BmFont();
		font->load(name);
		_fonts[name] = font;
	}
}

SpriteSheet *ResManager::spriteSheet(const Common::String &name) {
	Common::String key = getKey(name.c_str());
	if (!_spriteSheets.contains(key)) {
		loadSpriteSheet(key.c_str());
	}
	return &_spriteSheets[key];
}

Font *ResManager::font(const Common::String &name) {
	Common::String key = getKey(name.c_str());
	if (!_fonts.contains(key)) {
		loadFont(key.c_str());
	}
	return _fonts[key];
}

} // namespace Twp
