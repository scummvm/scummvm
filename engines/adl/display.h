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
class WriteStream;
class String;
struct Point;
}

namespace Graphics {
struct Surface;
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
	virtual bool saveThumbnail(Common::WriteStream &out) = 0;
	void setMode(Mode mode);
	void copyTextSurface();
	void copyGfxSurface();

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
	Display()
	  : _textBuf(nullptr)
	  , _textSurface(nullptr)
	  , _gfxSurface(nullptr)
	  , _cursorPos(0)
	  , _mode(kModeText)
	  , _splitHeight(0)
	  , _textWidth(0)
	  , _textHeight(0) {}

	void createSurfaces(uint gfxWidth, uint gfxHeight, uint splitHeight);
	void createTextBuffer(uint textWidth, uint textHeight);

	byte *_textBuf;
	Graphics::Surface *_textSurface;
	Graphics::Surface *_gfxSurface;
	uint _cursorPos;

private:
	virtual void updateTextSurface() = 0;
	virtual void updateGfxSurface() = 0;

	Mode _mode;
	uint _splitHeight;
	uint _textWidth;
	uint _textHeight;
};

} // End of namespace Adl

#endif
