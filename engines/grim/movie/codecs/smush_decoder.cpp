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

#include "common/endian.h"
#include "common/rational.h"
#include "common/memstream.h"

#include "engines/grim/debug.h"
#include "engines/grim/movie/codecs/blocky16.h"
#include "engines/grim/movie/codecs/smush_decoder.h"
#include "engines/grim/movie/smush_wave.h"

namespace Grim {

SmushDecoder::SmushDecoder() {
	_file = nullptr;

	_startPos = 0;

	_videoTrack = nullptr;
	_wave = nullptr;
	_blocky16Frames = 0;
	_frameDelay = 0;
}

SmushDecoder::~SmushDecoder() {
	delete _videoTrack;
}

void SmushDecoder::init() {
	_videoTrack->init();
}

void SmushDecoder::close() {
	VideoDecoder::close();
	_videoTrack = nullptr;
	_startPos = 0;
	if (_file) {
		delete _file;
		_file = nullptr;
	}
}

bool SmushDecoder::readHeader() {
	if (!_file) {
		return false;
	}

	uint32 mainTag = _file->readUint32BE();
	uint32 size = _file->readUint32BE(); // file-size

	if (mainTag != MKTAG('S', 'A', 'N', 'M'))
		error("Invalid SMUSH-header");

	uint32 tag = _file->readUint32BE();
	size = _file->readUint32BE();
	uint32 pos = _file->pos();

	assert(tag == MKTAG('S', 'H', 'D', 'R'));

	_file->readUint16LE();
	uint16 nbFrames = _file->readUint32LE();
	_file->readUint16LE();
	int width = _file->readUint16LE();
	int height = _file->readUint16LE();
	_file->readUint16LE();
	int frameRate = _file->readUint32LE();
	_frameDelay = frameRate;

	int16 flags = _file->readUint16LE();
	// Output information for checking out the flags
	if (Debug::isChannelEnabled(Debug::Movie)) {
		warning("SMUSH Flags:");
		for (int i = 0; i < 16; i++) {
			warning(" %d", (flags & (1 << i)) != 0);
		}
	}

	_file->seek(pos + size + (size & 1), SEEK_SET);

	_videoTrack = new SmushVideoTrack(width, height, frameRate, nbFrames);
	addTrack(_videoTrack);
	return handleFramesHeader();
}

bool SmushDecoder::handleFramesHeader() {
	uint32 tag;
	int32 size;
	int pos = 0;

	tag = _file->readUint32BE();
	if (tag != MKTAG('F', 'L', 'H', 'D')) {
		return false;
	}
	size = _file->readUint32BE();
	byte *f_header = new byte[size];
	_file->read(f_header, size);

	// The disasm stops with 4 bytes or less left: the Wave header is declared 8 bytes
	// long, but it is followed by the total size of the audio, which is part of FLHD
	while (size - pos > 4) {
		int blockSize = READ_BE_UINT32(f_header + pos + 4);

		if (READ_BE_UINT32(f_header + pos) == MKTAG('B', 'l', '1', '6')) {
			pos += blockSize + 8;
		} else if (READ_BE_UINT32(f_header + pos) == MKTAG('W', 'a', 'v', 'e')) {
			_wave->flobdecDecodeHeader(f_header + pos + 8);
			pos += blockSize + 8;
		} else {
			error("SmushDecoder::handleFramesHeader() unknown tag");
		}
	}
	delete[] f_header;

	return true;
}

bool SmushDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_file = stream;

	// Load the video
	if (!readHeader()) {
		warning("Failure loading SMUSH-file");
		close();
		return false;
	}

	_startPos = _file->pos();

	init();
	return true;
}

void SmushDecoder::decodeFrameBlock(const byte *block, uint32 size, int flags) {
	Common::MemoryReadStream *memStream = new Common::MemoryReadStream(block, size, DisposeAfterUse::NO);

	int remaining = size;
	while (remaining > 4) {
		uint32 subType = memStream->readUint32BE();
		uint32 subSize = memStream->readUint32BE();
		uint32 subPos = memStream->pos();

		switch (subType) {
		case MKTAG('B', 'l', '1', '6'):
			_videoTrack->handleBlocky16(memStream, subSize, flags);
			_blocky16Frames++;
			break;
		case MKTAG('W', 'a', 'v', 'e'):
			_wave->flobdecDecodeBody(block + subPos);
			break;
		default:
			break;
		}

		remaining -= subSize + 8 + (subSize & 1);
		memStream->seek(subPos + subSize + (subSize & 1), SEEK_SET);
	}

	delete memStream;
	_videoTrack->finishFrame();
}

const Graphics::Surface *SmushDecoder::getFrameSurface() {
	return _videoTrack->decodeNextFrame();
}

bool SmushDecoder::rewind() {
	_wave->flobdecReset();

	return seekToFrame(0);
}

bool SmushDecoder::seekIntern(const Audio::Timestamp &time) {
	_file->seek(_startPos, SEEK_SET);
	_videoTrack->setCurFrame(-1);

	VideoDecoder::seekIntern(time);
	return true;
}

SmushDecoder::SmushVideoTrack::SmushVideoTrack(int width, int height, int fps, int numFrames) {
	_format = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_blocky16 = new Blocky16();
	_blocky16->init(width, height);
	_width = width;
	_height = height;
	_nbframes = numFrames;
	_frameRate = Common::Rational(1000000, fps);
	_curFrame = 0;
}

SmushDecoder::SmushVideoTrack::~SmushVideoTrack() {
	delete _blocky16;
	_surface.free();
}

void SmushDecoder::SmushVideoTrack::init() {
	_curFrame = -1;
	_surface.create(_width, _height, _format);
}

void SmushDecoder::SmushVideoTrack::finishFrame() {
	_curFrame++;
}

void SmushDecoder::SmushVideoTrack::handleBlocky16(Common::SeekableReadStream *stream, uint32 size, int flags) {
	byte *ptr = new byte[size];
	stream->read(ptr, size);

	_blocky16->decode((byte *)_surface.getPixels(), ptr, flags);
	delete[] ptr;
}

Graphics::Surface *SmushDecoder::SmushVideoTrack::decodeNextFrame() {
	return &_surface;
}

} // end of namespace Grim
