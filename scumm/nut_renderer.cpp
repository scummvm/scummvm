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
#include "scumm.h"
#include "nut_renderer.h"


NutRenderer::NutRenderer(Scumm *vm) {
	_vm = vm;
	_initialized = false;
	_loaded = false;
	_nbChars = 0;
	
	for(int i = 0; i < 256; i++)
		_chars[i].src = NULL;
}

NutRenderer::~NutRenderer() {
	for(int i = 0; i < _nbChars; i++) {
		if(_chars[i].src)
			delete []_chars[i].src;
	}
}

void NutRenderer::decodeCodec44(byte *dst, const byte *src, uint32 length) {
	byte val;
	uint16 size_line, num;

	do {
		size_line = READ_LE_UINT16(src);
		src += 2;
		length -= 2;

		while (size_line != 0) {
			num = *src++;
			val = *src++;
			memset(dst, val, num);
			dst += num;
			length -= 2;
			size_line -= 2;
			if (size_line != 0) {
				num = READ_LE_UINT16(src) + 1;
				src += 2;
				memcpy(dst, src, num);
				dst += num;
				src += num;
				length -= num + 2;
				size_line -= num + 2;
			}
		}
		dst--;

	} while (length > 1);
}

bool NutRenderer::loadFont(const char *filename, const char *directory) {
	debug(8, "NutRenderer::loadFont() called");
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
	for (int l = 0; l < _nbChars; l++) {
		if (READ_BE_UINT32(dataSrc + offset) == 'FRME') {
			offset += 8;
			if (READ_BE_UINT32(dataSrc + offset) == 'FOBJ') {
				_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
				_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
				_chars[l].src = new byte[_chars[l].width * _chars[l].height + 1000];
				decodeCodec44(_chars[l].src, dataSrc + offset + 22, READ_BE_UINT32(dataSrc + offset + 4) - 14);
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

	if(c >= 0x80 && g_scumm->_CJKMode) {
		if(g_scumm->_gameId == GID_CMI)
			return 8;
		if(g_scumm->_gameId == GID_DIG)
			return 6;
		return 0;
	}

	if(c >= _nbChars)
		error("invalid character in NutRenderer::getCharWidth : %d (%d)", c, _nbChars);

	return _chars[c].width;
}

int NutRenderer::getCharHeight(byte c) {
	debug(8, "NutRenderer::getCharHeight() called");
	if (!_loaded) {
		warning("NutRenderer::getCharHeight() Font is not loaded");
		return 0;
	}

	if(c >= 0x80 && g_scumm->_CJKMode) {
		if(g_scumm->_gameId == GID_CMI)
			return 16;
		if(g_scumm->_gameId == GID_DIG)
			return 10;
		return 0;
	}

	if(c >= _nbChars)
		error("invalid character in NutRenderer::getCharHeight : %d (%d)", c, _nbChars);

	return _chars[c].height;
}

int NutRenderer::getStringWidth(const byte *str) {
	debug(8, "NutRenderer::getStringWidth() called");
	if (!_loaded) {
		warning("NutRenderer::getStringWidth() Font is not loaded");
		return 0;
	}
	int width = 0;

	while (*str) {
		width += getCharWidth(*str++);
	}

	return width;
}

void NutRenderer::drawShadowChar(int c, int x, int y, byte color, bool useMask) {
	debug(8, "NutRenderer::drawChar('%c', %d, %d, %d, %d) called", c, x, y, (int)color, useMask);
	if (!_loaded) {
		warning("NutRenderer::drawChar() Font is not loaded");
		return;
	}

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
	
		if(c >= 256 && _vm->_CJKMode)
			draw2byte(c, x, y, color, useMask);
		else
			drawChar((byte)c, x, y, color, useMask);
		
		x -= offsetX[i];
		y -= offsetY[i];
	}
}

void NutRenderer::drawChar(byte c, int x, int y, byte color, bool useMask) {
	const int width = _chars[c].width;
	const int height = _chars[c].height;
	const byte *src = _chars[c].src;

	byte *dst, *mask = NULL;
	byte maskmask;
	int maskpos;
	
	dst = _vm->virtscr[0].screenPtr + y * _vm->_screenWidth + x + _vm->virtscr[0].xstart;
	mask = _vm->getMaskBuffer(x, y, 0);

	for (int ty = 0; ty < height; ty++) {
		maskmask = revBitMask[x & 7];
		maskpos = 0;
		for (int tx = 0; tx < width; tx++) {
			byte pixel = *src++;
			if (x + tx < 0 || x + tx >= _vm->_screenWidth || y + ty < 0 || y + ty >= _vm->_screenHeight)
				continue;
			if (pixel != 0) {
				dst[tx] = color;
				if (useMask)
					mask[maskpos] |= maskmask;
			}
			maskmask >>= 1;
			if (maskmask == 0) {
				maskmask = 0x80;
				maskpos++;
			}
		}
		dst += _vm->_screenWidth;
		mask += _vm->gdi._numStrips;
	}
}

void NutRenderer::draw2byte(int c, int x, int y, byte color, bool useMask) {
	if (!_loaded) {
		debug(2, "NutRenderer::draw2byte() Font is not loaded");
		return;
	}

	int width = g_scumm->_2byteWidth;
	int height = g_scumm->_2byteHeight;
	byte *src = g_scumm->get2byteCharPtr(c);
	byte bits = 0;

	byte *dst, *mask = NULL;
	byte maskmask;
	int maskpos;
	
	dst = _vm->virtscr[0].screenPtr + y * _vm->_screenWidth + x + _vm->virtscr[0].xstart;
	mask = _vm->getMaskBuffer(x, y, 0);

//	drawBits1(&_vm->virtscr[0], dst, src, mask, ?, width, height);
	for (int ty = 0; ty < height; ty++) {
		maskmask = revBitMask[x & 7];
		maskpos = 0;
		for (int tx = 0; tx < width; tx++) {
			if((tx % 8) == 0)
				bits = *src++;
			if (x + tx < 0 || x + tx >= _vm->_screenWidth || y + ty < 0 || y + ty >= _vm->_screenHeight)
				continue;
			if (bits & revBitMask[tx % 8]) {
				dst[tx] = color;
				if (useMask) {
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
		mask += _vm->gdi._numStrips;
	}
}
