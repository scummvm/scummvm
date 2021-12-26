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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/memstream.h"
#include "common/substream.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/video.h"
#include "engines/nancy/decompress.h"
#include "engines/nancy/graphics.h"

namespace Nancy {

AVFDecoder::~AVFDecoder() {
	close();
}

bool AVFDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	char id[15];
	stream->read(id, 15);
	id[14] = 0;
	Common::String idString = id;

	bool earlyHeaderFormat = false;

	if (idString == "AVF WayneSikes") {
		stream->skip(1); // Unknown
	} else if (idString.hasPrefix("ALG")) {
		earlyHeaderFormat = true;
		stream->seek(10, SEEK_SET);
	}

	uint32 chunkFileFormat;
	chunkFileFormat = stream->readUint16LE() << 16;
	chunkFileFormat |= stream->readUint16LE();

	if (chunkFileFormat != 0x00020000 && chunkFileFormat != 0x00010000) {
		warning("Unsupported version %d.%d found in AVF", chunkFileFormat >> 16, chunkFileFormat & 0xffff);
		return false;
	}

	if (!earlyHeaderFormat) {
		stream->skip(1); // Unknown
	}

	addTrack(new AVFVideoTrack(stream, chunkFileFormat));

	return true;
}

const Graphics::Surface *AVFDecoder::decodeFrame(uint frameNr) {
	return ((AVFDecoder::AVFVideoTrack *)getTrack(0))->decodeFrame(frameNr);
}

void AVFDecoder::addFrameTime(const uint16 timeToAdd) {
	((AVFDecoder::AVFVideoTrack *)getTrack(0))->_frameTime += timeToAdd;
}

AVFDecoder::AVFVideoTrack::AVFVideoTrack(Common::SeekableReadStream *stream, uint32 chunkFileFormat) {
	assert(stream);
	_fileStream = stream;
	_curFrame = -1;
	_refFrame = -1;
	_reversed = false;
	_dec = new Decompressor;

	_frameCount = stream->readUint16LE();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_depth = stream->readByte();
	_frameTime = stream->readUint32LE();

	byte comp = stream->readByte();

	uint formatHi = chunkFileFormat >> 16;

	if (formatHi == 1) {
		stream->skip(1);
	}

	if (comp != 2)
		error("Unknown compression type %d found in AVF", comp);

	_surface = new Graphics::Surface();
	_pixelFormat = g_nancy->_graphicsManager->getInputPixelFormat();
	_surface->create(_width, _height, _pixelFormat);
	_frameSize = _width * _height * _pixelFormat.bytesPerPixel;

	_chunkInfo.reserve(_frameCount);
	for (uint i = 0; i < _frameCount; i++) {
		ChunkInfo info;

		if (formatHi == 1) {
			char buf[9];
			stream->read(buf, 9);
			buf[8] = '\0';
			info.name = buf;
			info.index = stream->readUint32LE();

			stream->skip(4); // unknown

			info.offset = stream->readUint32LE();
			info.compressedSize = stream->readUint32LE();
			info.size = _frameSize;
			info.type = 0;
		} else if (formatHi == 2) {
			info.index = stream->readUint16LE();
			info.offset = stream->readUint32LE();
			info.compressedSize = stream->readUint32LE();
			info.size = stream->readUint32LE();
			info.type = stream->readByte();
			stream->skip(4); // Unknown;
		}

		_chunkInfo.push_back(info);
	}
}

AVFDecoder::AVFVideoTrack::~AVFVideoTrack() {
	delete _fileStream;
	_surface->free();
	delete _surface;
	delete _dec;
}

bool AVFDecoder::AVFVideoTrack::seek(const Audio::Timestamp &time) {
	// TODO this will almost definitely break video type 2
	_curFrame = getFrameAtTime(time);
	return true;
}

bool AVFDecoder::AVFVideoTrack::setReverse(bool reverse) {
	_reversed = reverse;
	return true;
}

bool AVFDecoder::AVFVideoTrack::endOfTrack() const {
	if (_reversed)
		return _curFrame < 0;

	return _curFrame >= (getFrameCount() - 1);
}

bool AVFDecoder::AVFVideoTrack::decode(byte *outBuf, uint32 frameSize, Common::ReadStream &inBuf) const {
	byte cmd = inBuf.readByte();
	while (!inBuf.eos()) {
		uint32 len, offset;
		switch (cmd) {
		case 0x20:
			// Write literal block
			offset = inBuf.readUint32LE() * 2;
			len = inBuf.readUint32LE() * 2;
			if (offset + len > frameSize)
				return false;
			inBuf.read(outBuf + offset, len);
			break;
		case 0x40: {
			// Write literal value 'n' times
			uint16 val = inBuf.readUint16LE();
			offset = inBuf.readUint32LE() * 2;
			len = inBuf.readUint32LE() * 2;
			if (offset + len > frameSize)
				return false;
			for (uint i = 0; i < len; i += 2)
				WRITE_LE_UINT16(outBuf + offset + i, val);
			break;
		}
		case 0x80: {
			// Write literal block 'n' times
			len = inBuf.readByte() * 2;
			uint32 offsetCount = inBuf.readUint32LE();
			byte buf[510];

			inBuf.read(buf, len);
			for (uint i = 0; i < offsetCount; ++i) {
				offset = inBuf.readUint32LE() * 2;
				if (offset + len > frameSize)
					return false;
				memcpy(outBuf + offset, buf, len);
			}
			break;
		}
		default:
			break;
		}
		cmd = inBuf.readByte();
	}
	return true;
}

const Graphics::Surface *AVFDecoder::AVFVideoTrack::decodeFrame(uint frameNr) {
	if (frameNr >= _chunkInfo.size()) {
		warning("Frame %d doesn't exist", frameNr);
		return nullptr;
	}

	const ChunkInfo &info = _chunkInfo[frameNr];

	if (info.type == 2 && (_refFrame == -1 || _refFrame != (int)frameNr - 1)) {
		warning("Cannot decode frame %d, reference frame is invalid", frameNr);
		return nullptr;
	}

	if (!info.size && !info.compressedSize) {
		if (info.type != 2) {
			warning("Found empty frame %d of type %d", frameNr, info.type);
			return nullptr;
		}
		// Return previous frame
		_refFrame = frameNr;
		return _surface;
	}

	byte *decompBuf = nullptr;
	if (info.type == 0) {
		// For type 0 we decompress straight to the surface, make sure we don't go out of bounds
		if (info.size > _frameSize) {
			warning("Decompressed size %d exceeds frame size %d", info.size, _frameSize);
			return nullptr;
		}
	} else {
		// For types 1 and 2, we decompress to a temp buffer for decoding
		decompBuf = new byte[info.size];
	}

	Common::MemoryWriteStream output((info.type == 0 ? (byte *)_surface->getPixels() : decompBuf), info.size);
	Common::SeekableSubReadStream input(_fileStream, info.offset, info.offset + info.compressedSize);

	if (!_dec->decompress(input, output)) {
		warning("Failed to decompress frame %d", frameNr);
		delete[] decompBuf;
		return nullptr;
	}

	if (info.type != 0) {
		Common::MemoryReadStream decompStr(decompBuf, info.size);
		decode((byte *)_surface->getPixels(), _frameSize, decompStr);
	}

	_refFrame = frameNr;
	delete[] decompBuf;
	return _surface;
}

const Graphics::Surface *AVFDecoder::AVFVideoTrack::decodeNextFrame() {
	return decodeFrame(_reversed ? _curFrame-- : ++_curFrame);
}

} // End of namespace Nancy
