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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "avalanche/avalanche.h"
#include "avalanche/parser.h"

namespace Avalanche {

Parser::Parser(AvalancheEngine *vm) {
	_vm = vm;
}

void Parser::init() {
	_leftMargin = 0;
	if (!_inputText.empty())
		_inputText.clear();
	_inputTextPos = 0;
}

void Parser::handleInputText(const Common::Event &event) {
	byte inChar = event.kbd.ascii;
	warning("STUB: Parser::handleInputText()");
//	if (_vm->_dropdown->_activeMenuItem._activeNow) {
//		_vm->_dropdown->parseKey(inChar, _vm->_enhanced->extd);
//	} else {
		if (_inputText.size() < 76) {
			if ((inChar == '"') || (inChar == '`')) {
				if (_quote)
					inChar = '`';
				else
					inChar = '"';
				_quote = !_quote;  // quote - unquote
			}

			_inputText.insertChar(inChar, _inputTextPos);
			_inputTextPos++;
			plotText();
		} else
			_vm->_gyro->blip();
//	}
}

void Parser::handleBackspace() {
	if (!_vm->_dropdown->_activeMenuItem._activeNow) {
		if (_inputTextPos > _leftMargin) {
			_inputTextPos--;
			if ((_inputText[_inputTextPos] == '"') || (_inputText[_inputTextPos] == '`'))
				_quote = !_quote;
			_inputText.deleteChar(_inputTextPos);
			plotText();
		} else
			_vm->_gyro->blip();
	}
}

void Parser::handleReturn() {
	if (_vm->_dropdown->_activeMenuItem._activeNow)
		_vm->_parser->tryDropdown();
	else {
		if (!_inputText.empty()) {
			_inputTextBackup = _inputText;
			_vm->_acci->parse();
			_vm->_acci->doThat();
			_inputText.clear();
			wipeText();
		}
	}
}

void Parser::handleFunctionKey(const Common::Event &event) {
	switch (event.kbd.keycode) {
	case Common::KEYCODE_F5:
		_vm->_acci->_person = _vm->_acci->kPardon;
		_vm->_acci->_thing = _vm->_acci->kPardon;
		_vm->_lucerna->callVerb(_vm->_gyro->f5Does()[0]);
		break;
	case Common::KEYCODE_F7:
		_vm->_lucerna->callVerb(Acci::kVerbCodeOpen);
		break;
	default:
		break;
	}
}

void Parser::plotText() {
	CursorMan.showMouse(false);

	cursorOff();

	_vm->_graphics->_surface.fillRect(Common::Rect(24, 161, 640, 169), kColorBlack); // Black out the line of the text.

	_vm->_graphics->drawText(_vm->_graphics->_surface, _vm->_parser->_inputText, _vm->_gyro->_font, 8, 24, 161, kColorWhite);

	cursorOn();
	CursorMan.showMouse(true);
}

void Parser::cursorOn() {
	if (_cursorState == true)
		return;
	drawCursor();
	_cursorState = true;
}

void Parser::cursorOff() {
	if (_cursorState == false)
		return;
	drawCursor();
	_cursorState = false;
}

void Parser::tryDropdown() {
	warning("STUB: Parser::tryDropdown()"); // TODO: Implement at the same time with Dropdown's keyboard handling.
}

int16 Parser::pos(const Common::String &crit, const Common::String &src) {
	if (src.contains(crit))
		return strstr(src.c_str(),crit.c_str()) - src.c_str();
	else
		return -1;
}

void Parser::drawCursor() {
	// Draw the '_' character.
	for (int bit = 0; bit < 8; bit++)
		*(byte *)_vm->_graphics->_surface.getBasePtr(24 + _inputTextPos * 8 + 7 - bit, 168) = kColorWhite;

	ByteField bf;
	bf._x1 = _inputTextPos + 1;
	bf._x2 = _inputTextPos + 2;
	bf._y1 = 168;
	bf._y2 = 168;
}

void Parser::wipeText() {
	CursorMan.showMouse(false);
	cursorOff();

	_vm->_graphics->_surface.fillRect(Common::Rect(24, 161, 640, 169), kColorBlack); // Black out the line of the text.

	_quote = true;
	_inputTextPos = 0;

	cursorOn();
	CursorMan.showMouse(true);
}

} // End of namespace Avalanche
