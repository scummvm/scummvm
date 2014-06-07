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
 *
 */

/*
* This code is based on the original source code of Lord Avalot d'Argent version 1.3.
* Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
*/

/* Original name: HELPER	The help system unit. */

#include "avalanche/avalanche.h"
#include "avalanche/help.h"

#include "common/scummsys.h"

namespace Avalanche {

Help::Help(AvalancheEngine *vm) {
	_vm = vm;

	for (int i = 0; i < 10; i++) {
		_buttons[i]._trigger = Common::KEYCODE_INVALID;
		_buttons[i]._whither = 0;
	}
	_highlightWas = 0;
	_buttonNum = 0;
	_holdLeft = false;
}

/**
 * Loads and draws the chosen page of the help.
 * @remarks Originally called 'getme'
 */
void Help::switchPage(byte which) {
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

	_vm->_graphics->helpDrawBigText("help!", 549, 1, kColorBlack);
	_vm->_graphics->helpDrawBigText("help!", 550, 0, kColorCyan);

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

	// We are now at the end of the text. Next we must read the icons:
	y = 0;
	_buttonNum = 0;
	while (!file.eos()) {
		int trigger = file.readByte();

		if (trigger == 177)
			break;
		switch (trigger) {
		case 254: // Escape
			trigger = 27;
			break;
		case 214: // PageUp
			trigger = 280;
			break;
		case 216: // PageDown
			trigger = 281;
			break;
		default: // A - Z
			// The characters are stored in the file in uppercase, but we need the lowercase versions for KeyCode:
			trigger = tolower(trigger);
			break;
		}

		_buttons[y]._trigger = Common::KeyCode(trigger);
		index = file.readByte();
		if (_buttons[y]._trigger != Common::KEYCODE_INVALID)
			_vm->_graphics->helpDrawButton(13 + (y + 1) * 27, index);
		_buttons[y]._whither = file.readByte(); // This is the position to jump to.

		Common::String text = "";
		switch (_buttons[y]._trigger) {
		case Common::KEYCODE_ESCAPE:
			text = Common::String("Esc");
			break;
		case Common::KEYCODE_PAGEUP:
			text = Common::String(24);
			break;
		case Common::KEYCODE_PAGEDOWN:
			text = Common::String(25);
			break;
		default:
			text = Common::String(toupper(_buttons[y]._trigger));
			break;
		}

		_vm->_graphics->helpDrawBigText(text, 589 - (text.size() * 8), 18 + (y + 1) * 27, kColorBlack);
		_vm->_graphics->helpDrawBigText(text, 590 - (text.size() * 8), 17 + (y + 1) * 27, kColorCyan);

		y++;
		_buttonNum++;
	}

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

bool Help::handleMouse(const Common::Event &event) {
	Common::Point mousePos;
	mousePos.x = event.mouse.x;
	mousePos.y = event.mouse.y / 2;

	int index = -1;

	if (event.type == Common::EVENT_LBUTTONUP) { // Clicked *somewhere*...
		_holdLeft = false;

		if ((mousePos.x < 470) || (mousePos.x > 550) || (((mousePos.y - 13) % 27) > 20))
			index = -1;
		else // Clicked on a button.
			index = ((mousePos.y - 13) / 27) - 1;
	} else { // LBUTTONDOWN or MOUSEMOVE
		int highlightIs = 0;

		// Decide which button we are hovering the cursor over:
		if ((mousePos.x > 470) && (mousePos.x <= 550) && (((mousePos.y - 13) % 27) <= 20)) { // No click, so highlight.
			highlightIs = (mousePos.y - 13) / 27 - 1;
			if ((highlightIs < 0) || (5 < highlightIs))
				highlightIs = 177; // In case of silly values.
		} else
			highlightIs = 177;

		Color highlightColor = kColorLightblue;
		// If we clicked on a button or we are holding down the button, we have to highlight it with cyan:
		if (((highlightIs != 177) && (event.type == Common::EVENT_LBUTTONDOWN)) || _holdLeft) {
			_holdLeft = true;
			highlightColor = kColorLightcyan;
		}

		// Erase the previous highlight only if it's needed:
		if (_highlightWas != highlightIs)
			_vm->_graphics->helpDrawHighlight(_highlightWas, kColorBlue);

		// Highlight the current one with the proper color:
		if ((highlightIs != 177) && (_buttons[highlightIs]._trigger != Common::KEYCODE_INVALID)) {
			_highlightWas = highlightIs;
			_vm->_graphics->helpDrawHighlight(highlightIs, highlightColor);
		}
	}

	if ((index >= 0) && (_buttons[index]._trigger != Common::KEYCODE_INVALID)) {
		if (_buttons[index]._trigger == Common::KEYCODE_ESCAPE)
			return true;
		else {
			_vm->fadeOut();
			switchPage(_buttons[index]._whither);
			_vm->fadeIn();
			return false;
		}
	}

	return false;
}

bool Help::handleKeyboard(const Common::Event &event) {
	if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
		return true;

	for (int i = 0; i < _buttonNum; i++) {
		if (_buttons[i]._trigger == event.kbd.keycode) {
			_vm->fadeOut();
			switchPage(_buttons[i]._whither);
			_vm->fadeIn();
			return false;
		}
	}

	return false;
}

/**
 * @remarks Originally called 'boot_help'
 */
void Help::run() {
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	switchPage(0);
	_vm->fadeIn();

	_vm->_graphics->loadMouse(kCurHelp);

	// Originally it was the body of 'continue_help':
	bool close = false;
	while (!_vm->shouldQuit() && !close) {
		Common::Event event;
		_vm->getEvent(event);
		if (event.type == Common::EVENT_KEYDOWN)
			close = handleKeyboard(event);
		else if ((event.type == Common::EVENT_LBUTTONDOWN) || (event.type == Common::EVENT_LBUTTONUP) || (event.type == Common::EVENT_MOUSEMOVE))
			close = handleMouse(event);

		_vm->_graphics->refreshScreen();
	}
	// End of 'continue_help'.

	_vm->fadeOut();
	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
	_vm->fadeIn();
}

} // End of namespace Avalanche
