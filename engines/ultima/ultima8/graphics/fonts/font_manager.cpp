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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/graphics/fonts/font_manager.h"

#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/fonts/font_shape_archive.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/graphics/fonts/jp_font.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "graphics/fonts/ttf.h"

namespace Ultima {
namespace Ultima8 {

FontManager *FontManager::_fontManager = nullptr;

FontManager::FontManager(bool ttf_antialiasing) : _ttfAntialiasing(ttf_antialiasing) {
	debugN(MM_INFO, "Creating Font Manager...\n");

	_fontManager = this;

	SettingManager *settingman = SettingManager::get_instance();
	settingman->setDefault("ttf_highres", true);
}

FontManager::~FontManager() {
	debugN(MM_INFO, "Destroying Font Manager...\n");

	resetGameFonts();

	for (unsigned int i = 0; i < _ttFonts.size(); ++i)
		delete _ttFonts[i];
	_ttFonts.clear();

	TTFFonts::iterator iter;
	for (iter = _ttfFonts.begin(); iter != _ttfFonts.end(); ++iter)
		delete iter->_value;
	_ttfFonts.clear();

	assert(_fontManager == this);
	_fontManager = nullptr;
}

// Reset the font manager
void FontManager::resetGameFonts() {
	for (unsigned int i = 0; i < _overrides.size(); ++i)
		delete _overrides[i];
	_overrides.clear();
}

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


Graphics::Font *FontManager::getTTF_Font(const Std::string &filename, int pointsize) {
	TTFId id;
	id._filename = filename;
	id._pointSize = pointsize;

	TTFFonts::iterator iter;
	iter = _ttfFonts.find(id);

	if (iter != _ttfFonts.end())
		return iter->_value;

	Common::SeekableReadStream *fontids;
	fontids = FileSystem::get_instance()->ReadFile("@data/" + filename);
	if (!fontids) {
		perr << "Failed to open TTF: @data/" << filename << Std::endl;
		return nullptr;
	}

#ifdef USE_FREETYPE2
	// open font using ScummVM TTF API
	// Note: The RWops and ReadStream will be deleted by the TTF_Font
	Graphics::Font *font = Graphics::loadTTFFont(*fontids, pointsize);

	if (!font) {
		perr << "Failed to open TTF: @data/" << filename << Std::endl;
		return nullptr;
	}

	_ttfFonts[id] = font;

#ifdef DEBUG
	pout << "Opened TTF: @data/" << filename << "." << Std::endl;
#endif

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


bool FontManager::addTTFOverride(unsigned int fontnum, const Std::string &filename,
                                 int pointsize, uint32 rgb, int bordersize,
                                 bool SJIS) {
	Graphics::Font *f = getTTF_Font(filename, pointsize);
	if (!f)
		return false;

	TTFont *font = new TTFont(f, rgb, bordersize, _ttfAntialiasing, SJIS);
	SettingManager *settingman = SettingManager::get_instance();
	bool highres;
	settingman->get("ttf_highres", highres);
	font->setHighRes(highres);

	setOverride(fontnum, font);

#ifdef DEBUG
	pout << "Added TTF override for font " << fontnum << Std::endl;
#endif

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
		pal->_palette[3 * i + 0] = (rgb >> 16) & 0xFF;
		pal->_palette[3 * i + 1] = (rgb >> 8) & 0xFF;
		pal->_palette[3 * i + 2] = (rgb) & 0xFF;
	}
	palman->updatedPalette(fontpal);

#ifdef DEBUG
	pout << "Added JP override for font " << fontnum << Std::endl;
#endif

	return true;
}


bool FontManager::loadTTFont(unsigned int fontnum, const Std::string &filename,
                             int pointsize, uint32 rgb, int bordersize) {
	Graphics::Font *f = getTTF_Font(filename, pointsize);
	if (!f)
		return false;

	TTFont *font = new TTFont(f, rgb, bordersize, _ttfAntialiasing, false);

	// TODO: check if this is indeed what we want for non-gamefonts
	SettingManager *settingman = SettingManager::get_instance();
	bool highres;
	settingman->get("ttf_highres", highres);
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
