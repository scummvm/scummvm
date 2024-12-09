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

#ifndef GRAPHICS_MACGUI_MACFONTMANAGER_H
#define GRAPHICS_MACGUI_MACFONTMANAGER_H

#include "common/language.h"
#include "graphics/fonts/bdf.h"
#include "graphics/fontman.h"

namespace Common {
	class SeekableReadStream;
	class MacResManager;
}

namespace Graphics {

struct TTFMap {
	const char *ttfName;
	uint16 slant;
};

class MacFONTFont;
class MacFontFamily;

enum {
	kMacFontNonStandard = -1,
	kMacFontSystem = 0,
	kMacFontApplication = 1,

	kMacFontNewYork = 2,
	kMacFontGeneva = 3,
	kMacFontMonaco = 4,
	kMacFontVenice = 5,
	kMacFontLondon = 6,
	kMacFontAthens = 7,
	kMacFontSanFrancisco = 8,
	kMacFontToronto = 9,
	kMacFontCairo = 11,
	kMacFontLosAngeles = 12,
	kMacFontZapfDingbats = 13,
	kMacFontBookman = 14,
	kMacFontHelveticaNarrow = 15,
	kMacFontPalatino = 16,
	kMacFontZapfChancery = 18,
	kMacFontTimes = 20,
	kMacFontHelvetica = 21,
	kMacFontCourier = 22,
	kMacFontSymbol = 23,
	kMacFontTaliesin = 24,
	kMacFontAvantGarde = 33,
	kMacFontNewCenturySchoolbook = 34,
	kMacFontChicago = 16383,

	kMacFontOsaka = 16384,
	kMacFontBookMinchoM = 16396,
	kMacFontMonoGothic = 16433,
	kMacFontMonoMing = 16435,
	kMacFontOsakaMono = 16436,
	kMacFontMediumGothic = 16640,
	kMacFontMing = 16641,
	kMacFontHeiseiMincho = 16700,
	kMacFontHeiseiKakuGothic = 16701
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

struct FontInfo {
	Common::Language lang;
	Common::CodePage encoding;
	int aliasForId;
	Common::String name;

	FontInfo() : lang(Common::UNK_LANG), encoding(Common::kCodePageInvalid), aliasForId(-1) {}
};

class MacFont {
public:
	MacFont(int id = kMacFontSystem, int size = 12, int slant = kMacFontRegular) {
		_id = id;
		_size = size ? size : 12;
		_slant = slant;
		_fallback = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		_fallbackName = Common::String(((const BdfFont *)_fallback)->getFamilyName());
		_generated = false;
		_truetype = false;
		_font = NULL;
	}

	int getId() const { return _id; };
	void setId(int id) { _id = id; }
	int getSize() const { return _size; }
	int getSlant() const { return _slant; }
	Common::String getName() const { return _name; }
	void setName(Common::String &name) { setName(name.c_str()); }
	void setName(const char *name);
	const Graphics::Font *getFallback() const { return _fallback; }
	bool isGenerated() const { return _generated; }
	void setGenerated(bool gen) { _generated = gen; }
	bool isTrueType() const { return _truetype; }
	Font *getFont() const { return _font; }
	void setFont(Font *font, bool truetype) { _font = font; _truetype = truetype; }
	void setFallback(const Font *font, Common::String name = "");
	Common::String getFallbackName() const { return _fallbackName; }

private:
	int _id;
	int _size;
	int _slant;
	bool _truetype;
	Common::String _name;
	const Graphics::Font *_fallback;
	Common::String _fallbackName;

	bool _generated;
	Font *_font;
};

class MacFontManager {
public:
	MacFontManager(uint32 mode, Common::Language language);
	~MacFontManager();

	void setLocalizedFonts();

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
	const Font *getFont(MacFont *macFont);
	const Font *getFont(MacFont macFont);

	/**
	 * Return font name from standard ID
	 * @param id ID of the font
	 * @param size size of the font
	 * @return the font name or NULL if ID goes beyond the mapping
	 */
	const Common::String getFontName(uint16 id, int size, int slant = kMacFontRegular, bool tryGen = false);
	const Common::String getFontName(const MacFont &font);
	int getFontIdByName(Common::String name);

	Common::Language getFontLanguage(uint16 id);
	Common::CodePage getFontEncoding(uint16 id);
	int getFontAliasForId(uint16 id);
	Common::String getFontName(uint16 id);

	void loadFonts(Common::SeekableReadStream *stream);
	void loadFonts(const Common::Path &fileName);
	void loadFonts(Common::MacResManager *fontFile);
	void loadWindowsFont(const Common::Path &fileName);

	/**
	 * Register a font name if it doesn't already exist.
	 * @param name name of the font
	 * @return the font's ID
	 */
	int registerFontName(Common::String name, int preferredId = -1);

	void forceBuiltinFonts() { _builtInFonts = true; }
	int parseSlantFromName(const Common::String &name);

	const Common::Array<MacFontFamily *> &getFontFamilies() { return _fontFamilies; }

	void printFontRegistry(int debugLevel, uint32 channel);

	int registerTTFFont(const Graphics::TTFMap ttfList[]);

	int getFamilyId(int newId, int newSlant);

private:
	void loadFontsBDF();
	void loadFonts();
	void loadJapaneseFonts();

	void generateFontSubstitute(MacFont &macFont);
	void generateFONTFont(MacFont &toFont, MacFont &fromFont);

#ifdef USE_FREETYPE2
	void generateTTFFont(MacFont &toFront, Common::SeekableReadStream *stream);
#endif

private:
	bool _builtInFonts;
	bool _japaneseFontsLoaded;
	uint32 _mode;
	Common::Language _language;
	Common::HashMap<Common::String, Graphics::Font *> _winFontRegistry;
	Common::HashMap<Common::String, MacFont *> _fontRegistry;
	Common::Array<MacFontFamily *> _fontFamilies;

	Common::HashMap<int, FontInfo *> _fontInfo;
	Common::HashMap<Common::String, int> _fontIds;

	int parseFontSlant(Common::String slant);

	/* Unicode font */
	Common::HashMap<int, const Graphics::Font *> _uniFonts;

	Common::HashMap<Common::String, Common::SeekableReadStream *> _ttfData;
};

} // End of namespace Graphics

#endif
