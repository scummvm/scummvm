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

#ifndef ADL_DISPLAY_H
#define ADL_DISPLAY_H

#include "common/types.h"

namespace Common {
class String;
struct Point;
}

namespace Adl {

class Display {
public:
	enum Mode {
		kModeGraphics,
		kModeText,
		kModeMixed
	};

	virtual ~Display();

	virtual void init() = 0;
	void setMode(Mode mode);
	virtual void renderText() = 0;
	virtual void renderGraphics() = 0;

	virtual char asciiToNative(char c) const = 0;
	virtual void printChar(char c) = 0;
	virtual void showCursor(bool enable) = 0;
	void home();
	void moveCursorTo(const Common::Point &pos);
	void moveCursorForward();
	void moveCursorBackward();
	void printString(const Common::String &str);
	void printAsciiString(const Common::String &str);
	void setCharAtCursor(byte c);
	uint getTextWidth() const { return _textWidth; }
	uint getTextHeight() const { return _textHeight; }
	void scrollUp();

protected:
	Display() :	_textBuf(nullptr), _cursorPos(0), _mode(kModeText), _textWidth(0), _textHeight(0) { }

	void createTextBuffer(uint textWidth, uint textHeight);

	byte *_textBuf;
	uint _cursorPos;
	Mode _mode;
	uint _textWidth;
	uint _textHeight;
};

} // End of namespace Adl

#endif
