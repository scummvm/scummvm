/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "engine.h"
#include "nut_renderer.h"

NutRenderer::NutRenderer() {
	_initialized = false;
	_loaded = false;
	_dataSrc = NULL;
}

NutRenderer::~NutRenderer() {
	if (_dataSrc != NULL)
		free(_dataSrc);
}

void NutRenderer::decodeCodec44(byte *dst, byte *src, uint32 length) {
	byte val;
	uint16 size_line;
	uint16 num;

	do {
		size_line = READ_LE_UINT16(src);
		src += 2;
		length -= 2;

		for (; size_line != 0;) {
			num = *src++;
			val = *src++;
			memset(dst, val, num);
			dst += num;
			length -= 2;
			size_line -= 2;
			if (size_line == 0) break;

			num = READ_LE_UINT16(src) + 1;
			src += 2;
			memcpy(dst, src, num);
			dst += num;
			src += num;
			length -= num + 2;
			size_line -= num + 2;

		}
		dst--;

	} while (length > 1);
}

void NutRenderer::bindDisplay(byte *dst, int32 width, int32 height, int32 pitch) {
	debug(2,  "NutRenderer::init() called");
	if (_initialized == true) {
		debug(2, "NutRenderer::init() Already initialized, ok, changing...");
	}

	_dstPtr = dst;
	_dstWidth = width;
	_dstHeight = height;
	_dstPitch = pitch;

	_initialized = true;
}

bool NutRenderer::loadFont(char *filename, char *dir) {
	debug(2,  "NutRenderer::loadFont() called");
	if (_loaded == true) {
		debug(2, "NutRenderer::loadFont() Font already loaded, ok, loading...");
	}
	
	File file;
	file.open(filename, dir);
	if (file.isOpen() == false) {
		debug(2, "NutRenderer::loadFont() Can't open font file: %s", filename);
		return false;
	}

	uint32 tag = file.readUint32BE();
	if (tag != 'ANIM') {
		debug(2, "NutRenderer::loadFont() there is no ANIM chunk in font header");
		return false;
	}

	uint32 length = file.readUint32BE();
	_dataSrc = (byte *)malloc(length);
	file.read(_dataSrc, length);
	file.close();

	if (READ_BE_UINT32(_dataSrc) != 'AHDR') {
		debug(2, "NutRenderer::loadFont() there is no AHDR chunk in font header");
		free(_dataSrc);
		return false;
	}
	
	int32 l;
	uint32 offset = READ_BE_UINT32(_dataSrc + 4) + 8;
	for (l = 0; l < 256; l++) {
		if (READ_BE_UINT32(_dataSrc + offset) == 'FRME') {
			offset += 8;
			if (READ_BE_UINT32(_dataSrc + offset) == 'FOBJ') {
				_offsets[l] = offset + 8;
				offset += READ_BE_UINT32(_dataSrc + offset + 4) + 8;
			}
			else {
				debug(2, "NutRenderer::loadFont() there is no FRME chunk");
				free(_dataSrc);
				return false;
			}
		}
		else {
			debug(2, "NutRenderer::loadFont() there is no FOBJ chunk in FRME chunk");
			free(_dataSrc);
			return false;
		}
	}

	_loaded = true;
	return true;
}

int32 NutRenderer::getCharWidth(char c) {
	debug(2,  "NutRenderer::getCharWidth() called");
	if (_loaded == false) {
		debug(2, "NutRenderer::getCharWidth() Font is not loaded");
		return 0;
	}

	return READ_LE_UINT16(_dataSrc + _offsets[c] + 6);
}

int32 NutRenderer::getCharHeight(char c) {
	debug(2,  "NutRenderer::getCharHeight() called");
	if (_loaded == false) {
		debug(2, "NutRenderer::getCharHeight() Font is not loaded");
		return 0;
	}

	return READ_LE_UINT16(_dataSrc + _offsets[c] + 8);
}

int32 NutRenderer::getStringWidth(char *string) {
	debug(2,  "NutRenderer::getStringWidth() called");
	if (_loaded == false) {
		debug(2, "NutRenderer::getStringWidth() Font is not loaded");
		return 0;
	}
	int32 length = 0;
	int32 l = 0;
	
	do {
		length += getCharWidth(string[l]);
		l++;
	} while (string[l] != 0);

	return length;
}

void NutRenderer::drawString(char *string, int32 x, int32 y, byte color, int32 mode) {
	debug(2,  "NutRenderer::getDrawString() called");
	if (_loaded == false) {
		debug(2, "NutRenderer::getDrawString() Font is not loaded");
		return;
	}

	int32 l = 0;
	do {
		if ((x < 0) || (y < 0) || (x > _dstWidth) || (y > _dstHeight)) {
			debug(2, "NutRenderer::getDrawString() position x, y out of range");
			return;
		}

		drawChar(string[l], x, y, 0);
		x += getCharWidth(string[l]);
		l++;
	} while (string[l] != 0);

}

void NutRenderer::drawChar(char c, int32 x, int32 y, byte color) {
	debug(2,  "NutRenderer::getDrawChar() called");
	if (_loaded == false) {
		debug(2, "NutRenderer::getDrawChar() Font is not loaded");
		return;
	}

	byte * src = (byte*)(_dataSrc + _offsets[c] + 14);
	uint32 length = READ_BE_UINT32(_dataSrc + _offsets[c] - 4) - 14;

	decodeCodec44(_tmpCodecBuffer, src, length);

	int32 width = READ_LE_UINT16(_dataSrc + _offsets[c] + 6);
	int32 height = READ_LE_UINT16(_dataSrc + _offsets[c] + 8);

	for (int32 ty = 0; ty < height; ty++) {
		for (int32 tx = 0; tx < width; tx++) {
			byte pixel = *(_tmpCodecBuffer + ty * width + tx);
			if (pixel != 0) {
				if (color == 0) {
					if (pixel == 0x01)
						pixel = 0xf;
				}
				else {
					if (pixel == 0x01)
						pixel = color;
				}
				if (pixel == 0xff)
					pixel = 0x0;
				*(_dstPtr + ((ty + y) * _dstPitch + x + tx)) = pixel;
			}
		}
	}

}

