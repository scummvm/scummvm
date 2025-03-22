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

#include "common/textconsole.h"

#include "alg/video.h"

#include "audio/decoders/raw.h"

namespace Alg {

AlgVideoDecoder::AlgVideoDecoder() {
	_stream = nullptr;
	_frame = nullptr;
	_audioStream = nullptr;
}

AlgVideoDecoder::~AlgVideoDecoder() {
	delete _frame;
	delete _audioStream;
}

void AlgVideoDecoder::loadVideoFromStream(uint32 offset) {
	_stream->seek(offset);
	_size = _stream->readUint32LE();
	_currentFrame = 0;
	uint16 chunkType = _stream->readUint16LE();
	uint32 chunkSize = _stream->readUint32LE();
	_numChunks = _stream->readUint16LE();
	_frameRate = _stream->readUint16LE();
	_videoMode = _stream->readUint16LE();
	_width = _stream->readUint16LE();
	_height = _stream->readUint16LE();
	uint16 typeRaw = _stream->readUint16LE();
	uint16 typeInter = _stream->readUint16LE();
	uint16 typeIntraHh = _stream->readUint16LE();
	uint16 typeInterHh = _stream->readUint16LE();
	uint16 typeIntraHhv = _stream->readUint16LE();
	uint16 typeInterHhv = _stream->readUint16LE();
	if (chunkSize == 0x18) {
		_audioType = _stream->readUint16LE();
	}
	assert(chunkType == 0x00);
	assert(chunkSize == 0x16 || chunkSize == 0x18);
	assert(_frameRate == 10);
	assert(_videoMode == 0x13);
	assert(typeRaw == 0x02);
	assert(typeInter == 0x05);
	assert(typeIntraHh == 0x0c);
	assert(typeInterHh == 0x0d);
	assert(typeIntraHhv == 0x0e);
	assert(typeInterHhv == 0x0f);
	_frame = new Graphics::Surface();
	_frame->create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());
	_audioStream = makePacketizedRawStream(8000, Audio::FLAG_UNSIGNED);
	_currentChunk = 0;
	_bytesLeft = _size - chunkSize - 6;
}

void AlgVideoDecoder::skipNumberOfFrames(uint32 num) {
	uint32 videoFramesSkipped = 0;
	while(videoFramesSkipped < num && _bytesLeft > 0) {
		uint16 chunkType = _stream->readUint16LE();
		uint32 chunkSize = _stream->readUint32LE();
		_currentChunk++;
		switch (chunkType) {
		case MKTAG16(0x00, 0x08):
		case MKTAG16(0x00, 0x0c):
		case MKTAG16(0x00, 0x0e):
		case MKTAG16(0x00, 0x05):
		case MKTAG16(0x00, 0x0d):
		case MKTAG16(0x00, 0x0f):
		case MKTAG16(0x00, 0x02):
			videoFramesSkipped++;
			_currentFrame++;
			break;
		}
		_stream->skip(chunkSize);
		_bytesLeft -= chunkSize + 6;
	}
	// find next keyframe
	bool nextKeyframeFound = false;
	while(!nextKeyframeFound && _bytesLeft > 0) {
		uint16 chunkType = _stream->readUint16LE();
		uint32 chunkSize = _stream->readUint32LE();
		_currentChunk++;
		switch (chunkType) {
		case MKTAG16(0x00, 0x08):
		case MKTAG16(0x00, 0x0c):
		case MKTAG16(0x00, 0x0e):
			nextKeyframeFound = true;
			_stream->seek(-6, SEEK_CUR);
			break;
		case MKTAG16(0x00, 0x05):
		case MKTAG16(0x00, 0x0d):
		case MKTAG16(0x00, 0x0f):
		case MKTAG16(0x00, 0x02):
			_stream->skip(chunkSize);
			_bytesLeft -= chunkSize + 6;
			videoFramesSkipped++;
			_currentFrame++;
			break;
		default:
			_stream->skip(chunkSize);
			_bytesLeft -= chunkSize + 6;
		}
	}
}

void AlgVideoDecoder::readNextChunk() {
	uint16 chunkType = _stream->readUint16LE();
	uint32 chunkSize = _stream->readUint32LE();
	_currentChunk++;
	switch (chunkType) {
	case MKTAG16(0x00, 0x00):
		error("got repeated header chunk");
		break;
	case MKTAG16(0x00, 0x30):
		updatePalette(chunkSize, false);
		break;
	case MKTAG16(0x00, 0x31):
		updatePalette(chunkSize, true);
		break;
	case MKTAG16(0x00, 0x15):
		readAudioData(chunkSize, 8000);
		break;
	case MKTAG16(0x00, 0x16):
		readAudioData(chunkSize, 11000);
		break;
	case MKTAG16(0x00, 0x08):
		decodeIntraFrame(chunkSize, 0, 0);
		_gotVideoFrame = true;
		break;
	case MKTAG16(0x00, 0x0c):
		decodeIntraFrame(chunkSize, 1, 0);
		_gotVideoFrame = true;
		break;
	case MKTAG16(0x00, 0x0e):
		decodeIntraFrame(chunkSize, 1, 1);
		_gotVideoFrame = true;
		break;
	case MKTAG16(0x00, 0x05):
		decodeInterFrame(chunkSize, 0, 0);
		_gotVideoFrame = true;
		break;
	case MKTAG16(0x00, 0x0d):
		decodeInterFrame(chunkSize, 1, 0);
		_gotVideoFrame = true;
		break;
	case MKTAG16(0x00, 0x0f):
		decodeInterFrame(chunkSize, 1, 1);
		_gotVideoFrame = true;
		break;
	case MKTAG16(0x00, 0x02):
		warning("raw video not supported");
		_stream->skip(chunkSize);
		break;
	default:
		error("Unknown chunk encountered: %d", chunkType);
	}
	_bytesLeft -= chunkSize + 6;
}

void AlgVideoDecoder::getNextFrame() {
	_paletteDirty = false;
	_gotVideoFrame = false;
	while (!_gotVideoFrame && _bytesLeft > 0) {
		readNextChunk();
	}
	_currentFrame++;
}

void AlgVideoDecoder::decodeIntraFrame(uint32 size, uint8 hh, uint8 hv) {
	uint16 x = 0, y = 0;
	int32 bytesRemaining = size;
	int32 runLength = 0;
	uint8 readByte, color = 0;
	while (bytesRemaining > 0) {
		readByte = _stream->readByte();
		if (readByte & 0x80) {
			runLength = 1;
			color = readByte;
			bytesRemaining--;
		} else {
			runLength = (readByte & 0x7F) + 2;
			color = _stream->readByte();
			bytesRemaining -= 2;
		}
		while (runLength > 0) {
			if (color > 0) {
				_frame->setPixel(x, y, color);
				if (hh) {
					_frame->setPixel(x + 1, y, color);
				}
				if (hv) {
					_frame->setPixel(x, y + 1, color);
					_frame->setPixel(x + 1, y + 1, color);
				}
			}
			x += 1 + hh;
			runLength--;
			if (x >= _width) {
				x = 0;
				y += 1 + hv;
			}
		}
	}
	assert(bytesRemaining == 0);
}

void AlgVideoDecoder::decodeInterFrame(uint32 size, uint8 hh, uint8 hv) {
	uint32 bytesRead = 0;
	uint16 length = 0, x = 0, y = 0, replacementBytesLeft = 0;
	replacementBytesLeft = _stream->readUint16LE();
	bytesRead += 2;
	if (replacementBytesLeft == 0) {
		_stream->skip(size - 2);
		return;
	}
	Common::SeekableReadStream *replacement = _stream->readStream(replacementBytesLeft);
	bytesRead += replacementBytesLeft;
	while (replacementBytesLeft > 1) {
		length = replacement->readByte();
		x = replacement->readByte() + ((length & 0x80) << 1);
		length &= 0x7F;
		replacementBytesLeft -= 2;
		if (length == 0) {
			y += x;
			continue;
		}
		for (uint32 i = 0; i < length; i++) {
			uint8 replaceArray = replacement->readByte();
			for (uint8 j = 0x80; j > 0; j = j >> 1) {
				if (replaceArray & j) {
					uint8 color = _stream->readByte();
					bytesRead++;
					_frame->setPixel(x, y, color);
					if (hh) {
						_frame->setPixel(x + 1, y, color);
					}
					if (hv) {
						_frame->setPixel(x, y + 1, color);
						_frame->setPixel(x + 1, y + 1, color);
					}
				}
				x += 1 + hh;
			}
		}
		y += 1 + hv;
	}
	delete replacement;
	assert(bytesRead == size);
}

void AlgVideoDecoder::updatePalette(uint32 size, bool partial) {
	_paletteDirty = true;
	uint32 bytesRead = 0;
	uint16 start = 0, count = 256;
	if (partial) {
		start = _stream->readUint16LE();
		count = _stream->readUint16LE();
		bytesRead += 4;
	}
	uint16 paletteIndex = start * 3;
	for (uint16 i = 0; i < count; i++) {
		int8 r = _stream->readByte() * 4;
		int8 g = _stream->readByte() * 4;
		int8 b = _stream->readByte() * 4;
		_palette[paletteIndex++] = r;
		_palette[paletteIndex++] = g;
		_palette[paletteIndex++] = b;
		bytesRead += 3;
	}
	assert(bytesRead == size);
}

void AlgVideoDecoder::readAudioData(uint32 size, uint16 rate) {
	assert(_audioType == 21);
	_audioStream->queuePacket(_stream->readStream(size));
}

} // End of namespace Alg
