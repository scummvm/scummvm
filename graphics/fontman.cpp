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
 */

#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/bdf.h"

#include "common/translation.h"

namespace Common {
DECLARE_SINGLETON(Graphics::FontManager);
}

namespace Graphics {

FORWARD_DECLARE_FONT(g_sysfont);
FORWARD_DECLARE_FONT(g_sysfont_big);
FORWARD_DECLARE_FONT(g_consolefont);

FontManager::FontManager() {
	// This assert should *never* trigger, because
	// FontManager is a singleton, thus there is only
	// one instance of it per time. (g_sysfont gets
	// reset to 0 in the desctructor of this class).
	assert(g_sysfont == 0);
	INIT_FONT(g_sysfont);
	INIT_FONT(g_sysfont_big);
	INIT_FONT(g_consolefont);
}

FontManager::~FontManager() {
	delete g_sysfont;
	g_sysfont = 0;
	delete g_sysfont_big;
	g_sysfont_big = 0;
	delete g_consolefont;
	g_consolefont = 0;
}

const struct {
	const char *name;
	FontManager::FontUsage id;
} builtinFontNames[] = {
	{ "builtinConsole", FontManager::kConsoleFont },
	{ "fixed5x8.bdf", FontManager::kConsoleFont },
	{ "fixed5x8-iso-8859-1.bdf", FontManager::kConsoleFont },
	{ "fixed5x8-ascii.bdf", FontManager::kConsoleFont },
	{ "clR6x12.bdf", FontManager::kGUIFont },
	{ "clR6x12-iso-8859-1.bdf", FontManager::kGUIFont },
	{ "clR6x12-ascii.bdf", FontManager::kGUIFont },
	{ "helvB12.bdf", FontManager::kBigGUIFont },
	{ "helvB12-iso-8859-1.bdf", FontManager::kBigGUIFont },
	{ "helvB12-ascii.bdf", FontManager::kBigGUIFont },
	{ 0, FontManager::kConsoleFont }
};

bool FontManager::assignFontToName(const Common::String &name, const Font *font) {
	Common::String lowercaseName = name;
	lowercaseName.toLowercase();
	_fontMap[lowercaseName] = font;
	return true;
}

void FontManager::removeFontName(const Common::String &name) {
	Common::String lowercaseName = name;
	lowercaseName.toLowercase();
	_fontMap.erase(lowercaseName);
}

const Font *FontManager::getFontByName(const Common::String &name) const {
	for (int i = 0; builtinFontNames[i].name; i++)
		if (!scumm_stricmp(name.c_str(), builtinFontNames[i].name))
			return getFontByUsage(builtinFontNames[i].id);

	Common::String lowercaseName = name;
	lowercaseName.toLowercase();
	if (!_fontMap.contains(lowercaseName))
		return 0;
	return _fontMap[lowercaseName];
}

const Font *FontManager::getFontByUsage(FontUsage usage) const {
	switch (usage) {
	case kConsoleFont:
		return g_consolefont;
	case kGUIFont:
		return g_sysfont;
	case kBigGUIFont:
		return g_sysfont_big;
	case kLocalizedFont:
	{
		// First try to find a kBigGUIFont
		Common::String fontName = getLocalizedFontNameByUsage(kBigGUIFont);
		if (!fontName.empty()) {
			const Font *font = getFontByName(fontName);
			if (font)
				return font;
		}
		// Try kGUIFont
		fontName = getLocalizedFontNameByUsage(kGUIFont);
		if (!fontName.empty()) {
			const Font *font = getFontByName(fontName);
			if (font)
				return font;
		}
#ifdef USE_TRANSLATION
		// Accept any other font that has the charset in its name
		for (Common::HashMap<Common::String, const Font *>::const_iterator it = _fontMap.begin() ; it != _fontMap.end() ; ++it) {
			if (it->_key.contains(TransMan.getCurrentCharset()))
				return it->_value;
		}
#endif
		// Fallback: return a non localized kGUIFont.
		// Maybe we should return a null pointer instead?
		return g_sysfont;
	}
	}

	return 0;
}

Common::String FontManager::getLocalizedFontNameByUsage(FontUsage usage) const {
	// We look for a name that matches the usage and that ends in .bdf.
	// It should also not contain "-ascii" or "-iso-" in its name.
	// We take the first name that matches.
	for (int i = 0; builtinFontNames[i].name; i++) {
		if (builtinFontNames[i].id == usage) {
			Common::String fontName(builtinFontNames[i].name);
			if (!fontName.contains("-ascii") && !fontName.contains("-iso-") && fontName.contains(".bdf"))
				return genLocalizedFontFilename(fontName);
		}
	}
	return Common::String();
}

Common::String FontManager::genLocalizedFontFilename(const Common::String &filename) const {
#ifndef USE_TRANSLATION
	return filename;
#else
	// We will transform the font filename in the following way:
	//   name.bdf
	//  will become:
	//   name-charset.bdf
	// Note that name should not contain any dot here!

	// In the first step we look for the dot. In case there is none we will
	// return the normal filename.
	Common::String::const_iterator dot = Common::find(filename.begin(), filename.end(), '.');
	if (dot == filename.end())
		return filename;

	// Put the translated font filename string back together.
	Common::String result(filename.begin(), dot);
	result += '-';
	result += TransMan.getCurrentCharset();
	result += dot;

	return result;
#endif
}

} // End of namespace Graphics
