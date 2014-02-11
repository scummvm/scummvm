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
*/

/*
* This code is based on the original source code of Lord Avalot d'Argent version 1.3.
* Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
*/

/* Original name: HELPER	The help system unit. */

#include "avalanche/avalanche.h"
#include "avalanche/help.h"

namespace Avalanche {

Help::Help(AvalancheEngine *vm) {
	_vm = vm;

	_highlightWas = 0;
}

void Help::getMe(byte which) {
	// Help icons are 80x20.
	
	_highlightWas = 177; // Forget where the highlight was.

	Common::File file;

	if (!file.open("help.avd"))
		error("AVALANCHE: Help: File not found: help.avd");

	file.seek(which * 2);
	uint16 offset = file.readUint16LE();
	file.seek(offset);

	Common::String title = getLine(file);

	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlue);
	_vm->_graphics->drawFilledRectangle(Common::Rect(8, 40, 450, 200), kColorWhite);

	byte index = file.readByte();
	_vm->_graphics->helpDrawButton(-177, index);

	// Plot the title:
	_vm->_graphics->drawNormalText(title, _vm->_font, 8, 629 - 8 * title.size(), 26, kColorBlack);
	_vm->_graphics->drawNormalText(title, _vm->_font, 8, 630 - 8 * title.size(), 25, kColorCyan);

	_vm->_graphics->drawBigText("help!", _vm->_font, 8, 549, 1, kColorBlack);
	_vm->_graphics->drawBigText("help!", _vm->_font, 8, 550, 0, kColorCyan);
	
	byte y = 0;
	do {
		Common::String line = getLine(file);
		if (!line.empty()) {
			if (line.compareTo(Common::String('!')) == 0)  // End of the help text is signalled with a '!'.
				break;
			if (line[0] == '\\') {
				line.deleteChar(0);
				_vm->_graphics->drawNormalText(line, _vm->_font, 8, 16, 41 + y * 10, kColorRed);
			}
			else
				_vm->_graphics->drawNormalText(line, _vm->_font, 8, 16, 41 + y * 10, kColorBlack);
		}
		y++;
	} while (true);

	warning("STUB: Help::getMe()");

	_vm->_graphics->refreshScreen();

	file.close();
}

Common::String Help::getLine(Common::File &file) {
	Common::String line;
	byte length = file.readByte();
	for (int i = 0; i < length; i++) {
		char c = file.readByte();
		line += (c ^ 177);
	}
	return line;
}

byte Help::checkMouse() {
	warning("STUB: Help::checkMouse()");
	return 0;
}

void Help::continueHelp() {
	warning("STUB: Help::continueHelp()");
}

/**
 * @remarks Originally called 'boot_help'
 */
void Help::run() {
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	_vm->_graphics->drawFilledRectangle(Common::Rect(0, 0, 640, 200), kColorBlack); // Black out the whole screen.
	_vm->fadeIn();

	_vm->_graphics->loadMouse(kCurHelp);
	
	getMe(0);

	continueHelp();

	_vm->fadeOut();
	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
	_vm->fadeIn();
}

} // End of namespace Avalanche
