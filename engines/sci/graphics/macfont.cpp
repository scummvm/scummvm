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

#include "common/macresman.h"
#include "graphics/fontman.h"
#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "sci/graphics/macfont.h"

namespace Sci {

GfxMacFontManager::GfxMacFontManager(Common::MacResManager *macExecutable) :
	_defaultFont(nullptr) {

	if (macExecutable != nullptr) {
		// Load fonts from mac executable.
		// Use kWMModeForceBuiltinFonts to prevent MacFontManager from attempting
		// to load fonts from classicmacfonts.dat, as we don't use them and it
		// will produce a warning() when it's not present.
		_usesSystemFonts = false;
		uint32 mode = Graphics::MacGUIConstants::kWMModeForceBuiltinFonts;
		_macFontManager = new Graphics::MacFontManager(mode, Common::Language::UNK_LANG);
		_macFontManager->loadFonts(macExecutable);

		// Register each font family that was loaded from the executable so that
		// their Graphics::Font can be retrieved through FontManager::getFontByName().
		const Common::Array<Graphics::MacFontFamily *> &fontFamilies = _macFontManager->getFontFamilies();
		for (uint i = 0; i < fontFamilies.size(); ++i) {
			_macFontManager->registerFontName(fontFamilies[i]->getName(), fontFamilies[i]->getFontFamilyId());
		}

		if (!initFromFontTable(macExecutable)) {
			_macFonts.clear(true);
			_defaultFont = nullptr;
		}
	} else {
		// Load fonts from classicmacfonts.dat. This logs a warning if it can't be found.
		_usesSystemFonts = true;
		_macFontManager = new Graphics::MacFontManager(0, Common::Language::UNK_LANG);

		// Load Palatino. These values were hard-coded in the interpreter in SciToMacFont.
		const Graphics::Font *palatinoSmall = getMacFont(Graphics::kMacFontPalatino, 10);
		const Graphics::Font *palatinoLarge = getMacFont(Graphics::kMacFontPalatino, 18);
		if (palatinoSmall == nullptr || palatinoLarge == nullptr) {
			return;
		}

		// Map all fonts to Palatino.
		_defaultFont = new MacFontItem { palatinoSmall, palatinoLarge };
		_macFonts.setVal(0, _defaultFont);
	}
}

GfxMacFontManager::~GfxMacFontManager() {
	for (Common::HashMap<GuiResourceId, MacFontItem *>::iterator it = _macFonts.begin(); it != _macFonts.end(); ++it) {
		delete it->_value;
	}
	delete _macFontManager;
}

// The font mapping table is a small binary resource with id 128 and type `ftbl`
bool GfxMacFontManager::initFromFontTable(Common::MacResManager *macExecutable) {
	Common::String macExecutableName = macExecutable->getBaseFileName().baseName();
	Common::ScopedPtr<Common::SeekableReadStream> table(
		macExecutable->getResource(MKTAG('f', 't', 'b', 'l'), 128)
	);
	if (!table) {
		warning("Mac font table not found in \"%s\"", macExecutableName.c_str());
		return false;
	}

	// Table header is 4 bytes followed by entries of 10 bytes each
	uint16 defaultFontIndex = table->readUint16BE();
	uint16 numberOfFonts = table->readUint16BE();
	if (table->eos() || table->size() < 4 + numberOfFonts * 10) {
		warning("Invalid mac font table in \"%s\"", macExecutableName.c_str());
		return false;
	}

	for (uint16 i = 0; i < numberOfFonts; ++i) {
		uint16 sciFontId = table->readUint16BE();
		if (_macFonts.contains(sciFontId)) {
			warning("Duplicate Mac font table entry for %d in \"%s\"", sciFontId, macExecutableName.c_str());
			return false;
		}
		uint16 macFontId = table->readUint16BE();
		uint16 smallFontSize = table->readUint16BE();
		uint16 mediumFontSize = table->readUint16BE(); // large in KQ5 (not supported yet)
		uint16 largeFontSize = table->readUint16BE();

		const Graphics::Font *smallFont = getMacFont(macFontId, smallFontSize);
		const Graphics::Font *largeFont = getMacFont(macFontId, MAX(mediumFontSize, largeFontSize));
		if (smallFont == nullptr || largeFont == nullptr) {
			warning("Mac font %d not found in \"%s\"", macFontId, macExecutableName.c_str());
			return false;
		}

		MacFontItem *font = new MacFontItem { smallFont, largeFont };
		_macFonts.setVal(sciFontId, font);

		if (i == defaultFontIndex) {
			_defaultFont = font;
		}
	}

	return true;
}

const Graphics::Font *GfxMacFontManager::getMacFont(int macFontId, int size) {
	// Is this font in MacFontManager? This logs a warning if it isn't.
	if (_macFontManager->getFontName(macFontId).empty()) {
		return nullptr;
	}

	// Build a MacFont to get the full font name for this size and style (regular)
	Graphics::MacFont macFont(macFontId, size, 0);
	Common::String fontName = _macFontManager->getFontName(macFont);

	// Get the font through the regular FontManager through which MacFontManager
	// registered it when loading fonts. MacFontManager::getFont() does lots of
	// extra stuff and fallback handling which we're not interested in.
	// We just want the font if it's there and nullptr if it isn't. Our fallback
	// behavior if we can't get the mac fonts is to use SCI fonts.
	return FontMan.getFontByName(fontName);
}

bool GfxMacFontManager::hasFonts() {
	return _defaultFont != nullptr;
}

bool GfxMacFontManager::usesSystemFonts() {
	return _usesSystemFonts;
}

const Graphics::Font *GfxMacFontManager::getSmallFont(GuiResourceId sciFontId) {
	MacFontItem *item = _macFonts.getValOrDefault(sciFontId, _defaultFont);
	return item->smallFont;
}

const Graphics::Font *GfxMacFontManager::getLargeFont(GuiResourceId sciFontId) {
	MacFontItem *item = _macFonts.getValOrDefault(sciFontId, _defaultFont);
	return item->largeFont;
}

} // End of namespace Sci
