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
#include "graphics/fonts/bdf.h"

#include "graphics/macgui/macfontmanager.h"

namespace Graphics {

MacFontManager::MacFontManager() {
	loadFonts();
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

		Graphics::BdfFont *font = Graphics::BdfFont::loadFont(*stream);

		delete stream;

		Common::String fontName;
		if (font->getFamilyName() && *font->getFamilyName()) {
			fontName = Common::String::format("%s-%d", font->getFamilyName(), font->getFontSize());
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
		}

		FontMan.assignFontToName(fontName, font);

		debug(2, " %s", fontName.c_str());
	}

	_builtInFonts = false;

	delete dat;
}

const Font *MacFontManager::getFont(MacFont macFont) {
	const Font *font = 0;

	if (!_builtInFonts) {
		if (macFont.getName().empty())
			macFont.setName(getFontName(macFont.getId(), macFont.getSize()));

		font = FontMan.getFontByName(macFont.getName());

		if (!font) {
			warning("Cannot load font %s", macFont.getName().c_str());

			font = FontMan.getFontByName(MacFont(kMacFontChicago, 12).getName());
		}
	}

	if (_builtInFonts || !font)
		font = FontMan.getFontByUsage(macFont.getFallback());

	return font;
}

// Source: Apple IIGS Technical Note #41, "Font Family Numbers"
// http://apple2.boldt.ca/?page=til/tn.iigs.041
static const char *const fontNames[] = {
	"Chicago",	// system font
	"Geneva",	// application font
	"New York",
	"Geneva",

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

const char *MacFontManager::getFontName(int id, int size) {
	static char name[128];

	if (id > ARRAYSIZE(fontNames))
		return NULL;

	snprintf(name, 128, "%s-%d", fontNames[id], size);

	return name;
}

} // End of namespace Graphics
