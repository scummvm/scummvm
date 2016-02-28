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

#include "adl/display.h"
#include "common/stream.h"
#include "common/rect.h"
#include "graphics/surface.h"
#include "common/system.h"
#include "common/str.h"
#include "common/events.h"
#include "common/rect.h"
#include "common/array.h"
#include "engines/engine.h"

namespace Adl {

static byte font[64][5] = {
	{ 0x7c, 0x82, 0xba, 0xb2, 0x9c }, { 0xf8, 0x24, 0x22, 0x24, 0xf8 }, // @A
	{ 0xfe, 0x92, 0x92, 0x92, 0x6c }, { 0x7c, 0x82, 0x82, 0x82, 0x44 }, // BC
	{ 0xfe, 0x82, 0x82, 0x82, 0x7c }, { 0xfe, 0x92, 0x92, 0x92, 0x82 }, // DE
	{ 0xfe, 0x12, 0x12, 0x12, 0x02 }, { 0x7c, 0x82, 0x82, 0xa2, 0xe2 }, // FG
	{ 0xfe, 0x10, 0x10, 0x10, 0xfe }, { 0x00, 0x82, 0xfe, 0x82, 0x00 }, // HI
	{ 0x40, 0x80, 0x80, 0x80, 0x7e }, { 0xfe, 0x10, 0x28, 0x44, 0x82 }, // JK
	{ 0xfe, 0x80, 0x80, 0x80, 0x80 }, { 0xfe, 0x04, 0x18, 0x04, 0xfe }, // LM
	{ 0xfe, 0x08, 0x10, 0x20, 0xfe }, { 0x7c, 0x82, 0x82, 0x82, 0x7c }, // NO
	{ 0xfe, 0x12, 0x12, 0x12, 0x0c }, { 0x7c, 0x82, 0xa2, 0x42, 0xbc }, // PQ
	{ 0xfe, 0x12, 0x32, 0x52, 0x8c }, { 0x4c, 0x92, 0x92, 0x92, 0x64 }, // RS
	{ 0x02, 0x02, 0xfe, 0x02, 0x02 }, { 0x7e, 0x80, 0x80, 0x80, 0x7e }, // TU
	{ 0x3e, 0x40, 0x80, 0x40, 0x3e }, { 0xfe, 0x40, 0x30, 0x40, 0xfe }, // VW
	{ 0xc6, 0x28, 0x10, 0x28, 0xc6 }, { 0x06, 0x08, 0xf0, 0x08, 0x06 }, // XY
	{ 0xc2, 0xa2, 0x92, 0x8a, 0x86 }, { 0xfe, 0xfe, 0x82, 0x82, 0x82 }, // Z[
	{ 0x04, 0x08, 0x10, 0x20, 0x40 }, { 0x82, 0x82, 0x82, 0xfe, 0xfe }, // \]
	{ 0x20, 0x10, 0x08, 0x10, 0x20 }, { 0x80, 0x80, 0x80, 0x80, 0x80 }, // ^_
	{ 0x00, 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xbe, 0x00, 0x00 }, //  !
	{ 0x00, 0x0e, 0x00, 0x0e, 0x00 }, { 0x28, 0xfe, 0x28, 0xfe, 0x28 }, // "#
	{ 0x48, 0x54, 0xfe, 0x54, 0x24 }, { 0x46, 0x26, 0x10, 0xc8, 0xc4 }, // $%
	{ 0x6c, 0x92, 0xac, 0x40, 0xa0 }, { 0x00, 0x00, 0x0e, 0x00, 0x00 }, // &'
	{ 0x38, 0x44, 0x82, 0x00, 0x00 }, { 0x00, 0x00, 0x82, 0x44, 0x38 }, // ()
	{ 0x44, 0x28, 0xfe, 0x28, 0x44 }, { 0x10, 0x10, 0x7c, 0x10, 0x10 }, // *+
	{ 0x00, 0x80, 0x60, 0x00, 0x00 }, { 0x10, 0x10, 0x10, 0x10, 0x10 }, // ,-
	{ 0x00, 0x00, 0x80, 0x00, 0x00 }, { 0x40, 0x20, 0x10, 0x08, 0x04 }, // ./
	{ 0x7c, 0xa2, 0x92, 0x8a, 0x7c }, { 0x00, 0x84, 0xfe, 0x80, 0x00 }, // 01
	{ 0xc4, 0xa2, 0x92, 0x92, 0x8c }, { 0x42, 0x82, 0x92, 0x9a, 0x66 }, // 23
	{ 0x30, 0x28, 0x24, 0xfe, 0x20 }, { 0x4e, 0x8a, 0x8a, 0x8a, 0x72 }, // 45
	{ 0x78, 0x94, 0x92, 0x92, 0x62 }, { 0x02, 0xe2, 0x12, 0x0a, 0x06 }, // 67
	{ 0x6c, 0x92, 0x92, 0x92, 0x6c }, { 0x8c, 0x92, 0x92, 0x52, 0x3c }, // 89
	{ 0x00, 0x00, 0x28, 0x00, 0x00 }, { 0x00, 0x80, 0x68, 0x00, 0x00 }, // :;
	{ 0x10, 0x28, 0x44, 0x82, 0x00 }, { 0x28, 0x28, 0x28, 0x28, 0x28 }, // <=
	{ 0x00, 0x82, 0x44, 0x28, 0x10 }, { 0x04, 0x02, 0xb2, 0x0a, 0x04 }  // >?
};

Display::Display() :
		_scanlines(false),
		_cursorPos(0),
		_mode(kModeText) {
	_frameBuf = new byte[kFrameBufSize];
	_frameBufSurface = new Graphics::Surface;
	_frameBufSurface->create(kWidth * 2, kHeight * 2, Graphics::PixelFormat::createFormatCLUT8());

	_textBuf = new byte[kTextBufSize];
	memset(_textBuf, ' ' | 0x80, kTextBufSize);
	_textBufSurface = new Graphics::Surface;
	_textBufSurface->create(kWidth * 2, kHeight * 2, Graphics::PixelFormat::createFormatCLUT8());

	createFont();

	struct PixelPos rel = getPixelPos(0, 191);
	struct PixelPos absy;
	for (int i = 191; i >= 0; --i) {
		absy = getPixelPos(0, i);
		if (absy.rowAddr != rel.rowAddr)
			debug("%i: %04x %04x", i, absy.rowAddr, rel.rowAddr);
		moveY(rel, false);
	}
	absy = getPixelPos(0, 191);
	if (absy.rowAddr != rel.rowAddr)
		debug("%i: %04x %04x", 191, absy.rowAddr, rel.rowAddr);

	rel = getPixelPos(0, 0);
	for (int i = 0; i < 192; ++i) {
		absy = getPixelPos(0, i);
		if (absy.rowAddr != rel.rowAddr)
			debug("%i: %04x %04x", i, absy.rowAddr, rel.rowAddr);
		moveY(rel, true);
	}
	absy = getPixelPos(0, 0);
	if (absy.rowAddr != rel.rowAddr)
		debug("%i: %04x %04x", 191, absy.rowAddr, rel.rowAddr);
}

Display::~Display() {
	delete[] _frameBuf;
	_frameBufSurface->free();
	delete _frameBufSurface;

	delete[] _textBuf;
	_textBufSurface->free();
	delete _textBufSurface;

	_font->free();
	delete _font;
}

void Display::loadFrameBuffer(Common::ReadStream &stream) {
	stream.read(_frameBuf, kFrameBufSize);
}

void Display::decodeScanline(byte *dst, int pitch, byte *src) {
	// TODO: shift secondPal by half a pixel

	bool prevOn = false;

	for (uint j = 0; j < 39; ++j) {
		bool secondPal = src[j] & 0x80;
		byte cur = src[j];
		byte next = 0;
		if (j != 39)
			next = src[j + 1];

		for (uint k = 0; k < 7; ++k) {
			bool curOn = cur & (1 << k);
			bool nextOn;

			if (k != 6)
				nextOn = cur & (1 << (k + 1));
			else
				nextOn = next & 1;

			byte color;
			if (curOn == prevOn || curOn == nextOn)
				color = curOn ? 1 : 0;
			else {
				if (secondPal)
					color = (curOn == ((j + k) % 2) ? 5 : 4);
				else
					color = (curOn == ((j + k) % 2) ? 3 : 2);
			}

			dst[0] = color;
			dst[1] = color;

			if (!_scanlines) {
				dst[pitch] = color;
				dst[pitch + 1] = color;
			}

			dst += 2;
			prevOn = curOn;
		}
	}
}

Display::PixelPos Display::getPixelPos(byte x, byte y) {
	PixelPos pixelPos;

	// FIXME: check X, Y range

	byte offsetL = y & 0xc0;
	offsetL |= offsetL >> 2;
	byte offsetH = y;
	y <<= 2;
	offsetH <<= 1;
	offsetH |= y >> 7;
	y <<= 1;
	offsetH <<= 1;
	offsetH |= y >> 7;
	y <<= 1;
	offsetL >>= 1;
	offsetL |= y & 0x80;
	y <<= 1;
	offsetH = offsetH & 0x1f;
	pixelPos.rowAddr = (offsetH << 8) | offsetL;
	pixelPos.byteOffset = x / 7;
	pixelPos.bitMask = 0x80 | (1 << x % 7);

	return pixelPos;
}

byte Display::getPixelColor(byte offset, byte color) {
	if (offset & 1) {
		byte c = color << 1;
		if (c >= 0x40 && c < 0xc0)
			return color ^ 0x7f;
	}

	return color;
}

void Display::decodeFrameBuffer() {
	byte *src = _frameBuf;
	int pitch = _frameBufSurface->pitch;
	for (int j = 0; j < 8; ++j) {
		for (int i = 0; i < 8; ++i) {
			byte *dst = (byte *)_frameBufSurface->getPixels() + pitch * 2 * (i * 8 + j);
			decodeScanline(dst, pitch, src);
			src += 40;
			dst += pitch * 2 * 64;
			decodeScanline(dst, pitch, src);
			src += 40;
			dst += pitch * 2 * 64;
			decodeScanline(dst, pitch, src);
			src += 48;
			dst += pitch * 2 * 64;
		}
	}
}

void Display::drawPixel(byte x, byte y, byte color) {
	PixelPos p = getPixelPos(x, y);
	byte c = getPixelColor(p.byteOffset, color);
	byte *b = _frameBuf + p.rowAddr + p.byteOffset;
	c ^= *b;
	c &= p.bitMask;
	c ^= *b;
	*b = c;
}

void Display::moveX(PixelPos &p, byte &color, bool left) {
	if (left) {
		byte bit = p.bitMask;
		bool b = bit & 1;
		bit >>= 1;
		if (!b) {
			bit ^= 0xc0;
			p.bitMask = bit;
			return;
		}
		--p.byteOffset;
		if (p.byteOffset & 0x80)
			p.byteOffset = 39;
		p.bitMask = 0xc0;
	} else {
		byte bit = p.bitMask;
		bit <<= 1;
		bit ^= 0x80;
		if (bit & 0x80) {
			p.bitMask = bit;
			return;
		}
		p.bitMask = 0x81;
		++p.byteOffset;
		if (p.byteOffset == 40)
			p.byteOffset = 0;
	}

	color = getPixelColor(p.byteOffset, color);
}

void Display::moveY(PixelPos &p, bool down) {
	if (!down) {
		if (p.rowAddr & 0x1c00)
			p.rowAddr -= 0x400;
		else if (p.rowAddr & 0x380)
			p.rowAddr += 0x1b80;
		else {
			p.rowAddr += 0x1f58;
			if (!(p.rowAddr & 0x80))
				p.rowAddr += 0x78; // Wrap around
		}
	} else {
		p.rowAddr += 0x400;
		if (p.rowAddr & 0x1c00)
			return;
		else if ((p.rowAddr & 0x380) != 0x380)
			p.rowAddr -= 0x1f80;
		else {
			p.rowAddr -= 0x2358;
			if ((p.rowAddr & 0x78) == 0x78)
				p.rowAddr -= 0x78; // Wrap around
		}
	}
}

void Display::drawNextPixel(Display::PixelPos &p, byte &color, byte bits, byte quadrant) {
	if (bits & 4) {
		byte b = (_frameBuf[p.rowAddr + p.byteOffset] ^ color) & p.bitMask;
		_frameBuf[p.rowAddr + p.byteOffset] ^= b;
	}

	bits += quadrant;

	if (bits & 1)
		moveX(p, color, bits & 2);
	else
		moveY(p, bits & 2);
}

void Display::drawLineArt(const Common::Array<byte> &lineArt, Common::Point p, byte rotation, byte scaling, byte color) {
	const byte stepping[] = {
		0xff, 0xfe, 0xfa, 0xf4, 0xec, 0xe1, 0xd4, 0xc5,
		0xb4, 0xa1, 0x8d, 0x78, 0x61, 0x49, 0x31, 0x18,
		0xff
	};

	PixelPos pos = getPixelPos(p.x, p.y);
	byte c = getPixelColor(pos.byteOffset, color);

	byte quadrant = rotation >> 4;
	rotation &= 0xf;
	byte xStep = stepping[rotation];
	byte yStep = stepping[(rotation ^ 0xf) + 1] + 1;

	for (uint i = 0; i < lineArt.size(); ++i) {
		byte b = lineArt[i];

		do {
			byte xFrac = 0x80;
			byte yFrac = 0x80;
			for (uint j = 0; j < scaling; ++j) {
				if (xFrac + xStep + 1 > 255)
					drawNextPixel(pos, c, b, quadrant);
				xFrac += xStep + 1;
				if (yFrac + yStep > 255)
					drawNextPixel(pos, c, b, quadrant + 1);
				yFrac += yStep;
			}
			b >>= 3;
		} while (b != 0);
	}
}

void Display::drawLine(Common::Point p1, Common::Point p2, byte color) {
	PixelPos p = getPixelPos(p1.x, p1.y);
	byte c = getPixelColor(p.byteOffset, color);

	int16 deltaX = p2.x - p1.x;
	byte dir = deltaX >> 8;

	if (deltaX < 0)
		deltaX = -deltaX;

	int16 err = deltaX;

	int16 deltaY = p2.y - p1.y - 1;
	dir >>= 1;
	if (deltaY >= 0) {
		deltaY = -deltaY - 2;
		dir |= 0x80;
	}

	int16 steps = deltaY - deltaX;

	err += deltaY + 1;

	while (1) {
		byte *b = _frameBuf + p.rowAddr + p.byteOffset;
		byte d = *b;
		d ^= c;
		d &= p.bitMask;
		d ^= *b;
		*b = d;

		if (++steps == 0)
			return;

		if (err < 0) {
			moveY(p, dir & 0x80);
			err += deltaX;
		} else {
			moveX(p, c, dir & 0x40);
			err += deltaY + 1;
		}
	}
}

void Display::clear(byte color) {
	for (uint i = 0; i < kFrameBufSize; ++i)
		_frameBuf[i] = getPixelColor(i & 1, color);
}

void Display::updateTextSurface() {
	for (uint row = 0; row < 24; ++row)
		for (uint col = 0; col < 40; ++col) {
			char c = _textBuf[row * 40 + col];

			Common::Rect r(7 * 2, 8 * 2);
			r.translate(((c & 0x3f) % 16) * 7 * 2, (c & 0x3f) / 16 * 8 * 2);

			if (!(c & 0x80)) {
				if (!(c & 0x40) || ((g_system->getMillis() / 270) & 1))
					r.translate(0, 4 * 8 * 2);
			}

			_textBufSurface->copyRectToSurface(*_font, col * 7 * 2, row * 8 * 2, r);
		}
}

void Display::printString(const Common::String &str) {
	Common::String::const_iterator it;
	for (it = str.begin(); it != str.end(); ++it) {
		byte b = *it;

		if (b == ('\r' | 0x80))
			_cursorPos = (_cursorPos / 40 + 1) * 40;
		else if (b < 0x80 || b >= 0xa0)
			_textBuf[_cursorPos++] = b;

		if (_cursorPos == kTextBufSize) {
			memmove(_textBuf, _textBuf + 40, kTextBufSize - 40);
			memset(_textBuf + kTextBufSize - 40, ' ' | 0x80, 40);
			_cursorPos -= 40;
		}
	}

	updateTextSurface();
}

void Display::printASCIIString(const Common::String &str) {
	Common::String aStr;

	Common::String::const_iterator it;
	for (it = str.begin(); it != str.end(); ++it)
			aStr += APPLECHAR(*it);

	printString(aStr);
}

void Display::drawChar(byte c, int x, int y) {
	byte *buf = (byte *)_font->getPixels() + y * _font->pitch + x;

	for (uint row = 0; row < 8; ++row) {
		for (uint col = 1; col < 6; ++col)
			if (font[c][col - 1] & (1 << row)) {
				buf[col * 2] = 1;
				buf[col * 2 + 1] = 1;

				if (!_scanlines) {
					buf[_font->pitch + col * 2] = 1;
					buf[_font->pitch + col * 2 + 1] = 1;
				}
			}

		buf += 2 * _font->pitch;
	}
}

void Display::createFont() {
	_font = new Graphics::Surface;
	_font->create(16 * 7 * 2, 4 * 8 * 2 * 2, Graphics::PixelFormat::createFormatCLUT8());

	for (uint i = 0; i < 4; ++i)
		for (uint j = 0; j < 16; ++j)
			drawChar(i * 16 + j, j * 7 * 2, i * 8 * 2);

	// Create inverted font
	byte *buf = (byte *)_font->getPixels();
	byte *bufInv = buf + (_font->h / 2) * _font->pitch;

	for (uint row = 0; row < _font->h / 2; ++row) {
		if (!_scanlines || !(row & 1))
			for (uint col = 0; col < _font->w; ++col)
				bufInv[col] = buf[col] ? 0 : 1;

		buf += _font->pitch;
		bufInv += _font->pitch;
	}
}

void Display::updateScreen() {
	if (_mode == kModeText) {
		g_system->copyRectToScreen(_textBufSurface->getPixels(), _textBufSurface->pitch, 0, 0, _textBufSurface->w, _textBufSurface->h);
	} else if (_mode == kModeHires) {
		g_system->copyRectToScreen(_frameBufSurface->getPixels(), _frameBufSurface->pitch, 0, 0, _frameBufSurface->w, _frameBufSurface->h);
	} else {
		g_system->copyRectToScreen(_frameBufSurface->getPixels(), _frameBufSurface->pitch, 0, 0, _frameBufSurface->w, _frameBufSurface->h - 4 * 8 * 2);
		g_system->copyRectToScreen(_textBufSurface->getBasePtr(0, _textBufSurface->h - 4 * 8 * 2), _textBufSurface->pitch, 0, _textBufSurface->h - 4 * 8 * 2, _textBufSurface->w, 4 * 8 * 2);
	}
}

Common::String Display::inputString(byte prompt) {
	Common::String s;

	if (prompt > 0)
		printString(Common::String(prompt));

	while (1) {
		byte b = inputKey();

		if (g_engine->shouldQuit())
			return 0;

		if (b == 0)
			continue;

		if (b == ('\r' | 0x80)) {
			s += b;
			printString(Common::String(b));
			return s;
		}

		if (b < 0xa0) {
			switch (b) {
			case Common::KEYCODE_BACKSPACE | 0x80:
				if (!s.empty()) {
					--_cursorPos;
					_textBuf[_cursorPos] = ' ' | 0x80;
					s.deleteLastChar();
				}
				break;
			};
		} else {
			s += b;
			printString(Common::String(b));
		}
	}
}

byte Display::convertKey(uint16 ascii) {
	ascii = toupper(ascii);

	if (ascii >= 0x80)
		return 0;

	ascii |= 0x80;

	if (ascii >= 0x80 && ascii <= 0xe0)
		return ascii;

	return 0;
}

byte Display::inputKey() {
	Common::EventManager *ev = g_system->getEventManager();

	byte orgChar = _textBuf[_cursorPos];
	_textBuf[_cursorPos] = (orgChar & 0x3f) | 0x40;

	byte key = 0;

	while (!g_engine->shouldQuit() && key == 0) {
		Common::Event event;
		if (ev->pollEvent(event)) {
			if (event.type != Common::EVENT_KEYDOWN)
				continue;

			if (event.kbd.flags & Common::KBD_CTRL) {
				if (event.kbd.keycode == Common::KEYCODE_q)
					g_engine->quitGame();
				continue;
			}

			switch (event.kbd.keycode) {
			case Common::KEYCODE_BACKSPACE:
			case Common::KEYCODE_RETURN:
				key = convertKey(event.kbd.keycode);
				break;
			default:
				if (event.kbd.ascii >= 0x20 && event.kbd.ascii < 0x80)
					key = convertKey(event.kbd.ascii);
			};
		}

		updateTextSurface();
		updateScreen();
		g_system->updateScreen();
		g_system->delayMillis(16);
	}

	_textBuf[_cursorPos] = orgChar;
	return key;
}

void Display::delay(uint32 ms) {
	Common::EventManager *ev = g_system->getEventManager();

	uint32 start = g_system->getMillis();

	while (!g_engine->shouldQuit() && g_system->getMillis() - start < ms) {
		Common::Event event;
		if (ev->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && (event.kbd.flags & Common::KBD_CTRL)) {
				switch(event.kbd.keycode) {
				case Common::KEYCODE_q:
					g_engine->quitGame();
					break;
				default:
					break;
				}
			}
		}
		updateScreen();
		g_system->updateScreen();
		g_system->delayMillis(16);
	}
}

void Display::home() {
	memset(_textBuf, ' ' | 0x80, kTextBufSize);
	_cursorPos = 0;
}

} // End of namespace Adl
