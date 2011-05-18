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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#include "sci/video/seq_decoder.h"

namespace Sci {

enum seqPalTypes {
	kSeqPalVariable = 0,
	kSeqPalConstant = 1
};

enum seqFrameTypes {
	kSeqFrameFull = 0,
	kSeqFrameDiff = 1
};

SeqDecoder::SeqDecoder() {
	_fileStream = 0;
	_surface = 0;
	_dirtyPalette = false;
}

SeqDecoder::~SeqDecoder() {
	close();
}

bool SeqDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_fileStream = stream;
	_surface = new Graphics::Surface();
	_surface->create(SEQ_SCREEN_WIDTH, SEQ_SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	_frameCount = _fileStream->readUint16LE();

	// Set palette
	int paletteChunkSize = _fileStream->readUint32LE();
	readPaletteChunk(paletteChunkSize);

	return true;
}

void SeqDecoder::readPaletteChunk(uint16 chunkSize) {
	byte *paletteData = new byte[chunkSize];
	_fileStream->read(paletteData, chunkSize);

	// SCI1.1 palette
	byte palFormat = paletteData[32];
	uint16 palColorStart = READ_LE_UINT16(paletteData + 25);
	uint16 palColorCount = READ_LE_UINT16(paletteData + 29);

	int palOffset = 37;
	memset(_palette, 0, 256 * 3);

	for (uint16 colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
		if (palFormat == kSeqPalVariable)
			palOffset++;
		_palette[colorNo * 3 + 0] = paletteData[palOffset++];
		_palette[colorNo * 3 + 1] = paletteData[palOffset++];
		_palette[colorNo * 3 + 2] = paletteData[palOffset++];
	}

	_dirtyPalette = true;
	delete[] paletteData;
}

void SeqDecoder::close() {
	if (!_fileStream)
		return;

	_frameDelay = 0;

	delete _fileStream;
	_fileStream = 0;

	_surface->free();
	delete _surface;
	_surface = 0;

	reset();
}

const Graphics::Surface *SeqDecoder::decodeNextFrame() {
	int16 frameWidth = _fileStream->readUint16LE();
	int16 frameHeight = _fileStream->readUint16LE();
	int16 frameLeft = _fileStream->readUint16LE();
	int16 frameTop = _fileStream->readUint16LE();
	byte colorKey = _fileStream->readByte();
	byte frameType = _fileStream->readByte();
	_fileStream->skip(2);
	uint16 frameSize = _fileStream->readUint16LE();
	_fileStream->skip(2);
	uint16 rleSize = _fileStream->readUint16LE();
	_fileStream->skip(6);
	uint32 offset = _fileStream->readUint32LE();

	_fileStream->seek(offset);

	if (frameType == kSeqFrameFull) {
		byte *dst = (byte *)_surface->pixels + frameTop * SEQ_SCREEN_WIDTH + frameLeft;

		byte *linebuf = new byte[frameWidth];

		do {
			_fileStream->read(linebuf, frameWidth);
			memcpy(dst, linebuf, frameWidth);
			dst += SEQ_SCREEN_WIDTH;
		} while (--frameHeight);

		delete[] linebuf;
	} else {
		byte *buf = new byte[frameSize];
		_fileStream->read(buf, frameSize);
		decodeFrame(buf, rleSize, buf + rleSize, frameSize - rleSize, (byte *)_surface->pixels + SEQ_SCREEN_WIDTH * frameTop, frameLeft, frameWidth, frameHeight, colorKey);
		delete[] buf;
	}

	if (_curFrame == -1)
		_startTime = g_system->getMillis();

	_curFrame++;
	return _surface;
}

#define WRITE_TO_BUFFER(n) \
	if (writeRow * SEQ_SCREEN_WIDTH + writeCol + (n) > SEQ_SCREEN_WIDTH * height) { \
		warning("SEQ player: writing out of bounds, aborting"); \
		return false; \
	} \
	if (litPos + (n) > litSize) { \
		warning("SEQ player: reading out of bounds, aborting"); \
	} \
	memcpy(dest + writeRow * SEQ_SCREEN_WIDTH + writeCol, litData + litPos, n);

bool SeqDecoder::decodeFrame(byte *rleData, int rleSize, byte *litData, int litSize, byte *dest, int left, int width, int height, int colorKey) {
	int writeRow = 0;
	int writeCol = left;
	int litPos = 0;
	int rlePos = 0;

	while (rlePos < rleSize) {
		int op = rleData[rlePos++];

		if ((op & 0xc0) == 0xc0) {
			op &= 0x3f;
			if (op == 0) {
				// Go to next line in target buffer
				writeRow++;
				writeCol = left;
			} else {
				// Skip bytes on current line
				writeCol += op;
			}
		} else if (op & 0x80) {
			op &= 0x3f;
			if (op == 0) {
				// Copy remainder of current line
				int rem = width - (writeCol - left);

				WRITE_TO_BUFFER(rem);
				writeRow++;
				writeCol = left;
				litPos += rem;
			} else {
				// Copy bytes
				WRITE_TO_BUFFER(op);
				writeCol += op;
				litPos += op;
			}
		} else {
			uint16 count = ((op & 7) << 8) | rleData[rlePos++];

			switch (op >> 3) {
			case 2:
				// Skip bytes
				writeCol += count;
				break;
			case 3:
				// Copy bytes
				WRITE_TO_BUFFER(count);
				writeCol += count;
				litPos += count;
				break;
			case 6: {
				// Copy rows
				if (count == 0)
					count = height - writeRow;

				for (int i = 0; i < count; i++) {
					WRITE_TO_BUFFER(width);
					litPos += width;
					writeRow++;
				}
				break;
			}
			case 7:
				// Skip rows
				if (count == 0)
					count = height - writeRow;

				writeRow += count;
				break;
			default:
				warning("Unsupported operation %i encountered", op >> 3);
				return false;
			}
		}
	}

	return true;
}

} // End of namespace Sci
