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

namespace {

inline void convertYUVToRGB(const byte *clipTable, byte y, int8 u, int8 v, byte &r, byte &g, byte &b) {
	r = clipTable[y + (v << 1)];
	g = clipTable[y - (u >> 1) - v];
	b = clipTable[y + (u << 1)];
}

inline uint32 convertYUVToColor(const byte *clipTable, const Graphics::PixelFormat &format, byte y, byte u, byte v) {
	byte r, g, b;
	convertYUVToRGB(clipTable, y, u, v, r, g, b);
	return format.RGBToColor(r, g, b);
}

inline uint16 createDitherTableIndex(const byte *clipTable, byte y, int8 u, int8 v) {
	byte r, g, b;
	convertYUVToRGB(clipTable, y, u, v, r, g, b);
	return ((r & 0xF8) << 6) |
	       ((g & 0xF8) << 1) |
	       ((b & 0xF0) >> 4);
}

/**
 * Put a raw pixel to the destination surface
 */
template<typename PixelInt>
inline void putPixelRaw(PixelInt *dst, const byte *clipTable, const Graphics::PixelFormat &format, byte y, byte u, byte v) {
	*dst = convertYUVToColor(clipTable, format, y, u, v);
}

/**
 * Specialized putPixelRaw for palettized 8bpp output
 */
template<>
inline void putPixelRaw(byte *dst, const byte *clipTable, const Graphics::PixelFormat &format, byte y, byte u, byte v) {
	*dst = y;
}

/**
 * The default codebook converter: raw output.
 */
struct CodebookConverterRaw {
	template<typename PixelInt>
	static inline void decodeBlock1(byte codebookIndex, const CinepakStrip &strip, PixelInt *(&rows)[4], const byte *clipTable, const byte *colorMap, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook = strip.v1_codebook[codebookIndex];
		putPixelRaw(rows[0] + 0, clipTable, format, codebook.y[0], codebook.u, codebook.v);
		putPixelRaw(rows[0] + 1, clipTable, format, codebook.y[0], codebook.u, codebook.v);
		putPixelRaw(rows[1] + 0, clipTable, format, codebook.y[0], codebook.u, codebook.v);
		putPixelRaw(rows[1] + 1, clipTable, format, codebook.y[0], codebook.u, codebook.v);

		putPixelRaw(rows[0] + 2, clipTable, format, codebook.y[1], codebook.u, codebook.v);
		putPixelRaw(rows[0] + 3, clipTable, format, codebook.y[1], codebook.u, codebook.v);
		putPixelRaw(rows[1] + 2, clipTable, format, codebook.y[1], codebook.u, codebook.v);
		putPixelRaw(rows[1] + 3, clipTable, format, codebook.y[1], codebook.u, codebook.v);

		putPixelRaw(rows[2] + 0, clipTable, format, codebook.y[2], codebook.u, codebook.v);
		putPixelRaw(rows[2] + 1, clipTable, format, codebook.y[2], codebook.u, codebook.v);
		putPixelRaw(rows[3] + 0, clipTable, format, codebook.y[2], codebook.u, codebook.v);
		putPixelRaw(rows[3] + 1, clipTable, format, codebook.y[2], codebook.u, codebook.v);

		putPixelRaw(rows[2] + 2, clipTable, format, codebook.y[3], codebook.u, codebook.v);
		putPixelRaw(rows[2] + 3, clipTable, format, codebook.y[3], codebook.u, codebook.v);
		putPixelRaw(rows[3] + 2, clipTable, format, codebook.y[3], codebook.u, codebook.v);
		putPixelRaw(rows[3] + 3, clipTable, format, codebook.y[3], codebook.u, codebook.v);
	}

	template<typename PixelInt>
	static inline void decodeBlock4(const byte (&codebookIndex)[4], const CinepakStrip &strip, PixelInt *(&rows)[4], const byte *clipTable, const byte *colorMap, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook1 = strip.v4_codebook[codebookIndex[0]];
		putPixelRaw(rows[0] + 0, clipTable, format, codebook1.y[0], codebook1.u, codebook1.v);
		putPixelRaw(rows[0] + 1, clipTable, format, codebook1.y[1], codebook1.u, codebook1.v);
		putPixelRaw(rows[1] + 0, clipTable, format, codebook1.y[2], codebook1.u, codebook1.v);
		putPixelRaw(rows[1] + 1, clipTable, format, codebook1.y[3], codebook1.u, codebook1.v);

		const CinepakCodebook &codebook2 = strip.v4_codebook[codebookIndex[1]];
		putPixelRaw(rows[0] + 2, clipTable, format, codebook2.y[0], codebook2.u, codebook2.v);
		putPixelRaw(rows[0] + 3, clipTable, format, codebook2.y[1], codebook2.u, codebook2.v);
		putPixelRaw(rows[1] + 2, clipTable, format, codebook2.y[2], codebook2.u, codebook2.v);
		putPixelRaw(rows[1] + 3, clipTable, format, codebook2.y[3], codebook2.u, codebook2.v);

		const CinepakCodebook &codebook3 = strip.v4_codebook[codebookIndex[2]];
		putPixelRaw(rows[2] + 0, clipTable, format, codebook3.y[0], codebook3.u, codebook3.v);
		putPixelRaw(rows[2] + 1, clipTable, format, codebook3.y[1], codebook3.u, codebook3.v);
		putPixelRaw(rows[3] + 0, clipTable, format, codebook3.y[2], codebook3.u, codebook3.v);
		putPixelRaw(rows[3] + 1, clipTable, format, codebook3.y[3], codebook3.u, codebook3.v);

		const CinepakCodebook &codebook4 = strip.v4_codebook[codebookIndex[3]];
		putPixelRaw(rows[2] + 2, clipTable, format, codebook4.y[0], codebook4.u, codebook4.v);
		putPixelRaw(rows[2] + 3, clipTable, format, codebook4.y[1], codebook4.u, codebook4.v);
		putPixelRaw(rows[3] + 2, clipTable, format, codebook4.y[2], codebook4.u, codebook4.v);
		putPixelRaw(rows[3] + 3, clipTable, format, codebook4.y[3], codebook4.u, codebook4.v);
	}
};

/**
 * Codebook converter that dithers in VFW-style
 */
struct CodebookConverterDitherVFW {
	static inline void decodeBlock1(byte codebookIndex, const CinepakStrip &strip, byte *(&rows)[4], const byte *clipTable, const byte *colorMap, const Graphics::PixelFormat &format) {
		const CinepakCodebook &codebook = strip.v1_codebook[codebookIndex];
		byte blockBuffer[16];
		ditherCodebookSmooth(codebook, blockBuffer, colorMap);
		rows[0][0] = blockBuffer[0];
		rows[0][1] = blockBuffer[1];
		rows[0][2] = blockBuffer[2];
		rows[0][3] = blockBuffer[3];
		rows[1][0] = blockBuffer[4];
		rows[1][1] = blockBuffer[5];
		rows[1][2] = blockBuffer[6];
		rows[1][3] = blockBuffer[7];
		rows[2][0] = blockBuffer[8];
		rows[2][1] = blockBuffer[9];
		rows[2][2] = blockBuffer[10];
		rows[2][3] = blockBuffer[11];
		rows[3][0] = blockBuffer[12];
		rows[3][1] = blockBuffer[13];
		rows[3][2] = blockBuffer[14];
		rows[3][3] = blockBuffer[15];
	}

	static inline void decodeBlock4(const byte (&codebookIndex)[4], const CinepakStrip &strip, byte *(&rows)[4], const byte *clipTable, const byte *colorMap, const Graphics::PixelFormat &format) {
		byte blockBuffer[16];
		ditherCodebookDetail(strip.v4_codebook[codebookIndex[0]], blockBuffer, colorMap);
		rows[0][0] = blockBuffer[0];
		rows[0][1] = blockBuffer[1];
		rows[1][0] = blockBuffer[4];
		rows[1][1] = blockBuffer[5];

		ditherCodebookDetail(strip.v4_codebook[codebookIndex[1]], blockBuffer, colorMap);
		rows[0][2] = blockBuffer[2];
		rows[0][3] = blockBuffer[3];
		rows[1][2] = blockBuffer[6];
		rows[1][3] = blockBuffer[7];

		ditherCodebookDetail(strip.v4_codebook[codebookIndex[2]], blockBuffer, colorMap);
		rows[2][0] = blockBuffer[8];
		rows[2][1] = blockBuffer[9];
		rows[3][0] = blockBuffer[12];
		rows[3][1] = blockBuffer[13];

		ditherCodebookDetail(strip.v4_codebook[codebookIndex[3]], blockBuffer, colorMap);
		rows[2][2] = blockBuffer[10];
		rows[2][3] = blockBuffer[11];
		rows[3][2] = blockBuffer[14];
		rows[3][3] = blockBuffer[15];
	}

private:
	static inline void ditherCodebookDetail(const CinepakCodebook &codebook, byte *dst, const byte *colorMap) {
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

		dst[0] = getRGBLookupEntry(colorMap, pixelGroup1 & 0xFFFF);
		dst[1] = getRGBLookupEntry(colorMap, pixelGroup2 >> 16);
		dst[2] = getRGBLookupEntry(colorMap, pixelGroup5 & 0xFFFF);
		dst[3] = getRGBLookupEntry(colorMap, pixelGroup6 >> 16);
		dst[4] = getRGBLookupEntry(colorMap, pixelGroup3 & 0xFFFF);
		dst[5] = getRGBLookupEntry(colorMap, pixelGroup4 >> 16);
		dst[6] = getRGBLookupEntry(colorMap, pixelGroup7 & 0xFFFF);
		dst[7] = getRGBLookupEntry(colorMap, pixelGroup8 >> 16);
		dst[8] = getRGBLookupEntry(colorMap, pixelGroup1 >> 16);
		dst[9] = getRGBLookupEntry(colorMap, pixelGroup6 & 0xFFFF);
		dst[10] = getRGBLookupEntry(colorMap, pixelGroup5 >> 16);
		dst[11] = getRGBLookupEntry(colorMap, pixelGroup2 & 0xFFFF);
		dst[12] = getRGBLookupEntry(colorMap, pixelGroup3 >> 16);
		dst[13] = getRGBLookupEntry(colorMap, pixelGroup8 & 0xFFFF);
		dst[14] = getRGBLookupEntry(colorMap, pixelGroup7 >> 16);
		dst[15] = getRGBLookupEntry(colorMap, pixelGroup4 & 0xFFFF);
	}

	static inline void ditherCodebookSmooth(const CinepakCodebook &codebook, byte *dst, const byte *colorMap) {
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

		dst[0] = getRGBLookupEntry(colorMap, pixelGroup1 & 0xFFFF);
		dst[1] = getRGBLookupEntry(colorMap, pixelGroup1 >> 16);
		dst[2] = getRGBLookupEntry(colorMap, pixelGroup2 & 0xFFFF);
		dst[3] = getRGBLookupEntry(colorMap, pixelGroup2 >> 16);
		dst[4] = getRGBLookupEntry(colorMap, pixelGroup3 & 0xFFFF);
		dst[5] = getRGBLookupEntry(colorMap, pixelGroup3 >> 16);
		dst[6] = getRGBLookupEntry(colorMap, pixelGroup4 & 0xFFFF);
		dst[7] = getRGBLookupEntry(colorMap, pixelGroup4 >> 16);
		dst[8] = getRGBLookupEntry(colorMap, pixelGroup5 >> 16);
		dst[9] = getRGBLookupEntry(colorMap, pixelGroup6 & 0xFFFF);
		dst[10] = getRGBLookupEntry(colorMap, pixelGroup7 >> 16);
		dst[11] = getRGBLookupEntry(colorMap, pixelGroup8 & 0xFFFF);
		dst[12] = getRGBLookupEntry(colorMap, pixelGroup6 >> 16);
		dst[13] = getRGBLookupEntry(colorMap, pixelGroup5 & 0xFFFF);
		dst[14] = getRGBLookupEntry(colorMap, pixelGroup8 >> 16);
		dst[15] = getRGBLookupEntry(colorMap, pixelGroup7 & 0xFFFF);
	}

	static inline byte getRGBLookupEntry(const byte *colorMap, uint16 index) {
		return colorMap[s_defaultPaletteLookup[CLIP<int>(index, 0, 1023)]];
	}
};

/**
 * Codebook converter that dithers in QT-style
 */
struct CodebookConverterDitherQT {
	static inline void decodeBlock1(byte codebookIndex, const CinepakStrip &strip, byte *(&rows)[4], const byte *clipTable, const byte *colorMap, const Graphics::PixelFormat &format) {
		const byte *colorPtr = strip.v1_dither + (codebookIndex << 2);
		WRITE_UINT32(rows[0], READ_UINT32(colorPtr));
		WRITE_UINT32(rows[1], READ_UINT32(colorPtr + 1024));
		WRITE_UINT32(rows[2], READ_UINT32(colorPtr + 2048));
		WRITE_UINT32(rows[3], READ_UINT32(colorPtr + 3072));
	}

	static inline void decodeBlock4(const byte (&codebookIndex)[4], const CinepakStrip &strip, byte *(&rows)[4], const byte *clipTable, const byte *colorMap, const Graphics::PixelFormat &format) {
		const byte *colorPtr = strip.v4_dither + (codebookIndex[0] << 2);
		WRITE_UINT16(rows[0] + 0, READ_UINT16(colorPtr + 0));
		WRITE_UINT16(rows[1] + 0, READ_UINT16(colorPtr + 2));

		colorPtr = strip.v4_dither + (codebookIndex[1] << 2);
		WRITE_UINT16(rows[0] + 2, READ_UINT16(colorPtr + 1024));
		WRITE_UINT16(rows[1] + 2, READ_UINT16(colorPtr + 1026));

		colorPtr = strip.v4_dither + (codebookIndex[2] << 2);
		WRITE_UINT16(rows[2] + 0, READ_UINT16(colorPtr + 2048));
		WRITE_UINT16(rows[3] + 0, READ_UINT16(colorPtr + 2050));

		colorPtr = strip.v4_dither + (codebookIndex[3] << 2);
		WRITE_UINT16(rows[2] + 2, READ_UINT16(colorPtr + 3072));
		WRITE_UINT16(rows[3] + 2, READ_UINT16(colorPtr + 3074));
	}
};

template<typename PixelInt, typename CodebookConverter>
void decodeVectorsTmpl(CinepakFrame &frame, const byte *clipTable, const byte *colorMap, Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	uint32 flag = 0, mask = 0;
	PixelInt *iy[4];
	int32 startPos = stream.pos();

	for (uint16 y = frame.strips[strip].rect.top; y < frame.strips[strip].rect.bottom; y += 4) {
		iy[0] = (PixelInt *)frame.surface->getBasePtr(frame.strips[strip].rect.left, + y);
		iy[1] = iy[0] + frame.width;
		iy[2] = iy[1] + frame.width;
		iy[3] = iy[2] + frame.width;

		for (uint16 x = frame.strips[strip].rect.left; x < frame.strips[strip].rect.right; x += 4) {
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
					byte codebook = stream.readByte();
					CodebookConverter::decodeBlock1(codebook, frame.strips[strip], iy, clipTable, colorMap, frame.surface->format);
				} else if (flag & mask) {
					if ((stream.pos() - startPos + 4) > (int32)chunkSize)
						return;

					byte codebook[4];
					stream.read(codebook, 4);
					CodebookConverter::decodeBlock4(codebook, frame.strips[strip], iy, clipTable, colorMap, frame.surface->format);
				}
			}

			for (byte i = 0; i < 4; i++)
				iy[i] += 4;
		}
	}
}

} // End of anonymous namespace

CinepakDecoder::CinepakDecoder(int bitsPerPixel) : Codec(), _bitsPerPixel(bitsPerPixel) {
	_curFrame.surface = 0;
	_curFrame.strips = 0;
	_y = 0;
	_colorMap = 0;
	_ditherPalette = 0;
	_ditherType = kDitherTypeUnknown;

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

	if (!_curFrame.strips) {
		_curFrame.strips = new CinepakStrip[_curFrame.stripCount];
		for (uint16 i = 0; i < _curFrame.stripCount; i++) {
			initializeCodebook(i, 1);
			initializeCodebook(i, 4);
		}
	}

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

	_y = 0;

	for (uint16 i = 0; i < _curFrame.stripCount; i++) {
		if (i > 0 && !(_curFrame.flags & 1)) { // Use codebooks from last strip

			for (uint16 j = 0; j < 256; j++) {
				_curFrame.strips[i].v1_codebook[j] = _curFrame.strips[i - 1].v1_codebook[j];
				_curFrame.strips[i].v4_codebook[j] = _curFrame.strips[i - 1].v4_codebook[j];
			}

			// Copy the QuickTime dither tables
			memcpy(_curFrame.strips[i].v1_dither, _curFrame.strips[i - 1].v1_dither, 256 * 4 * 4 * 4);
			memcpy(_curFrame.strips[i].v4_dither, _curFrame.strips[i - 1].v4_dither, 256 * 4 * 4 * 4);
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

void CinepakDecoder::initializeCodebook(uint16 strip, byte codebookType) {
	CinepakCodebook *codebook = (codebookType == 1) ? _curFrame.strips[strip].v1_codebook : _curFrame.strips[strip].v4_codebook;

	for (uint16 i = 0; i < 256; i++) {
		memset(codebook[i].y, 0, 4);
		codebook[i].u = 0;
		codebook[i].v = 0;

		if (_ditherType == kDitherTypeQT)
			ditherCodebookQT(strip, codebookType, i);
	}
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

			stream.read(codebook[i].y, 4);

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

			// Dither the codebook if we're dithering for QuickTime
			if (_ditherType == kDitherTypeQT)
				ditherCodebookQT(strip, codebookType, i);
		}
	}
}

void CinepakDecoder::ditherCodebookQT(uint16 strip, byte codebookType, uint16 codebookIndex) {
	if (codebookType == 1) {
		const CinepakCodebook &codebook = _curFrame.strips[strip].v1_codebook[codebookIndex];
		byte *output = _curFrame.strips[strip].v1_dither + (codebookIndex << 2);

		byte *ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[0], codebook.u, codebook.v);
		output[0x000] = ditherEntry[0x0000];
		output[0x001] = ditherEntry[0x4000];
		output[0x400] = ditherEntry[0xC000];
		output[0x401] = ditherEntry[0x0000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[1], codebook.u, codebook.v);
		output[0x002] = ditherEntry[0x8000];
		output[0x003] = ditherEntry[0xC000];
		output[0x402] = ditherEntry[0x4000];
		output[0x403] = ditherEntry[0x8000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[2], codebook.u, codebook.v);
		output[0x800] = ditherEntry[0x4000];
		output[0x801] = ditherEntry[0x8000];
		output[0xC00] = ditherEntry[0x8000];
		output[0xC01] = ditherEntry[0xC000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[3], codebook.u, codebook.v);
		output[0x802] = ditherEntry[0xC000];
		output[0x803] = ditherEntry[0x0000];
		output[0xC02] = ditherEntry[0x0000];
		output[0xC03] = ditherEntry[0x4000];
	} else {
		const CinepakCodebook &codebook = _curFrame.strips[strip].v4_codebook[codebookIndex];
		byte *output = _curFrame.strips[strip].v4_dither + (codebookIndex << 2);

		byte *ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[0], codebook.u, codebook.v);
		output[0x000] = ditherEntry[0x0000];
		output[0x400] = ditherEntry[0x8000];
		output[0x800] = ditherEntry[0x4000];
		output[0xC00] = ditherEntry[0xC000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[1], codebook.u, codebook.v);
		output[0x001] = ditherEntry[0x4000];
		output[0x401] = ditherEntry[0xC000];
		output[0x801] = ditherEntry[0x8000];
		output[0xC01] = ditherEntry[0x0000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[2], codebook.u, codebook.v);
		output[0x002] = ditherEntry[0xC000];
		output[0x402] = ditherEntry[0x4000];
		output[0x802] = ditherEntry[0x8000];
		output[0xC02] = ditherEntry[0x0000];

		ditherEntry = _colorMap + createDitherTableIndex(_clipTable, codebook.y[3], codebook.u, codebook.v);
		output[0x003] = ditherEntry[0x0000];
		output[0x403] = ditherEntry[0x8000];
		output[0x803] = ditherEntry[0xC000];
		output[0xC03] = ditherEntry[0x4000];
	}
}

void CinepakDecoder::decodeVectors(Common::SeekableReadStream &stream, uint16 strip, byte chunkID, uint32 chunkSize) {
	if (_curFrame.surface->format.bytesPerPixel == 1) {
		decodeVectorsTmpl<byte, CodebookConverterRaw>(_curFrame, _clipTable, _colorMap, stream, strip, chunkID, chunkSize);
	} else if (_curFrame.surface->format.bytesPerPixel == 2) {
		decodeVectorsTmpl<uint16, CodebookConverterRaw>(_curFrame, _clipTable, _colorMap, stream, strip, chunkID, chunkSize);
	} else if (_curFrame.surface->format.bytesPerPixel == 4) {
		decodeVectorsTmpl<uint32, CodebookConverterRaw>(_curFrame, _clipTable, _colorMap, stream, strip, chunkID, chunkSize);
	}
}

bool CinepakDecoder::canDither(DitherType type) const {
	return (type == kDitherTypeVFW || type == kDitherTypeQT) && _bitsPerPixel == 24;
}

void CinepakDecoder::setDither(DitherType type, const byte *palette) {
	assert(canDither(type));

	delete[] _colorMap;
	delete[] _ditherPalette;

	_ditherPalette = new byte[256 * 3];
	memcpy(_ditherPalette, palette, 256 * 3);

	_dirtyPalette = true;
	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	_ditherType = type;

	if (type == kDitherTypeVFW) {
		_colorMap = new byte[221];

		for (int i = 0; i < 221; i++)
			_colorMap[i] = findNearestRGB(i);
	} else {
		// Generate QuickTime dither table
		// 4 blocks of 0x4000 bytes (RGB554 lookup)
		_colorMap = createQuickTimeDitherTable(palette, 256);
	}
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
	if (_ditherType == kDitherTypeVFW)
		decodeVectorsTmpl<byte, CodebookConverterDitherVFW>(_curFrame, _clipTable, _colorMap, stream, strip, chunkID, chunkSize);
	else
		decodeVectorsTmpl<byte, CodebookConverterDitherQT>(_curFrame, _clipTable, _colorMap, stream, strip, chunkID, chunkSize);
}

} // End of namespace Image
