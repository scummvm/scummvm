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
	_dec = new Decompressor;

	stream->skip(1); // Unknown

	_frameCount = stream->readUint16LE();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_depth = stream->readByte();
	stream->skip(4); // Unknown

	byte comp = stream->readByte();

	if (comp != 1 && comp != 2)
		error("Unknown compression type %d found in AVF", comp);

	_compressed = comp == 2;

	_surface = new Graphics::Surface();
	_pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	_surface->create(_width, _height, _pixelFormat);

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

const Graphics::Surface *AVFDecoder::AVFVideoTrack::decodeNextFrame() {
	_curFrame++;
	_fileStream->seek(_chunkInfo[_curFrame].offset);
	uint size = _chunkInfo[_curFrame].size;

	if (_chunkInfo[_curFrame].type != 0) {
		warning("Skipping frame type %d", _chunkInfo[_curFrame].type);
		return _surface;
	}

	if (_compressed) {
		Common::ReadStream *input = _fileStream->readStream(_chunkInfo[_curFrame].compressedSize);
		Common::MemoryWriteStream output((byte *)_surface->getPixels(), size);
		if (!_dec->decompress(*input, output))
			warning("Failed to decompress frame %d in AVF", _curFrame);
		delete input;
	} else {
		if (_fileStream->read(_surface->getPixels(), size) < size)
			warning("Read error trying to copy uncompressed frame");
	}

	return _surface;
}

} // End of namespace Nancy
