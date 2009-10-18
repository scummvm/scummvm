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
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/endian.h"
#include "common/archive.h"
#include "common/system.h"
#include "common/util.h"

#include "graphics/surface.h"

#include "sci/seq_decoder.h"

namespace Sci {

enum seqPalTypes {
	kSeqPalVariable = 0,
	kSeqPalConstant = 1
};

enum seqFrameTypes {
	kSeqFrameFull = 0,
	kSeqFrameDiff = 1
};

SeqDecoder::~SeqDecoder() {
	closeFile();
}

bool SeqDecoder::loadFile(const char *fileName, int frameDelay) {
	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;

	// Seek to the first frame
	_videoInfo.currentFrame = 0;

	_videoInfo.width = 320;
	_videoInfo.height = 200;
	_videoInfo.frameCount = _fileStream->readUint16LE();
	// Our frameDelay is calculated in 1/100 ms, so we convert it here
	_videoInfo.frameDelay = 100 * frameDelay * 1000 / 60;
	_videoFrameBuffer = new byte[_videoInfo.width * _videoInfo.height];

	// Set palette
	int paletteSize = _fileStream->readUint32LE();

	byte *paletteData = new byte[paletteSize];
	_fileStream->read(paletteData, paletteSize);

	// SCI1.1 palette
	byte palFormat = paletteData[32];
	uint16 palColorStart = READ_LE_UINT16(paletteData + 25);
	uint16 palColorCount = READ_LE_UINT16(paletteData + 29);

	byte palette[256 * 4];
	int palOffset = 37;

	for (uint16 colorNo = palColorStart; colorNo < palColorStart + palColorCount; colorNo++) {
		if (palFormat == kSeqPalVariable)
			palOffset++;
		palette[colorNo * 4 + 0] = paletteData[palOffset++];
		palette[colorNo * 4 + 1] = paletteData[palOffset++];
		palette[colorNo * 4 + 2] = paletteData[palOffset++];
		palette[colorNo * 4 + 3] = 0;
	}

	g_system->setPalette(palette, 0, 256);

	delete paletteData;

	_videoInfo.firstframeOffset = _fileStream->pos();

	return true;
}

void SeqDecoder::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;

	delete[] _videoFrameBuffer;
	_videoFrameBuffer = 0;
}

bool SeqDecoder::decodeNextFrame() {
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

	if (_videoInfo.currentFrame == 0)
		_videoInfo.startTime = g_system->getMillis();

	if (frameType == kSeqFrameFull) {
		byte *dst = _videoFrameBuffer + frameTop * 320 + frameLeft;
		
		byte *linebuf = new byte[frameWidth];

		do {
			_fileStream->read(linebuf, frameWidth);
			memcpy(dst, linebuf, frameWidth);
			dst += 320;
		} while (--frameHeight);

		delete[] linebuf;
	} else {
		byte *buf = new byte[frameSize];
		_fileStream->read(buf, frameSize);
		decodeFrame(buf, rleSize, buf + rleSize, frameSize - rleSize, _videoFrameBuffer + 320 * frameTop, frameLeft, frameWidth, frameHeight, colorKey);
		delete buf;
	}

	return ++_videoInfo.currentFrame < _videoInfo.frameCount;
}

#define WRITE_TO_BUFFER(n) \
	if (writeRow * 320 + writeCol + (n) > 320 * height) { \
		warning("SEQ player: writing out of bounds, aborting"); \
		return false; \
	} \
	if (litPos + (n) > litSize) { \
		warning("SEQ player: reading out of bounds, aborting"); \
	} \
	memcpy(dest + writeRow * 320 + writeCol, litData + litPos, n);

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
