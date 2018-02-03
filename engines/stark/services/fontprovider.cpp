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
#include "engines/stark/gfx/driver.h"

#include "common/archive.h"

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
	_ttfFileMap["Arial"] = "ARIAL.TTF";
	_ttfFileMap["Bradley Hand ITC"] = "bradhitc.ttf";
	_ttfFileMap["Slurry"] = "SLURRY.TTF";

	// TODO: Read from gui.ini
	_smallFont = FontHolder(this, "Garamond", 12);
	_bigFont = FontHolder(this, "Florentine Script", 19);

	_customFonts[0] = FontHolder(this, "Folkard", 20);
	_customFonts[1] = FontHolder(this, "Folkard", 12);
	_customFonts[2] = FontHolder(this, "Arial", 14);
	_customFonts[3] = FontHolder(this, "Bradley Hand ITC", 16);
	_customFonts[4] = FontHolder(this, "Bradley Hand ITC", 20);
	_customFonts[5] = FontHolder(this, "Bradley Hand ITC", 16);
	_customFonts[6] = FontHolder(this, "Bradley Hand ITC", 15);
	_customFonts[7] = FontHolder(this, "Florentine Script", 13);
}

FontProvider::FontHolder::FontHolder(FontProvider *fontProvider, Common::String name, uint32 height, uint32 charset) {
	_name = name;
	_originalHeight = height;
	_scaledHeight = StarkGfx->scaleHeightOriginalToCurrent(_originalHeight);
	_charset = charset;

	// Fetch the font file name
	Common::String ttfFileName = "fonts/" + fontProvider->_ttfFileMap[_name];

	// Initialize the font
	Common::SeekableReadStream *s = SearchMan.createReadStreamForMember(ttfFileName);
	if (s) {
		_font = Common::SharedPtr<Graphics::Font>(
				Graphics::loadTTFFont(*s, _scaledHeight, Graphics::kTTFSizeModeCell, 0, Graphics::kTTFRenderModeMonochrome)
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
