/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "scumm/scumm.h"
#include "scumm/file.h"
#include "scumm/nut_renderer.h"
#include "scumm/util.h"

namespace Scumm {

NutRenderer::NutRenderer(ScummEngine *vm, const char *filename) :
	_vm(vm),
	_numChars(0),
	_maxCharSize(0),
	_fontHeight(0),
	_charBuffer(0),
	_decodedData(0),
	_2byteColorTable(0),
	_2byteShadowXOffsetTable(0),
	_2byteShadowYOffsetTable(0),
	_2byteMainColor(0),
	_spacing(vm->_useCJKMode && vm->_language != Common::JA_JPN ? 1 : 0),
	_2byteSteps(vm->_game.version == 8 ? 4 : 2),
	_direction(vm->_language == Common::HE_ISR ? -1 : 1) {
		static const int8 cjkShadowOffsetsX[4] = { -1, 0, 1, 0 };
		static const int8 cjkShadowOffsetsY[4] = { 0, 1, 0, 0 };
		_2byteShadowXOffsetTable = &cjkShadowOffsetsX[ARRAYSIZE(cjkShadowOffsetsX) - _2byteSteps];
		_2byteShadowYOffsetTable = &cjkShadowOffsetsY[ARRAYSIZE(cjkShadowOffsetsY) - _2byteSteps];
		_2byteColorTable = new uint8[_2byteSteps];
		memset(_2byteColorTable, 0, _2byteSteps);
		_2byteMainColor = &_2byteColorTable[_2byteSteps - 1];
		memset(_chars, 0, sizeof(_chars));
		loadFont(filename);
}

NutRenderer::~NutRenderer() {
	delete[] _charBuffer;
	delete[] _decodedData;
	delete[] _2byteColorTable;
}

void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch);

void NutRenderer::codec1(byte *dst, const byte *src, int width, int height, int pitch) {
	smushDecodeRLE(dst, src, 0, 0, width, height, pitch);
	for (int i = 0; i < width * height; i++)
		_paletteMap[dst[i]] = 1;
}

void NutRenderer::codec21(byte *dst, const byte *src, int width, int height, int pitch) {
	while (height--) {
		byte *dstPtrNext = dst + pitch;
		const byte *srcPtrNext = src + 2 + READ_LE_UINT16(src);
		src += 2;
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
			for (int i = 0; i < w; i++) {
				_paletteMap[src[i]] = 1;
			}
			memcpy(dst, src, w);
			dst += w;
			src += w;
		} while (len > 0);
		dst = dstPtrNext;
		src = srcPtrNext;
	}
}

void NutRenderer::loadFont(const char *filename) {
	ScummFile *file = _vm->instantiateScummFile();

	_vm->openFile(*file, filename);
	if (!file->isOpen()) {
		error("NutRenderer::loadFont() Can't open font file: %s", filename);
	}

	uint32 tag = file->readUint32BE();
	if (tag != MKTAG('A','N','I','M')) {
		error("NutRenderer::loadFont() there is no ANIM chunk in font header");
	}

	uint32 length = file->readUint32BE();
	byte *dataSrc = new byte[length];
	file->read(dataSrc, length);
	file->close();
	delete file;

	if (READ_BE_UINT32(dataSrc) != MKTAG('A','H','D','R')) {
		error("NutRenderer::loadFont() there is no AHDR chunk in font header");
	}

	// We pre-decode the font, which may seem wasteful at first. Actually,
	// the memory needed for just the decoded glyphs is smaller than the
	// whole of the undecoded font file.

	_numChars = READ_LE_UINT16(dataSrc + 10);
	assert(_numChars <= ARRAYSIZE(_chars));

	uint32 offset = 0;
	uint32 decodedLength = 0;
	int l;

	_paletteMap = new byte[256]();

	for (l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 16;
		int width = READ_LE_UINT16(dataSrc + offset + 14);
		_fontHeight = READ_LE_UINT16(dataSrc + offset + 16);
		int size = width * _fontHeight;
		decodedLength += size;
		if (size > _maxCharSize)
			_maxCharSize = size;
	}

	debug(1, "NutRenderer::loadFont('%s') - decodedLength = %d", filename, decodedLength);

	_decodedData = new byte[decodedLength];
	byte *decodedPtr = _decodedData;

	offset = 0;
	for (l = 0; l < _numChars; l++) {
		offset += READ_BE_UINT32(dataSrc + offset + 4) + 8;
		if (READ_BE_UINT32(dataSrc + offset) != MKTAG('F','R','M','E')) {
			error("NutRenderer::loadFont(%s) there is no FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		offset += 8;
		if (READ_BE_UINT32(dataSrc + offset) != MKTAG('F','O','B','J')) {
			error("NutRenderer::loadFont(%s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, l, offset);
			break;
		}
		int codec = READ_LE_UINT16(dataSrc + offset + 8);
		// _chars[l].xoffs = READ_LE_UINT16(dataSrc + offset + 10);
		// _chars[l].yoffs = READ_LE_UINT16(dataSrc + offset + 12);
		_chars[l].width = READ_LE_UINT16(dataSrc + offset + 14);
		_chars[l].height = READ_LE_UINT16(dataSrc + offset + 16);
		_chars[l].src = decodedPtr;

		decodedPtr += (_chars[l].width * _chars[l].height);

		// If characters have transparency, then bytes just get skipped and
		// so there may appear some garbage. That's why we have to fill it
		// with a default color first.
		if (codec == 44) {
			memset(_chars[l].src, kSmush44TransparentColor, _chars[l].width * _chars[l].height);
			_paletteMap[kSmush44TransparentColor] = 1;
			_chars[l].transparency = kSmush44TransparentColor;
		} else {
			memset(_chars[l].src, kDefaultTransparentColor, _chars[l].width * _chars[l].height);
			_paletteMap[kDefaultTransparentColor] = 1;
			_chars[l].transparency = kDefaultTransparentColor;
		}

		const uint8 *fobjptr = dataSrc + offset + 22;
		switch (codec) {
		case 1:
			codec1(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		case 21:
		case 44:
			codec21(_chars[l].src, fobjptr, _chars[l].width, _chars[l].height, _chars[l].width);
			break;
		default:
			error("NutRenderer::loadFont: unknown codec: %d", codec);
		}
	}

	// We have decoded the font. Now let's see if we can re-compress it to
	// a more compact format. Start by counting the number of colors.

	int numColors = 0;
	for (l = 0; l < 256; l++) {
		if (_paletteMap[l]) {
			if (numColors < ARRAYSIZE(_palette)) {
				_paletteMap[l] = numColors;
				_palette[numColors] = l;
			}
			numColors++;
		}
	}

	// Now _palette contains all the used colors, and _paletteMap maps the
	// real color to the palette index.

	if (numColors <= 2)
		_bpp = 1;
	else if (numColors <= 4)
		_bpp = 2;
	else if (numColors <= 16)
		_bpp = 4;
	else
		_bpp = 8;

	if (_bpp < 8) {
		int compressedLength = 0;
		for (l = 0; l < 256; l++) {
			compressedLength += (((_bpp * _chars[l].width + 7) / 8) * _chars[l].height);
		}

		debug(1, "NutRenderer::loadFont('%s') - compressedLength = %d (%d bpp)", filename, compressedLength, _bpp);

		byte *compressedData = new byte[compressedLength]();

		offset = 0;

		for (l = 0; l < 256; l++) {
			byte *src = _chars[l].src;
			byte *dst = compressedData + offset;
			int srcPitch = _chars[l].width;
			int dstPitch = (_bpp * _chars[l].width + 7) / 8;

			for (int h = 0; h < _chars[l].height; h++) {
				byte bit = 0x80;
				byte *nextDst = dst + dstPitch;
				for (int w = 0; w < srcPitch; w++) {
					byte color = _paletteMap[src[w]];
					for (int i = 0; i < _bpp; i++) {
						if (color & (1 << i))
							*dst |= bit;
						bit >>= 1;
					}
					if (!bit) {
						bit = 0x80;
						dst++;
					}
				}
				src += srcPitch;
				dst = nextDst;
			}
			_chars[l].src = compressedData + offset;
			offset += (dstPitch * _chars[l].height);
		}

		delete[] _decodedData;
		_decodedData = compressedData;

		_charBuffer = new byte[_maxCharSize];
	}

	delete[] dataSrc;
	delete[] _paletteMap;
}

int NutRenderer::getCharWidth(byte c) const {
	if (c >= 0x80 && _vm->_useCJKMode)
		return _vm->_2byteWidth + _spacing;

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

byte *NutRenderer::unpackChar(byte c) {
	if (_bpp == 8)
		return _chars[c].src;

	byte *src = _chars[c].src;
	int pitch = (_bpp * _chars[c].width + 7) / 8;

	for (int ty = 0; ty < _chars[c].height; ty++) {
		for (int tx = 0; tx < _chars[c].width; tx++) {
			byte val;
			int offset;
			byte bit;

			switch (_bpp) {
			case 1:
				offset = tx / 8;
				bit = 0x80 >> (tx % 8);
				break;
			case 2:
				offset = tx / 4;
				bit = 0x80 >> (2 * (tx % 4));
				break;
			default:
				offset = tx / 2;
				bit = 0x80 >> (4 * (tx % 2));
				break;
			}

			val = 0;

			for (int i = 0; i < _bpp; i++) {
				if (src[offset] & (bit >> i))
					val |= (1 << i);
			}

			_charBuffer[ty * _chars[c].width + tx] = _palette[val];
		}
		src += pitch;
	}

	return _charBuffer;
}

void NutRenderer::drawFrame(byte *dst, int c, int x, int y) {
	const int width = MIN((int)_chars[c].width, _vm->_screenWidth - x);
	const int height = MIN((int)_chars[c].height, _vm->_screenHeight - y);
	const byte *src = unpackChar(c);
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

int NutRenderer::drawCharV7(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, TextStyleFlags flags, byte chr, bool hardcodedColors, bool smushColorMode) {
	if (_direction < 0)
		x -= _chars[chr].width;

	int width = MIN((int)_chars[chr].width, clipRect.right - x);
	int height = MIN((int)_chars[chr].height, clipRect.bottom - y);
	int minX = x < clipRect.left ? clipRect.left - x : 0;
	int minY = y < clipRect.top ? clipRect.top - y : 0;
	const byte *src = unpackChar(chr);
	byte *dst = buffer + pitch * y + x;

	if (width <= 0 || height <= 0)
		return 0;

	if (minY) {
		src += minY * _chars[chr].width;
		dst += minY * pitch;
	}

	if (minX)
		dst += minX;

	int clipWdth = (_chars[chr].width - width);
	char color = (col != -1) ? col : 1;

	if (_vm->_game.version == 7) {
		if (hardcodedColors) {
			for (int j = minY; j < height; j++) {
				for (int i = minX; i < width; i++) {
					int8 value = *src++;
					if (value != _chars[chr].transparency)
						dst[i] = value;
				}
				src += clipWdth;
				dst += pitch;
			}
		} else {
			for (int j = minY; j < height; j++) {
				for (int i = minX; i < width; i++) {
					int8 value = *src++;
					if (value == 1)
						dst[i] = color;
					else if (value != _chars[chr].transparency)
						dst[i] = 0;
				}
				src += clipWdth;
				dst += pitch;
			}
		}
	} else {
		if (smushColorMode) {
			for (int j = minY; j < height; j++) {
				for (int i = minX; i < width; i++) {
					int8 value = *src++;
					if (value == -color)
						dst[i] = 0xFF;
					else if (value == -31)
						dst[i] = 0;
					else if (value != _chars[chr].transparency)
						dst[i] = value;
				}
				src += clipWdth;
				dst += pitch;
			}
		} else {
			for (int j = minY; j < height; j++) {
				for (int i = minX; i < width; i++) {
					int8 value = *src++;
					if (value != _chars[chr].transparency)
						dst[i] = (value == 1) ? color : value;
				}
				src += clipWdth;
				dst += pitch;
			}
		}
	}
	return _direction * width;
}

int NutRenderer::draw2byte(byte *buffer, Common::Rect &clipRect, int x, int y, int pitch, int16 col, uint16 chr) {
	int width = MIN((int)_vm->_2byteWidth, clipRect.right - x);
	int height = MIN((int)_vm->_2byteHeight, clipRect.bottom - y);
	int minX = x < clipRect.left ? clipRect.left - x : 0;
	int minY = y < clipRect.top ? clipRect.top - y : 0;
	*_2byteMainColor = col;

	if (width <= 0 || height <= 0)
		return 0;

	const byte *src = _vm->get2byteCharPtr(chr);

	if (minY) {
		src += ((minY * _vm->_2byteWidth) >> 3);
		buffer += (minY * pitch);
	}

	if (minX) {
		src += (minX >> 3);
		buffer += minX;
	}

	int clipWdth = (_vm->_2byteWidth - width);
	byte bits = *src;
	const byte *origSrc = src;

	int startFrame = (_2byteSteps == 4 && col == 0) ? _2byteSteps - 1 : 0;

	for (int step = startFrame; step < _2byteSteps; ++step) {
		int offX = MAX<int>(x + _2byteShadowXOffsetTable[step], clipRect.left);
		int offY = MAX<int>(y + _2byteShadowYOffsetTable[step], clipRect.top);
		byte drawColor = _2byteColorTable[step];

		src = origSrc;
		byte *dst = buffer + pitch * offY + offX;

		for (int j = minY; j < height; j++) {
			for (int i = minX; i < width; i++) {
				if (offX + i < 0)
					continue;
				if ((i % 8) == 0)
					bits = *src++;
				if (bits & revBitMask(i % 8))
					dst[i] = drawColor;
			}
			for (int i = width; i < width + clipWdth; ++i) {
				if (i % 8 == 0)
					bits = *src++;
			}
			dst += pitch;
		}
	}

	return width + _spacing;
}

} // End of namespace Scumm
