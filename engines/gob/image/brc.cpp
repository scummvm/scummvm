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

#include "brc.h"

#include "common/stream.h"
#include "common/textconsole.h"

namespace Image {

BRCDecoder::BRCDecoder(): _palette(0), _format(2, 5, 5, 5, 0, 10, 5, 0, 0) {
}


BRCDecoder::~BRCDecoder() {
	destroy();
}

void BRCDecoder::destroy() {
	_surface.free();
}

void BRCDecoder::readRawChunk(Common::SeekableReadStream &stream, uint16 *&dest) {
	byte chunkSize = stream.readByte();
	for (byte i = 0; i < chunkSize; i++) {
		*dest++ = stream.readUint16LE();
	}
}

void BRCDecoder::readRLEChunk(Common::SeekableReadStream &stream, uint16 *&dest) {
	byte chunkSize = stream.readByte();
	for (byte i = 0; i < chunkSize; i++) {
		byte runLength = stream.readByte();
		uint16 pixel = stream.readUint16LE();
		for (byte j = 0; j < runLength; j++)
			*dest++ = pixel;
	}
}

void BRCDecoder::readRawChunkColumnWise(Common::SeekableReadStream &stream, uint16 *&dest, uint16 *&destColumnStart, uint32 &remainingHeight) {
	byte chunkSize = stream.readByte();
	for (byte i = 0; i < chunkSize; i++) {
		uint16 pixel = stream.readUint16LE();
		*dest = pixel;
		remainingHeight--;
		if (remainingHeight == 0) {
			++destColumnStart;
			dest = destColumnStart;
			remainingHeight = _surface.h;
		} else {
			dest += _surface.w;
		}
	}
}

void BRCDecoder::readRLEChunkColumnWise(Common::SeekableReadStream &stream, uint16 *&dest, uint16 *&destColumnStart, uint32 &remainingHeight) {
	byte chunkSize = stream.readByte();
	for (byte i = 0; i < chunkSize; i++) {
		byte runLength = stream.readByte();
		uint16 pixel = stream.readUint16LE();
		for (byte j = 0; j < runLength; j++) {
			*dest = pixel;
			remainingHeight--;
			if (remainingHeight == 0) {
				++destColumnStart;
				dest = destColumnStart;
				remainingHeight = _surface.h;
			} else {
				dest += _surface.w;
			}
		}
	}
}

void BRCDecoder::loadBRCData(Common::SeekableReadStream &stream, uint32 nbrOfChunks, bool firstChunkIsRLE) {
	uint16 *dest = (uint16*) _surface.getBasePtr(0, 0);
	if (firstChunkIsRLE) {
		for (uint32 i = 0; i < nbrOfChunks; i++) {
			readRLEChunk(stream, dest);
			readRawChunk(stream, dest);
		}
	} else {
		for (uint32 i = 0; i < nbrOfChunks; i++) {
			readRawChunk(stream, dest);
			readRLEChunk(stream, dest);
		}
	}
}
void BRCDecoder::loadBRCDataColumnWise(Common::SeekableReadStream &stream, uint32 nbrOfChunks, bool firstChunkIsRLE) {
	uint16 *dest = (uint16*) _surface.getBasePtr(0, 0);
	uint16 *destColumnStart = dest;
	uint32 remainingHeight = _surface.h;
	if (firstChunkIsRLE) {
		for (uint32 i = 0; i < nbrOfChunks; i++) {
			readRLEChunkColumnWise(stream, dest, destColumnStart, remainingHeight);
			readRawChunkColumnWise(stream, dest, destColumnStart, remainingHeight);
		}
	} else {
		for (uint32 i = 0; i < nbrOfChunks; i++) {
			readRawChunkColumnWise(stream, dest, destColumnStart, remainingHeight);
			readRLEChunkColumnWise(stream, dest, destColumnStart, remainingHeight);
		}
	}
}

bool BRCDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	uint32 fileType = stream.readUint32BE();

	if (fileType != MKTAG('B', 'R', 'C', '\0')) {
		warning("Missing BRC header");
		return false;
	}

	stream.skip(8); // unknown
	stream.readUint32LE(); // data size

	uint32 width = stream.readUint32LE();
	uint32 height = stream.readUint32LE();

	if (width == 0 || height == 0)
		return false;

	uint32 bpp = stream.readUint32LE();
	if (bpp != 2) {
		warning("Unsupported bpp (%d) for BRCDecoder", bpp);
		return false;
	}

	_surface.create(width, height, _format);

	uint32 flags = stream.readUint32LE();
	uint32 headerSize = stream.readUint32LE();
	uint32 nbrOfChunks = stream.readUint32LE();

	bool columnWise = !(flags & 0x10);
	bool firstChunkIsRLE = flags & 2;

	stream.seek(headerSize, SEEK_SET);
	if (columnWise)
		loadBRCDataColumnWise(stream, nbrOfChunks, firstChunkIsRLE);
	else
		loadBRCData(stream, nbrOfChunks, firstChunkIsRLE);

	return true;
}


} // End of namespace Image
