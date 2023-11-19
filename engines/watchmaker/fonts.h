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

#ifndef WATCHMAKER_FONTS_H
#define WATCHMAKER_FONTS_H

#include "common/stream.h"
#include "watchmaker/renderer.h"
#include "watchmaker/sysdef.h"

namespace Watchmaker {

struct SFont {
	uint16 *table = nullptr;
	int32 color[MAX_FONT_COLORS] = {};
};

class WGame;
class Fonts {
	Common::Array<uint16 *> _tables;
	uint16 *setupFontTable(Common::SeekableReadStream &stream);
	void loadFont(WGame &game, struct SFont *f, const Common::String &n);
public:
	SFont *fontForKind(FontKind);
	SFont StandardFont;
	SFont ComputerFont;
	SFont PDAFont;
	~Fonts();

	void loadFonts(WGame &game, WindowInfo &windowInfo);
	void getTextDim(const char *s, FontKind font, int *x, int *y);
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_FONTS_H

