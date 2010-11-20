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

#include "common/events.h"
#include "common/file.h"

#include "common/memstream.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"
#include "sound/decoders/raw.h"

#include "prisoner/prisoner.h"
#include "prisoner/muxplayer.h"

namespace Prisoner {

class MemoryBitReadStream : public Common::MemoryReadStream {
public:
	MemoryBitReadStream(const byte *dataPtr, uint32 dataSize, int bitSize)
		: MemoryReadStream(dataPtr, dataSize),
		_bitsLeft(0), _bitSize(bitSize), _bitBuf(0) {};
	int readBit();
protected:
	uint32 _bitBuf;
	int _bitsLeft, _bitSize;
};

int MemoryBitReadStream::readBit() {
	if (_bitsLeft == 0) {
		_bitBuf = readUint32LE();
		_bitsLeft = _bitSize;
	}
	int theBit = _bitBuf & 0x80000000;
	_bitBuf <<= 1;
	_bitsLeft--;
	return theBit;
}

MuxPlayer::MuxPlayer(PrisonerEngine *vm) : _vm(vm), _opened(false) {
}

MuxPlayer::~MuxPlayer() {
}

bool MuxPlayer::open(const char *filename) {
	close();

	if (!_fd.open(filename))
		return false;

	_header.id = _fd.readUint32LE();
	_header.startOfs = _fd.readUint16LE();
	_header.frameCount = _fd.readUint32LE();
	_header.u3 = _fd.readUint16LE();
	_header.u4 = _fd.readUint16LE();
	_header.soundFreq = _fd.readUint16LE();
	_header.u5 = _fd.readUint32LE();
	_header.soundBufSize = _fd.readUint16LE();
	_header.u6 = _fd.readUint16LE();
	_header.u7 = _fd.readUint16LE();
	_header.u8 = _fd.readUint16LE();
	_header.width = _fd.readUint16LE();
	_header.height = _fd.readUint16LE();

	debug("width = %d; height = %d", _header.width, _header.height);
	debug("u3 = %d; u4 = %d; u5 = %d; u6 = %d; u7 = %d; u8 = %d", _header.u3, _header.u4, _header.u5, _header.u6, _header.u7, _header.u8);

	_frameBuffer = new byte[_header.width * _header.height];

	_chunkTableCount = (_header.startOfs - 32) / 4;
	_chunkTable = new uint32[_chunkTableCount];

	for (int i = 0; i < _chunkTableCount; i++)
		_chunkTable[i] = _fd.readUint32LE() & 0x3FFFFFFF;

	_stream = Audio::makeQueuingAudioStream(_header.soundFreq, false);

	_opened = true;

	return true;
}

void MuxPlayer::close() {
	if (_opened) {
		if (_vm->_mixer->isSoundHandleActive(_sound)) {
			_vm->_mixer->stopHandle(_sound);
		}
		delete[] _chunkTable;
		delete[] _frameBuffer;
		_fd.close();
		_opened = false;
	}
}

bool MuxPlayer::play() {

	uint32 startTick;
	bool aborted = false;

	if (!_opened)
		return true;

	startTick = _vm->_system->getMillis();

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sound, _stream);

	for (uint curFrame = 0; curFrame < _header.frameCount && !aborted; curFrame++) {

		if (_chunkTable[curFrame] != 0x3FFFFFFF) {
			_fd.seek(_chunkTable[curFrame]);
			handleFrame();
		}

		// TODO: Implement frame skipping?

		while (!aborted) {
			uint32 elapsedTime;

			if (_vm->_mixer->isSoundHandleActive(_sound)) {
				elapsedTime = _vm->_mixer->getSoundElapsedTime(_sound);
			} else {
				elapsedTime = _vm->_system->getMillis() - startTick;
			}

			// TODO: Figure out real framerate from header, 12fps is a good guess
			if (elapsedTime >= (curFrame * 1000) / 12)
				break;

			aborted = _vm->handleMuxInput();

		}

	}

	debug("Mux playback done.");

	return !aborted;

}

void MuxPlayer::handleFrame() {
	uint16 chunkType;
	uint32 chunkSize;

	static int chunkNum = 0;

	do {
		chunkType = _fd.readUint16LE();
		chunkSize = _fd.readUint32LE();

		debug("chunkNum = %d; ", chunkNum++);
		debug("chunkType = %04X; chunkSize = %d (%08X); ofs = %08X", chunkType, chunkSize, chunkSize, _fd.pos());

		switch (chunkType) {
		case kEndOfChunk:
			handleEndOfChunk(chunkSize);
			break;
		case kAudio1:
		case kAudio2:
			handleAudio(chunkSize);
			break;
		case kVideo:
			handleVideo(chunkSize);
			break;
		case kPalette:
			handlePalette(chunkSize);
			break;
		case kEndOfFile:
			// Nothing (handled in while condition)
			break;
		default:
			debug("skipping unknown chunk %02X", chunkType);
			_fd.seek(chunkSize, SEEK_CUR);
		}

	} while (chunkType != kEndOfChunk && chunkType != kEndOfFile);


}

void MuxPlayer::handleEndOfChunk(uint32 chunkSize) {
	_fd.seek(chunkSize, SEEK_CUR);
	_vm->_system->copyRectToScreen(_frameBuffer, _header.width, 0, 0, _header.width, _header.height);
	_vm->_system->updateScreen();
}

void MuxPlayer::handleAudio(uint32 chunkSize) {

	if (_stream) {
		byte *data = new byte[chunkSize];
		_fd.read(data, chunkSize);
		_stream->queueBuffer(data, chunkSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
	} else {
		_fd.seek(chunkSize, SEEK_CUR);
	}

}

void MuxPlayer::handleVideo(uint32 chunkSize) {
	byte flags;
	uint32 bufSize1, bufSize2;
	byte *buffer;

	chunkSize -= 13;

	flags = _fd.readByte();

	debug("flags = %02X", flags);

	_fd.readUint32LE();
	bufSize1 = _fd.readUint32LE();
	bufSize2 = _fd.readUint32LE();

	debug("chunkSize = %d; bufSize1 = %d; bufSize2 = %d", chunkSize, bufSize1, bufSize2);

	buffer = new byte[chunkSize];
	_fd.read(buffer, chunkSize);

	if (flags & 4) {
		byte *compBuffer = buffer;
		buffer = new byte[bufSize1 + bufSize2];
		debug("deompress...");
		decompress(compBuffer, buffer, chunkSize, bufSize1 + bufSize2);
		debug("deompress ok");
		delete[] compBuffer;
	}

	debug("decodeFrame...");
	decodeFrame(buffer, buffer + bufSize1, _frameBuffer, bufSize1, bufSize2, !(flags & 2));
	debug("decodeFrame ok");

	delete[] buffer;

}

void MuxPlayer::handlePalette(uint32 chunkSize) {
	_fd.read(_palette, 768);

	byte colors[1024];
	for (int i = 0; i < 256; i++) {
		colors[i * 4 + 0] = _palette[i * 3 + 0];
		colors[i * 4 + 1] = _palette[i * 3 + 1];
		colors[i * 4 + 2] = _palette[i * 3 + 2];
		colors[i * 4 + 3] = 0;
	}
	_vm->_system->setPalette(colors, 0, 256);

}

void MuxPlayer::decompress(byte *source, byte *dest, uint32 sourceSize, uint32 destSize) {
	MemoryBitReadStream src(source, sourceSize, 31);
	byte *dst = dest, *dstEnd = dest + destSize;
	byte ofs = 0, bitCounter = 1, lengthTmp = 0, length = 0;
	while (dst < dstEnd) {
		if (src.readBit() == 0) {
			ofs = src.readByte();
			if (bitCounter & 1) {
				lengthTmp = src.readByte();
				length = ((lengthTmp >> 4) & 0x0F) + 2;
			} else {
				length = (lengthTmp & 0x0F) + 2;
			}
			bitCounter ^= 1;
			while (length-- && dst < dstEnd)
				*dst++ = *(dst - ofs);
		} else {
			if (bitCounter & 1) {
				lengthTmp = src.readByte();
				length = ((lengthTmp >> 4) & 0x0F) + 1;
			} else {
				length = (lengthTmp & 0x0F) + 1;
			}
			bitCounter ^= 1;
			while (length-- && dst < dstEnd)
				*dst++ = src.readByte();
		}
	}
}

void MuxPlayer::decodeFrame(byte *buf1, byte *buf2, byte *dest, uint32 bufSize1, uint32 bufSize2,
	bool isKeyframe) {

	MemoryBitReadStream src(buf1, bufSize1, 32);
	byte *dst = dest;
	byte *buf2p = buf2, *buf2e = buf2 + bufSize2;
	bool whichNibble = true;
	uint16 copyLen, copyLenTmp = 0;

	while (buf2p < buf2e) {
		if (src.readBit() == 0) {
			*dst++ = *buf2p++;
		} else if (src.readBit() == 0) {
			if (whichNibble) {
				copyLenTmp = src.readByte();
				copyLen = (copyLenTmp >> 4) & 0x0F;
			} else {
				copyLen = copyLenTmp & 0x0F;
			}
			if (copyLen == 0) {
				copyLen = src.readByte() + 20;
			} else {
				copyLen += 4;
			}
			memset(dst, *buf2p, copyLen);
			dst += copyLen;
			buf2p++;
			whichNibble = !whichNibble;
		} else if (src.readBit() == 0) {
			*dst++ = *buf2p;
			*dst++ = *buf2p;
			buf2p++;
		} else if (src.readBit() == 0) {
			*dst++ = *buf2p;
			*dst++ = *buf2p;
			*dst++ = *buf2p;
			buf2p++;
		} else if (isKeyframe || src.readBit() == 0) {
			*dst++ = *buf2p;
			*dst++ = *buf2p;
			*dst++ = *buf2p;
			*dst++ = *buf2p;
			buf2p++;
		} else {
			dst += src.readByte();
		}
	}

}

} // End of namespace Prisoner
