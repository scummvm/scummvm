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
#include "common/events.h"

#include "agi/preagi.h"
#include "agi/font.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"

#include "agi/preagi_common.h"

namespace Agi {

// Screen functions
void PreAgiEngine::clearScreen(int attr) {
	_defaultColor = attr;

	_gfx->clearScreen((attr & 0xF0) / 0x10); 
}

void PreAgiEngine::clearGfxScreen(int attr) {
	_gfx->drawRectangle(0, 0, GFX_WIDTH - 1, IDI_MAX_ROW_PIC * 8 -1, (attr & 0xF0) / 0x10);
}

// String functions

void PreAgiEngine::drawStr(int row, int col, int attr, const char *buffer) {
	int code;

	if (attr == kColorDefault)
		attr = _defaultColor;

	for (int iChar = 0; iChar < (int)strlen(buffer); iChar++) {
		code = buffer[iChar];

		switch (code) {
		case '\n':
		case 0x8D:
			if (++row == 200 / 8) return;
			col = 0;
			break;

		case '|':
			// swap attribute nibbles
			break;

		default:
			_gfx->putTextCharacter(1, col * 8 , row * 8, static_cast<char>(code), attr & 0x0f, (attr & 0xf0) / 0x10, false, getGameID() == GID_MICKEY ? mickey_fontdata : ibm_fontdata);

			if (++col == 320 / 8) {
				col = 0;
				if (++row == 200 / 8) return;
			}
		}
	}
}

void PreAgiEngine::drawStrMiddle(int row, int attr, const char *buffer) {
	int col = (25 / 2) - (strlen(buffer) / 2);	// 25 = 320 / 8 (maximum column)
	drawStr(row, col, attr, buffer);
}

void PreAgiEngine::clearTextArea() {
	for (int row = IDI_MAX_ROW_PIC; row < 200 / 8; row++) {
		clearRow(row);		
	}
}

void PreAgiEngine::clearRow(int row) {
	drawStr(row, 0, IDA_DEFAULT, "                                        ");	// 40 spaces
}

void PreAgiEngine::printStr(const char* szMsg) {
	clearTextArea();
	drawStr(21, 0, IDA_DEFAULT, szMsg);
	_gfx->doUpdate();
	_system->updateScreen();
}

void PreAgiEngine::XOR80(char *buffer) {
	for (size_t i = 0; i < strlen(buffer); i++)
		if (buffer[i] & 0x80)
			buffer[i] ^= 0x80;
}

void PreAgiEngine::printStrXOR(char *szMsg) {
	XOR80(szMsg);
	printStr(szMsg);
}

// Input functions

int PreAgiEngine::getSelection(SelectionTypes type) {
	Common::Event event;

	// Selection types:
	// 0: Y/N
	// 1: 1-9
	for (;;) {
		while (_eventMan->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_system->quit();
			case Common::EVENT_LBUTTONUP:
				if (type == 0)
					return 1;
			case Common::EVENT_RBUTTONUP:
				return 0;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_y:
					if (type == kSelYesNo)
						return 1;
				case Common::KEYCODE_n:
					if (type == kSelYesNo)
						return 0;
				case Common::KEYCODE_ESCAPE:
					if (type == kSelNumber)
						return 0;
				case Common::KEYCODE_1:
				case Common::KEYCODE_2:
				case Common::KEYCODE_3:
				case Common::KEYCODE_4:
				case Common::KEYCODE_5:
				case Common::KEYCODE_6:
				case Common::KEYCODE_7:
				case Common::KEYCODE_8:
				case Common::KEYCODE_9:
					if (type == kSelNumber)
						return event.kbd.keycode - Common::KEYCODE_1 + 1;
				case Common::KEYCODE_SPACE:
					if (type == kSelSpace)
						return 1;
				default:
					if (type == kSelYesNo) {
						return 2;
					} else {
						return 10;
					}
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(10);
	}
	return 0;
}

bool PreAgiEngine::waitAnyKeyChoice() {
	Common::Event event;

	for (;;) {
		while (_eventMan->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_system->quit();
			case Common::EVENT_LBUTTONUP:
				return true;
			case Common::EVENT_RBUTTONUP:
				return false;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE: //Escape
					return false;
				default:
					return true;
				}
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

}
