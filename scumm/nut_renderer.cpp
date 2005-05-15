/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "scumm/scumm.h"
#include "scumm/nut_renderer.h"
#include "scumm/bomp.h"
#include "scumm/util.h"


namespace Scumm {

NutRenderer::NutRenderer(ScummEngine *vm) :
	_vm(vm),
	_initialized(false),
	_loaded(false),
	_numChars(0) {

	for (int i = 0; i < 256; i++)
		_chars[i].src = NULL;
}

NutRenderer::~NutRenderer() {
	for (int i = 0; i < _numChars; i++) {
		if (_chars[i].src)
			delete []_chars[i].src;
	}
}

int32 NutRenderer::decodeCodec44(byte *dst, const byte *src, uint32 length) {
	byte val;
	uint16 size_line, num;
	int16 decoded_length = 0;

	do {
		size_line = READ_LE_UINT16(src);
		src += 2;
		length -= 2;

		while (size_line != 0) {
			num = *src++;
			val = *src++;
			memset(dst, val, num);
			dst += num;
			decoded_length += num;
			length -= 2;
			size_line -= 2;
			if (size_line != 0) {
				num = READ_LE_UINT16(src) + 1;
				src += 2;
				memcpy(dst, src, num);
				dst += num;
				decoded_length += num;
				src += num;
				length -= num + 2;
				size_line -= num + 2;
			}
		}
		dst--;
		decoded_length--;

	} while (length > 1);
	return decoded_length;
}

static int32 codec1(byte *dst, byte *src, int height) {
	byte val, code;
	int32 length, decoded_length = 0;
	int h = height, size_line;

	for (h = 0; h < height; h++) {
		size_line = READ_LE_UINT16(src);
		src += 2;
		while (size_line > 0) {
			code = *src++;
			size_line--;
			length = (code >> 1) + 1;
			if (code & 1) {
				val = *src++;
				size_line--;
				if (val)
					memset(dst, val, length);
				dst += length;
				decoded_length += length;
			} else {
				size_line -= length;
				while (length--) {
					val = *src++;
					if (val)
						*dst = val;
					dst++;
					decoded_length++;
				}
			}
		}
	}
	return decoded_length;
}

bool NutRenderer::loadFont(const char *filename) {
	if (_loaded) {
		warning("NutRenderer::loadFont() Font already loaded, ok, loading...");
	}

	ScummFile file;
	_vm->openFile(file, filename);
	if (file.isOpen() == false) {
		warning("NutRenderer::loadFont() Can't open font file: %s", filename);
		return false;
	}

	uint32 tag = file.readUint32BE();
	if (tag != 'ANIM') {
		warning("NutRenderer::loadFont() there is no ANIM chunk in font header");
		return false;
	}

	uint32 length = file.readUint32BE();
	byte *dataSrc = (byte *)malloc(length);
	file.read(dataSrc, length);
	file.close();

	if (READ_BE_UINT32(dataSrc) != 'AHDR') {
		warning("NutRenderer::loadFont() there is no AHDR chunk in font header");
		free(dataSrc);
		return false;
	}

	_numChars = READ_LE_UINT16(dataSrc + 10);
	uint32 offset = 0;
	int32 decoded_length;

	for (int l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 8;
		if (READ_BE_UINT32(dataSrc + offset) == 'FRME') {
			offset += 8;

			if (READ_BE_UINT32(dataSrc + offset) == 'FOBJ') {
				int codec = READ_LE_UINT16(dataSrc + offset + 8);
				_chars[l].xoffs = READ_LE_UINT16(dataSrc + offset + 10);
				_chars[l].yoffs = READ_LE_UINT16(dataSrc + offset + 12);
				_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
				_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
				_chars[l].src = new byte[(_chars[l].width + 2) * _chars[l].height + 1000];

				// If characters have transparency, then bytes just get skipped and
				// so there may appear some garbage. That's why we have to fill it
				// with zeroes first.
				memset(_chars[l].src, 0, (_chars[l].width + 2) * _chars[l].height + 1000);
				if ((codec == 44) || (codec == 21)) 
					decoded_length = decodeCodec44(_chars[l].src, dataSrc + offset + 22, READ_BE_UINT32(dataSrc + offset + 4) - 14);
				else if (codec == 1) {
					decoded_length = codec1(_chars[l].src, dataSrc + offset + 22, _chars[l].height);
				} else
					error("NutRenderer::loadFont: unknown codec: %d", codec);

				// FIXME: This is used to work around wrong font file format in Russian 
				// version of FT. Font files there contain wrong information about
				// glyphs width. See patch #823031.
				if (_vm->_language == Common::RU_RUS) {
					// try to rely on length of returned data
				  	if (l > 127)
						_chars[l].width = decoded_length / _chars[l].height;
					// but even this not always works
					if (l == 134 && !strcmp(filename, "titlfnt.nut"))
						_chars[l].width--;
				}
			} else {
				warning("NutRenderer::loadFont(%s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, l, offset);
				break;
			}
		} else {
			warning("NutRenderer::loadFont(%s) there is no FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
	}

	free(dataSrc);
	_loaded = true;
	return true;
}

int NutRenderer::getCharWidth(byte c) const {
	if (!_loaded) {
		warning("NutRenderer::getCharWidth() Font is not loaded");
		return 0;
	}

	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteWidth / 2;

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharWidth : %d (%d)", c, _numChars);

	return _chars[c].width;
}

int NutRenderer::getCharHeight(byte c) const {
	if (!_loaded) {
		warning("NutRenderer::getCharHeight() Font is not loaded");
		return 0;
	}

	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteHeight;

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharHeight : %d (%d)", c, _numChars);

	return _chars[c].height;
}

int NutRenderer::getCharOffsX(byte c) const {
	if (!_loaded) {
		warning("NutRenderer::getCharOffsX() Font is not loaded");
		return 0;
	}

	if (c >= 0x80 && _vm->_useCJKMode) {
		return 0;
	}

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharOffsX : %d (%d)", c, _numChars);

	return _chars[c].xoffs;
}

int NutRenderer::getCharOffsY(byte c) const {
	if (!_loaded) {
		warning("NutRenderer::getCharOffsY() Font is not loaded");
		return 0;
	}

	if (c >= 0x80 && _vm->_useCJKMode) {
		return 0;
	}

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharOffsY : %d (%d)", c, _numChars);

	return _chars[c].yoffs;
}

void NutRenderer::drawShadowChar(const Graphics::Surface &s, int c, int x, int y, byte color, bool showShadow) {
	if (!_loaded) {
		warning("NutRenderer::drawShadowChar() Font is not loaded");
		return;
	}

	// HACK: we draw the character a total of 7 times: 6 times shifted
	// and in black for the shadow, and once in the right color and position.
	// This way we achieve the exact look as the original CMI had. However,
	// the question remains whether they did it this way, too, or if there is
	// some "font shadow" resource we don't know yet.

	int offsetX[7] = { -1,  0, 1, 0, 1, 2, 0 };
	int offsetY[7] = {  0, -1, 0, 1, 2, 1, 0 };
	int cTable[7] =  {  0,  0, 0, 0, 0, 0, color };
	int i = 0;
	
	if (!showShadow)
		i = 6;

	for (; i < 7; i++) {
		x += offsetX[i];
		y += offsetY[i];
		color = cTable[i];
		
		if (c >= 256 && _vm->_useCJKMode)
			draw2byte(s, c, x, y, color);
		else
			drawChar(s, (byte)c, x, y, color);
		
		x -= offsetX[i];
		y -= offsetY[i];
	}
}

void NutRenderer::drawFrame(byte *dst, int c, int x, int y) {
	const int width = MIN(_chars[c].width, _vm->_screenWidth - x);
	const int height = MIN(_chars[c].height, _vm->_screenHeight - y);
	const byte *src = _chars[c].src;
	const int srcPitch = _chars[c].width;
	byte bits = 0;

	const int minX = x < 0 ? -x : 0;
	const int minY = y < 0 ? -y : 0;

	if (height <= 0 || width <= 0) {
		return;
	}

	dst += _vm->_screenWidth * y + x;
	if (minY) {
		src += minY * srcPitch;
		dst += minY * _vm->_screenWidth;
	}

	for (int ty = minY; ty < height; ty++) {
		for (int tx = minX; tx < width; tx++) {
			bits = src[tx];
			if (bits != 231 && bits) {
				dst[tx] = bits;
			}
		}
		src += srcPitch;
		dst += _vm->_screenWidth;
	}
}

void NutRenderer::drawChar(const Graphics::Surface &s, byte c, int x, int y, byte color) {
	byte *dst = (byte *)s.pixels + y * s.pitch + x;
	const int width = MIN(_chars[c].width, s.w - x);
	const int height = MIN(_chars[c].height, s.h - y);
	const byte *src = _chars[c].src;
	const int srcPitch = _chars[c].width;

	const int minX = x < 0 ? -x : 0;
	const int minY = y < 0 ? -y : 0;

	if (height <= 0 || width <= 0) {
		return;
	}

	if (minY) {
		src += minY * srcPitch;
		dst += minY * s.pitch;
	}

	for (int ty = minY; ty < height; ty++) {
		for (int tx = minX; tx < width; tx++) {
			if (src[tx] != 0) {
				dst[tx] = color;
			}
		}
		src += srcPitch;
		dst += s.pitch;
	}
}

void NutRenderer::draw2byte(const Graphics::Surface &s, int c, int x, int y, byte color) {
	if (!_loaded) {
		warning("NutRenderer::draw2byte() Font is not loaded");
		return;
	}

	byte *dst = (byte *)s.pixels + y * s.pitch + x;
	const int width = _vm->_2byteWidth;
	const int height = MIN(_vm->_2byteHeight, s.h - y);
	byte *src = _vm->get2byteCharPtr(c);
	byte bits = 0;

	if (height <= 0 || width <= 0) {
		return;
	}

	for (int ty = 0; ty < height; ty++) {
		for (int tx = 0; tx < width; tx++) {
			if ((tx & 7) == 0)
				bits = *src++;
			if (x + tx < 0 || x + tx >= s.w || y + ty < 0)
				continue;
			if (bits & revBitMask(tx % 8)) {
				dst[tx] = color;
			}
		}
		dst += s.pitch;
	}
}

} // End of namespace Scumm
