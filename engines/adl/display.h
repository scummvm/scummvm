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
class ReadStream;
class WriteStream;
class String;
struct Point;
}

namespace Graphics {
struct Surface;
}

namespace Adl {

#define DISPLAY_WIDTH 280
#define DISPLAY_HEIGHT 192
#define DISPLAY_PITCH (DISPLAY_WIDTH / 7)
#define DISPLAY_SIZE (DISPLAY_PITCH * DISPLAY_HEIGHT)
#define TEXT_WIDTH 40
#define TEXT_HEIGHT 24

enum DisplayMode {
	DISPLAY_MODE_HIRES,
	DISPLAY_MODE_TEXT,
	DISPLAY_MODE_MIXED
};

#define APPLECHAR(C) ((char)((C) | 0x80))

class Display {
public:
	Display();
	~Display();

	void setMode(DisplayMode mode);
	void updateTextScreen();
	void updateHiResScreen();
	bool saveThumbnail(Common::WriteStream &out);

	// Graphics
	static void loadFrameBuffer(Common::ReadStream &stream, byte *dst);
	void loadFrameBuffer(Common::ReadStream &stream);
	void putPixel(const Common::Point &p, byte color);
	void setPixelByte(const Common::Point &p, byte color);
	void setPixelBit(const Common::Point &p, byte color);
	void setPixelPalette(const Common::Point &p, byte color);
	byte getPixelByte(const Common::Point &p) const;
	bool getPixelBit(const Common::Point &p) const;
	void clear(byte color);

	// Text
	void home();
	void moveCursorTo(const Common::Point &pos);
	void moveCursorForward();
	void moveCursorBackward();
	void printChar(char c);
	void printString(const Common::String &str);
	void printAsciiString(const Common::String &str);
	void setCharAtCursor(byte c);
	void showCursor(bool enable);

private:
	void writeFrameBuffer(const Common::Point &p, byte color, byte mask);
	void updateHiResSurface();
	void showScanlines(bool enable);

	void updateTextSurface();
	void drawChar(byte c, int x, int y);
	void createFont();
	void scrollUp();

	DisplayMode _mode;

	byte *_frameBuf;
	Graphics::Surface *_frameBufSurface;
	bool _scanlines;
	bool _monochrome;

	byte *_textBuf;
	Graphics::Surface *_textBufSurface;
	Graphics::Surface *_font;
	uint _cursorPos;
	bool _showCursor;
	uint32 _startMillis;
};

} // End of namespace Adl

#endif
