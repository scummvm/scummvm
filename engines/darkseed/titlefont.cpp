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

#include "graphics/screen.h"
#include "darkseed/titlefont.h"
#include "darkseed/darkseed.h"

namespace Darkseed {
extern DarkseedEngine *g_engine;

TitleFont::TitleFont() {
	_letters.load("art/letters.anm");
}

uint8 letterIndexLookupTbl[] = {
	50, 52, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 2, 4, 6,
	8, 10, 12, 14,
	16, 18, 20, 22,
	24, 26, 28, 30,
	0, 32, 34, 36,
	38, 40, 42, 44,
	46, 48
};

int16 letterWidthLookupTbl[] = {
	18, 18, 18, 18,
	18, 18, 18, 18,
	10, 12, 16, 18,
	20, 20, 18, 18,
	18, 18, 18, 18,
	18, 20, 20, 18,
	20, 18, 10
};

void TitleFont::displayString(uint16 x, uint16 y, const Common::String &text) {
	for (unsigned int i = 0; i < text.size(); i++) {
		if (text[i] == ' ') {
			x += 0x12;
			continue;
		}
		Img letterShadow;
		Img letter;
		int letterId = letterIndexLookupTbl[text[i] - 45];
		_letters.getImg(letterId, letterShadow, false);
		_letters.getImg(letterId + 1, letter, false);

		int w = letterWidthLookupTbl[letterId / 2];
		letterShadow.drawAt(x, y, 2, w - 1); // TODO the original doesn't seem to need to override the width here.
		letter.drawAt(x, y + 1, 3);
		debug("%c %d %d %d", text[i], w, letter.getWidth(), letterShadow.getWidth());
		x += w;
	}
}

} // namespace Darkseed
