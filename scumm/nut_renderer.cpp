/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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


namespace Scumm {

NutRenderer::NutRenderer(ScummEngine *vm) :
	_vm(vm),
	_initialized(false),
	_loaded(false),
	_nbChars(0) {
	
	for (int i = 0; i < 256; i++)
		_chars[i].src = NULL;
}

NutRenderer::~NutRenderer() {
	for (int i = 0; i < _nbChars; i++) {
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

bool NutRenderer::loadFont(const char *filename, const char *directory) {
	debug(8, "NutRenderer::loadFont(\"%s\", \"%s\") called", filename, directory);
	if (_loaded) {
		warning("NutRenderer::loadFont() Font already loaded, ok, loading...");
	}

	File file;
	file.open(filename, directory);
	if (file.isOpen() == false) {
		warning("NutRenderer::loadFont() Can't open font file: %s/%s", directory, filename);
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

	_nbChars = READ_LE_UINT16(dataSrc + 10);
	uint32 offset = READ_BE_UINT32(dataSrc + 4) + 8;
	int32 decoded_length;
	for (int l = 0; l < _nbChars; l++) {
		if ((READ_BE_UINT32(dataSrc + offset) == 'FRME') || (READ_BE_UINT32(dataSrc + offset + 1) == 'FRME')) {
			if (READ_BE_UINT32(dataSrc + offset) == 'FRME')
				offset += 8;
			else if (READ_BE_UINT32(dataSrc + offset + 1) == 'FRME') // hack for proper offset
				offset += 9;
			if (READ_BE_UINT32(dataSrc + offset) == 'FOBJ') {
				int codec = READ_LE_UINT16(dataSrc + offset + 8);
				_chars[l].xoffs = READ_LE_UINT16(dataSrc + offset + 10);
				_chars[l].yoffs = READ_LE_UINT16(dataSrc + offset + 12);
				_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
				_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
				_chars[l].src = new byte[(_chars[l].width + 2) * _chars[l].height + 1000];
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

				offset += READ_BE_UINT32(dataSrc + offset + 4) + 8;
			} else {
				warning("NutRenderer::loadFont(%s, %s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, directory, l, offset);
				break;
			}
		} else {
			warning("NutRenderer::loadFont(%s, %s) there is no FRME chunk %d (offset %x)", filename, directory, l, offset);
			break;
		}
	}

	free(dataSrc);
	_loaded = true;
	return true;
}

int NutRenderer::getCharWidth(byte c) {
	debug(8, "NutRenderer::getCharWidth() called");
	if (!_loaded) {
		warning("NutRenderer::getCharWidth() Font is not loaded");
		return 0;
	}

	if (c >= 0x80 && _vm->_CJKMode) {
		if (_vm->_gameId == GID_CMI)
			return 8;
		if (_vm->_gameId == GID_DIG)
			return 6;
		return 0;
	}

	if (c >= _nbChars)
		error("invalid character in NutRenderer::getCharWidth : %d (%d)", c, _nbChars);

	return _chars[c].width;
}

int NutRenderer::getCharHeight(byte c) {
	debug(8, "NutRenderer::getCharHeight() called");
	if (!_loaded) {
		warning("NutRenderer::getCharHeight() Font is not loaded");
		return 0;
	}

	if (c >= 0x80 && _vm->_CJKMode) {
		if (_vm->_gameId == GID_CMI)
			return 16;
		if (_vm->_gameId == GID_DIG)
			return 10;
		return 0;
	}

	if (c >= _nbChars)
		error("invalid character in NutRenderer::getCharHeight : %d (%d)", c, _nbChars);

	return _chars[c].height;
}

void NutRenderer::drawShadowChar(int c, int x, int y, byte color, bool useMask) {
	debug(8, "NutRenderer::drawChar('%c', %d, %d, %d, %d) called", c, x, y, (int)color, useMask);
	if (!_loaded) {
		warning("NutRenderer::drawChar() Font is not loaded");
		return;
	}

	byte *dst, *mask = NULL;

	// HACK: we draw the character a total of 7 times: 6 times shifted
	// and in black for the shadow, and once in the right color and position.
	// This way we achieve the exact look as the original CMI had. However,
	// the question remains whether they did it this way, too, or if there is
	// some "font shadow" resource we don't know yet.
	// One problem remains: the fonts on the save/load screen don't have a
	// shadow. So how do we know whether to draw text with or without shadow?
	
	int offsetX[7] = { -1,  0, 1, 0, 1, 2, 0 };
	int offsetY[7] = {  0, -1, 0, 1, 2, 1, 0 };
	int cTable[7] =  {  0,  0, 0, 0, 0, 0, color };

	for (int i = 0; i < 7; i++) {
		x += offsetX[i];
		y += offsetY[i];
		color = cTable[i];
		
		dst = _vm->virtscr[0].screenPtr + (y + _vm->_screenTop) * _vm->_screenWidth + x + _vm->virtscr[0].xstart;
		if (useMask)
			mask = _vm->getMaskBuffer(x, y + _vm->_screenTop, 0);
		
		if (c >= 256 && _vm->_CJKMode)
			draw2byte(dst, mask, c, x, y, color);
		else
			drawChar(dst, mask, (byte)c, x, y, color);
		
		x -= offsetX[i];
		y -= offsetY[i];
	}
}

void NutRenderer::drawFrame(byte *dst, int c, int x, int y) {
	const int width = _chars[c].width;
	const int height = _chars[c].height;
	const byte *src = _chars[c].src;
	byte bits = 0;

	dst += _vm->_screenWidth * y + x;
	for (int ty = 0; ty < height; ty++) {
		for (int tx = 0; tx < width; tx++) {
			bits = *src++;
			if (x + tx < 0 || x + tx >= _vm->_screenWidth || y + ty < 0 || y + ty >= _vm->_screenHeight)
				continue;
			if (bits != 231) {
				dst[tx] = bits;
			}
		}
		dst += _vm->_screenWidth;
	}
}

void NutRenderer::drawChar(byte *dst, byte *mask, byte c, int x, int y, byte color) {
	const int width = _chars[c].width;
	const int height = _chars[c].height;
	const byte *src = _chars[c].src;
	byte bits = 0;

	byte maskmask;
	int maskpos;

	for (int ty = 0; ty < height; ty++) {
		maskmask = revBitMask[x & 7];
		maskpos = 0;
		for (int tx = 0; tx < width; tx++) {
			bits = *src++;
			if (x + tx < 0 || x + tx >= _vm->_screenWidth || y + ty < 0 || y + ty >= _vm->_screenHeight)
				continue;
			if (bits != 0) {
				dst[tx] = color;
				if (mask)
					mask[maskpos] |= maskmask;
			}
			maskmask >>= 1;
			if (maskmask == 0) {
				maskmask = 0x80;
				maskpos++;
			}
		}
		dst += _vm->_screenWidth;
		if (mask)
			mask += _vm->gdi._numStrips;
	}
}

void NutRenderer::draw2byte(byte *dst, byte *mask, int c, int x, int y, byte color) {
	if (!_loaded) {
		debug(2, "NutRenderer::draw2byte() Font is not loaded");
		return;
	}

	int width = _vm->_2byteWidth;
	int height = _vm->_2byteHeight;
	byte *src = _vm->get2byteCharPtr(c);
	byte bits = 0;

	byte maskmask;
	int maskpos;

	for (int ty = 0; ty < height; ty++) {
		maskmask = revBitMask[x & 7];
		maskpos = 0;
		for (int tx = 0; tx < width; tx++) {
			if ((tx % 8) == 0)
				bits = *src++;
			if (x + tx < 0 || x + tx >= _vm->_screenWidth || y + ty < 0 || y + ty >= _vm->_screenHeight)
				continue;
			if (bits & revBitMask[tx % 8]) {
				dst[tx] = color;
				if (mask) {
					mask[maskpos] |= maskmask;
				}
			}

			maskmask >>= 1;
			if (maskmask == 0) {
				maskmask = 0x80;
				maskpos++;
			}
		}
		dst += _vm->_screenWidth;
		if (mask)
			mask += _vm->gdi._numStrips;
	}
}

} // End of namespace Scumm
