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
 */

#include "common/archive.h"
#include "common/stream.h"
#include "common/compression/unzip.h"
#include "common/macresman.h"
#include "graphics/fonts/bdf.h"
#include "graphics/fonts/macfont.h"
#include "graphics/fonts/winfont.h"
#include "graphics/fonts/ttf.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"

namespace Graphics {

// Source: Apple IIGS Technical Note #41, "Font Family Numbers"
// https://www.1000bit.it/support/manuali/apple/technotes/iigs/tn.iigs.041.html
static const struct FontProto {
	int id;
	Common::Language lang;
	Common::CodePage encoding;
	const char *name;
} defaultFonts[] = {
	{ kMacFontNewYork,		Common::UNK_LANG,	Common::kMacRoman,	"New York" },
	{ kMacFontGeneva,		Common::UNK_LANG,	Common::kMacRoman,	"Geneva" },
	{ kMacFontMonaco,		Common::UNK_LANG,	Common::kMacRoman,	"Monaco" },
	{ kMacFontVenice,		Common::UNK_LANG,	Common::kMacRoman,	"Venice" },
	{ kMacFontLondon,		Common::UNK_LANG,	Common::kMacRoman,	"London" },
	{ kMacFontAthens,		Common::UNK_LANG,	Common::kMacRoman,	"Athens" },
	{ kMacFontSanFrancisco,		Common::UNK_LANG,	Common::kMacRoman,	"San Francisco" },
	{ kMacFontToronto,		Common::UNK_LANG,	Common::kMacRoman,	"Toronto" },
	{ kMacFontCairo,		Common::UNK_LANG,	Common::kMacRoman,	"Cairo" },
	{ kMacFontLosAngeles,		Common::UNK_LANG,	Common::kMacRoman,	"Los Angeles" },
	{ kMacFontZapfDingbats,		Common::UNK_LANG,	Common::kMacRoman,	"Zapf Dingbats" },
	{ kMacFontBookman,		Common::UNK_LANG,	Common::kMacRoman,	"Bookman" },
	{ kMacFontHelveticaNarrow,	Common::UNK_LANG,	Common::kMacRoman,	"Helvetica Narrow" },
	{ kMacFontPalatino,		Common::UNK_LANG,	Common::kMacRoman,	"Palatino" },
	{ kMacFontZapfChancery,		Common::UNK_LANG,	Common::kMacRoman,	"Zapf Chancery" },
	{ kMacFontTimes,		Common::UNK_LANG,	Common::kMacRoman,	"Times" },
	{ kMacFontHelvetica,		Common::UNK_LANG,	Common::kMacRoman,	"Helvetica" },
	{ kMacFontCourier,		Common::UNK_LANG,	Common::kMacRoman,	"Courier" },
	{ kMacFontSymbol,		Common::UNK_LANG,	Common::kMacRoman,	"Symbol" },
	{ kMacFontTaliesin,		Common::UNK_LANG,	Common::kMacRoman,	"Taliesin" }, // mobile?
	{ kMacFontAvantGarde,		Common::UNK_LANG,	Common::kMacRoman,	"Avant Garde" },
	{ kMacFontNewCenturySchoolbook,	Common::UNK_LANG,	Common::kMacRoman,	"New Century Schoolbook" },
	{ kMacFontChicago,		Common::UNK_LANG,	Common::kMacRoman,	"Chicago" },

	// Japanese (names are Shift JIS encoded)
	{ kMacFontOsaka,		Common::JA_JPN,		Common::kUtf8,		"Osaka" },
	{ kMacFontOsakaMono,		Common::JA_JPN,		Common::kUtf8,		"Osaka\x81\x7C\x93\x99\x95\x9D" },

	{ kMacFontNonStandard,		Common::UNK_LANG,	Common::kCodePageInvalid,	NULL }
};

struct AliasProto {
	int id;
	int aliasForId;
	const char *name;
};

static const AliasProto defaultAliases[] = {
	// English names for Japanese fonts
	{ kMacFontOsakaMono,		kMacFontOsakaMono,	"OsakaMono" },

	// Missing Japanese fonts
	// These technically should be separate fonts, not just aliases for Osaka.
	// However, we don't have a free source for these right now.
	{ kMacFontBookMinchoM,		kMacFontOsaka,		"\x96\x7B\x96\xBE\x92\xA9\x81\x7C\x82\x6C" }, // Book Mincho - M
	{ kMacFontMonoGothic,		kMacFontOsakaMono,	"\x93\x99\x95\x9D\x83\x53\x83\x56\x83\x62\x83\x4E" }, // Mono Gothic
	{ kMacFontMonoMing,		kMacFontOsakaMono,	"\x93\x99\x95\x9D\x96\xBE\x92\xA9" }, // Mono Ming
	{ kMacFontMediumGothic,		kMacFontOsaka,		"\x92\x86\x83\x53\x83\x56\x83\x62\x83\x4E\x91\xCC" }, // Medium Gothic
	{ kMacFontMing,			kMacFontOsaka,		"\x8D\xD7\x96\xBE\x92\xA9\x91\xCC" }, // Ming
	{ kMacFontHeiseiMincho,		kMacFontOsaka,		"\x95\xBD\x90\xAC\x96\xBE\x92\xA9" }, // Heisei Mincho
	{ kMacFontHeiseiKakuGothic,	kMacFontOsaka,		"\x95\xBD\x90\xAC\x8A\x70\x83\x53\x83\x56\x83\x62\x83\x4E" }, // Heisei Kaku Gothic

	{ kMacFontNonStandard,		kMacFontNonStandard,	NULL }
};

static const AliasProto latinModeAliases[] = {
	{ kMacFontSystem,		kMacFontChicago,	"System" },
	{ kMacFontApplication,		kMacFontGeneva,		"Application" },

	{ kMacFontNonStandard,		kMacFontNonStandard,	NULL }
};

static const AliasProto japaneseModeAliases[] = {
	{ kMacFontSystem,		kMacFontOsaka,		"System" },
	{ kMacFontApplication,		kMacFontOsaka,		"Application" },

	{ kMacFontNonStandard,		kMacFontNonStandard,	NULL }
};

static const char *const fontStyleSuffixes[] = {
	"Regular",
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
	for (int i = 0; i < 7; i++) {
		if ((pos = strstr(f.c_str(), fontStyleSuffixes[i]))) {
			while (pos > f.c_str() && *(pos - 1) == '-')
				pos--;

			f = Common::String(f.c_str(), pos);
		}
	}
	f.trim();

	return f;
}

MacFontManager::MacFontManager(uint32 mode, Common::Language language) : _mode(mode),
	_language(language), _japaneseFontsLoaded(false) {
	for (const FontProto *font = defaultFonts; font->name; font++) {
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
	for (const AliasProto *alias = defaultAliases; alias->name; alias++) {
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
}

MacFontManager::~MacFontManager() {
	for (auto &it: _fontInfo)
		delete it._value;
	for (auto &it: _uniFonts)
		delete it._value;
	for (auto &it: _ttfData)
		delete it._value;
	for (auto &it: _fontRegistry)
		delete it._value;
	for (auto &it: _winFontRegistry)
		delete it._value;
	for (auto &it: _fontFamilies)
		delete it;
}

void MacFontManager::setLocalizedFonts() {
	const AliasProto *aliases = latinModeAliases;
	if (_language == Common::JA_JPN) {
		aliases = japaneseModeAliases;
		loadJapaneseFonts();
	}
	for (const AliasProto *alias = aliases; alias->name; alias++) {
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
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getPathInArchive());

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

		debugC(5, kDebugLevelMacGUI, " %s", fontName.c_str());
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
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getPathInArchive());

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
		Common::SeekableReadStream *stream = dat->createReadStreamForMember((*it)->getPathInArchive());
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

	if (!fontFile.loadFromMacBinary(stream))
		return;

	loadFonts(&fontFile);
}

void MacFontManager::loadFonts(const Common::Path &fileName) {
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

			familyName = cleanFontName(familyName);

			Graphics::MacFontFamily *fontFamily = new MacFontFamily(familyName);
			fontFamily->load(*fond);

			Common::Array<Graphics::MacFontFamily::AsscEntry> *assoc = fontFamily->getAssocTable();

			bool fontFamilyUsed = false;

			for (uint i = 0; i < assoc->size(); i++) {
				debugC(5, kDebugLevelMacGUI, "size: %d style: %d id: %d", (*assoc)[i]._fontSize, (*assoc)[i]._fontStyle | familySlant,
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

				Common::String name = fontFamily->getName();

				if (!_fontIds.contains(name)) {
					int id = fontFamily->getFontFamilyId();

					FontInfo *info = new FontInfo;
					info->name = fontFamily->getName();
					if (id >= 0x4000) {
						info->lang = Common::JA_JPN;
						info->encoding = Common::kWindows932;
					} else {
						info->encoding = Common::kMacRoman;
					}

					_fontIds[name] = id;
					_fontInfo[id] = info;
				}

				Common::String fontName = Common::String::format("%s-%d-%d", familyName.c_str(), (*assoc)[i]._fontStyle | familySlant, (*assoc)[i]._fontSize);

				macfont = new MacFont(_fontIds.getValOrDefault(familyName, kMacFontNonStandard), (*assoc)[i]._fontSize, (*assoc)[i]._fontStyle | familySlant);

				FontMan.assignFontToName(fontName, font);
				macfont->setFont(font, false);
				_fontRegistry.setVal(fontName, macfont);

				debugC(5, kDebugLevelMacGUI, " %s", fontName.c_str());
			}

			delete fond;

			if (fontFamilyUsed)
				_fontFamilies.push_back(fontFamily);
			else
				delete fontFamily;
		}
	}
}

void MacFontManager::loadWindowsFont(const Common::Path &fileName) {
	Graphics::WinFont *winFont = new Graphics::WinFont();
	bool isLoaded = winFont->loadFromFON(fileName);

	if (!isLoaded) {
		warning("MacFontManager::loadWindowsFont(): Windows Font data from file %s not loaded",
				fileName.toString(Common::Path::kNativeSeparator).c_str());
		delete winFont;
		return;
	}

	Common::String fontName = winFont->getName();
	_winFontRegistry.setVal(fontName, winFont);
	MacFont *font = new MacFont();
	Common::String fullName = Common::String::format("%s-%d-%d", fontName.c_str(), winFont->getStyle(), winFont->getFontHeight());
	font->setName(fullName);
	font->setFont(winFont, false);
	_fontRegistry.setVal(font->getName(), font);

	debugC(1, kDebugLevelMacGUI, "MacFontManager::loadWindowsFont(): Loaded font %s", fullName.c_str());
}

const Font *MacFontManager::getFont(MacFont *macFont) {
	Common::String name;
	const Font *font = 0;

	debugC(2, kDebugLevelMacGUI, "MacFontManager::getFont(%s), id: %d", getFontName(macFont->getId(), macFont->getSize(), macFont->getSlant(), 0).c_str(), macFont->getId());

	int aliasForId = getFontAliasForId(macFont->getId());
	if (aliasForId > -1) {
		macFont->setId(aliasForId);
	}

	printFontRegistry(3, kDebugLevelMacGUI);

	if (!_builtInFonts) {
		Common::Language lang = getFontLanguage(macFont->getId());
		if (lang == Common::JA_JPN && !_japaneseFontsLoaded) {
			loadJapaneseFonts();
		}

		if (macFont->getName().empty()) {
			name = getFontName(macFont->getId(), macFont->getSize(), macFont->getSlant());
			macFont->setName(name);
		}

		if (!_fontRegistry.contains(macFont->getName())) {
			// Let's try to generate name
			if (macFont->getSlant() != kMacFontRegular) {
				name = getFontName(macFont->getId(), macFont->getSize(), macFont->getSlant(), true);
				macFont->setName(name);
			}

			if (!_fontRegistry.contains(macFont->getName()))
				generateFontSubstitute(*macFont);
		}

		font = FontMan.getFontByName(macFont->getName());

		if (!font) {
			debugC(1, kDebugLevelMacGUI, "Cannot load font '%s'", macFont->getName().c_str());

			font = FontMan.getFontByName(MacFont(kMacFontSystem, 12).getName());
		}
	}

#ifdef USE_FREETYPE2
	if (!font && !(_mode & MacGUIConstants::kWMModeForceMacFonts)) {
		if (_mode & kWMModeUnicode) {
			if (macFont->getSize() <= 0) {
				debugC(1, kDebugLevelMacGUI, "MacFontManager::getFont() - Font size <= 0!");
			}
			Common::HashMap<int, const Graphics::Font *>::iterator pFont = _uniFonts.find(macFont->getSize());

			if (pFont != _uniFonts.end()) {
				font = pFont->_value;
			} else {
				int newId = macFont->getId();
				int newSlant = macFont->getSlant();
				int familyId = getFamilyId(newId, newSlant);
				if (_fontInfo.contains(familyId) && !(_mode & kWMModeForceMacFontsInWin95)) {
					font = Graphics::loadTTFFontFromArchive(_fontInfo[familyId]->name, macFont->getSize(), Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeMonochrome);
					_uniFonts[macFont->getSize()] = font;
				} else {
					font = Graphics::loadTTFFontFromArchive("FreeSans.ttf", macFont->getSize(), Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeMonochrome);
					_uniFonts[macFont->getSize()] = font;
				}
			}
		} else {
			int newId = macFont->getId();
			int newSlant = macFont->getSlant();
			int familyId = getFamilyId(newId, newSlant);
			font = Graphics::loadTTFFontFromArchive(_fontInfo[familyId]->name, macFont->getSize(), Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeMonochrome);
			_uniFonts[macFont->getSize()] = font;
		}
	}
#endif

	if (!font) {
		int id = macFont->getId();

		if (_fontInfo.contains(id) && _winFontRegistry.contains(_fontInfo.getVal(id)->name)) {
			font = _winFontRegistry.getVal(_fontInfo.getVal(id)->name);
			const Graphics::WinFont *winfont = (const Graphics::WinFont *)font;

			if (winfont->getFontHeight() != macFont->getSize()) {
				debugC(1, kDebugLevelMacGUI, "MacFontManager::getFont(): For font '%s' windows font '%s' is used of a different size %d", macFont->getName().c_str(), winfont->getName().c_str(), winfont->getFontHeight());

				Common::String fullFontName = Common::String::format("%s-%d-%d", winfont->getName().c_str(), winfont->getStyle(), macFont->getSize());

				if (_winFontRegistry.contains(fullFontName)) {
					// Check if we have generated this earlier, in that case reuse it.
					font = _winFontRegistry.getVal(fullFontName);
				} else {
					// Generate a scaledFont
					Graphics::WinFont *scaledWinFont = WinFont::scaleFont(winfont, macFont->getSize());
					if (scaledWinFont) {
						debugC(1, kDebugLevelMacGUI, "MacFontManager::getFont(): Generated scaled winFont %s", fullFontName.c_str());

						// register font generated for reuse
						_winFontRegistry.setVal(fullFontName, scaledWinFont);

						font = scaledWinFont;
					}
				}
			}
		}
	}

	if (!font) {
		font = macFont->getFallback();

		for (auto &it : _fontInfo) {
			if (it._value->name == macFont->getFallbackName()) {
				macFont->setId(it._key);
			}
		}
	}

	return font;
}

const Font *MacFontManager::getFont(MacFont macFont) {
	return getFont(&macFont);
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

	if (name.contains(" Bold") || name.equalsIgnoreCase("Bold"))
		slantVal |= kMacFontBold;
	if (name.contains(" Italic") || name.equalsIgnoreCase("Italic"))
		slantVal |= kMacFontItalic;
	if (name.contains(" Regular") || name.equalsIgnoreCase("Regular"))
		slantVal |= kMacFontRegular;
	if (name.contains(" Underline") || name.equalsIgnoreCase("Underline"))
		slantVal |= kMacFontUnderline;
	if (name.contains(" Shadow") || name.equalsIgnoreCase("Shadow"))
		slantVal |= kMacFontShadow;
	if (name.contains(" Outline") || name.equalsIgnoreCase("Outline"))
		slantVal |= kMacFontOutline;
	if (name.contains(" Condense") || name.equalsIgnoreCase("Condense"))
		slantVal |= kMacFontCondense;
	if (name.contains(" Extend") || name.equalsIgnoreCase("Extend"))
		slantVal |= kMacFontExtend;
	if (name.contains(" Plain") || name.equalsIgnoreCase("Plain"))
		slantVal = kMacFontRegular;

	return slantVal;
}

int MacFontManager::registerFontName(Common::String name, int preferredId) {
	// Don't register an empty font name, just return Geneva's ID.
	if (name.empty())
		return 1;

	if (_fontIds.contains(name))
		return _fontIds[name];

	debugC(1, kDebugLevelMacGUI, "MacFontManager::registerFontName('%s', %d)", toPrintable(name).c_str(), preferredId);

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

int MacFontManager::registerTTFFont(const TTFMap ttfList[]) {
	int defaultValue = 1;
	int realId = 100;
	auto checkId = [&](int id) {
		for (const TTFMap *i = ttfList; i->ttfName; i++) {
			if (_fontInfo.contains(id + i->slant)) {
				return true;
			}
		}
		return false;
	};

	while (checkId(realId))
		realId++;

	for (const TTFMap *i = ttfList; i->ttfName; i++) {
		int id = realId;
		Common::String name = i->ttfName;

		if (name.empty()) {
			if (defaultValue == 1)
				defaultValue = id;
			continue;
		}

		if (_fontIds.contains(name)) {
			if (defaultValue == 1)
				defaultValue = _fontIds[name];
			continue;
		}

		int slant = 0;

		id += slant | i->slant;

		FontInfo *info = new FontInfo;
		info->name = name;
		_fontInfo[id] = info;
		_fontIds[name] = id;
		if (defaultValue == 1)
			defaultValue = id;
	}
	return defaultValue;
}

int MacFontManager::getFamilyId(int newId, int newSlant) {
	if (_fontInfo.contains(newId + newSlant)) {
		return newId + newSlant;
	}
	warning("MacFontManager::getFamilyId(): No font with slant %d found, setting to kMacFontRegular", newSlant);
	return newId;
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

const Common::String MacFontManager::getFontName(const MacFont &font) {
	return getFontName(font.getId(), font.getSize(), font.getSlant());
}

int MacFontManager::getFontIdByName(Common::String name) {
	if (_fontIds.contains(name))
		return _fontIds[name];

	for (auto it = _fontIds.begin(); it != _fontIds.end(); it++) {
		if (it->_key.equalsIgnoreCase(name)) {
			return it->_value;
		}
	}
	return 1;
}

Common::Language MacFontManager::getFontLanguage(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontLanguage: No _fontInfo entry for font ID %d", id);
		return Common::UNK_LANG;
	}
	if (_fontInfo[id]->aliasForId > -1) {
		return getFontLanguage(_fontInfo[id]->aliasForId);
	}
	return _fontInfo[id]->lang;
}

Common::CodePage MacFontManager::getFontEncoding(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontEncoding: No _fontInfo entry for font ID %d", id);
		return Common::kCodePageInvalid;
	}
	if (_fontInfo[id]->aliasForId > -1) {
		return getFontEncoding(_fontInfo[id]->aliasForId);
	}
	return _fontInfo[id]->encoding;
}

int MacFontManager::getFontAliasForId(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontAliasForId: No _fontInfo entry for font ID %d", id);
		return -1;
	}
	return _fontInfo[id]->aliasForId;
}

Common::String MacFontManager::getFontName(uint16 id) {
	if (!_fontInfo.contains(id)) {
		warning("MacFontManager::getFontName: No _fontInfo entry for font ID %d", id);
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
	if (!(_mode & MacGUIConstants::kWMModeForceMacFonts)) {
		// Check if we have TTF data for this font.
		name = getFontName(macFont.getId(), 0, macFont.getSlant());
		if (_ttfData.contains(name)) {
			generateTTFFont(macFont, _ttfData[name]);
			return;
		}
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
			debugC(1, kDebugLevelMacGUI, "No viable substitute found (1) for font %s", toPrintable(getFontName(macFont)).c_str());
			return;
		}

		// Now let's try to find a regular font
		for (Common::HashMap<Common::String, MacFont *>::iterator i = _fontRegistry.begin(); i != _fontRegistry.end(); ++i) {
			if (i->_value->getId() == macFont.getId() && i->_value->getSlant() == kMacFontRegular && !i->_value->isGenerated())
				sizes.push_back(i->_value);
		}

		if (sizes.empty()) {
			debugC(1, kDebugLevelMacGUI, "No viable substitute found (2) for font %s", toPrintable(getFontName(macFont)).c_str());
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
	debugC(1, kDebugLevelMacGUI, "Generating TTF font '%s'", toPrintable(getFontName(toFont)).c_str());

	// TODO: Handle getSlant() flags

	stream->seek(0);
	Font *font = Graphics::loadTTFFont(stream, DisposeAfterUse::NO, toFont.getSize(), Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeMonochrome);

	if (!font) {
		warning("Failed to generate font '%s'", toPrintable(getFontName(toFont)).c_str());
	}

	toFont.setGenerated(true);
	toFont.setFont(font, true);

	FontMan.assignFontToName(getFontName(toFont), font);
	_fontRegistry.setVal(getFontName(toFont), new MacFont(toFont));

	debugC(1, kDebugLevelMacGUI, "Generated font '%s'", toPrintable(getFontName(toFont)).c_str());
}
#endif

void MacFontManager::generateFONTFont(MacFont &toFont, MacFont &fromFont) {
	if (fromFont.isTrueType()) {
		warning("Cannot generate FONT font '%s' from TTF font '%s'", toPrintable(getFontName(toFont)).c_str(), toPrintable(getFontName(fromFont)).c_str());
		return;
	}

	debugC(1, kDebugLevelMacGUI, "Found font substitute for font '%s' as '%s'", toPrintable(getFontName(toFont)).c_str(), toPrintable(getFontName(fromFont)).c_str());

	int slant = kMacFontRegular;
	if (fromFont.getSlant() == kMacFontRegular)
		slant = toFont.getSlant();

	MacFONTFont *fromFONTFont = static_cast<MacFONTFont *>(fromFont.getFont());
	MacFONTFont *font = Graphics::MacFONTFont::scaleFont(fromFONTFont, toFont.getSize(), slant);

	if (!font) {
		warning("Failed to generate font '%s'", toPrintable(getFontName(toFont)).c_str());
	}

	toFont.setGenerated(true);
	toFont.setFont(font, false);

	FontMan.assignFontToName(getFontName(toFont), font);
	_fontRegistry.setVal(getFontName(toFont), new MacFont(toFont));

	debugC(1, kDebugLevelMacGUI, "Generated font '%s'", toPrintable(getFontName(toFont)).c_str());
}

void MacFont::setFallback(const Font *font, Common::String name) {
	_fallback = font;
	_fallbackName = name;
}

void MacFontManager::printFontRegistry(int debugLevel, uint32 channel) {
		debugC(debugLevel, channel, "Font Registry: %d items", _fontRegistry.size());

		for (Common::HashMap<Common::String, MacFont *>::iterator i = _fontRegistry.begin(); i != _fontRegistry.end(); ++i) {
			MacFont *f = i->_value;
			debugC(debugLevel, channel, "name: '%s' gen:%c ttf:%c ID: %d size: %d slant: %d fallback: '%s'",
				toPrintable(f->getName()).c_str(), f->isGenerated() ? 'y' : 'n', f->isTrueType() ? 'y' : 'n',
				f->getId(), f->getSize(), f->getSlant(), toPrintable(f->getFallbackName()).c_str());
		}
}

} // End of namespace Graphics
