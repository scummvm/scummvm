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

#include "common/archive.h"
#include "common/stream.h"
#include "common/unzip.h"
#include "common/macresman.h"
#include "graphics/fonts/bdf.h"
#include "graphics/fonts/macfont.h"
#include "graphics/fonts/ttf.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"

namespace Graphics {

// Source: Apple IIGS Technical Note #41, "Font Family Numbers"
// http://apple2.boldt.ca/?page=til/tn.iigs.041
static struct FontProto {
	int id;
	Common::Language lang;
	Common::CodePage encoding;
	const char *name;
} defaultFonts[] = {
	{ 2,		Common::UNK_LANG,	Common::kMacRoman,	"New York" },
	{ 3,		Common::UNK_LANG,	Common::kMacRoman,	"Geneva" },
	{ 4,		Common::UNK_LANG,	Common::kMacRoman,	"Monaco" },
	{ 5,		Common::UNK_LANG,	Common::kMacRoman,	"Venice" },
	{ 6,		Common::UNK_LANG,	Common::kMacRoman,	"London" },
	{ 7,		Common::UNK_LANG,	Common::kMacRoman,	"Athens" },
	{ 8,		Common::UNK_LANG,	Common::kMacRoman,	"San Francisco" },
	{ 9,		Common::UNK_LANG,	Common::kMacRoman,	"Toronto" },
	{ 11,		Common::UNK_LANG,	Common::kMacRoman,	"Cairo" },
	{ 12,		Common::UNK_LANG,	Common::kMacRoman,	"Los Angeles" },
	{ 13,		Common::UNK_LANG,	Common::kMacRoman,	"Zapf Dingbats" },
	{ 14,		Common::UNK_LANG,	Common::kMacRoman,	"Bookman" },
	{ 15,		Common::UNK_LANG,	Common::kMacRoman,	"Helvetica Narrow" },
	{ 16,		Common::UNK_LANG,	Common::kMacRoman,	"Palatino" },
	{ 18,		Common::UNK_LANG,	Common::kMacRoman,	"Zapf Chancery" },
	{ 20,		Common::UNK_LANG,	Common::kMacRoman,	"Times" },
	{ 21,		Common::UNK_LANG,	Common::kMacRoman,	"Helvetica" },
	{ 22,		Common::UNK_LANG,	Common::kMacRoman,	"Courier" },
	{ 23,		Common::UNK_LANG,	Common::kMacRoman,	"Symbol" },
	{ 24,		Common::UNK_LANG,	Common::kMacRoman,	"Taliesin" }, // mobile?
	{ 33,		Common::UNK_LANG,	Common::kMacRoman,	"Avant Garde" },
	{ 34,		Common::UNK_LANG,	Common::kMacRoman,	"New Century Schoolbook" },
	{ 16383,	Common::UNK_LANG,	Common::kMacRoman,	"Chicago" },

	// Japanese (names are Shift JIS encoded)
	{ 16384,	Common::JA_JPN,		Common::kUtf8,		"Osaka" },
	{ 16436,	Common::JA_JPN,		Common::kUtf8,		"Osaka\x81\x7C\x93\x99\x95\x9D" }, // Osaka Mono

	{ -1,		Common::UNK_LANG,	Common::kCodePageInvalid,	NULL }
};

struct AliasProto {
	int id;
	int aliasForId;
	const char *name;
};

static AliasProto defaultAliases[] = {
	// English names for Japanese fonts
	{ 16436,	16436,	"OsakaMono" },

	// Missing Japanese fonts
	// These technically should be separate fonts, not just aliases for Osaka.
	// However, we don't have a free source for these right now.
	{ 16396,	16384,	"\x96\x7B\x96\xBE\x92\xA9\x81\x7C\x82\x6C" }, // Book Mincho - M
	{ 16433,	16436,	"\x93\x99\x95\x9D\x83\x53\x83\x56\x83\x62\x83\x4E" }, // Mono Gothic
	{ 16435,	16436,	"\x93\x99\x95\x9D\x96\xBE\x92\xA9" }, // Mono Ming
	{ 16640,	16384,	"\x92\x86\x83\x53\x83\x56\x83\x62\x83\x4E\x91\xCC" }, // Medium Gothic
	{ 16641,	16384,	"\x8D\xD7\x96\xBE\x92\xA9\x91\xCC" }, // Ming
	{ 16700,	16384,	"\x95\xBD\x90\xAC\x96\xBE\x92\xA9" }, // Heisei Mincho
	{ 16701,	16384,	"\x95\xBD\x90\xAC\x8A\x70\x83\x53\x83\x56\x83\x62\x83\x4E" }, // Heisei Kaku Gothic

	{ -1,		-1,		NULL }
};

static AliasProto latinModeAliases[] = {
	{ 0,		16383,	"System" }, // Chicago
	{ 1,		3,		"Application" }, // Geneva
	{ -1,		-1,		NULL }
};

static AliasProto japaneseModeAliases[] = {
	{ 0,		16384,	"System" }, // Osaka
	{ 1,		16384,	"Application" }, // Osaka
	{ -1,		-1,		NULL }
};

static const char *const fontStyleSuffixes[] = {
	"",
	"Bold",
	"Italic",
	"Underline",
	"Outline",
	"Shadow",
	"Condense",
	"Extend"
};

int parseSlant(const Common::String fontname) {
	int res = 0;

	for (int i = 1; i < 7; i++)
		if (fontname.contains(fontStyleSuffixes[i]))
			res |= (1 << (i - 1));

	return res;
}

Common::String cleanFontName(const Common::String fontname) {
	const char *pos;
	Common::String f = fontname;
	for (int i = 1; i < 7; i++) {
		if ((pos = strstr(f.c_str(), fontStyleSuffixes[i])))
			f = Common::String(f.c_str(), pos);
	}
	f.trim();

	return f;
}

MacFontManager::MacFontManager(uint32 mode, Common::Language language) : _mode(mode), _language(language) {
	for (FontProto *font = defaultFonts; font->name; font++) {
		if (!_fontInfo.contains(font->id)) {
			FontInfo *info = new FontInfo;
			info->lang = font->lang;
			info->encoding = font->encoding;
			info->name = font->name;
			_fontInfo[font->id] = info;
		}
		if (!_fontIds.contains(font->name)) {
			_fontIds[font->name] = font->id;
		}
	}
	for (AliasProto *alias = defaultAliases; alias->name; alias++) {
		if (!_fontInfo.contains(alias->id)) {
			FontInfo *info = new FontInfo;
			info->aliasForId = alias->aliasForId;
			info->name = alias->name;
			_fontInfo[alias->id] = info;
		}
		if (!_fontIds.contains(alias->name)) {
			_fontIds[alias->name] = alias->id;
		}
	}
	setLocalizedFonts();

	if (_mode & MacGUIConstants::kWMModeForceBuiltinFonts) {
		_builtInFonts = true;
	} else {
		loadFonts();
	}
	_japaneseFontsLoaded = false;
}

MacFontManager::~MacFontManager() {
	for (Common::HashMap<int, FontInfo *>::iterator it = _fontInfo.begin(); it != _fontInfo.end(); it++)
		delete it->_value;
	for (Common::HashMap<int, const Graphics::Font *>::iterator it = _uniFonts.begin(); it != _uniFonts.end(); it++)
		delete it->_value;
	for (Common::HashMap<Common::String, Common::SeekableReadStream *>::iterator it = _ttfData.begin(); it != _ttfData.end(); it++)
		delete it->_value;
	for (Common::HashMap<Common::String, MacFont *>::iterator it = _fontRegistry.begin(); it != _fontRegistry.end(); it++)
		delete it->_value;
	for (Common::HashMap<Common::String, MacFontFamily *>::iterator it = _fontFamilies.begin(); it != _fontFamilies.end(); it++)
		delete it->_value;
}

void MacFontManager::setLocalizedFonts() {
	AliasProto *aliases = latinModeAliases;
	if (_language == Common::JA_JPN) {
		aliases = japaneseModeAliases;
		loadJapaneseFonts();
	}
	for (AliasProto *alias = aliases; alias->name; alias++) {
		if (_fontInfo.contains(alias->id)) {
			// Overwrite the font info that's already registered in case
			// we're switching languages or something.
			delete _fontInfo[alias->id];
		}
		FontInfo *info = new FontInfo;
		info->aliasForId = alias->aliasForId;
		info->name = alias->name;
		_fontInfo[alias->id] = info;
		_fontIds[alias->name] = alias->id;
	}
}

void MacFontManager::loadFontsBDF() {
	Common::Archive *dat;

	dat = Common::makeZipArchive("classicmacfonts.dat");

	if (!dat) {
		warning("Could not find classicmacfonts.dat. Falling back to built-in fonts");
		_builtInFonts = true;

		return;
	}

	Common::ArchiveMemberList list;
	dat->listMembers(list);

	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getName());

		Graphics::BdfFont *font = Graphics::BdfFont::loadFont(*stream);

		delete stream;

		Common::String fontName;
		MacFont *macfont;

		if (font->getFamilyName() && *font->getFamilyName()) {
			fontName = Common::String::format("%s-%s-%d", font->getFamilyName(), font->getFontSlant(), font->getFontSize());

			macfont = new MacFont(_fontIds.getValOrDefault(font->getFamilyName(), kMacFontNonStandard), font->getFontSize(), parseFontSlant(font->getFontSlant()));
		} else { // Get it from the file name
			fontName = (*it)->getName();

			// Trim the .bdf extension
			for (int i = fontName.size() - 1; i >= 0; --i) {
				if (fontName[i] == '.') {
					while ((uint)i < fontName.size()) {
						fontName.deleteLastChar();
					}
					break;
				}
			}

			macfont = new MacFont(kMacFontNonStandard);
			macfont->setName(fontName);
		}

		FontMan.assignFontToName(fontName, font);
		//macfont->setFont(font);
		_fontRegistry.setVal(fontName, macfont);

		debug(2, " %s", fontName.c_str());
	}

	_builtInFonts = false;

	delete dat;
}

void MacFontManager::loadFonts() {
	Common::Archive *dat;

	dat = Common::makeZipArchive("classicmacfonts.dat");

	if (!dat) {
		warning("Could not find classicmacfonts.dat. Falling back to built-in fonts");
		_builtInFonts = true;

		return;
	}

	Common::ArchiveMemberList list;
	dat->listMembers(list);

	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getName());

		loadFonts(stream);
	}

	_builtInFonts = false;

	delete dat;
}

void MacFontManager::loadJapaneseFonts() {
	if (_japaneseFontsLoaded)
		return;

#ifdef USE_FREETYPE2
	Common::Archive *dat;

	dat = Common::makeZipArchive("japanesemacfonts.dat");

	if (!dat) {
		warning("Could not find japanesemacfonts.dat");
		return;
	}

	Common::ArchiveMemberList list;
	dat->listMembers(list);

	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getName());
		Common::String fontName = (*it)->getName();

		// Trim the .ttf extension
		for (int i = fontName.size() - 1; i >= 0; --i) {
			if (fontName[i] == '.') {
				while ((uint)i < fontName.size()) {
					fontName.deleteLastChar();
				}
				break;
			}
		}

		_ttfData[fontName + "-0-0"] = stream;
	}

	delete dat;
#else
	warning("Japanese fonts require FreeType");
#endif

	// Set this to true even if we don't have FreeType so we don't spam warnings.
	_japaneseFontsLoaded = true;
}

void MacFontManager::loadFonts(Common::SeekableReadStream *stream) {
	Common::MacResManager fontFile;

	if (!fontFile.loadFromMacBinary(*stream))
		return;

	loadFonts(&fontFile);
}

void MacFontManager::loadFonts(const Common::String &fileName) {
	Common::MacResManager fontFile;

	if (!fontFile.open(fileName))
		return;

	loadFonts(&fontFile);
}

void MacFontManager::loadFonts(Common::MacResManager *fontFile) {
	Common::MacResIDArray fonds = fontFile->getResIDArray(MKTAG('F','O','N','D'));
	if (fonds.size() > 0) {
		for (Common::Array<uint16>::iterator iterator = fonds.begin(); iterator != fonds.end(); ++iterator) {
			Common::SeekableReadStream *fond = fontFile->getResource(MKTAG('F', 'O', 'N', 'D'), *iterator);

			Common::String familyName = fontFile->getResName(MKTAG('F', 'O', 'N', 'D'), *iterator);
			int familySlant = parseSlant(familyName);

			if (familySlant) {
				familyName = cleanFontName(familyName);
			}

			Graphics::MacFontFamily *fontFamily = new MacFontFamily();
			fontFamily->load(*fond);

			Common::Array<Graphics::MacFontFamily::AsscEntry> *assoc = fontFamily->getAssocTable();

			bool fontFamilyUsed = false;

			for (uint i = 0; i < assoc->size(); i++) {
				debug(8, "size: %d style: %d id: %d", (*assoc)[i]._fontSize, (*assoc)[i]._fontStyle | familySlant,
										(*assoc)[i]._fontID);

				Common::SeekableReadStream *fontstream;
				MacFont *macfont;
				Graphics::MacFONTFont *font;

				fontstream = fontFile->getResource(MKTAG('N', 'F', 'N', 'T'), (*assoc)[i]._fontID);

				if (!fontstream)
					fontstream = fontFile->getResource(MKTAG('F', 'O', 'N', 'T'), (*assoc)[i]._fontID);

#ifdef USE_FREETYPE2
				if (!fontstream) {
					// The sfnt resource should be just a copy of a TTF
					fontstream = fontFile->getResource(MKTAG('s', 'f', 'n', 't'), (*assoc)[i]._fontID);
					Common::String fontName = Common::String::format("%s-%d-0", familyName.c_str(), (*assoc)[i]._fontStyle | familySlant);
					_ttfData[fontName] = fontstream;
					continue;
				}
#endif

				if (!fontstream) {
					if ((*assoc)[i]._fontSize == 0) {
						warning("MacFontManager: Detected possible TrueType FontID %d, but no TrueType support detected", (*assoc)[i]._fontID);
					} else {
						warning("MacFontManager: Unknown FontId: %d", (*assoc)[i]._fontID);
					}
					continue;
				}

				fontFamilyUsed = true;

				font = new Graphics::MacFONTFont;
				font->loadFont(*fontstream, fontFamily, (*assoc)[i]._fontSize, (*assoc)[i]._fontStyle | familySlant);

				delete fontstream;

				Common::String fontName = Common::String::format("%s-%d-%d", familyName.c_str(), (*assoc)[i]._fontStyle | familySlant, (*assoc)[i]._fontSize);

				macfont = new MacFont(_fontIds.getValOrDefault(familyName, kMacFontNonStandard), (*assoc)[i]._fontSize, (*assoc)[i]._fontStyle | familySlant);

				FontMan.assignFontToName(fontName, font);
				macfont->setFont(font, false);
				_fontRegistry.setVal(fontName, macfont);

				debug(2, " %s", fontName.c_str());
			}

			delete fond;

			if (fontFamilyUsed)
				_fontFamilies[familyName] = fontFamily;
			else
				delete fontFamily;
		}
	}
}

const Font *MacFontManager::getFont(MacFont macFont) {
	Common::String name;
	const Font *font = 0;

	int aliasForId = getFontAliasForId(macFont.getId());
	if (aliasForId > -1) {
		macFont.setId(aliasForId);
	}

	if (!_builtInFonts) {
		Common::Language lang = getFontLanguage(macFont.getId());
		if (lang == Common::JA_JPN && !_japaneseFontsLoaded) {
			loadJapaneseFonts();
		}

		if (macFont.getName().empty()) {
			name = getFontName(macFont.getId(), macFont.getSize(), macFont.getSlant());
			macFont.setName(name);
		}

		if (!_fontRegistry.contains(macFont.getName())) {
			// Let's try to generate name
			if (macFont.getSlant() != kMacFontRegular) {
				name = getFontName(macFont.getId(), macFont.getSize(), macFont.getSlant(), true);
				macFont.setName(name);
			}

			if (!_fontRegistry.contains(macFont.getName()))
				generateFontSubstitute(macFont);
		}

		font = FontMan.getFontByName(macFont.getName());

		if (!font) {
			debug(1, "Cannot load font '%s'", macFont.getName().c_str());

			font = FontMan.getFontByName(MacFont(kMacFontChicago, 12).getName());
		}
	}

#ifdef USE_FREETYPE2
	if (!font) {
		if (_mode & kWMModeUnicode) {
			if (macFont.getSize() <= 0) {
				debug(1, "MacFontManager::getFont() - Font size <= 0!");
			}
			Common::HashMap<int, const Graphics::Font *>::iterator pFont = _uniFonts.find(macFont.getSize());

			if (pFont != _uniFonts.end()) {
				font = pFont->_value;
			} else {
				font = Graphics::loadTTFFontFromArchive("FreeSans.ttf", macFont.getSize(), Graphics::kTTFSizeModeCharacter, 0, Graphics::kTTFRenderModeMonochrome);
				_uniFonts[macFont.getSize()] = font;
			}
		}
	}
#endif

	if (!font)
		font = FontMan.getFontByUsage(macFont.getFallback());

	return font;
}

int MacFontManager::parseFontSlant(Common::String slant) {
	slant.toUppercase();
	int slantVal = 0;

	if (slant == "I")
		slantVal |= kMacFontItalic;
	if (slant == "B")
		slantVal |= kMacFontBold;
	if (slant == "R")
		slantVal |= kMacFontRegular;

	return slantVal;
}

int MacFontManager::parseSlantFromName(const Common::String &name) {
	int slantVal = 0;

	if (name.contains(" Bold"))
		slantVal |= kMacFontBold;
	if (name.contains(" Italic"))
		slantVal |= kMacFontItalic;
	if (name.contains(" Regular"))
		slantVal |= kMacFontRegular;
	if (name.contains(" Underline"))
		slantVal |= kMacFontUnderline;
	if (name.contains(" Shadow"))
		slantVal |= kMacFontShadow;
	if (name.contains(" Outline"))
		slantVal |= kMacFontOutline;
	if (name.contains(" Condense"))
		slantVal |= kMacFontCondense;
	if (name.contains(" Extend"))
		slantVal |= kMacFontExtend;

	return slantVal;
}

int MacFontManager::registerFontName(Common::String name, int preferredId) {
	// Don't register an empty font name, just return Geneva's ID.
	if (name.empty())
		return 1;

	if (_fontIds.contains(name))
		return _fontIds[name];

	int id;
	if (preferredId > -1 && !_fontInfo.contains(preferredId)) {
		id = preferredId;
	} else {
		// Preferred ID is already registered, find an unused one.
		id = 100;
		while (_fontInfo.contains(id))
			id++;
	}

	FontInfo *info = new FontInfo;
	info->name = name;
	if (preferredId >= 0x4000) {
		info->lang = Common::JA_JPN;
		info->encoding = Common::kWindows932; // default to Shift JIS
	} else {
		info->encoding = Common::kMacRoman;
	}
	_fontInfo[id] = info;
	_fontIds[name] = id;
	return id;
}

void MacFont::setName(const char *name) {
	_name = name;
}

const Common::String MacFontManager::getFontName(uint16 id, int size, int slant, bool tryGen) {
	Common::String rawName = getFontName(id);
	Common::String n = cleanFontName(rawName);
	int extraSlant = parseFontSlant(rawName);
	// let's try parse slant from name
	if (!extraSlant)
		extraSlant = parseSlantFromName(rawName);

	if (n.empty()) {
		warning("MacFontManager: Requested font ID %d not found. Falling back to Geneva", id);
		n = "Geneva";
	}

	return Common::String::format("%s-%d-%d", n.c_str(), slant | extraSlant, size);
}

const Common::String MacFontManager::getFontName(MacFont &font) {
	return getFontName(font.getId(), font.getSize(), font.getSlant());
}

int MacFontManager::getFontIdByName(Common::String name) {
	if (_fontIds.contains(name))
		return _fontIds[name];

	return 1;
}

Common::Language MacFontManager::getFontLanguage(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontLanguage: No _fontInfo entry for font %d", id);
		return Common::UNK_LANG;
	}
	if (_fontInfo[id]->aliasForId > -1) {
		return getFontLanguage(_fontInfo[id]->aliasForId);
	}
	return _fontInfo[id]->lang;
}

Common::CodePage MacFontManager::getFontEncoding(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontEncoding: No _fontInfo entry for font %d", id);
		return Common::kCodePageInvalid;
	}
	if (_fontInfo[id]->aliasForId > -1) {
		return getFontEncoding(_fontInfo[id]->aliasForId);
	}
	return _fontInfo[id]->encoding;
}

int MacFontManager::getFontAliasForId(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontAliasForId: No _fontInfo entry for font %d", id);
		return -1;
	}
	return _fontInfo[id]->aliasForId;
}

Common::String MacFontManager::getFontName(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontAliasForId: No _fontInfo entry for font %d", id);
		return "";
	}
	if (_fontInfo[id]->aliasForId > -1) {
		return getFontName(_fontInfo[id]->aliasForId);
	}
	return _fontInfo[id]->name;
}

void MacFontManager::generateFontSubstitute(MacFont &macFont) {
	Common::String name;

#ifdef USE_FREETYPE2
	// Check if we have TTF data for this font.
	name = getFontName(macFont.getId(), 0, macFont.getSlant());
	if (_ttfData.contains(name)) {
		generateTTFFont(macFont, _ttfData[name]);
		return;
	}
#endif

	// Try to see if we have regular font
	if (macFont.getSlant() != kMacFontRegular) {
		name = getFontName(macFont.getId(), macFont.getSize(), kMacFontRegular);

		if (_fontRegistry.contains(name) && !_fontRegistry[name]->isGenerated()) {
			generateFONTFont(macFont, *_fontRegistry[name]);

			return;
		}
	}

	// Now try twice size
	name = getFontName(macFont.getId(), macFont.getSize() * 2, macFont.getSlant());
	if (_fontRegistry.contains(name) && !_fontRegistry[name]->isGenerated()) {
		generateFONTFont(macFont, *_fontRegistry[name]);

		return;
	}

	// Now half size
	name = getFontName(macFont.getId(), macFont.getSize() / 2, macFont.getSlant());
	if (_fontRegistry.contains(name) && !_fontRegistry[name]->isGenerated()) {
		generateFONTFont(macFont, *_fontRegistry[name]);

		return;
	}

	// No simple substitute was found. Looking for neighborhood fonts

	// First we gather all font sizes for this font
	Common::Array<MacFont *> sizes;
	for (Common::HashMap<Common::String, MacFont *>::iterator i = _fontRegistry.begin(); i != _fontRegistry.end(); ++i) {
		if (i->_value->getId() == macFont.getId() && i->_value->getSlant() == macFont.getSlant() && !i->_value->isGenerated())
			sizes.push_back(i->_value);
	}

	if (sizes.empty()) {
		if (macFont.getSlant() == kMacFontRegular) {
			debug(1, "No viable substitute found (1) for font %s", getFontName(macFont).c_str());
			return;
		}

		// Now let's try to find a regular font
		for (Common::HashMap<Common::String, MacFont *>::iterator i = _fontRegistry.begin(); i != _fontRegistry.end(); ++i) {
			if (i->_value->getId() == macFont.getId() && i->_value->getSlant() == kMacFontRegular && !i->_value->isGenerated())
				sizes.push_back(i->_value);
		}

		if (sizes.empty()) {
			debug(1, "No viable substitute found (2) for font %s", getFontName(macFont).c_str());
			return;
		}
	}

	// Now looking for the next larger font, and store the largest one for next check
	MacFont *candidate = nullptr;
	MacFont *maxSize = sizes[0];
	for (uint i = 0; i < sizes.size(); i++) {
		if (sizes[i]->getSize() == macFont.getSize()) { // Same size but regular slant
			candidate = sizes[i];
			break;
		}

		if ((!candidate && sizes[i]->getSize() > macFont.getSize())
				|| (candidate && sizes[i]->getSize() < candidate->getSize()))
			candidate = sizes[i];

		if (sizes[i]->getSize() > maxSize->getSize())
			maxSize = sizes[i];
	}

	if (candidate) {
		generateFONTFont(macFont, *candidate);
		return;
	}

	// Now next smaller font, which is the biggest we have
	generateFONTFont(macFont, *maxSize);
}

#ifdef USE_FREETYPE2
void MacFontManager::generateTTFFont(MacFont &toFont, Common::SeekableReadStream *stream) {
	debug("Generating TTF font '%s'", getFontName(toFont).c_str());

	// TODO: Handle getSlant() flags

	stream->seek(0);
	Font *font = Graphics::loadTTFFont(*stream, toFont.getSize());

	if (!font) {
		warning("Failed to generate font '%s'", getFontName(toFont).c_str());
	}

	toFont.setGenerated(true);
	toFont.setFont(font, true);

	FontMan.assignFontToName(getFontName(toFont), font);
	_fontRegistry.setVal(getFontName(toFont), new MacFont(toFont));

	debug("Generated font '%s'", getFontName(toFont).c_str());
}
#endif

void MacFontManager::generateFONTFont(MacFont &toFont, MacFont &fromFont) {
	if (fromFont.isTrueType()) {
		warning("Cannot generate FONT font '%s' from TTF font '%s'", getFontName(toFont).c_str(), getFontName(fromFont).c_str());
		return;
	}

	debugN("Found font substitute for font '%s' ", getFontName(toFont).c_str());
	debug("as '%s'", getFontName(fromFont).c_str());

	int slant = kMacFontRegular;
	if (fromFont.getSlant() == kMacFontRegular)
		slant = toFont.getSlant();

	MacFONTFont *fromFONTFont = static_cast<MacFONTFont *>(fromFont.getFont());
	MacFONTFont *font = Graphics::MacFONTFont::scaleFont(fromFONTFont, toFont.getSize(), slant);

	if (!font) {
		warning("Failed to generate font '%s'", getFontName(toFont).c_str());
	}

	toFont.setGenerated(true);
	toFont.setFont(font, false);

	FontMan.assignFontToName(getFontName(toFont), font);
	_fontRegistry.setVal(getFontName(toFont), new MacFont(toFont));

	debug("Generated font '%s'", getFontName(toFont).c_str());
}

} // End of namespace Graphics
