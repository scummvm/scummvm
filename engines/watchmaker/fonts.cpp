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

#include "watchmaker/fonts.h"
#include "watchmaker/main.h"
#include "watchmaker/game.h"

namespace Watchmaker {

Fonts::~Fonts() {
	for (auto table : _tables) {
		delete[] table;
	}
}

uint16 *Fonts::setupFontTable(Common::SeekableReadStream &stream) {
	uint32 dim = stream.size();
	uint16 *tab = new uint16[dim] {};

	for (int i = 0; i < dim / sizeof(uint16); i++) {
		tab[i] = stream.readUint16LE();
	}

	_tables.push_back(tab);

	return tab;
}

void Fonts::loadFont(WGame &game, struct SFont *f, const Common::String &n) {
	Common::String name = constructPath(game.workDirs._miscDir, n, "fnt");
	auto stream = game.workDirs.resolveFile(name);
	if ((f->table = setupFontTable(*stream)) == nullptr) {
		DebugLogFile("Failed to load Font. Quitting ...");
		CloseSys(game);
	}

	Common::String basename = n.substr(0, n.findLastOf('.'));
	for (char a = 0; a < MAX_FONT_COLORS; a++) {
		Common::String tgaName = constructPath(game.workDirs._miscDir, basename + (a + '0')) + ".tga";
		if ((f->color[a] = rLoadBitmapImage(game, tgaName.c_str(), rBITMAPSURFACE | rSURFACEFLIP)) <= 0) {
			DebugLogFile("Failed to load Font2. Quitting ...");
			CloseSys(game);
		}
	}
}

void Fonts::loadFonts(WGame &game, WindowInfo &windowInfo) {
	if ((windowInfo.width >= 1024) || (windowInfo.height >= 768)) {
		loadFont(game, &StandardFont, "1024NlFont.fnt");
		loadFont(game, &ComputerFont, "1024ComputerFont.fnt");
		loadFont(game, &PDAFont,      "1024PDAFont.fnt");
	} else if ((windowInfo.width >= 800) || (windowInfo.height >= 600)) {
		loadFont(game, &StandardFont, "800NlFont.fnt");
		loadFont(game, &ComputerFont, "800ComputerFont.fnt");
		loadFont(game, &PDAFont,      "800PDAFont.fnt");
	} else if ((windowInfo.width >= 640) || (windowInfo.height >= 480)) {
		loadFont(game, &StandardFont, "640NlFont.fnt");
		loadFont(game, &ComputerFont, "640ComputerFont.fnt");
		loadFont(game, &PDAFont,      "640PDAFont.fnt");
	}
}

SFont *Fonts::fontForKind(FontKind font) {
	switch (font) {
	case FontKind::Standard:
		return &StandardFont;
	case FontKind::Computer:
		return &ComputerFont;
	case FontKind::PDA:
		return &PDAFont;
	}
}

void Fonts::getTextDim(const char *s, FontKind font, int *x, int *y) {
	if (!s)
		return;

	SFont *fontTable = fontForKind(font);

	int i = 0;
	int nextx = 0, nexty = 0;
	byte c = 0;
	while ((c = s[i]) != 0) {
		i++;

		nextx += fontTable->table[c * 4 + 2];
		nexty = fontTable->table[c * 4 + 3];
	}

	*x = nextx;
	*y = nexty;
}

} // End of namespace Watchmaker
