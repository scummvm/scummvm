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
#include "common/file.h"

#include "ultima/ultima.h"
#include "ultima/ultima8/misc/common_types.h"

#include "ultima/ultima8/gfx/fonts/font_manager.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/fonts/shape_font.h"
#include "ultima/ultima8/gfx/fonts/font_shape_archive.h"
#include "ultima/ultima8/gfx/fonts/tt_font.h"
#include "ultima/ultima8/gfx/fonts/jp_font.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/palette_manager.h"

#include "graphics/fonts/ttf.h"

namespace Ultima {
namespace Ultima8 {

FontManager *FontManager::_fontManager = nullptr;

FontManager::FontManager() {
	debug(1, "Creating Font Manager...");

	_fontManager = this;

	ConfMan.registerDefault("font_highres", true);
}

FontManager::~FontManager() {
	debug(1, "Destroying Font Manager...");

	resetGameFonts();

	assert(_fontManager == this);
	_fontManager = nullptr;
}

// Reset the font manager
void FontManager::resetGameFonts() {
	for (unsigned int i = 0; i < _overrides.size(); ++i)
		delete _overrides[i];
	_overrides.clear();

	for (unsigned int i = 0; i < _ttFonts.size(); ++i)
		delete _ttFonts[i];
	_ttFonts.clear();

	for (auto &i : _ttfFonts)
		delete i._value;
	_ttfFonts.clear();}

Font *FontManager::getGameFont(unsigned int fontnum,
		bool allowOverride) {
	if (allowOverride && fontnum < _overrides.size() && _overrides[fontnum])
		return _overrides[fontnum];

	return GameData::get_instance()->getFonts()->getFont(fontnum);
}

Font *FontManager::getTTFont(unsigned int fontnum) {
	if (fontnum >= _ttFonts.size())
		return nullptr;
	return _ttFonts[fontnum];
}


Graphics::Font *FontManager::getTTF_Font(const Common::Path &filename, int pointsize, bool antialiasing) {
#ifdef USE_FREETYPE2
	TTFId id;
	id._filename = filename;
	id._pointSize = pointsize;

	TTFFonts::iterator iter;
	iter = _ttfFonts.find(id);

	if (iter != _ttfFonts.end())
		return iter->_value;

	Common::File* fontids = new Common::File();
	if (!fontids->open(filename)) {
		warning("Failed to open TTF: %s", filename.toString().c_str());
		delete fontids;
		return nullptr;
	}

	// open font using ScummVM TTF API
	// Note: The RWops and ReadStream will be deleted by the TTF_Font
	Graphics::TTFRenderMode mode = antialiasing ? Graphics::kTTFRenderModeNormal : Graphics::kTTFRenderModeMonochrome;
	Graphics::Font *font = Graphics::loadTTFFont(fontids, DisposeAfterUse::YES, pointsize, Graphics::kTTFSizeModeCharacter, 0, 0, mode, 0, false);

	if (!font) {
		warning("Failed to open TTF: %s", filename.toString().c_str());
		delete fontids;
		return nullptr;
	}

	_ttfFonts[id] = font;

	debugC(kDebugGraphics, "Opened TTF: %s.", filename.toString().c_str());
	return font;
#else // !USE_FREETYPE2
	return nullptr;
#endif
}

void FontManager::setOverride(unsigned int fontnum, Font *newFont) {
	if (fontnum >= _overrides.size())
		_overrides.resize(fontnum + 1);

	if (_overrides[fontnum])
		delete _overrides[fontnum];

	_overrides[fontnum] = newFont;
}


bool FontManager::addTTFOverride(unsigned int fontnum, const Common::Path &filename,
								 int pointsize, uint32 rgb, int bordersize,
								 bool SJIS) {
	bool antialiasing = ConfMan.getBool("font_antialiasing");
	Graphics::Font *f = getTTF_Font(filename, pointsize, antialiasing);
	if (!f)
		return false;

	TTFont *font = new TTFont(f, rgb, bordersize, antialiasing, SJIS);
	bool highres = ConfMan.getBool("font_highres");
	font->setHighRes(highres);

	setOverride(fontnum, font);

	debugC(kDebugGraphics, "Added TTF override for font %u", fontnum);
	return true;
}

bool FontManager::addJPOverride(unsigned int fontnum,
								unsigned int jpfont, uint32 rgb) {
	ShapeFont *jf = dynamic_cast<ShapeFont *>(GameData::get_instance()->getFonts()->getFont(jpfont));
	if (!jf)
		return false;

	JPFont *font = new JPFont(jf, fontnum);

	setOverride(fontnum, font);

	PaletteManager *palman = PaletteManager::get_instance();
	PaletteManager::PalIndex fontpal = static_cast<PaletteManager::PalIndex>
	                                   (PaletteManager::Pal_JPFontStart + fontnum);
	palman->duplicate(PaletteManager::Pal_Game, fontpal);
	Palette *pal = palman->getPalette(fontpal);
	// TODO: maybe a small gradient
	// the main text uses index 3
	// indices 1,2 and 3 are in use for the bullets for conversation options
	for (int i = 1; i < 4; ++i) {
		pal->set(i, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, (rgb) & 0xFF);
	}
	palman->updatedPalette(fontpal);

	debugC(kDebugGraphics, "Added JP override for font %u", fontnum);
	return true;
}


bool FontManager::loadTTFont(unsigned int fontnum, const Common::Path &filename,
							 int pointsize, uint32 rgb, int bordersize) {
	bool antialiasing = ConfMan.getBool("font_antialiasing");
	Graphics::Font *f = getTTF_Font(filename, pointsize, antialiasing);
	if (!f)
		return false;

	TTFont *font = new TTFont(f, rgb, bordersize, antialiasing, false);

	// TODO: check if this is indeed what we want for non-gamefonts
	bool highres = ConfMan.getBool("font_highres");
	font->setHighRes(highres);

	if (fontnum >= _ttFonts.size())
		_ttFonts.resize(fontnum + 1);

	if (_ttFonts[fontnum])
		delete _ttFonts[fontnum];

	_ttFonts[fontnum] = font;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
