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

void Parser::handleInputText(const Common::Event &event) {
	byte inChar = event.kbd.ascii;
	if (_vm->_dropdown->ddm_o.menunow) {
		_vm->_dropdown->parsekey(inChar, _vm->_enhanced->extd);
	} else {
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
	}
}

void Parser::handleBackspace() {
	if (!_vm->_dropdown->ddm_o.menunow) {
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
	if (_vm->_dropdown->ddm_o.menunow)
		_vm->_parser->tryDropdown();
	else {
		_vm->_logger->log_command(_inputText);
		
		if (!_inputText.empty()) {
			_inputTextBackup = _inputText;
			_vm->_acci->parse();
			_vm->_acci->do_that();
			_inputText.clear();
			wipeText();
		}
	}
}

void Parser::plotText() {
	if (_vm->_gyro->mouse_near_text())
		_vm->_gyro->super_off();

	cursorOff();

	_vm->_graphics->drawBar(24, 161, 640, 169, black); // Black out the line of the text.

	// Draw the text. Similar to chalk(), but here we don't have to bother with the color of the characters.
	for (byte i = 0; i < _vm->_parser->_inputText.size(); i++)
		for (byte j = 0; j < 8; j++) {
			byte pixel = _vm->_gyro->characters[_vm->_parser->_inputText[i]][j];
			for (byte bit = 0; bit < 8; bit++) {
				byte pixelBit = (pixel >> bit) & 1;
				if (pixelBit != 0)
					*_vm->_graphics->getPixel(24 + i * 8 + 7 - bit, 161 + j) = white;
			}
		}

	cursorOn();
	_vm->_gyro->super_on();
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
	warning("STUB: Parser::tryDropdown()"); // TODO: Implement at the same time with Dropdown.
}

int16 Parser::pos(const Common::String &crit, const Common::String &src) {
	if (src.contains(crit))
		return strstr(src.c_str(),crit.c_str()) - src.c_str();
	else
		return -1;
}

void Parser::drawCursor() {
	// Draw the '_' character. Similar to plotText().
	char cursor = '_';

	for (byte j = 0; j < 8; j++) {
		byte pixel = _vm->_gyro->characters[cursor][j];
		for (byte bit = 0; bit < 8; bit++) {
			byte pixelBit = (pixel >> bit) & 1;
			if (pixelBit != 0)
				*_vm->_graphics->getPixel(24 + _inputTextPos * 8 + 7 - bit, 161 + j) = white;
		}
	}
	


	bytefield bf;
	bf.x1 = _inputTextPos + 1;
	bf.x2 = _inputTextPos + 2;
	bf.y1 = 168;
	bf.y2 = 168;
	for (byte fv = 0; fv <= 1; fv ++)
		_vm->_trip->getset[fv].remember(bf);
}

void Parser::wipeText() {
	if (_vm->_gyro->mouse_near_text())
		_vm->_gyro->super_off();

	cursorOff();
	
	_vm->_graphics->drawBar(24, 161, 640, 169, black); // Black out the line of the text.

	_quote = true;
	_inputTextPos = 0;
	cursorOn();
	_vm->_gyro->super_on();
}

} // End of namespace Avalanche
