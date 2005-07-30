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

#include "common/stdafx.h"
#include "scumm/scumm.h"
#include "scumm/nut_renderer.h"
#include "scumm/util.h"

namespace Scumm {

NutRenderer::NutRenderer(ScummEngine *vm) :
	_vm(vm),
	_loaded(false),
	_numChars(0) {
	memset(_chars, 0, sizeof(_chars));
}

NutRenderer::~NutRenderer() {
	for (int i = 0; i < _numChars; i++) {
		delete []_chars[i].src;
	}
}

void smush_decode_codec1(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

static void smush_decode_codec21(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		uint8 *dstPtrNext = dst + pitch;
		const uint8 *srcPtrNext = src + 2 + READ_LE_UINT16(src); src += 2;
		int len = width;
		do {
			int offs = READ_LE_UINT16(src); src += 2;
			dst += offs;
			len -= offs;
			if (len <= 0) {
				break;
			}
			int w = READ_LE_UINT16(src) + 1; src += 2;
			len -= w;
			if (len < 0) {
				w += len;
			}
			// the original codec44 handles this part slightly differently (this is the only difference with codec21) :
			//  src bytes equal to 255 are replaced by 0 in dst
			//  src bytes equal to 1 are replaced by a color passed as an argument in the original function
			//  other src bytes values are copied as-is
			memcpy(dst, src, w); dst += w; src += w;
		} while (len > 0);
		dst = dstPtrNext;
		src = srcPtrNext;
	}
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
	assert(_numChars <= ARRAYSIZE(_chars));
	uint32 offset = 0;
	for (int l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 8;
		if (READ_BE_UINT32(dataSrc + offset) != 'FRME') {
			warning("NutRenderer::loadFont(%s) there is no FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		offset += 8;
		if (READ_BE_UINT32(dataSrc + offset) != 'FOBJ') {
			warning("NutRenderer::loadFont(%s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		int codec = READ_LE_UINT16(dataSrc + offset + 8);
		_chars[l].xoffs = READ_LE_UINT16(dataSrc + offset + 10);
		_chars[l].yoffs = READ_LE_UINT16(dataSrc + offset + 12);
		_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
		_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
		const int srcSize = _chars[l].width * _chars[l].height;
		_chars[l].src = new byte[srcSize];
		// If characters have transparency, then bytes just get skipped and
		// so there may appear some garbage. That's why we have to fill it
		// with zeroes first.
		memset(_chars[l].src, 0, srcSize);

		const uint8 *fobjptr = dataSrc + offset + 22;
		switch (codec) {
		case 1:
			smush_decode_codec1(_chars[l].src, fobjptr, 0, 0, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		case 21:
		case 44:
			smush_decode_codec21(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		default:
			error("NutRenderer::loadFont: unknown codec: %d", codec);
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
