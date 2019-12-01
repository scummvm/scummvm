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

#include "ultima8/misc/pent_include.h"

#if defined(HAVE_SDL_SDL_TTF_H)
#include <SDL/SDL_ttf.h>
#else
#include "SDL_ttf.h"
#endif

#include "ultima8/graphics/fonts/font_manager.h"

#include "ultima8/graphics/fonts/font.h"
#include "ultima8/games/game_data.h"
#include "ShapeFont.h"
#include "FontShapeArchive.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/file_system.h"
#include "TTFont.h"
#include "JPFont.h"
#include "ultima8/graphics/palette_manager.h"
#include "ultima8/graphics/palette.h"
#include "ultima8/conf/setting_manager.h"

namespace Ultima8 {

FontManager *FontManager::fontmanager = 0;

FontManager::FontManager(bool ttf_antialiasing_) : ttf_antialiasing(ttf_antialiasing_) {
	con.Print(MM_INFO, "Creating Font Manager...\n");

	assert(fontmanager == 0);
	fontmanager = this;

	SettingManager *settingman = SettingManager::get_instance();
	settingman->setDefault("ttf_highres", true);

	TTF_Init();
}

FontManager::~FontManager() {
	con.Print(MM_INFO, "Destroying Font Manager...\n");

	resetGameFonts();

	for (unsigned int i = 0; i < ttfonts.size(); ++i)
		delete ttfonts[i];
	ttfonts.clear();

	std::map<TTFId, TTF_Font *>::iterator iter;
	for (iter = ttf_fonts.begin(); iter != ttf_fonts.end(); ++iter)
		TTF_CloseFont(iter->_value);
	ttf_fonts.clear();

	TTF_Quit();

	assert(fontmanager == this);
	fontmanager = 0;
}

// Reset the font manager
void FontManager::resetGameFonts() {
	for (unsigned int i = 0; i < overrides.size(); ++i)
		delete overrides[i];
	overrides.clear();
}

Pentagram::Font *FontManager::getGameFont(unsigned int fontnum,
        bool allowOverride) {
	if (allowOverride && fontnum < overrides.size() && overrides[fontnum])
		return overrides[fontnum];

	return GameData::get_instance()->getFonts()->getFont(fontnum);
}

Pentagram::Font *FontManager::getTTFont(unsigned int fontnum) {
	if (fontnum >= ttfonts.size())
		return 0;
	return ttfonts[fontnum];
}


TTF_Font *FontManager::getTTF_Font(std::string filename, int pointsize) {
	TTFId id;
	id.filename = filename;
	id.pointsize = pointsize;

	std::map<TTFId, TTF_Font *>::iterator iter;
	iter = ttf_fonts.find(id);

	if (iter != ttf_fonts.end())
		return iter->_value;

	IDataSource *fontids;
	fontids = FileSystem::get_instance()->ReadFile("@data/" + filename);
	if (!fontids) {
		perr << "Failed to open TTF: @data/" << filename << std::endl;
		return 0;
	}

	// open font using SDL_RWops.
	// Note: The RWops and IDataSource will be deleted by the TTF_Font
	TTF_Font *font = TTF_OpenFontRW(fontids->getRWops(), 1, pointsize);

	if (!font) {
		perr << "Failed to open TTF: @data/" << filename
		     << ": " << TTF_GetError() << std::endl;
		return 0;
	}

	ttf_fonts[id] = font;

#ifdef DEBUG
	pout << "Opened TTF: @data/" << filename << "." << std::endl;
#endif

	return font;
}

void FontManager::setOverride(unsigned int fontnum, Pentagram::Font *override) {
	if (fontnum >= overrides.size())
		overrides.resize(fontnum + 1);

	if (overrides[fontnum])
		delete overrides[fontnum];

	overrides[fontnum] = override;
}


bool FontManager::addTTFOverride(unsigned int fontnum, std::string filename,
                                 int pointsize, uint32 rgb, int bordersize,
                                 bool SJIS) {
	TTF_Font *f = getTTF_Font(filename, pointsize);
	if (!f)
		return false;

	TTFont *font = new TTFont(f, rgb, bordersize, ttf_antialiasing, SJIS);
	SettingManager *settingman = SettingManager::get_instance();
	bool highres;
	settingman->get("ttf_highres", highres);
	font->setHighRes(highres);

	setOverride(fontnum, font);

#ifdef DEBUG
	pout << "Added TTF override for font " << fontnum << std::endl;
#endif

	return true;
}

bool FontManager::addJPOverride(unsigned int fontnum,
                                unsigned int jpfont, uint32 rgb) {
	ShapeFont *jf = p_dynamic_cast<ShapeFont *>(GameData::get_instance()->getFonts()->getFont(jpfont));
	if (!jf)
		return false;

	JPFont *font = new JPFont(jf, fontnum);

	setOverride(fontnum, font);

	PaletteManager *palman = PaletteManager::get_instance();
	PaletteManager::PalIndex fontpal = static_cast<PaletteManager::PalIndex>
	                                   (PaletteManager::Pal_JPFontStart + fontnum);
	palman->duplicate(PaletteManager::Pal_Game, fontpal);
	Pentagram::Palette *pal = palman->getPalette(fontpal);
	// TODO: maybe a small gradient
	// the main text uses index 3
	// indices 1,2 and 3 are in use for the bullets for conversation options
	for (int i = 1; i < 4; ++i) {
		pal->palette[3 * i + 0] = (rgb >> 16) & 0xFF;
		pal->palette[3 * i + 1] = (rgb >> 8) & 0xFF;
		pal->palette[3 * i + 2] = (rgb) & 0xFF;
	}
	palman->updatedFont(fontpal);

#ifdef DEBUG
	pout << "Added JP override for font " << fontnum << std::endl;
#endif

	return true;
}


bool FontManager::loadTTFont(unsigned int fontnum, std::string filename,
                             int pointsize, uint32 rgb, int bordersize) {
	TTF_Font *f = getTTF_Font(filename, pointsize);
	if (!f)
		return false;

	TTFont *font = new TTFont(f, rgb, bordersize, ttf_antialiasing, false);

	// TODO: check if this is indeed what we want for non-gamefonts
	SettingManager *settingman = SettingManager::get_instance();
	bool highres;
	settingman->get("ttf_highres", highres);
	font->setHighRes(highres);

	if (fontnum >= ttfonts.size())
		ttfonts.resize(fontnum + 1);

	if (ttfonts[fontnum])
		delete ttfonts[fontnum];

	ttfonts[fontnum] = font;

	return true;
}

} // End of namespace Ultima8
