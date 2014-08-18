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

#include "image/codecs/cinepak.h"
#include "image/codecs/cinepak_tables.h"

#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/surface.h"

// Code here partially based off of ffmpeg ;)

namespace Image {

#define PUT_PIXEL(offset, lum, u, v) \
	if (_pixelFormat.bytesPerPixel != 1) { \
		byte r = _clipTable[lum + (v << 1)]; \
		byte g = _clipTable[lum - (u >> 1) - v]; \
		byte b = _clipTable[lum + (u << 1)]; \
		\
		if (_pixelFormat.bytesPerPixel == 2) \
			*((uint16 *)_curFrame.surface->getPixels() + offset) = _pixelFormat.RGBToColor(r, g, b); \
		else \
			*((uint32 *)_curFrame.surface->getPixels() + offset) = _pixelFormat.RGBToColor(r, g, b); \
	} else \
		*((byte *)_curFrame.surface->getPixels() + offset) = lum

CinepakDecoder::CinepakDecoder(int bitsPerPixel) : Codec(), _bitsPerPixel(bitsPerPixel) {
	_curFrame.surface = 0;
	_curFrame.strips = 0;
	_y = 0;
	_colorMap = 0;
	_ditherPalette = 0;

	if (bitsPerPixel == 8) {
		_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	} else {
		_pixelFormat = g_system->getScreenFormat();

		// Default to a 32bpp format, if in 8bpp mode
		if (_pixelFormat.bytesPerPixel == 1)
			_pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0);
	}

	// Create a lookup for the clip function
	// This dramatically improves the performance of the color conversion
	_clipTableBuf = new byte[1024];

	for (uint i = 0; i < 1024; i++) {
		if (i <= 512)
			_clipTableBuf[i] = 0;
		else if (i >= 768)
			_clipTableBuf[i] = 255;
		else
			_clipTableBuf[i] = i - 512;
	}

	_clipTable = _clipTableBuf + 512;
}

CinepakDecoder::~CinepakDecoder() {
	if (_curFrame.surface) {
		_curFrame.surface->free();
		delete _curFrame.surface;
	}

	delete[] _curFrame.strips;
	delete[] _clipTableBuf;

	delete[] _colorMap;
	delete[] _ditherPalette;
}

const Graphics::Surface *CinepakDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	_curFrame.flags = stream.readByte();
	_curFrame.length = (stream.readByte() << 16);
	_curFrame.length |= stream.readUint16BE();
	_curFrame.width = stream.readUint16BE();
	_curFrame.height = stream.readUint16BE();
	_curFrame.stripCount = stream.readUint16BE();

	if (!_curFrame.strips)
		_curFrame.strips = new CinepakStrip[_curFrame.stripCount];

	debug(4, "Cinepak Frame: Width = %d, Height = %d, Strip Count = %d", _curFrame.width, _curFrame.height, _curFrame.stripCount);

	// Borrowed from FFMPEG. This should cut out the extra data Cinepak for Sega has (which is useless).
	// The theory behind this is that this is here to confuse standard Cinepak decoders. But, we won't let that happen! ;)
	if (_curFrame.length != (uint32)stream.size()) {
		if (stream.readUint16BE() == 0xFE00)
			stream.readUint32BE();
		else if ((stream.size() % _curFrame.length) == 0)
			stream.seek(-2, SEEK_CUR);
	}

	if (!_curFrame.surface) {
		_curFrame.surface = new Graphics::Surface();
		_curFrame.surface->create(_curFrame.width, _curFrame.height, _pixelFormat);
	}

	// Reset the y variable.
	_y = 0;

	for (uint16 i = 0; i < _curFrame.stripCount; i++) {
		if (i > 0 && !(_curFrame.flags & 1)) { // Use codebooks from last strip
			for (uint16 j = 0; j < 256; j++) {
				_curFrame.strips[i].v1_codebook[j] = _curFrame.strips[i - 1].v1_codebook[j];
				_curFrame.strips[i].v4_codebook[j] = _curFrame.strips[i - 1].v4_codebook[j];
			}
		}

		_curFrame.strips[i].id = stream.readUint16BE();
		_curFrame.strips[i].length = stream.readUint16BE() - 12; // Subtract the 12 byte header
		_curFrame.strips[i].rect.top = _y; stream.readUint16BE(); // Ignore, substitute with our own.
		_curFrame.strips[i].rect.left = 0; stream.readUint16BE(); // Ignore, substitute with our own
		_curFrame.strips[i].rect.bottom = _y + stream.readUint16BE();
		_curFrame.strips[i].rect.right = _curFrame.width; stream.readUint16BE(); // Ignore, substitute with our own

		// Sanity check. Because Cinepak is based on 4x4 blocks, the width and height of each strip needs to be divisible by 4.
		assert(!(_curFrame.strips[i].rect.width() % 4) && !(_curFrame.strips[i].rect.height() % 4));

		uint32 pos = stream.pos();

		while ((uint32)stream.pos() < (pos + _curFrame.strips[i].length) && !stream.eos()) {
			byte chunkID = stream.readByte();

			if (stream.eos())
				break;

			// Chunk Size is 24-bit, ignore the first 4 bytes
			uint32 chunkSize = stream.readByte() << 16;
			chunkSize += stream.readUint16BE() - 4;

			int32 startPos = stream.pos();

			switch (chunkID) {
			case 0x20:
			case 0x21:
			case 0x24:
			case 0x25:
				loadCodebook(stream, i, 4, chunkID, chunkSize);
				break;
			case 0x22:
			case 0x23:
			case 0x26:
			case 0x27:
				loadCodebook(stream, i, 1, chunkID, chunkSize);
				break;
			case 0x30:
			case 0x31:
			case 0x32:
				if (_ditherPalette)
					ditherVectors(stream, i, chunkID, chunkSize);
				else
					decodeVectors(stream, i, chunkID, chunkSize);
				break;
			default:
				warning("Unknown Cinepak chunk ID %02x", chunkID);
				return _curFrame.surface;
			}

			if (stream.pos() != startPos + (int32)chunkSize)
				stream.seek(startPos + chunkSize);
		}

		_y = _curFrame.strips[i].rect.bottom;
	}

	return _curFrame.surface;
}

void CinepakDecoder::loadCodebook(Common::SeekableReadStream &stream, uint16 strip, byte codebookType, byte chunkID, uint32 chunkSize) {
	CinepakCodebook *codebook = (codebookType == 1) ? _curFrame.strips[strip].v1_codebook : _curFrame.strips[strip].v4_codebook;

	int32 startPos = stream.pos();
	uint32 flag = 0, mask = 0;

	for (uint16 i = 0; i < 256; i++) {
		if ((chunkID & 0x01) && !(mask >>= 1)) {
			if ((stream.pos() - startPos + 4) > (int32)chunkSize)
				break;

			flag  = stream.readUint32BE();
			mask  = 0x80000000;
		}

		if (!(chunkID & 0x01) || (flag & mask)) {
			byte n = (chunkID & 0x04) ? 4 : 6;
			if ((stream.pos() - startPos + n) > (int32)chunkSize)
				break;

			for (byte j = 0; j < 4; j++)
				codebook[i].y[j] = stream.readByte();

			if (n == 6) {
				codebook[i].u = stream.readSByte();
				codebook[i].v = stream.readSByte();
			} else {
				// This codebook type indicates either greyscale or
				// palettized video. For greyscale, default us to
				// 0 for both u and v.
				codebook[i].u = 0;
				codebook[i].v = 0;
			}
		}
	}
}

void CinepakDecoder::decodeVectors(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	uint32 flag = 0, mask = 0;
	uint32 iy[4];
	int32 startPos = stream.pos();

	for (uint16 y = _curFrame.strips[strip].rect.top; y < _curFrame.strips[strip].rect.bottom; y += 4) {
		iy[0] = _curFrame.strips[strip].rect.left + y * _curFrame.width;
		iy[1] = iy[0] + _curFrame.width;
		iy[2] = iy[1] + _curFrame.width;
		iy[3] = iy[2] + _curFrame.width;

		for (uint16 x = _curFrame.strips[strip].rect.left; x < _curFrame.strips[strip].rect.right; x += 4) {
			if ((chunkID & 0x01) && !(mask >>= 1)) {
				if ((stream.pos() - startPos + 4) > (int32)chunkSize)
					return;

				flag  = stream.readUint32BE();
				mask  = 0x80000000;
			}

			if (!(chunkID & 0x01) || (flag & mask)) {
				if (!(chunkID & 0x02) && !(mask >>= 1)) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					flag  = stream.readUint32BE();
					mask  = 0x80000000;
				}

				if ((chunkID & 0x02) || (~flag & mask)) {
					if ((stream.pos() - startPos + 1) > (int32)chunkSize)
						return;

					// Get the codebook
					CinepakCodebook codebook = _curFrame.strips[strip].v1_codebook[stream.readByte()];

					PUT_PIXEL(iy[0] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 1, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 1, codebook.y[0], codebook.u, codebook.v);

					PUT_PIXEL(iy[0] + 2, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 3, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 2, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 3, codebook.y[1], codebook.u, codebook.v);

					PUT_PIXEL(iy[2] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 1, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 1, codebook.y[2], codebook.u, codebook.v);

					PUT_PIXEL(iy[2] + 2, codebook.y[3], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 3, codebook.y[3], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 2, codebook.y[3], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 3, codebook.y[3], codebook.u, codebook.v);
				} else if (flag & mask) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					CinepakCodebook codebook = _curFrame.strips[strip].v4_codebook[stream.readByte()];
					PUT_PIXEL(iy[0] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 1, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 1, codebook.y[3], codebook.u, codebook.v);

					codebook = _curFrame.strips[strip].v4_codebook[stream.readByte()];
					PUT_PIXEL(iy[0] + 2, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[0] + 3, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 2, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[1] + 3, codebook.y[3], codebook.u, codebook.v);

					codebook = _curFrame.strips[strip].v4_codebook[stream.readByte()];
					PUT_PIXEL(iy[2] + 0, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 1, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 0, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 1, codebook.y[3], codebook.u, codebook.v);

					codebook = _curFrame.strips[strip].v4_codebook[stream.readByte()];
					PUT_PIXEL(iy[2] + 2, codebook.y[0], codebook.u, codebook.v);
					PUT_PIXEL(iy[2] + 3, codebook.y[1], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 2, codebook.y[2], codebook.u, codebook.v);
					PUT_PIXEL(iy[3] + 3, codebook.y[3], codebook.u, codebook.v);
				}
			}

			for (byte i = 0; i < 4; i++)
				iy[i] += 4;
		}
	}
}

bool CinepakDecoder::canDither(DitherType type) const {
	return type == kDitherTypeVFW && _bitsPerPixel == 24;
}

void CinepakDecoder::setDither(DitherType type, const byte *palette) {
	assert(canDither(type));

	delete[] _colorMap;
	delete[] _ditherPalette;

	_ditherPalette = new byte[256 * 3];
	memcpy(_ditherPalette, palette, 256 * 3);

	_dirtyPalette = true;
	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	_colorMap = new byte[221];

	for (int i = 0; i < 221; i++)
		_colorMap[i] = findNearestRGB(i);
}

byte CinepakDecoder::findNearestRGB(int index) const {
	int r = s_defaultPalette[index * 3];
	int g = s_defaultPalette[index * 3 + 1];
	int b = s_defaultPalette[index * 3 + 2];

	byte result = 0;
	int diff = 0x7FFFFFFF;

	for (int i = 0; i < 256; i++) {
		int bDiff = b - (int)_ditherPalette[i * 3 + 2];
		int curDiffB = diff - (bDiff * bDiff);

		if (curDiffB > 0) {
			int gDiff = g - (int)_ditherPalette[i * 3 + 1];
			int curDiffG = curDiffB - (gDiff * gDiff);

			if (curDiffG > 0) {
				int rDiff = r - (int)_ditherPalette[i * 3];
				int curDiffR = curDiffG - (rDiff * rDiff);

				if (curDiffR > 0) {
					diff -= curDiffR;
					result = i;

					if (diff == 0)
						break;
				}
			}
		}
	}

	return result;
}

void CinepakDecoder::ditherVectors(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	uint32 flag = 0, mask = 0;
	byte *iy[4];
	int32 startPos = stream.pos();

	for (uint16 y = _curFrame.strips[strip].rect.top; y < _curFrame.strips[strip].rect.bottom; y += 4) {
		iy[0] = (byte *)_curFrame.surface->getPixels() + _curFrame.strips[strip].rect.left + y * _curFrame.width;
		iy[1] = iy[0] + _curFrame.width;
		iy[2] = iy[1] + _curFrame.width;
		iy[3] = iy[2] + _curFrame.width;

		for (uint16 x = _curFrame.strips[strip].rect.left; x < _curFrame.strips[strip].rect.right; x += 4) {
			if ((chunkID & 0x01) && !(mask >>= 1)) {
				if ((stream.pos() - startPos + 4) > (int32)chunkSize)
					return;

				flag  = stream.readUint32BE();
				mask  = 0x80000000;
			}

			if (!(chunkID & 0x01) || (flag & mask)) {
				if (!(chunkID & 0x02) && !(mask >>= 1)) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					flag  = stream.readUint32BE();
					mask  = 0x80000000;
				}

				byte blockBuffer[16];

				if ((chunkID & 0x02) || (~flag & mask)) {
					if ((stream.pos() - startPos + 1) > (int32)chunkSize)
						return;

					ditherCodebookSmooth(_curFrame.strips[strip].v1_codebook[stream.readByte()], blockBuffer);
					iy[0][0] = blockBuffer[0];
					iy[0][1] = blockBuffer[1];
					iy[0][2] = blockBuffer[2];
					iy[0][3] = blockBuffer[3];
					iy[1][0] = blockBuffer[4];
					iy[1][1] = blockBuffer[5];
					iy[1][2] = blockBuffer[6];
					iy[1][3] = blockBuffer[7];
					iy[2][0] = blockBuffer[8];
					iy[2][1] = blockBuffer[9];
					iy[2][2] = blockBuffer[10];
					iy[2][3] = blockBuffer[11];
					iy[3][0] = blockBuffer[12];
					iy[3][1] = blockBuffer[13];
					iy[3][2] = blockBuffer[14];
					iy[3][3] = blockBuffer[15];
				} else if (flag & mask) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					ditherCodebookDetail(_curFrame.strips[strip].v4_codebook[stream.readByte()], blockBuffer);
					iy[0][0] = blockBuffer[0];
					iy[0][1] = blockBuffer[1];
					iy[1][0] = blockBuffer[4];
					iy[1][1] = blockBuffer[5];

					ditherCodebookDetail(_curFrame.strips[strip].v4_codebook[stream.readByte()], blockBuffer);
					iy[0][2] = blockBuffer[2];
					iy[0][3] = blockBuffer[3];
					iy[1][2] = blockBuffer[6];
					iy[1][3] = blockBuffer[7];

					ditherCodebookDetail(_curFrame.strips[strip].v4_codebook[stream.readByte()], blockBuffer);
					iy[2][0] = blockBuffer[8];
					iy[2][1] = blockBuffer[9];
					iy[3][0] = blockBuffer[12];
					iy[3][1] = blockBuffer[13];

					ditherCodebookDetail(_curFrame.strips[strip].v4_codebook[stream.readByte()], blockBuffer);
					iy[2][2] = blockBuffer[10];
					iy[2][3] = blockBuffer[11];
					iy[3][2] = blockBuffer[14];
					iy[3][3] = blockBuffer[15];
				}
			}

			for (byte i = 0; i < 4; i++)
				iy[i] += 4;
		}
	}
}

void CinepakDecoder::ditherCodebookDetail(const CinepakCodebook &codebook, byte *dst) const {
	int uLookup = (byte)codebook.u * 2;
	int vLookup = (byte)codebook.v * 2;
	uint32 uv1 = s_uLookup[uLookup] | s_vLookup[vLookup];
	uint32 uv2 = s_uLookup[uLookup + 1] | s_vLookup[vLookup + 1];

	int yLookup1 = codebook.y[0] * 2;
	int yLookup2 = codebook.y[1] * 2;
	int yLookup3 = codebook.y[2] * 2;
	int yLookup4 = codebook.y[3] * 2;

	uint32 pixelGroup1 = uv2 | s_yLookup[yLookup1 + 1];
	uint32 pixelGroup2 = uv2 | s_yLookup[yLookup2 + 1];
	uint32 pixelGroup3 = uv1 | s_yLookup[yLookup3];
	uint32 pixelGroup4 = uv1 | s_yLookup[yLookup4];
	uint32 pixelGroup5 = uv1 | s_yLookup[yLookup1];
	uint32 pixelGroup6 = uv1 | s_yLookup[yLookup2];
	uint32 pixelGroup7 = uv2 | s_yLookup[yLookup3 + 1];
	uint32 pixelGroup8 = uv2 | s_yLookup[yLookup4 + 1];

	dst[0] = getRGBLookupEntry(pixelGroup1 & 0xFFFF);
	dst[1] = getRGBLookupEntry(pixelGroup2 >> 16);
	dst[2] = getRGBLookupEntry(pixelGroup5 & 0xFFFF);
	dst[3] = getRGBLookupEntry(pixelGroup6 >> 16);
	dst[4] = getRGBLookupEntry(pixelGroup3 & 0xFFFF);
	dst[5] = getRGBLookupEntry(pixelGroup4 >> 16);
	dst[6] = getRGBLookupEntry(pixelGroup7 & 0xFFFF);
	dst[7] = getRGBLookupEntry(pixelGroup8 >> 16);
	dst[8] = getRGBLookupEntry(pixelGroup1 >> 16);
	dst[9] = getRGBLookupEntry(pixelGroup6 & 0xFFFF);
	dst[10] = getRGBLookupEntry(pixelGroup5 >> 16);
	dst[11] = getRGBLookupEntry(pixelGroup2 & 0xFFFF);
	dst[12] = getRGBLookupEntry(pixelGroup3 >> 16);
	dst[13] = getRGBLookupEntry(pixelGroup8 & 0xFFFF);
	dst[14] = getRGBLookupEntry(pixelGroup7 >> 16);
	dst[15] = getRGBLookupEntry(pixelGroup4 & 0xFFFF);
}

void CinepakDecoder::ditherCodebookSmooth(const CinepakCodebook &codebook, byte *dst) const {
	int uLookup = (byte)codebook.u * 2;
	int vLookup = (byte)codebook.v * 2;
	uint32 uv1 = s_uLookup[uLookup] | s_vLookup[vLookup];
	uint32 uv2 = s_uLookup[uLookup + 1] | s_vLookup[vLookup + 1];

	int yLookup1 = codebook.y[0] * 2;
	int yLookup2 = codebook.y[1] * 2;
	int yLookup3 = codebook.y[2] * 2;
	int yLookup4 = codebook.y[3] * 2;

	uint32 pixelGroup1 = uv2 | s_yLookup[yLookup1 + 1];
	uint32 pixelGroup2 = uv1 | s_yLookup[yLookup2];
	uint32 pixelGroup3 = uv1 | s_yLookup[yLookup1];
	uint32 pixelGroup4 = uv2 | s_yLookup[yLookup2 + 1];
	uint32 pixelGroup5 = uv2 | s_yLookup[yLookup3 + 1];
	uint32 pixelGroup6 = uv1 | s_yLookup[yLookup3];
	uint32 pixelGroup7 = uv1 | s_yLookup[yLookup4];
	uint32 pixelGroup8 = uv2 | s_yLookup[yLookup4 + 1];

	dst[0] = getRGBLookupEntry(pixelGroup1 & 0xFFFF);
	dst[1] = getRGBLookupEntry(pixelGroup1 >> 16);
	dst[2] = getRGBLookupEntry(pixelGroup2 & 0xFFFF);
	dst[3] = getRGBLookupEntry(pixelGroup2 >> 16);
	dst[4] = getRGBLookupEntry(pixelGroup3 & 0xFFFF);
	dst[5] = getRGBLookupEntry(pixelGroup3 >> 16);
	dst[6] = getRGBLookupEntry(pixelGroup4 & 0xFFFF);
	dst[7] = getRGBLookupEntry(pixelGroup4 >> 16);
	dst[8] = getRGBLookupEntry(pixelGroup5 >> 16);
	dst[9] = getRGBLookupEntry(pixelGroup6 & 0xFFFF);
	dst[10] = getRGBLookupEntry(pixelGroup7 >> 16);
	dst[11] = getRGBLookupEntry(pixelGroup8 & 0xFFFF);
	dst[12] = getRGBLookupEntry(pixelGroup6 >> 16);
	dst[13] = getRGBLookupEntry(pixelGroup5 & 0xFFFF);
	dst[14] = getRGBLookupEntry(pixelGroup8 >> 16);
	dst[15] = getRGBLookupEntry(pixelGroup7 & 0xFFFF);
}

byte CinepakDecoder::getRGBLookupEntry(uint16 index) const {
	return _colorMap[s_defaultPaletteLookup[CLIP<int>(index, 0, 1024)]];
}

} // End of namespace Image
