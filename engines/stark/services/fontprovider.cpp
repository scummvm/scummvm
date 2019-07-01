/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/services/fontprovider.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/gfx/driver.h"

#include "common/archive.h"
#include "common/ini-file.h"

#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"

namespace Stark {

FontProvider::FontProvider() {
}

FontProvider::~FontProvider() {
}

void FontProvider::initFonts() {
	// TODO: Use SystemFontMan instead when it gets merged
	_ttfFileMap["Garamond"] = "Gara.ttf";
	_ttfFileMap["Florentine Script"] = "flornt.TTF";
	_ttfFileMap["Folkard"] = "folkard.ttf";
	_ttfFileMap["Folkard\231"] = "folkard.ttf";
	_ttfFileMap["Arial"] = "ARIAL.TTF";
	_ttfFileMap["Bradley Hand ITC"] = "bradhitc.ttf";
	_ttfFileMap["Slurry"] = "SLURRY.TTF";
	_ttfFileMap["President Cyr"] = "President Cyr Regular.Ttf";
	_ttfFileMap["VictorianCyr"] = "Victorian Cyr.ttf";
	_ttfFileMap["Zapf Chance Italic"] = "Zapf Chance Italic.Ttf";
	_ttfFileMap["Arial_tlj"] = "arial_tlj.ttf";

	// Clear any previously used fonts
	_smallFont = FontHolder();
	_bigFont   = FontHolder();
	for (uint i = 0; i < ARRAYSIZE(_customFonts); i++) {
		_customFonts[i] = FontHolder();
	}

	// Load the font settings from gui.ini when possible
	if (!StarkSettings->shouldIgnoreFontSettings()) {
		Common::INIFile gui;
		if (gui.loadFromFile("gui.ini")) {
			readFontEntry(&gui, _smallFont,        "smallfont", "smallheight");
			readFontEntry(&gui, _bigFont,          "bigfont",   "bigheight");
			readFontEntry(&gui, _customFonts[0],   "font0",     "fontsize0");
			readFontEntry(&gui, _customFonts[1],   "font1",     "fontsize1");
			readFontEntry(&gui, _customFonts[2],   "font2",     "fontsize2");
			readFontEntry(&gui, _customFonts[3],   "font3",     "fontsize3");
			readFontEntry(&gui, _customFonts[4],   "font4",     "fontsize4");
			readFontEntry(&gui, _customFonts[5],   "font5",     "fontsize5");
			readFontEntry(&gui, _customFonts[6],   "font6",     "fontsize6");
			readFontEntry(&gui, _customFonts[7],   "font7",     "fontsize7");
		} else {
			warning("Unable to open 'gui.ini' to read the font settings");
		}
	}

	// Default fonts
	if (!_smallFont._font)      _smallFont      = FontHolder(this, "Garamond", 12);
	if (!_bigFont._font)        _bigFont        = FontHolder(this, "Florentine Script", 19);
	if (!_customFonts[0]._font) _customFonts[0] = FontHolder(this, "Folkard", 20);
	if (!_customFonts[1]._font) _customFonts[1] = FontHolder(this, "Folkard", 12);
	if (!_customFonts[2]._font) _customFonts[2] = FontHolder(this, "Arial", 14);
	if (!_customFonts[3]._font) _customFonts[3] = FontHolder(this, "Bradley Hand ITC", 16);
	if (!_customFonts[4]._font) _customFonts[4] = FontHolder(this, "Bradley Hand ITC", 20);
	if (!_customFonts[5]._font) _customFonts[5] = FontHolder(this, "Bradley Hand ITC", 16);
	if (!_customFonts[6]._font) _customFonts[6] = FontHolder(this, "Bradley Hand ITC", 15);
	if (!_customFonts[7]._font) _customFonts[7] = FontHolder(this, "Florentine Script", 13);
}

void FontProvider::readFontEntry(const Common::INIFile *gui, FontHolder &holder, const char *nameKey, const char *sizeKey) {
	Common::String section = "TEXT95";
	if (gui->hasSection("Western")) {
		section = "Western";
	}

	Common::String name, sizeStr;
	bool gotName = gui->getKey(nameKey, section, name);
	bool gotSize = gui->getKey(sizeKey, section, sizeStr);

	long size = strtol(sizeStr.c_str(), nullptr, 10);

	// WORKAROUND: In the GOG.com release the computer font (location 36 00)
	// is too small, preventing the "White Cardinal" label from being clickable.
	if (strcmp(nameKey, "font2") == 0 && name.equalsIgnoreCase("Arial") && size < 14) {
		size = 14;
	}

	if (gotName && gotSize && size > 0) {
		holder = FontHolder(this, name, size);
	} else {
		warning("Unable to read font entry '%s' from 'gui.ini'", nameKey);
	}
}

FontProvider::FontHolder::FontHolder(FontProvider *fontProvider, const Common::String &name, uint32 height) {
	_name = name;
	_originalHeight = height;
	_scaledHeight = StarkGfx->scaleHeightOriginalToCurrent(_originalHeight);

	// Fetch the font file name
	Common::String ttfFileName = "fonts/" + fontProvider->_ttfFileMap[_name];

	// Initialize the font
	Common::SeekableReadStream *s = SearchMan.createReadStreamForMember(ttfFileName);
	if (s) {
		Graphics::TTFRenderMode renderMode = StarkSettings->isFontAntialiasingEnabled() ?
		            Graphics::kTTFRenderModeLight : Graphics::kTTFRenderModeMonochrome;
		bool stemDarkening = StarkSettings->isFontAntialiasingEnabled();

		_font = Common::SharedPtr<Graphics::Font>(
				Graphics::loadTTFFont(*s, _scaledHeight, Graphics::kTTFSizeModeCell, 0, renderMode, nullptr, stemDarkening)
		);
		delete s;
	} else {
		warning("Unable to load the font '%s'", ttfFileName.c_str());
	}
}

FontProvider::FontHolder *FontProvider::getFontHolder(FontProvider::FontType type, int32 customFontIndex) {
	if (type == kSmallFont) {
		return &_smallFont;
	} else if (type == kBigFont) {
		return &_bigFont;
	} else {
		assert(customFontIndex >= 0 && customFontIndex < 8);
		return &_customFonts[customFontIndex];
	}
}

const Graphics::Font *FontProvider::getScaledFont(FontProvider::FontType type, int32 customFontIndex) {
	FontHolder *holder = getFontHolder(type, customFontIndex);
	if (holder->_font) {
		return holder->_font.get();
	} else {
		// Fallback to a default font
		return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	}
}

uint FontProvider::getScaledFontHeight(FontProvider::FontType type, int32 customFontIndex) {
	FontHolder *holder = getFontHolder(type, customFontIndex);
	return holder->_scaledHeight;
}

uint FontProvider::getOriginalFontHeight(FontProvider::FontType type, int32 customFontIndex) {
	FontHolder *holder = getFontHolder(type, customFontIndex);
	return holder->_originalHeight;
}

} // End of namespace Stark
