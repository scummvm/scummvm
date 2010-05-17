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

#include "graphics/video/flic_decoder.h"
#include "common/archive.h"
#include "common/stream.h"
#include "common/endian.h"
#include "common/system.h"

namespace Graphics {

FlicDecoder::FlicDecoder() {
	_paletteChanged = false;
	_fileStream = 0;
	_videoFrameBuffer = 0;
	memset(&_videoInfo, 0, sizeof(_videoInfo));
}

FlicDecoder::~FlicDecoder() {
	closeFile();
}

bool FlicDecoder::loadFile(const char *fileName) {
	closeFile();

	_fileStream = SearchMan.createReadStreamForMember(fileName);
	if (!_fileStream)
		return false;

	/* uint32 frameSize = */ _fileStream->readUint32LE();
	uint16 frameType = _fileStream->readUint16LE();

	// Check FLC magic number
	if (frameType != 0xAF12) {
		warning("FlicDecoder::FlicDecoder(): attempted to load non-FLC data (type = 0x%04X)", frameType);
		delete _fileStream;
		_fileStream = 0;
		return false;
	}

	_videoInfo.frameCount = _fileStream->readUint16LE();
	_videoInfo.width = _fileStream->readUint16LE();
	_videoInfo.height = _fileStream->readUint16LE();
	uint16 colorDepth = _fileStream->readUint16LE();
	if (colorDepth != 8) {
		warning("FlicDecoder::FlicDecoder(): attempted to load an FLC with a palette of color depth %d. Only 8-bit color palettes are supported", frameType);
		delete _fileStream;
		_fileStream = 0;
		return false;
	}
	_fileStream->readUint16LE();	// flags
	// Note: The normal delay is a 32-bit integer (dword), whereas the overriden delay is a 16-bit integer (word)
	// frameDelay is the FLIC "speed", in milliseconds. Our frameDelay is calculated in 1/100 ms, so we convert it here
	_videoInfo.frameDelay = 100 * _fileStream->readUint32LE();
	_videoInfo.frameRate = 100 * 1000 / _videoInfo.frameDelay;

	_fileStream->seek(80);
	_offsetFrame1 = _fileStream->readUint32LE();
	_offsetFrame2 = _fileStream->readUint32LE();

	_videoFrameBuffer = new byte[_videoInfo.width * _videoInfo.height];
	_palette = (byte *)malloc(3 * 256);
	memset(_palette, 0, 3 * 256);
	_paletteChanged = false;

	// Seek to the first frame
	_videoInfo.currentFrame = -1;
	_fileStream->seek(_offsetFrame1);
	return true;
}

void FlicDecoder::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;

	delete[] _videoFrameBuffer;
	_videoFrameBuffer = 0;

	free(_palette);

	_dirtyRects.clear();
}

void FlicDecoder::decodeByteRun(uint8 *data) {
	byte *ptr = (uint8 *)_videoFrameBuffer;
	while ((uint32)(ptr - _videoFrameBuffer) < (_videoInfo.width * _videoInfo.height)) {
		int chunks = *data++;
		while (chunks--) {
			int count = (int8)*data++;
			if (count > 0) {
				memset(ptr, *data++, count);
			} else {
				count = -count;
				memcpy(ptr, data, count);
				data += count;
			}
			ptr += count;
		}
	}

	// Redraw
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, _videoInfo.width, _videoInfo.height));
}

#define OP_PACKETCOUNT   0
#define OP_UNDEFINED     1
#define OP_LASTPIXEL     2
#define OP_LINESKIPCOUNT 3

void FlicDecoder::decodeDeltaFLC(uint8 *data) {
	uint16 linesInChunk = READ_LE_UINT16(data); data += 2;
	uint16 currentLine = 0;
	uint16 packetCount = 0;

	while (linesInChunk--) {
		uint16 opcode;

		// First process all the opcodes.
		do {
			opcode = READ_LE_UINT16(data); data += 2;

			switch ((opcode >> 14) & 3) {
			case OP_PACKETCOUNT:
				packetCount = opcode;
				break;
			case OP_UNDEFINED:
				break;
			case OP_LASTPIXEL:
				_videoFrameBuffer[currentLine * _videoInfo.width + _videoInfo.width - 1] = (opcode & 0xFF);
				_dirtyRects.push_back(Common::Rect(_videoInfo.width - 1, currentLine, _videoInfo.width, currentLine + 1));
				break;
			case OP_LINESKIPCOUNT:
				currentLine += -(int16)opcode;
				break;
			}
		} while (((opcode >> 14) & 3) != OP_PACKETCOUNT);

		uint16 column = 0;

		// Now interpret the RLE data
		while (packetCount--) {
			column += *data++;
			int rleCount = (int8)*data++;
			if (rleCount > 0) {
				memcpy(_videoFrameBuffer + (currentLine * _videoInfo.width) + column, data, rleCount * 2);
				data += rleCount * 2;
				_dirtyRects.push_back(Common::Rect(column, currentLine, column + rleCount * 2, currentLine + 1));
			} else if (rleCount < 0) {
				rleCount = -rleCount;
				uint16 dataWord = READ_UINT16(data); data += 2;
				for (int i = 0; i < rleCount; ++i) {
					WRITE_UINT16(_videoFrameBuffer + currentLine * _videoInfo.width + column + i * 2, dataWord);
				}
				_dirtyRects.push_back(Common::Rect(column, currentLine, column + rleCount * 2, currentLine + 1));
			} else { // End of cutscene ?
				return;
			}
			column += rleCount * 2;
		}

		currentLine++;
	}
}

#define FLI_SETPAL 4
#define FLI_SS2    7
#define FLI_BRUN   15
#define PSTAMP     18
#define FRAME_TYPE 0xF1FA

bool FlicDecoder::decodeNextFrame() {
	// Read chunk
	uint32 frameSize = _fileStream->readUint32LE();
	uint16 frameType = _fileStream->readUint16LE();
	uint16 chunkCount = 0;

	switch (frameType) {
	case FRAME_TYPE: {
		chunkCount = _fileStream->readUint16LE();
		// Note: The overriden delay is a 16-bit integer (word), whereas the normal delay is a 32-bit integer (dword)
		// frameDelay is the FLIC "speed", in milliseconds. Our frameDelay is calculated in 1/100 ms, so we convert it here
		uint16 newFrameDelay = _fileStream->readUint16LE();	// "speed", in milliseconds
		if (newFrameDelay > 0) {
			_videoInfo.frameDelay = 100 * newFrameDelay;
			_videoInfo.frameRate = 100 * 1000 / _videoInfo.frameDelay;
		}
		_fileStream->readUint16LE();	// reserved, always 0
		uint16 newWidth = _fileStream->readUint16LE();
		uint16 newHeight = _fileStream->readUint16LE();
		if (newWidth > 0)
			_videoInfo.width = newWidth;
		if (newHeight > 0)
			_videoInfo.height = newHeight;

		_videoInfo.currentFrame++;

		if (_videoInfo.currentFrame == 0)
			_videoInfo.startTime = g_system->getMillis();
		}
		break;
	default:
		error("FlicDecoder::decodeFrame(): unknown main chunk type (type = 0x%02X)", frameType);
		break;
	 }

	// Read subchunks
	if (frameType == FRAME_TYPE) {
		for (uint32 i = 0; i < chunkCount; ++i) {
			frameSize = _fileStream->readUint32LE();
			frameType = _fileStream->readUint16LE();
			uint8 *data = new uint8[frameSize - 6];
			_fileStream->read(data, frameSize - 6);
			switch (frameType) {
			case FLI_SETPAL:
				unpackPalette(data);
				setPalette(_palette);
				_paletteChanged = true;
				break;
			case FLI_SS2:
				decodeDeltaFLC(data);
				break;
			case FLI_BRUN:
				decodeByteRun(data);
				break;
			case PSTAMP:
				/* PSTAMP - skip for now */
				break;
			default:
				error("FlicDecoder::decodeFrame(): unknown subchunk type (type = 0x%02X)", frameType);
				break;
			 }

			delete[] data;
		}
	}

	// If we just processed the ring frame, set the next frame
	if (_videoInfo.currentFrame == (int32)_videoInfo.frameCount + 1) {
		_videoInfo.currentFrame = 1;
		_fileStream->seek(_offsetFrame2);
	}

	return !endOfVideo();
}

void FlicDecoder::reset() {
	_videoInfo.currentFrame = 0;
	_fileStream->seek(_offsetFrame1);
}

void FlicDecoder::unpackPalette(uint8 *data) {
	uint16 numPackets = READ_LE_UINT16(data); data += 2;

	if (0 == READ_LE_UINT16(data)) { //special case
		data += 2;
		for (int i = 0; i < 256; ++i) {
			memcpy(_palette + i * 3, data + i * 3, 3);
		}
	} else {
		uint8 palPos = 0;

		while (numPackets--) {
			palPos += *data++;
			uint8 change = *data++;

			for (int i = 0; i < change; ++i) {
				memcpy(_palette + (palPos + i) * 3, data + i * 3, 3);
			}

			palPos += change;
			data += (change * 3);
		}
	}
}

void FlicDecoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		for (int y = (*it).top; y < (*it).bottom; ++y) {
			const int x = (*it).left;
			memcpy(dst + y * pitch + x, _videoFrameBuffer + y * _videoInfo.width + x, (*it).right - x);
		}
	}
	_dirtyRects.clear();
}

} // End of namespace Graphics
