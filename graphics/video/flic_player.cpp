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

#include "graphics/video/flic_player.h"
#include "common/archive.h"
#include "common/stream.h"
#include "common/endian.h"

namespace Graphics {

FlicPlayer::FlicPlayer()
	: _paletteDirty(false), _offscreen(0), _currFrame(0), _fileStream(0) {
	memset(&_flicInfo, 0, sizeof(_flicInfo));
}

FlicPlayer::~FlicPlayer() {
	closeFile();
}

int FlicPlayer::getWidth() {
	if (!_fileStream)
		return 0;
	return _flicInfo.width;
}

int FlicPlayer::getHeight() {
	if (!_fileStream)
		return 0;
	return _flicInfo.height;
}

int32 FlicPlayer::getCurFrame() {
	if (!_fileStream)
		return -1;
	return _currFrame;
}

int32 FlicPlayer::getFrameCount() {
	if (!_fileStream)
		return 0;
	return _flicInfo.numFrames;
}

bool FlicPlayer::loadFile(const char *fileName) {
	closeFile();

	_fileStream = SearchMan.openFile(fileName);
	if (!_fileStream)
		return false;

	_flicInfo.size = _fileStream->readUint32LE();
	_flicInfo.type = _fileStream->readUint16LE();

	// Check FLC magic number
	if (_flicInfo.type != 0xAF12) {
		warning("FlicPlayer::FlicPlayer(): attempted to load non-FLC data (type = 0x%04X)", _flicInfo.type);
		delete _fileStream;
		return false;
	}

	_flicInfo.numFrames = _fileStream->readUint16LE();
	_flicInfo.width = _fileStream->readUint16LE();
	_flicInfo.height = _fileStream->readUint16LE();
	_fileStream->skip(4);
	_flicInfo.speed = _fileStream->readUint32LE();

	_fileStream->seek(80);
	_flicInfo.offsetFrame1 = _fileStream->readUint32LE();
	_flicInfo.offsetFrame2 = _fileStream->readUint32LE();

	_offscreen = new uint8[_flicInfo.width * _flicInfo.height];
	memset(_palette, 0, sizeof(_palette));
	_paletteDirty = false;

	// Seek to the first frame
	_currFrame = 0;
	_fileStream->seek(_flicInfo.offsetFrame1);
	return true;
}

void FlicPlayer::closeFile() {
	if (!_fileStream)
		return;

	delete _fileStream;
	_fileStream = 0;

	delete[] _offscreen;
	_offscreen = 0;
}

void FlicPlayer::redraw() {
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, _flicInfo.width, _flicInfo.height));
}

ChunkHeader FlicPlayer::readChunkHeader() {
	ChunkHeader head;

	head.size = _fileStream->readUint32LE();
	head.type = _fileStream->readUint16LE();

	return head;
}

FrameTypeChunkHeader FlicPlayer::readFrameTypeChunkHeader(ChunkHeader chunkHead) {
	FrameTypeChunkHeader head;

	head.header = chunkHead;
	head.numChunks = _fileStream->readUint16LE();
	head.delay = _fileStream->readUint16LE();
	head.reserved = _fileStream->readUint16LE();
	head.widthOverride = _fileStream->readUint16LE();
	head.heightOverride = _fileStream->readUint16LE();

	return head;
}

void FlicPlayer::decodeByteRun(uint8 *data) {
	uint8 *ptr = (uint8 *)_offscreen;
	while ((ptr - _offscreen) < (_flicInfo.width * _flicInfo.height)) {
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

	redraw();
}

#define OP_PACKETCOUNT   0
#define OP_UNDEFINED     1
#define OP_LASTPIXEL     2
#define OP_LINESKIPCOUNT 3

void FlicPlayer::decodeDeltaFLC(uint8 *data) {
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
				_offscreen[currentLine * _flicInfo.width + _flicInfo.width - 1] = (opcode & 0xFF);
				_dirtyRects.push_back(Common::Rect(_flicInfo.width - 1, currentLine, _flicInfo.width, currentLine + 1));
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
				memcpy(_offscreen + (currentLine * _flicInfo.width) + column, data, rleCount * 2);
				data += rleCount * 2;
				_dirtyRects.push_back(Common::Rect(column, currentLine, column + rleCount * 2, currentLine + 1));
			} else if (rleCount < 0) {
				rleCount = -rleCount;
				uint16 dataWord = READ_UINT16(data); data += 2;
				for (int i = 0; i < rleCount; ++i) {
					WRITE_UINT16(_offscreen + currentLine * _flicInfo.width + column + i * 2, dataWord);
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

void FlicPlayer::decodeNextFrame() {
	FrameTypeChunkHeader frameHeader;

	// Read chunk
	ChunkHeader cHeader = readChunkHeader();
	switch (cHeader.type) {
	case FRAME_TYPE:
		frameHeader = readFrameTypeChunkHeader(cHeader);
		_currFrame++;
		break;
	default:
		error("FlicPlayer::decodeFrame(): unknown main chunk type (type = 0x%02X)", cHeader.type);
		break;
	 }

	// Read subchunks
	if (cHeader.type == FRAME_TYPE) {
		for (int i = 0; i < frameHeader.numChunks; ++i) {
			cHeader = readChunkHeader();
			uint8 *data = new uint8[cHeader.size - 6];
			_fileStream->read(data, cHeader.size - 6);
			switch (cHeader.type) {
			case FLI_SETPAL:
				setPalette(data);
				_paletteDirty = true;
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
				error("FlicPlayer::decodeFrame(): unknown subchunk type (type = 0x%02X)", cHeader.type);
				break;
			 }

			delete[] data;
		}
	}

	// If we just processed the ring frame, set the next frame
	if (_currFrame == _flicInfo.numFrames + 1) {
		_currFrame = 1;
		_fileStream->seek(_flicInfo.offsetFrame2);
	}
}

void FlicPlayer::reset() {
	_currFrame = 0;
	_fileStream->seek(_flicInfo.offsetFrame1);
}

void FlicPlayer::setPalette(uint8 *mem) {
	uint16 numPackets = READ_LE_UINT16(mem); mem += 2;

	if (0 == READ_LE_UINT16(mem)) { //special case
		mem += 2;
		for (int i = 0; i < 256; ++i) {
			memcpy(_palette + i * 4, mem + i * 3, 3);
			_palette[i * 4 + 3] = 0;
		}
	} else {
		uint8 palPos = 0;

		while (numPackets--) {
			palPos += *mem++;
			uint8 change = *mem++;

			for (int i = 0; i < change; ++i) {
				memcpy(_palette + (palPos + i) * 4, mem + i * 3, 3);
				_palette[(palPos + i) * 4 + 3] = 0;
			}

			palPos += change;
			mem += (change * 3);
		}
	}
}

void FlicPlayer::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = _flicInfo.height;
	uint w = _flicInfo.width;

	byte *src = (byte *)_offscreen;
	dst += y * pitch + x;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += w;
	} while (--h);
}

} // End of namespace Graphics
