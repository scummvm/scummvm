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
#include "common/debug.h"
#include "common/memstream.h"
#include "common/substream.h"

#include "graphics/surface.h"

#include "nancy/video.h"
#include "nancy/decompress.h"

namespace Nancy {

AVFDecoder::~AVFDecoder() {
	close();
}

bool AVFDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	char id[15];
	stream->read(id, 15);
	id[14] = 0;

	if (stream->eos() || Common::String(id) != "AVF WayneSikes") {
		warning("Invalid id string found in AVF");
		return false;
	}

	stream->skip(1); // Unknown

	uint32 ver;
	ver = stream->readUint16LE() << 16;
	ver |= stream->readUint16LE();

	if (ver != 0x00020000) {
		warning("Unsupported version %d.%d found in AVF", ver >> 16, ver & 0xffff);
		return false;
	}

	addTrack(new AVFVideoTrack(stream));

	return true;
}

AVFDecoder::AVFVideoTrack::AVFVideoTrack(Common::SeekableReadStream *stream) {
	assert(stream);
	_fileStream = stream;
	_curFrame = -1;
	_refFrame = -1;
	_dec = new Decompressor;

	stream->skip(1); // Unknown

	_frameCount = stream->readUint16LE();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_depth = stream->readByte();
	_frameTime = stream->readUint32LE();

	byte comp = stream->readByte();

	if (comp != 2)
		error("Unknown compression type %d found in AVF", comp);

	_surface = new Graphics::Surface();
	_pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	_surface->create(_width, _height, _pixelFormat);
	_frameSize = _width * _height * _pixelFormat.bytesPerPixel;

	for (uint i = 0; i < _frameCount; i++) {
		ChunkInfo info;
		info.index = stream->readUint16LE();
		info.offset = stream->readUint32LE();
		info.compressedSize = stream->readUint32LE();
		info.size = stream->readUint32LE();
		info.type = stream->readByte();
		stream->skip(4); // Unknown;
		_chunkInfo.push_back(info);
	}
}

AVFDecoder::AVFVideoTrack::~AVFVideoTrack() {
	delete _fileStream;
	_surface->free();
	delete _surface;
	delete _dec;
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
		return 0;
	}

	const ChunkInfo &info = _chunkInfo[frameNr];

	if (info.type == 2 && (_refFrame == -1 || _refFrame != (int)frameNr - 1)) {
		warning("Cannot decode frame %d, reference frame is invalid", frameNr);
		return 0;
	}

	if (!info.size && !info.compressedSize) {
		if (info.type != 2) {
			warning("Found empty frame %d of type %d", frameNr, info.type);
			return 0;
		}
		// Return previous frame
		_refFrame = frameNr;
		return _surface;
	}

	byte *decompBuf = 0;
	if (info.type == 0) {
		// For type 0 we decompress straight to the surface, make sure we don't go out of bounds
		if (info.size > _frameSize) {
			warning("Decompressed size %d exceeds frame size %d", info.size, _frameSize);
			return 0;
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
		return 0;
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
	return decodeFrame(++_curFrame);
}

} // End of namespace Nancy
