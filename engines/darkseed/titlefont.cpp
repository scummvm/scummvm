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

#include "titlefont.h"
#include "darkseed.h"
#include "graphics/screen.h"

namespace Darkseed {
extern DarkseedEngine *g_engine;

TitleFont::TitleFont() {
	letters.load("art/letters.anm");
}

int letterIndexLookupTbl[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							  0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04,
							  0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12,
							  0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x00,
							  0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c,
							  0x2e, 0x30 };

int16 letterWidthLookupTbl[] = {
 0x12,  0x12,    0x12,    0x12,
 0x12,  0x12,    0x12,    0x12,
 0xA,  0xC,    0x10,    0x12,
 0x14,  0x14,    0x12,    0x12,
 0x12,  0x12,    0x12,    0x12,
 0x12,  0x14,    0x14,    0x12,
 0x14,  0x12,    0xA,    0x0,
 0x0,  -1,    0x0,    0x0,
 0x0,  0x0,    0x0,    0x0,
 0x0,  0x0,    0x0,    0x0,
 0x0,  0x0,    0x0,    0x0,
 0x0,  0x0,    0x0,    0x0,
 0x0,  0x0,    0x0,    0x0,
 0x0,  0x0
};

void TitleFont::displayString(uint16 x, uint16 y, const Common::String &text) {
	for (int i = 0; i < text.size(); i++) {
		if (text[i] == ' ') {
			x += 0x12;
			continue;
		}
		Img letterShadow;
		Img letter;
		int letterId = letterIndexLookupTbl[text[i] - 0x2f];
		letters.getImg(letterId, letterShadow);
		letters.getImg(letterId+1, letter);

		g_engine->_screen->copyRectToSurfaceWithKey(letterShadow.getPixels().data(), letterShadow.getWidth(), x, y, letterShadow.getWidth(), letterShadow.getHeight(), 0xf);
		g_engine->_screen->copyRectToSurfaceWithKey(letter.getPixels().data(), letter.getWidth(), x + 1, y, letter.getWidth(), letter.getHeight(), 0);
		debug("%c %d %d", text[i], letterWidthLookupTbl[text[i] - 0x41], letter.getWidth());
		x += letterWidthLookupTbl[text[i] - 0x41]; //letter.getWidth();
	}
}

} // namespace Darkseed