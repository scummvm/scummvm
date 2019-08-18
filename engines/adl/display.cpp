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

#include "common/debug.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"

#include "adl/display.h"

namespace Adl {

Display::~Display() {
	delete[] _textBuf;
}

void Display::createTextBuffer(uint textWidth, uint textHeight) {
	_textWidth = textWidth;
	_textHeight = textHeight;

	_textBuf = new byte[textWidth * textHeight];
	memset(_textBuf, (byte)asciiToNative(' '), textWidth * textHeight);
}

void Display::setMode(Display::Mode mode) {
	_mode = mode;

	if (_mode == Display::kModeText || _mode == Display::kModeMixed)
		renderText();
	if (_mode == Display::kModeGraphics || _mode == Display::kModeMixed)
		renderGraphics();
}

void Display::home() {
	memset(_textBuf, (byte)asciiToNative(' '), _textWidth * _textHeight);
	_cursorPos = 0;
}

void Display::moveCursorForward() {
	++_cursorPos;

	if (_cursorPos >= _textWidth * _textHeight)
		scrollUp();
}

void Display::moveCursorBackward() {
	if (_cursorPos > 0)
		--_cursorPos;
}

void Display::moveCursorTo(const Common::Point &pos) {
	_cursorPos = pos.y * _textWidth + pos.x;

	if (_cursorPos >= _textWidth * _textHeight)
		error("Cursor position (%i, %i) out of bounds", pos.x, pos.y);
}

void Display::printString(const Common::String &str) {
	Common::String::const_iterator c;
	for (c = str.begin(); c != str.end(); ++c)
		printChar(*c);

	renderText();
}

void Display::printAsciiString(const Common::String &str) {
	Common::String::const_iterator c;
	for (c = str.begin(); c != str.end(); ++c)
		printChar(asciiToNative(*c));

	renderText();
}

void Display::setCharAtCursor(byte c) {
	_textBuf[_cursorPos] = c;
}

void Display::scrollUp() {
	memmove(_textBuf, _textBuf + _textWidth, (_textHeight - 1) * _textWidth);
	memset(_textBuf + (_textHeight - 1) * _textWidth, asciiToNative(' '), _textWidth);
	if (_cursorPos >= _textWidth)
		_cursorPos -= _textWidth;
}

} // End of namespace Adl
