/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "PanelKeyboard.h"

namespace CEGUI {

	const char KEYBOARD_MAPPING_ALPHA_HIGH[] = {"abcdefghijklm"};
	const char KEYBOARD_MAPPING_NUMERIC_HIGH[] = {"12345"};
	const char KEYBOARD_MAPPING_ALPHA_LOW[] = {"nopqrstuvwxyz"};
	const char KEYBOARD_MAPPING_NUMERIC_LOW[] = {"67890"};

	PanelKeyboard::PanelKeyboard(WORD reference) : Toolbar() {
		setBackground(reference);
	} 

	
	PanelKeyboard::~PanelKeyboard() {
	}

	bool PanelKeyboard::action(int x, int y, bool pushed) {

		if (checkInside(x, y)) {
			char keyAscii = 0;
			char keyCode = 0;
			if (x < 185) {
				// Alpha selection
				if (y <= _y + 20) 
					keyAscii = KEYBOARD_MAPPING_ALPHA_HIGH[((x + 10) / 14) - 1];
				else
					keyAscii = KEYBOARD_MAPPING_ALPHA_LOW[((x + 10) / 14) - 1];
				keyCode = tolower(keyAscii);
			}
			else
			if (x >= 186 && x <= 255) {
				// Numeric selection
				if (y <= _y + 20)
					keyAscii = KEYBOARD_MAPPING_NUMERIC_HIGH[((x - 187 + 10) / 14) - 1];
				else
					keyAscii = KEYBOARD_MAPPING_NUMERIC_LOW[((x - 187 + 10) / 14) - 1];
				keyCode = keyAscii;
			}
			else
			if (x >= 302 && x <= 316 && y < _y + 20) {
				// Backspace
				keyAscii = VK_BACK;
				keyCode = keyAscii;
			}
			else
			if (x >= 302 && x <= 316 && y >= _y + 20) {
				// Enter
				keyAscii = 13;
				keyCode = 10;
			}

			if (keyAscii != 0) {
				_key.setAscii(keyAscii);
				_key.setKeycode(tolower(keyAscii));
				return EventsBuffer::simulateKey(&_key, pushed);
			}
			else
				return false;
		}
		else
			return false;
	}
}

