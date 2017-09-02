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

 // Based off ffmpeg's RPZA decoder

#include "image/codecs/rpza.h"

#include "common/debug.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Image {

RPZADecoder::RPZADecoder(uint16 width, uint16 height) : Codec() {
	_format = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	_ditherPalette = 0;
	_dirtyPalette = false;
	_colorMap = 0;
	_width = width;
	_height = height;
	_blockWidth = (width + 3) / 4;
	_blockHeight = (height + 3) / 4;
	_surface = 0;
}

RPZADecoder::~RPZADecoder() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}

	delete[] _ditherPalette;
	delete[] _colorMap;
}

#define ADVANCE_BLOCK() \
	blockPtr += 4; \
	if (blockPtr >= endPtr) { \
		blockPtr += pitch * 3; \
		endPtr = blockPtr + pitch; \
	} \
	totalBlocks--; \
	if (totalBlocks < 0) \
		error("rpza block counter just went negative (this should not happen)") \

struct BlockDecoderRaw {
	static inline void drawFillBlock(uint16 *blockPtr, uint16 pitch, uint16 color, const byte *colorMap) {
		blockPtr[0] = color;
		blockPtr[1] = color;
		blockPtr[2] = color;
		blockPtr[3] = color;
		blockPtr += pitch;
		blockPtr[0] = color;
		blockPtr[1] = color;
		blockPtr[2] = color;
		blockPtr[3] = color;
		blockPtr += pitch;
		blockPtr[0] = color;
		blockPtr[1] = color;
		blockPtr[2] = color;
		blockPtr[3] = color;
		blockPtr += pitch;
		blockPtr[0] = color;
		blockPtr[1] = color;
		blockPtr[2] = color;
		blockPtr[3] = color;
	}

	static inline void drawRawBlock(uint16 *blockPtr, uint16 pitch, const uint16 (&colors)[16], const byte *colorMap) {
		blockPtr[0] = colors[0];
		blockPtr[1] = colors[1];
		blockPtr[2] = colors[2];
		blockPtr[3] = colors[3];
		blockPtr += pitch;
		blockPtr[0] = colors[4];
		blockPtr[1] = colors[5];
		blockPtr[2] = colors[6];
		blockPtr[3] = colors[7];
		blockPtr += pitch;
		blockPtr[0] = colors[8];
		blockPtr[1] = colors[9];
		blockPtr[2] = colors[10];
		blockPtr[3] = colors[11];
		blockPtr += pitch;
		blockPtr[0] = colors[12];
		blockPtr[1] = colors[13];
		blockPtr[2] = colors[14];
		blockPtr[3] = colors[15];
	}

	static inline void drawBlendBlock(uint16 *blockPtr, uint16 pitch, const uint16 (&colors)[4], const byte (&indexes)[4], const byte *colorMap) {
		blockPtr[0] = colors[(indexes[0] >> 6) & 0x03];
		blockPtr[1] = colors[(indexes[0] >> 4) & 0x03];
		blockPtr[2] = colors[(indexes[0] >> 2) & 0x03];
		blockPtr[3] = colors[(indexes[0] >> 0) & 0x03];
		blockPtr += pitch;
		blockPtr[0] = colors[(indexes[1] >> 6) & 0x03];
		blockPtr[1] = colors[(indexes[1] >> 4) & 0x03];
		blockPtr[2] = colors[(indexes[1] >> 2) & 0x03];
		blockPtr[3] = colors[(indexes[1] >> 0) & 0x03];
		blockPtr += pitch;
		blockPtr[0] = colors[(indexes[2] >> 6) & 0x03];
		blockPtr[1] = colors[(indexes[2] >> 4) & 0x03];
		blockPtr[2] = colors[(indexes[2] >> 2) & 0x03];
		blockPtr[3] = colors[(indexes[2] >> 0) & 0x03];
		blockPtr += pitch;
		blockPtr[0] = colors[(indexes[3] >> 6) & 0x03];
		blockPtr[1] = colors[(indexes[3] >> 4) & 0x03];
		blockPtr[2] = colors[(indexes[3] >> 2) & 0x03];
		blockPtr[3] = colors[(indexes[3] >> 0) & 0x03];
	}
};

struct BlockDecoderDither {
	static inline void drawFillBlock(byte *blockPtr, uint16 pitch, uint16 color, const byte *colorMap) {
		const byte *mapOffset = colorMap + (color >> 1);
		byte pixel1 = mapOffset[0x0000];
		byte pixel2 = mapOffset[0x4000];
		byte pixel3 = mapOffset[0x8000];
		byte pixel4 = mapOffset[0xC000];

		blockPtr[0] = pixel1;
		blockPtr[1] = pixel2;
		blockPtr[2] = pixel3;
		blockPtr[3] = pixel4;
		blockPtr += pitch;
		blockPtr[0] = pixel4;
		blockPtr[1] = pixel1;
		blockPtr[2] = pixel2;
		blockPtr[3] = pixel3;
		blockPtr += pitch;
		blockPtr[0] = pixel2;
		blockPtr[1] = pixel3;
		blockPtr[2] = pixel4;
		blockPtr[3] = pixel1;
		blockPtr += pitch;
		blockPtr[0] = pixel3;
		blockPtr[1] = pixel4;
		blockPtr[2] = pixel1;
		blockPtr[3] = pixel2;
	}

	static inline void drawRawBlock(byte *blockPtr, uint16 pitch, const uint16 (&colors)[16], const byte *colorMap) {
		blockPtr[0] = colorMap[(colors[0] >> 1) + 0x0000];
		blockPtr[1] = colorMap[(colors[1] >> 1) + 0x4000];
		blockPtr[2] = colorMap[(colors[2] >> 1) + 0x8000];
		blockPtr[3] = colorMap[(colors[3] >> 1) + 0xC000];
		blockPtr += pitch;
		blockPtr[0] = colorMap[(colors[4] >> 1) + 0xC000];
		blockPtr[1] = colorMap[(colors[5] >> 1) + 0x0000];
		blockPtr[2] = colorMap[(colors[6] >> 1) + 0x4000];
		blockPtr[3] = colorMap[(colors[7] >> 1) + 0x8000];
		blockPtr += pitch;
		blockPtr[0] = colorMap[(colors[8] >> 1) + 0x4000];
		blockPtr[1] = colorMap[(colors[9] >> 1) + 0x8000];
		blockPtr[2] = colorMap[(colors[10] >> 1) + 0xC000];
		blockPtr[3] = colorMap[(colors[11] >> 1) + 0x0000];
		blockPtr += pitch;
		blockPtr[0] = colorMap[(colors[12] >> 1) + 0x8000];
		blockPtr[1] = colorMap[(colors[13] >> 1) + 0xC000];
		blockPtr[2] = colorMap[(colors[14] >> 1) + 0x0000];
		blockPtr[3] = colorMap[(colors[15] >> 1) + 0x4000];
	}

	static inline void drawBlendBlock(byte *blockPtr, uint16 pitch, const uint16 (&colors)[4], const byte (&indexes)[4], const byte *colorMap) {
		blockPtr[0] = colorMap[(colors[(indexes[0] >> 6) & 0x03] >> 1) + 0x0000];
		blockPtr[1] = colorMap[(colors[(indexes[0] >> 4) & 0x03] >> 1) + 0x4000];
		blockPtr[2] = colorMap[(colors[(indexes[0] >> 2) & 0x03] >> 1) + 0x8000];
		blockPtr[3] = colorMap[(colors[(indexes[0] >> 0) & 0x03] >> 1) + 0xC000];
		blockPtr += pitch;
		blockPtr[0] = colorMap[(colors[(indexes[1] >> 6) & 0x03] >> 1) + 0xC000];
		blockPtr[1] = colorMap[(colors[(indexes[1] >> 4) & 0x03] >> 1) + 0x0000];
		blockPtr[2] = colorMap[(colors[(indexes[1] >> 2) & 0x03] >> 1) + 0x4000];
		blockPtr[3] = colorMap[(colors[(indexes[1] >> 0) & 0x03] >> 1) + 0x8000];
		blockPtr += pitch;
		blockPtr[0] = colorMap[(colors[(indexes[2] >> 6) & 0x03] >> 1) + 0x4000];
		blockPtr[1] = colorMap[(colors[(indexes[2] >> 4) & 0x03] >> 1) + 0x8000];
		blockPtr[2] = colorMap[(colors[(indexes[2] >> 2) & 0x03] >> 1) + 0xC000];
		blockPtr[3] = colorMap[(colors[(indexes[2] >> 0) & 0x03] >> 1) + 0x0000];
		blockPtr += pitch;
		blockPtr[0] = colorMap[(colors[(indexes[3] >> 6) & 0x03] >> 1) + 0x8000];
		blockPtr[1] = colorMap[(colors[(indexes[3] >> 4) & 0x03] >> 1) + 0xC000];
		blockPtr[2] = colorMap[(colors[(indexes[3] >> 2) & 0x03] >> 1) + 0x0000];
		blockPtr[3] = colorMap[(colors[(indexes[3] >> 0) & 0x03] >> 1) + 0x4000];
	}
};

template<typename PixelInt, typename BlockDecoder>
static inline void decodeFrameTmpl(Common::SeekableReadStream &stream, PixelInt *ptr, uint16 pitch, uint16 blockWidth, uint16 blockHeight, const byte *colorMap) {
	uint16 colorA = 0, colorB = 0;
	uint16 color4[4];

	PixelInt *blockPtr = ptr;
	PixelInt *endPtr = ptr + pitch;
	uint16 ta;
	uint16 tb;

	// First byte is always 0xe1. Warn if it's different
	byte firstByte = stream.readByte();
	if (firstByte != 0xe1)
		warning("First RPZA chunk byte is 0x%02x instead of 0xe1", firstByte);

	// Get chunk size, ingnoring first byte
	uint32 chunkSize = stream.readUint16BE() << 8;
	chunkSize += stream.readByte();

	// If length mismatch use size from MOV file and try to decode anyway
	if (chunkSize != (uint32)stream.size()) {
		warning("MOV chunk size != encoded chunk size; using MOV chunk size");
		chunkSize = stream.size();
	}

	// Number of 4x4 blocks in frame
	int32 totalBlocks = blockWidth * blockHeight;

	// Process chunk data
	while ((uint32)stream.pos() < chunkSize) {
		byte opcode = stream.readByte(); // Get opcode
		byte numBlocks = (opcode & 0x1f) + 1; // Extract block counter from opcode

		// If opcode MSbit is 0, we need more data to decide what to do
		if ((opcode & 0x80) == 0) {
			colorA = (opcode << 8) | stream.readByte();
			opcode = 0;
			if (stream.readByte() & 0x80) {
				// Must behave as opcode 110xxxxx, using colorA computed
				// above. Use fake opcode 0x20 to enter switch block at
				// the right place
				opcode = 0x20;
				numBlocks = 1;
			}
			stream.seek(-1, SEEK_CUR);
		}

		switch (opcode & 0xe0) {
		case 0x80: // Skip blocks
			while (numBlocks--) {
				ADVANCE_BLOCK();
			}
			break;
		case 0xa0: // Fill blocks with one color
			colorA = stream.readUint16BE();

			while (numBlocks--) {
				BlockDecoder::drawFillBlock(blockPtr, pitch, colorA, colorMap);
				ADVANCE_BLOCK();
			}
			break;

		// Fill blocks with 4 colors
		case 0xc0:
			colorA = stream.readUint16BE();
			// fall through
		case 0x20:
			colorB = stream.readUint16BE();

			// Sort out the colors
			color4[0] = colorB & 0x7FFF;
			color4[1] = 0;
			color4[2] = 0;
			color4[3] = colorA & 0x7FFF;

			// Red components
			ta = (colorA >> 10) & 0x1F;
			tb = (colorB >> 10) & 0x1F;
			color4[1] |= ((11 * ta + 21 * tb) >> 5) << 10;
			color4[2] |= ((21 * ta + 11 * tb) >> 5) << 10;

			// Green components
			ta = (colorA >> 5) & 0x1F;
			tb = (colorB >> 5) & 0x1F;
			color4[1] |= ((11 * ta + 21 * tb) >> 5) << 5;
			color4[2] |= ((21 * ta + 11 * tb) >> 5) << 5;

			// Blue components
			ta = colorA & 0x1F;
			tb = colorB & 0x1F;
			color4[1] |= ((11 * ta + 21 * tb) >> 5);
			color4[2] |= ((21 * ta + 11 * tb) >> 5);

			while (numBlocks--) {
				byte indexes[4];
				stream.read(indexes, 4);

				BlockDecoder::drawBlendBlock(blockPtr, pitch, color4, indexes, colorMap);
				ADVANCE_BLOCK();
			}
			break;

		// Fill block with 16 colors
		case 0x00: {
			uint16 colors[16];
			colors[0] = colorA;

			for (int i = 0; i < 15; i++)
				colors[i + 1] = stream.readUint16BE();

			BlockDecoder::drawRawBlock(blockPtr, pitch, colors, colorMap);
			ADVANCE_BLOCK();
			break;
		}

		// Unknown opcode
		default:
			error("Unknown opcode %02x in rpza chunk", opcode);
		}
	}
}

const Graphics::Surface *RPZADecoder::decodeFrame(Common::SeekableReadStream &stream) {
	if (!_surface) {
		_surface = new Graphics::Surface();

		// Allocate enough space in the surface for the blocks
		_surface->create(_blockWidth * 4, _blockHeight * 4, getPixelFormat());

		// Adjust width/height to be the right ones
		_surface->w = _width;
		_surface->h = _height;
	}

	if (_colorMap)
		decodeFrameTmpl<byte, BlockDecoderDither>(stream, (byte *)_surface->getPixels(), _surface->pitch, _blockWidth, _blockHeight, _colorMap);
	else
		decodeFrameTmpl<uint16, BlockDecoderRaw>(stream, (uint16 *)_surface->getPixels(), _surface->pitch / 2, _blockWidth, _blockHeight, _colorMap);

	return _surface;
}

bool RPZADecoder::canDither(DitherType type) const {
	return type == kDitherTypeQT;
}

void RPZADecoder::setDither(DitherType type, const byte *palette) {
	assert(canDither(type));

	_ditherPalette = new byte[256 * 3];
	memcpy(_ditherPalette, palette, 256 * 3);

	_dirtyPalette = true;
	_format = Graphics::PixelFormat::createFormatCLUT8();

	delete[] _colorMap;
	_colorMap = createQuickTimeDitherTable(palette, 256);
}

} // End of namespace Image
