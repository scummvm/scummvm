/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"
#include "scumm/scumm.h"
#include "scumm/file.h"
#include "scumm/nut_renderer.h"
#include "scumm/util.h"

namespace Scumm {

NutRenderer::NutRenderer(ScummEngine *vm, const char *filename, bool bitmap) :
	_vm(vm),
	_bitmapFont(bitmap),
	_numChars(0),
	_decodedData(0) {
	memset(_chars, 0, sizeof(_chars));
	loadFont(filename);
}

NutRenderer::~NutRenderer() {
	delete [] _decodedData;
}

void NutRenderer::codec1(byte *dst, const byte *src, int width, int height, int pitch) {
	byte val, code;
	int32 length;
	int h = height, size_line;

	for (h = 0; h < height; h++) {
		size_line = READ_LE_UINT16(src);
		src += 2;
		byte bit = 0x80;
		byte *dstPtrNext = dst + pitch;
		while (size_line > 0) {
			code = *src++;
			size_line--;
			length = (code >> 1) + 1;
			if (code & 1) {
				val = *src++;
				size_line--;
				if (_bitmapFont) {
					for (int i = 0; i < length; i++) {
						if (val)
							*dst |= bit;
						bit >>= 1;
						if (!bit) {
							bit = 0x80;
							dst++;
						}
					}
				} else {
					if (val)
						memset(dst, val, length);
					dst += length;
				}
			} else {
				size_line -= length;
				while (length--) {
					val = *src++;
					if (_bitmapFont) {
						if (val)
							*dst |= bit;
						bit >>= 1;
						if (!bit) {
							bit = 0x80;
							dst++;
						}
					} else {
						if (val)
							*dst = val;
						dst++;
					}
				}
			}
		}
		dst = dstPtrNext;
	}
}

void NutRenderer::codec21(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstPtrNext = dst + pitch;
		const byte *srcPtrNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
		int len = width;
		byte bit = 0x80;
		do {
			int i;
			int offs = READ_LE_UINT16(src); src += 2;
			if (_bitmapFont) {
				for (i = 0; i < offs; i++) {
					bit >>= 1;
					if (!bit) {
						bit = 0x80;
						dst++;
					}
				}
			} else {
				dst += offs;
			}
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
			if (_bitmapFont) {
				for (i = 0; i < w; i++) {
					if (src[i])
						*dst |= bit;
					bit >>= 1;
					if (!bit) {
						bit = 0x80;
						dst++;
					}
				}
			} else {
				memcpy(dst, src, w);
				dst += w;
			}
			src += w;
		} while (len > 0);
		dst = dstPtrNext;
		src = srcPtrNext;
	}
}

void NutRenderer::loadFont(const char *filename) {
	ScummFile file;
	_vm->openFile(file, filename);
	if (!file.isOpen()) {
		error("NutRenderer::loadFont() Can't open font file: %s", filename);
	}

	uint32 tag = file.readUint32BE();
	if (tag != 'ANIM') {
		error("NutRenderer::loadFont() there is no ANIM chunk in font header");
	}

	uint32 length = file.readUint32BE();
	byte *dataSrc = new byte[length];
	file.read(dataSrc, length);
	file.close();

	if (READ_BE_UINT32(dataSrc) != 'AHDR') {
		error("NutRenderer::loadFont() there is no AHDR chunk in font header");
	}

	// We pre-decode the font, which may seem wasteful at first. Actually,
	// the memory needed for just the decoded glyphs is smaller than the
	// whole of the undecoded font file.

	_numChars = READ_LE_UINT16(dataSrc + 10);
	assert(_numChars <= ARRAYSIZE(_chars));

	uint32 offset = 0;
	uint32 decodedLength = 0;
	for (int l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 16;
		int width = READ_LE_UINT16(dataSrc + offset + 14);
		int height = READ_LE_UINT16(dataSrc + offset + 16);
		if (_bitmapFont) {
			decodedLength += (((width + 7) / 8) * height);
		} else {
			decodedLength += (width * height);
		}
	}

	// If characters have transparency, then bytes just get skipped and
	// so there may appear some garbage. That's why we have to fill it
	// with zeroes first.

	_decodedData = new byte[decodedLength];
	memset(_decodedData, 0, decodedLength);

	byte *decodedPtr = _decodedData;

	offset = 0;
	for (int l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 8;
		if (READ_BE_UINT32(dataSrc + offset) != 'FRME') {
			error("NutRenderer::loadFont(%s) there is no FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		offset += 8;
		if (READ_BE_UINT32(dataSrc + offset) != 'FOBJ') {
			error("NutRenderer::loadFont(%s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		int codec = READ_LE_UINT16(dataSrc + offset + 8);
		// _chars[l].xoffs = READ_LE_UINT16(dataSrc + offset + 10);
		// _chars[l].yoffs = READ_LE_UINT16(dataSrc + offset + 12);
		_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
		_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
		_chars[l].src = decodedPtr;

		int pitch;

		if (_bitmapFont) {
			pitch = (_chars[l].width + 7) / 8;
		} else {
			pitch = _chars[l].width;
		}

		decodedPtr += (pitch * _chars[l].height);

		const uint8 *fobjptr = dataSrc + offset + 22;
		switch (codec) {
		case 1:
			codec1(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, pitch);
			break;
		case 21:
		case 44:
			codec21(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, pitch);
			break;
		default:
			error("NutRenderer::loadFont: unknown codec: %d", codec);
		}
	}

	delete [] dataSrc;
}

int NutRenderer::getCharWidth(byte c) const {
	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteWidth / 2;

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharWidth : %d (%d)", c, _numChars);

	return _chars[c].width;
}

int NutRenderer::getCharHeight(byte c) const {
	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteHeight;

	if (c >= _numChars)
		error("invalid character in NutRenderer::getCharHeight : %d (%d)", c, _numChars);

	return _chars[c].height;
}

void NutRenderer::drawShadowChar(const Graphics::Surface &s, int c, int x, int y, byte color, bool showShadow) {

	// We draw the character a total of 7 times: 6 times shifted and in black
	// for the shadow, and once in the right color and position. This way we
	// achieve the exact look as the original CMI had.
	// However, this is not how the original engine handled it. Char glyphs
	// were compressed with codec 44. In the decoding routine, transparent
	// pixels are skipped. Other pixels are just filled with the decoded color
	// which can be equal to 0 (==shadow), 1 (==char color) or another value
	// (255, 224) which is just copied as-is in the destination buffer.

	static const int offsetX[7] = { -1,  0, 1, 0, 1, 2, 0 };
	static const int offsetY[7] = {  0, -1, 0, 1, 2, 1, 0 };
	const int cTable[7] = { 0, 0, 0, 0, 0, 0, color };
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
	assert(!_bitmapFont);

	const int width = MIN((int)_chars[c].width, _vm->_screenWidth - x);
	const int height = MIN((int)_chars[c].height, _vm->_screenHeight - y);
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
	const int width = MIN((int)_chars[c].width, s.w - x);
	const int height = MIN((int)_chars[c].height, s.h - y);
	const byte *src = _chars[c].src;
	int srcPitch;

	if (_bitmapFont) {
		srcPitch = (_chars[c].width + 7) / 8;
	} else {
		srcPitch = _chars[c].width;
	}

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
		int tx;

		for (tx = minX; tx < width; tx++) {
			if (_bitmapFont) {
				if (src[tx / 8] & (0x80 >> (tx % 8))) {
					dst[tx] = color;
				}
			} else {
				if (src[tx] != 0) {
					dst[tx] = color;
				}
			}
		}
		src += srcPitch;
		dst += s.pitch;
	}
}

void NutRenderer::draw2byte(const Graphics::Surface &s, int c, int x, int y, byte color) {
	byte *dst = (byte *)s.pixels + y * s.pitch + x;
	const int width = _vm->_2byteWidth;
	const int height = MIN(_vm->_2byteHeight, s.h - y);
	const byte *src = _vm->get2byteCharPtr(c);
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
