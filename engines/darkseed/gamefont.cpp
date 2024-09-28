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

#include "darkseed/darkseed.h"
#include "darkseed/gamefont.h"

namespace Darkseed {
extern DarkseedEngine *g_engine;

Darkseed::GameFont::GameFont() {
}

bool GameFont::load() {
	return _letters.load("tosfont.nsp");
}

void GameFont::displayString(uint16 x, uint16 y, const Common::String &text) {
	for (uint i = 0; i < text.size(); i++) {
		auto letter = getCharacterSprite(text[i]);
		if (letter) {
			letter->draw(x, y);
			x += letter->_width + 1;
		}
	}
}

int GameFont::stringLength(const Common::String &text) {
	int width = 0;
	for (uint i = 0; i < text.size(); i++) {
		const Sprite *sprite = getCharacterSprite(text[i]);
		if (sprite) {
			width += sprite->_width + 1;
		}
	}
	return width;
}

const Sprite *GameFont::getCharacterSprite(char c) {
	int letterIdx = 1000;
	switch (c) {
	case 0x20 :
		letterIdx = 0x46;
		break;
	case 0x21 :
		letterIdx = 0x36;
		break;
	case 0x22 :
		letterIdx = 0x3a;
		break;
	case 0x27 :
		letterIdx = 0x45;
		break;
	case 0x28 :
		letterIdx = 0x37;
		break;
	case 0x29 :
		letterIdx = 0x38;
		break;
	case 0x2b :
		letterIdx = 0xa;
		break;
	case 0x2c :
		letterIdx = 0x34;
		break;
	case 0x2d :
		letterIdx = 0x46;
		break;
	case 0x2e :
		letterIdx = 0x35;
		break;
	case 0x3a :
		letterIdx = 0x47;
		break;
	case 0x3f :
		letterIdx = 0x39;
		break;
	case 0x5e :
		letterIdx = 0x3a;
		break;

	default: {
		if (c < 0x41 || c > 0x5a) {
			if (c < 0x61 || c > 0x7a) {
				if (c > 0x2f && c < 0x3a) {
					letterIdx = c + 0xb;
				}
			} else {
				letterIdx = c - 0x61;
			}
		} else {
			letterIdx = c - 0x27;
		}
		break;
	}
	}

	if (letterIdx != 1000) {
		return &_letters.getSpriteAt(letterIdx);
	}

	return nullptr;
}

} // namespace Darkseed
