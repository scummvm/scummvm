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

#ifndef GRAPHICS_MACGUI_MACFONTMANAGER_H
#define GRAPHICS_MACGUI_MACFONTMANAGER_H

#include "graphics/fontman.h"

namespace Common {
	class SeekableReadStream;
	class MacResManager;
}

namespace Graphics {

class MacFONTFont;

enum {
	kMacFontNonStandard = -1,
	kMacFontChicago = 0,
	kMacFontGeneva = 1,
	kMacFontNewYork = 2,
	kMacFontMonaco = 4,
	kMacFontVenice = 5,
	kMacFontLondon = 6,
	kMacFontAthens = 7,
	kMacFontSanFrancisco = 8,
	kMacFontCairo = 11,
	kMacFontLosAngeles = 12,
	kMacFontPalatino = 16,
	kMacFontTimes = 20,
	kMacFontHelvetica = 21,
	kMacFontCourier = 22,
	kMacFontSymbol = 23
};

enum {
	kMacFontRegular,
	kMacFontBold = 1,
	kMacFontItalic = 2,
	kMacFontUnderline = 4,
	kMacFontOutline = 8,
	kMacFontShadow = 16,
	kMacFontCondense = 32,
	kMacFontExtend = 64
};

class Font;

class MacFont {
public:
	MacFont(int id = kMacFontChicago, int size = 12, int slant = kMacFontRegular, FontManager::FontUsage fallback = Graphics::FontManager::kBigGUIFont) {
		_id = id == 3 ? 1 : id;	// Substitude duplicate "Geneva"
		_size = size ? size : 12;
		_slant = slant;
		_fallback = fallback;
		_generated = false;
		_truetype = false;
		_font = NULL;
	}

	int getId() const { return _id; };
	int getSize() const { return _size; }
	int getSlant() const { return _slant; }
	Common::String getName() { return _name; }
	void setName(Common::String &name) { setName(name.c_str()); }
	void setName(const char *name);
	FontManager::FontUsage getFallback() { return _fallback; }
	bool isGenerated() { return _generated; }
	void setGenerated(bool gen) { _generated = gen; }
	bool isTrueType() { return _truetype; }
	Font *getFont() { return _font; }
	void setFont(Font *font, bool truetype) { _font = font; _truetype = truetype; }

private:
	int _id;
	int _size;
	int _slant;
	bool _truetype;
	Common::String _name;
	FontManager::FontUsage _fallback;

	bool _generated;
	Font *_font;
};

class MacFontManager {
public:
	MacFontManager(uint32 mode);
	~MacFontManager();

	/**
	 * Accessor method to check the presence of built-in fonts.
	 * @return True if there are bult-in fonts.
	 */
	bool hasBuiltInFonts() { return _builtInFonts; }
	/**
	 * Retrieve a font from the available ones.
	 * @param name Name of the desired font.
	 * @param fallback Fallback policy in case the desired font isn't there.
	 * @return The requested font or the fallback.
	 */
	const Font *getFont(MacFont macFont);

	/**
	 * Return font name from standard ID
	 * @param id ID of the font
	 * @param size size of the font
	 * @return the font name or NULL if ID goes beyond the mapping
	 */
	const Common::String getFontName(int id, int size, int slant = kMacFontRegular, bool tryGen = false);
	const Common::String getFontName(MacFont &font);
	int getFontIdByName(Common::String name);

	void loadFonts(Common::SeekableReadStream *stream);
	void loadFonts(const Common::String &fileName);
	void loadFonts(Common::MacResManager *fontFile);

	void registerFontMapping(uint16 id, Common::String name);
	void clearFontMapping();

	void forceBuiltinFonts() { _builtInFonts = true; }

private:
	void loadFontsBDF();
	void loadFonts();

	void generateFontSubstitute(MacFont &macFont);
	void generateFONTFont(MacFont &toFont, MacFont &fromFont);

#ifdef USE_FREETYPE2
	void generateTTFFont(MacFont &toFront, Common::SeekableReadStream *stream);
#endif

private:
	bool _builtInFonts;
	uint32 _mode;
	Common::HashMap<Common::String, MacFont *> _fontRegistry;

	Common::HashMap<Common::String, int> _fontIds;

	Common::HashMap<uint16, Common::String> _extraFontNames;
	Common::HashMap<Common::String, int> _extraFontIds;

	int parseFontSlant(Common::String slant);

	/* Unicode font */
	Common::HashMap<int, const Graphics::Font *> _uniFonts;

	Common::HashMap<int, Common::SeekableReadStream *> _ttfData;
};

} // End of namespace Graphics

#endif
