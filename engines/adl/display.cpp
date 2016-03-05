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

#include "common/stream.h"
#include "common/rect.h"
#include "common/system.h"
#include "common/str.h"
#include "common/config-manager.h"

#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/thumbnail.h"

#include "engines/util.h"

#include "adl/display.h"

namespace Adl {

// This implements the Apple II "Hi-Res" display mode

#define DISPLAY_PITCH (DISPLAY_WIDTH / 7)

#define COLOR_PALETTE_ENTRIES 8
const byte colorPalette[COLOR_PALETTE_ENTRIES * 3] = {
	0x00, 0x00, 0x00,
	0xff, 0xff, 0xff,
	0xc7, 0x34, 0xff,
	0x38, 0xcb, 0x00,
	0x00, 0x00, 0x00,
	0xff, 0xff, 0xff,
	0x0d, 0xa1, 0xff,
	0xf2, 0x5e, 0x00
};

// Green monochrome palette
#define MONO_PALETTE_ENTRIES 2
const byte monoPalette[MONO_PALETTE_ENTRIES * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0xc0, 0x01
};

static const byte font[64][5] = {
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
		_mode(DISPLAY_MODE_TEXT),
		_cursorPos(0),
		_showCursor(false) {

	initGraphics(560, 384, true);

	_monochrome = !ConfMan.getBool("color");
	_scanlines = ConfMan.getBool("scanlines");

	if (_monochrome)
		g_system->getPaletteManager()->setPalette(monoPalette, 0, MONO_PALETTE_ENTRIES);
	else
		g_system->getPaletteManager()->setPalette(colorPalette, 0, COLOR_PALETTE_ENTRIES);

	showScanlines(_scanlines);

	_frameBuf = new byte[DISPLAY_PITCH * DISPLAY_HEIGHT];
	_frameBufSurface = new Graphics::Surface;
	// We need 2x scaling to properly render the half-pixel shift
	// of the second palette
	_frameBufSurface->create(DISPLAY_WIDTH * 2, DISPLAY_HEIGHT * 2, Graphics::PixelFormat::createFormatCLUT8());

	_textBuf = new byte[kTextBufSize];
	memset(_textBuf, APPLECHAR(' '), kTextBufSize);
	_textBufSurface = new Graphics::Surface;
	// For ease of copying, also use 2x scaling here
	_textBufSurface->create(DISPLAY_WIDTH * 2, DISPLAY_HEIGHT * 2, Graphics::PixelFormat::createFormatCLUT8());

	createFont();
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

void Display::setMode(DisplayMode mode) {
	_mode = mode;

	if (_mode == DISPLAY_MODE_TEXT || _mode == DISPLAY_MODE_MIXED)
		updateTextScreen();
	if (_mode == DISPLAY_MODE_HIRES || _mode == DISPLAY_MODE_MIXED)
		updateHiResScreen();
}

void Display::updateTextScreen() {
	updateTextSurface();

	if (_mode == DISPLAY_MODE_TEXT)
		g_system->copyRectToScreen(_textBufSurface->getPixels(), _textBufSurface->pitch, 0, 0, _textBufSurface->w, _textBufSurface->h);
	else if (_mode == DISPLAY_MODE_MIXED)
		g_system->copyRectToScreen(_textBufSurface->getBasePtr(0, _textBufSurface->h - 4 * 8 * 2), _textBufSurface->pitch, 0, _textBufSurface->h - 4 * 8 * 2, _textBufSurface->w, 4 * 8 * 2);

	g_system->updateScreen();
}

void Display::updateHiResScreen() {
	updateHiResSurface();

	if (_mode == DISPLAY_MODE_HIRES)
		g_system->copyRectToScreen(_frameBufSurface->getPixels(), _frameBufSurface->pitch, 0, 0, _frameBufSurface->w, _frameBufSurface->h);
	else if (_mode == DISPLAY_MODE_MIXED)
		g_system->copyRectToScreen(_frameBufSurface->getPixels(), _frameBufSurface->pitch, 0, 0, _frameBufSurface->w, _frameBufSurface->h - 4 * 8 * 2);

	g_system->updateScreen();
}

bool Display::saveThumbnail(Common::WriteStream &out) {
	if (_scanlines) {
		showScanlines(false);
		g_system->updateScreen();
	}

	bool retval = Graphics::saveThumbnail(out);

	if (_scanlines) {
		showScanlines(true);
		g_system->updateScreen();
	}

	return retval;
}

void Display::loadFrameBuffer(Common::ReadStream &stream) {
	for (uint j = 0; j < 8; ++j) {
		for (uint i = 0; i < 8; ++i) {
			byte *dst = _frameBuf + DISPLAY_PITCH  * (i * 8 + j);
			stream.read(dst, DISPLAY_PITCH);
			dst += DISPLAY_PITCH * 64;
			stream.read(dst, DISPLAY_PITCH);
			dst += DISPLAY_PITCH * 64;
			stream.read(dst, DISPLAY_PITCH);
			stream.readUint32LE();
			stream.readUint32LE();
			dst += DISPLAY_PITCH * 64;
		}
	}
}

void Display::putPixel(Common::Point p, byte color) {
	byte offset = p.x / 7;

	if (offset & 1) {
		byte c = color << 1;
		if (c >= 0x40 && c < 0xc0)
			color ^= 0x7f;
	}

	byte *b = _frameBuf + p.y * DISPLAY_PITCH + offset;
	color ^= *b;
	color &= 1 << (p.x % 7);
	*b ^= color;
}

void Display::clear(byte color) {
	byte val = 0;

	byte c = color << 1;
	if (c >= 0x40 && c < 0xc0)
		val = 0x7f;

	for (uint i = 0; i < DISPLAY_PITCH * DISPLAY_HEIGHT; ++i) {
		_frameBuf[i] = color;
		color ^= val;
	}
}

void Display::home() {
	memset(_textBuf, APPLECHAR(' '), kTextBufSize);
	_cursorPos = 0;
}

void Display::moveCursorForward() {
	++_cursorPos;

	if (_cursorPos >= kTextBufSize)
		scrollUp();
}

void Display::moveCursorBackward() {
	--_cursorPos;

	if (_cursorPos < 0)
		_cursorPos = 0;
}

void Display::moveCursorTo(const Common::Point &pos) {
	_cursorPos = pos.y * 40 + pos.x;

	if (_cursorPos >= kTextBufSize)
		error("Cursor position (%i, %i) out of bounds", pos.x, pos.y);
}

void Display::printString(const Common::String &str) {
	Common::String::const_iterator c;
	for (c = str.begin(); c != str.end(); ++c) {
		byte b = *c;

		if (*c == APPLECHAR('\r'))
			_cursorPos = (_cursorPos / 40 + 1) * 40;
		else if (b < 0x80 || b >= 0xa0) {
			setCharAtCursor(b);
			++_cursorPos;
		}

		if (_cursorPos == kTextBufSize)
			scrollUp();
	}

	updateTextScreen();
}

void Display::setCharAtCursor(byte c) {
	_textBuf[_cursorPos] = c;
}

void Display::showCursor(bool enable) {
	_showCursor = enable;
}

void Display::showScanlines(bool enable) {
	byte pal[COLOR_PALETTE_ENTRIES * 3] = { };

	if (enable)
		g_system->getPaletteManager()->setPalette(pal, COLOR_PALETTE_ENTRIES, COLOR_PALETTE_ENTRIES);
	else {
		g_system->getPaletteManager()->grabPalette(pal, 0, COLOR_PALETTE_ENTRIES);
		g_system->getPaletteManager()->setPalette(pal, COLOR_PALETTE_ENTRIES, COLOR_PALETTE_ENTRIES);
	}
}

void Display::updateHiResSurface() {
	byte *src = _frameBuf;
	byte *dst = (byte *)_frameBufSurface->getPixels();

	for (uint i = 0; i < DISPLAY_HEIGHT; ++i) {
		decodeScanline(dst, _frameBufSurface->pitch, src);
		src += DISPLAY_PITCH;
		dst += _frameBufSurface->pitch * 2;
	}
}

void Display::decodeScanlineColor(byte *dst, int pitch, byte *src) const {
	bool prevOn = false;

	if (src[0] & 0x80)
		dst++;

	for (uint j = 0; j < 40; ++j) {
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
			else
				color = (curOn == ((j + k) % 2) ? 3 : 2);

			if (secondPal)
				color |= 4;

			dst[0] = color;
			dst[pitch] = color + COLOR_PALETTE_ENTRIES;
			++dst;

			if (k == 6) {
				if (secondPal) {
					if (next & 0x80) {
						dst[0] = color;
						dst[pitch] = color + COLOR_PALETTE_ENTRIES;
						++dst;
					}
				} else {
					dst[0] = color;
					dst[pitch] = color + COLOR_PALETTE_ENTRIES;
					++dst;
					if (next & 0x80) {
						dst[0] = color | 4;
						dst[pitch] = (color | 4) + COLOR_PALETTE_ENTRIES;
						++dst;
					}
				}
			} else {
				dst[0] = color;
				dst[pitch] = color + COLOR_PALETTE_ENTRIES;
				++dst;
			}

			prevOn = curOn;
		}
	}
}

void Display::decodeScanlineMono(byte *dst, int pitch, byte *src) const {
	// TODO: shift secondPal by half a pixel

	for (uint j = 0; j < 39; ++j) {
		for (uint k = 0; k < 7; ++k) {
			byte color = 0;

			if (src[j] & (1 << k))
				color = 1;

			dst[0] = color;
			dst[1] = color;
			dst[pitch] = color + 6;
			dst[pitch + 1] = color + 6;

			dst += 2;
		}
	}
}

void Display::decodeScanline(byte *dst, int pitch, byte *src) const {
	if (_monochrome)
		decodeScanlineMono(dst, pitch, src);
	else
		decodeScanlineColor(dst, pitch, src);
}

void Display::updateTextSurface() {
	for (uint row = 0; row < 24; ++row)
		for (uint col = 0; col < 40; ++col) {
			int charPos = row * 40 + col;
			char c = _textBuf[row * 40 + col];

			if (charPos == _cursorPos && _showCursor)
				c = (c & 0x3f) | 0x40;

			Common::Rect r(7 * 2, 8 * 2);
			r.translate(((c & 0x3f) % 16) * 7 * 2, (c & 0x3f) / 16 * 8 * 2);

			if (!(c & 0x80)) {
				if (!(c & 0x40) || ((g_system->getMillis() / 270) & 1))
					r.translate(0, 4 * 8 * 2);
			}

			_textBufSurface->copyRectToSurface(*_font, col * 7 * 2, row * 8 * 2, r);
		}
}

void Display::drawChar(byte c, int x, int y) {
	byte *buf = (byte *)_font->getPixels() + y * _font->pitch + x;

	for (uint row = 0; row < 8; ++row) {
		if (row & 1) {
			buf[_font->pitch] = COLOR_PALETTE_ENTRIES;
			buf[_font->pitch + 1] = COLOR_PALETTE_ENTRIES;
			buf[_font->pitch + 6 * 2] = COLOR_PALETTE_ENTRIES;
			buf[_font->pitch + 6 * 2 + 1] = COLOR_PALETTE_ENTRIES;
		}
		for (uint col = 1; col < 6; ++col) {
			if (font[c][col - 1] & (1 << row)) {
				buf[col * 2] = 1;
				buf[col * 2 + 1] = 1;
				buf[_font->pitch + col * 2] = 1 + COLOR_PALETTE_ENTRIES;
				buf[_font->pitch + col * 2 + 1] = 1 + COLOR_PALETTE_ENTRIES;
			} else {
				buf[_font->pitch + col * 2] = COLOR_PALETTE_ENTRIES;
				buf[_font->pitch + col * 2 + 1] = COLOR_PALETTE_ENTRIES;
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

	for (uint row = 0; row < _font->h / 2; row += 2) {
		for (uint col = 0; col < _font->w; ++col)
			bufInv[col] = (buf[col] ? 0 : 1);

		buf += _font->pitch;
		bufInv += _font->pitch;

		for (uint col = 0; col < _font->w; ++col)
			bufInv[col] = (buf[col] == COLOR_PALETTE_ENTRIES + 1 ? COLOR_PALETTE_ENTRIES : COLOR_PALETTE_ENTRIES + 1);

		buf += _font->pitch;
		bufInv += _font->pitch;
	}
}

void Display::scrollUp() {
	memmove(_textBuf, _textBuf + 40, kTextBufSize - 40);
	memset(_textBuf + kTextBufSize - 40, APPLECHAR(' '), 40);
	_cursorPos -= 40;
}

} // End of namespace Adl
