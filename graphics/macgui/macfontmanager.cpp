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
static const char *const fontNames[] = {
	"Chicago",	// system font
	"Geneva",	// application font
	"New York",
	NULL, // FIXME: "Geneva",

	"Monaco",
	"Venice",
	"London",
	"Athens",

	"San Francisco",
	"Toronto",
	NULL,
	"Cairo",
	"Los Angeles", // 12

	"Zapf Dingbats",
	"Bookman",
	"Helvetica Narrow",
	"Palatino",
	NULL,
	"Zapf Chancery",
	NULL,

	"Times", // 20
	"Helvetica",
	"Courier",
	"Symbol",
	"Taliesin", // mobile?
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 30
	NULL,
	NULL,
	"Avant Garde",
	"New Century Schoolbook"
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

MacFontManager::MacFontManager(uint32 mode) : _mode(mode) {
	for (uint i = 0; i < ARRAYSIZE(fontNames); i++)
		if (fontNames[i])
			_fontIds.setVal(fontNames[i], i);

	if (_mode & MacGUIConstants::kWMModeForceBuiltinFonts) {
		_builtInFonts = true;
	} else {
		loadFonts();
	}
}

MacFontManager::~MacFontManager() {
	for (Common::HashMap<int, const Graphics::Font *>::iterator it = _uniFonts.begin(); it != _uniFonts.end(); it++)
		delete it->_value;
	for (Common::HashMap<int, Common::SeekableReadStream *>::iterator it = _ttfData.begin(); it != _ttfData.end(); it++)
		delete it->_value;
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

			macfont = new MacFont(_fontIds.getVal(font->getFamilyName(), kMacFontNonStandard), font->getFontSize(), parseFontSlant(font->getFontSlant()));
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
					_ttfData[_fontIds.getVal(familyName, kMacFontNonStandard)] = fontstream;
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

				macfont = new MacFont(_fontIds.getVal(familyName, kMacFontNonStandard), (*assoc)[i]._fontSize, (*assoc)[i]._fontStyle | familySlant);

				FontMan.assignFontToName(fontName, font);
				macfont->setFont(font, false);
				_fontRegistry.setVal(fontName, macfont);

				debug(2, " %s", fontName.c_str());
			}

			delete fond;

			if (!fontFamilyUsed)
				delete fontFamily;
		}
	}
}

const Font *MacFontManager::getFont(MacFont macFont) {
	Common::String name;
	const Font *font = 0;

	if (!_builtInFonts) {
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

void MacFontManager::registerFontMapping(uint16 id, Common::String name) {
	_extraFontNames[id] = name;
	_extraFontIds[name] = id;
}

void MacFontManager::clearFontMapping() {
	_extraFontNames.clear();
	_extraFontIds.clear();
}

void MacFont::setName(const char *name) {
	_name = name;
}

const Common::String MacFontManager::getFontName(int id, int size, int slant, bool tryGen) {
	Common::String n;

	if (id == 3) // This is Geneva
		id = 1;

	int extraSlant = 0;

	if (_extraFontNames.contains(id)) {
		n = cleanFontName(_extraFontNames[id]);
		extraSlant = parseFontSlant(_extraFontNames[id]);
	} else if (id < ARRAYSIZE(fontNames)) {
		if (fontNames[id])
			n = fontNames[id];
	}

	if (n.empty()) {
		warning("MacFontManager: Requested font ID %d not found. Falling back to Geneva", id);
		n = fontNames[1]; // Fallback to Geneva
	}

	return Common::String::format("%s-%d-%d", n.c_str(), slant | extraSlant, size);
}

const Common::String MacFontManager::getFontName(MacFont &font) {
	return getFontName(font.getId(), font.getSize(), font.getSlant());
}

int MacFontManager::getFontIdByName(Common::String name) {
	if (_extraFontIds.contains(name))
		return _extraFontIds[name];

	for (int f = 0; f < ARRAYSIZE(fontNames); f++)
		if (fontNames[f] != NULL && strcmp(fontNames[f], name.c_str()) == 0)
			return f;
	return 1;
}

void MacFontManager::generateFontSubstitute(MacFont &macFont) {
	Common::String name;

	// Try to see if we have regular font
	if (macFont.getSlant() != kMacFontRegular) {
		name = getFontName(macFont.getId(), macFont.getSize(), kMacFontRegular);

		if (_fontRegistry.contains(name) && !_fontRegistry[name]->isGenerated()) {
			generateFONTFont(macFont, *_fontRegistry[name]);

			return;
		}
	}


#ifdef USE_FREETYPE2
	// Checking if it's a TTF font. Restrict it only to regular fonts now
	if (_ttfData.contains(macFont.getId()) && macFont.getSlant() == kMacFontRegular) {
		generateTTFFont(macFont, _ttfData[macFont.getId()]);
		return;
	}
#endif

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

	bool bold = false, italic = false, outline = false;

	if (fromFont.getSlant() == kMacFontRegular) {
		bold = toFont.getSlant() & kMacFontBold;
		italic = toFont.getSlant() & kMacFontItalic;
		outline = toFont.getSlant() & kMacFontOutline;
	}

	MacFONTFont *fromFONTFont = static_cast<MacFONTFont *>(fromFont.getFont());
	MacFONTFont *font = Graphics::MacFONTFont::scaleFont(fromFONTFont, toFont.getSize(), bold, italic, outline);

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
