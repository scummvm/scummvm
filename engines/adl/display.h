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

#include <common/types.h>
#include <common/array.h>

namespace Common {
class ReadStream;
class WriteStream;
class String;
class Point;
}

namespace Graphics {
class Surface;
}

namespace Adl {

#define APPLECHAR(C) ((char)((C) | 0x80))

class Display {
public:
	enum Mode {
		kModeHires,
		kModeText,
		kModeMixed
	};

	Display();
	~Display();
	void enableScanlines(bool enable);
	void setMonoPalette();
	void setColorPalette();
	void loadFrameBuffer(Common::ReadStream &stream);
	void decodeFrameBuffer();
	void updateScreen();
	void setMode(Mode mode) { _mode = mode; }
	void drawPixel(byte x, byte y, byte color);
	void drawLine(Common::Point p1, Common::Point p2, byte color);
	void clear(byte color);
	void drawLineArt(const Common::Array<byte> &lineArt, Common::Point p, byte rotation = 0, byte scaling = 1, byte color = 0x7f);
	void setCursorPos(Common::Point pos);

	void home();
	void moveCursorTo(const Common::Point &pos);
	void moveCursorForward();
	void moveCursorBackward();
	void printString(const Common::String &str);
	void setCharAtCursor(byte c);
	void showCursor(bool enable);
	void updateTextSurface();
	bool saveThumbnail(Common::WriteStream &out);

private:
	enum {
		kWidth = 280,
		kHeight = 192,
		kFrameBufSize = 0x2000,
		kTextBufSize = 40 * 24
	};

	struct PixelPos {
		uint16 rowAddr;
		byte byteOffset;
		byte bitMask;
	};

	void decodeScanline(byte *dst, int pitch, byte *src);
	void decodeScanlineColor(byte *dst, int pitch, byte *src);
	void decodeScanlineMono(byte *dst, int pitch, byte *src);
	PixelPos getPixelPos(byte x, byte y);
	byte getPixelColor(byte x, byte color);
	void drawChar(byte c, int x, int y);
	void createFont();
	void moveX(PixelPos &p, byte &color, bool left);
	void moveY(PixelPos &p, bool down);
	void drawNextPixel(Display::PixelPos &p, byte &color, byte bits, byte quadrant);

	void scrollUp();

	bool _scanlines;
	byte *_frameBuf;
	byte *_textBuf;
	Graphics::Surface *_frameBufSurface;
	Graphics::Surface *_textBufSurface;
	Graphics::Surface *_font;
	int _cursorPos;
	Mode _mode;
	bool _showCursor;
	bool _monochrome;
};
 
} // End of namespace Adl

#endif
