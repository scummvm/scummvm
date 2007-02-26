/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "PanelKeyboard.h"

namespace CEGUI {

	const char KEYBOARD_MAPPING_ALPHA[][14] = { {"abcdefghijklm"}, {"nopqrstuvwxyz"} };
	const char KEYBOARD_MAPPING_NUMERIC[][6] = { {"12345"}, {"67890"} };
	const int  KEYBOARD_MAPPING_SPECIAL[][3][2] = { { {1,SDLK_ESCAPE}, {224,SDLK_UP}, {32,SDLK_SPACE} },
						      { {224,SDLK_LEFT}, {224,SDLK_DOWN}, {224,SDLK_RIGHT} } };

	PanelKeyboard::PanelKeyboard(WORD reference) : Toolbar() {
		setBackground(reference);
	}


	PanelKeyboard::~PanelKeyboard() {
	}

	bool PanelKeyboard::action(int x, int y, bool pushed) {

		if (checkInside(x, y)) {
			int keyAscii = 0;
			int keyCode = 0;
			if (x < 185) {
				// Alpha selection
				keyCode = keyAscii = KEYBOARD_MAPPING_ALPHA[y >= _y+20][((x + 10) / 14) - 1];
			} else if (x >= 186 && x <= 255) {
				// Numeric selection
				keyCode = keyAscii = KEYBOARD_MAPPING_NUMERIC[y >= _y+20][((x - 187 + 10) / 14) - 1];
			} else if (x >= 258 && x <= 300) {
				// Special keys
				keyAscii = KEYBOARD_MAPPING_SPECIAL[y >= _y+20][((x - 259 + 10) / 14) - 1][0];
				keyCode = KEYBOARD_MAPPING_SPECIAL[y >= _y+20][((x - 259 + 10) / 14) - 1][1];
			} else if (x >= 302 && x <= 316) {
				if (y < _y +20) {
					// Backspace
					keyAscii = VK_BACK; keyCode = keyAscii;
				} else {
					// Enter
					keyAscii = 13; keyCode = 10;
				}
			}

			if (keyAscii != 0) {
				_key.setAscii(keyAscii);
				_key.setKeycode(tolower(keyCode));
				return EventsBuffer::simulateKey(&_key, pushed);
			}
			else
				return false;
		}
		else
			return false;
	}
}

