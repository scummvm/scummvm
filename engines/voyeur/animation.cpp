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

#include "voyeur/animation.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Video {

RL2Decoder::RL2Decoder() {
}

RL2Decoder::~RL2Decoder() {
	close();
}

bool RL2Decoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	stream->seek(8);

	// Check RL2 magic number
	uint32 signature = stream->readUint32LE();
	if (signature != 0x33564c52 /* RLV3 */ && signature != 0x32564c52 /* RLV2 */) {
		warning("RL2Decoder::loadStream(): attempted to load non-RL2 data (type = 0x%08X)", signature);
		return false;
	}

	addTrack(new RL2VideoTrack(stream));
	return true;
}

const Common::List<Common::Rect> *RL2Decoder::getDirtyRects() const {
	const Track *track = getTrack(0);

	if (track)
		return ((const RL2VideoTrack *)track)->getDirtyRects();

	return 0;
}

void RL2Decoder::clearDirtyRects() {
	Track *track = getTrack(0);

	if (track)
		((RL2VideoTrack *)track)->clearDirtyRects();
}

void RL2Decoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	Track *track = getTrack(0);

	if (track)
		((RL2VideoTrack *)track)->copyDirtyRectsToBuffer(dst, pitch);
}

RL2Decoder::RL2VideoTrack::RL2VideoTrack(Common::SeekableReadStream *stream) {
	_fileStream = stream;

	stream->seek(4);
	uint32 backSize = stream->readUint32LE();
	assert(backSize == 0 || backSize == (320 * 200));

	stream->seek(16);
	_frameCount = stream->readUint16LE();

	// Calculate the frame rate
	stream->seek(22);
	int soundRate = stream->readUint16LE();
	int rate = stream->readUint16LE();
	stream->skip(2);
	int defSoundSize = stream->readUint16LE();

	int fps = (soundRate > 0) ? rate / defSoundSize : 11025 / 1103;
	_frameDelay = 1000 / fps;

	_surface = new Graphics::Surface();
	_surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	if (backSize == 0) {
		_backSurface = NULL;
	} else {
		_backSurface = new Graphics::Surface();
		_backSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

		stream->seek(0x324);
		_fileStream->read((byte *)_backSurface->pixels, 320 * 200);
	}

	stream->seek(30);
	_videoBase = stream->readUint16LE();
	stream->seek(36);
	_palette = new byte[3 * 256];
	stream->read(_palette, 256 * 3);
	_dirtyPalette = true;

	_curFrame = 0;
	_nextFrameStartTime = 0;

	// Read in the frame offsets. Since we're only worried about the video data
	// here, we'll calculate video offsets taking into account sound data size
	stream->seek(0x324 + backSize + 4 * _frameCount);

	_frameOffset = new uint32[_frameCount];
	for (uint i = 0; i < _frameCount; ++i)
		_frameOffset[i] = _fileStream->readUint32LE();

	// Adust frame offsets to skip sound data
	for (uint i = 0; i < _frameCount; ++i)
		_frameOffset[i] += _fileStream->readUint32LE() & 0xffff;
}

RL2Decoder::RL2VideoTrack::~RL2VideoTrack() {
	delete _fileStream;
	delete[] _palette;
	delete[] _frameOffset;

	_surface->free();
	delete _surface;
	if (_backSurface) {
		_backSurface->free();
		delete _backSurface;
	}
}

bool RL2Decoder::RL2VideoTrack::endOfTrack() const {
	return getCurFrame() >= getFrameCount();
}

bool RL2Decoder::RL2VideoTrack::rewind() {
	_curFrame = 0;
	_nextFrameStartTime = 0;

	_fileStream->seek(_frameOffset[0]);

	return true;
}

uint16 RL2Decoder::RL2VideoTrack::getWidth() const {
	return _surface->w;
}

uint16 RL2Decoder::RL2VideoTrack::getHeight() const {
	return _surface->h;
}

Graphics::PixelFormat RL2Decoder::RL2VideoTrack::getPixelFormat() const {
	return _surface->format;
}

const Graphics::Surface *RL2Decoder::RL2VideoTrack::decodeNextFrame() {
	if (_curFrame == 0 && _backSurface) {
		// Read in the background frame
		_fileStream->seek(0x324);
		rl2DecodeFrameWithoutBackground(0);
		_dirtyRects.push_back(Common::Rect(0, 0, _surface->w, _surface->h));
	}

	_fileStream->seek(_frameOffset[_curFrame]);
		
	if (_backSurface) 
		rl2DecodeFrameWithBackground();
	else
		rl2DecodeFrameWithoutBackground();

	_curFrame++;
	_nextFrameStartTime += _frameDelay;

	return _surface;
}

void RL2Decoder::RL2VideoTrack::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		for (int y = (*it).top; y < (*it).bottom; ++y) {
			const int x = (*it).left;
			memcpy(dst + y * pitch + x, (byte *)_surface->pixels + y * getWidth() + x, (*it).right - x);
		}
	}

	clearDirtyRects();
}

void RL2Decoder::RL2VideoTrack::copyFrame(uint8 *data) {
	memcpy((byte *)_surface->pixels, data, getWidth() * getHeight());

	// Redraw
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, getWidth(), getHeight()));
}

void RL2Decoder::RL2VideoTrack::rl2DecodeFrameWithoutBackground(int screenOffset) {
	if (screenOffset == -1)
		screenOffset = _videoBase;
	byte *destP = (byte *)_surface->pixels + screenOffset;
	int frameSize = _surface->w * _surface->h - screenOffset;

	_fileStream->seek(_frameOffset[_curFrame]);
	while (frameSize > 0) {
		byte nextByte = _fileStream->readByte();

		if (nextByte < 0x80) {
			*destP++ = nextByte;
			--frameSize;
		} else if (nextByte == 0x80) {
			int runLength = _fileStream->readByte();
			if (runLength == 0)
				return;

			runLength = MIN(runLength, frameSize);
			Common::fill(destP, destP + runLength, 0);
			destP += runLength;
			frameSize -= runLength;
		} else {
			int runLength = _fileStream->readByte();
			
			runLength = MIN(runLength, frameSize);
			Common::fill(destP, destP + runLength, nextByte & 0x7f);
			destP += runLength;
			frameSize -= runLength;
		}
	}
}

void RL2Decoder::RL2VideoTrack::rl2DecodeFrameWithBackground() {
	int screenOffset = _videoBase;
	int frameSize = _surface->w * _surface->h - _videoBase;
	byte *src = (byte *)_backSurface->pixels;
	byte *dest = (byte *)_surface->pixels;

	_fileStream->seek(_frameOffset[_curFrame]);
	while (frameSize > 0) {
		byte nextByte = _fileStream->readByte();

		if (nextByte == 0) {
			dest[screenOffset] = src[screenOffset];
			++screenOffset;
			--frameSize;
		} else if (nextByte < 0x80) {
			dest[screenOffset] = nextByte | 0x80;
			++screenOffset;
			--frameSize;
		} else if (nextByte == 0x80) {
			byte runLength = _fileStream->readByte();
			if (runLength == 0)
				return;

			assert(runLength <= frameSize);
			Common::copy(src + screenOffset, src + screenOffset + runLength, dest);
			screenOffset += runLength;
			frameSize -= runLength;
		} else {
			byte runLength = _fileStream->readByte();
			
			assert(runLength <= frameSize);
			Common::fill(dest + screenOffset, dest + screenOffset + runLength, nextByte & 0x7f);
			screenOffset += runLength;
			frameSize -= runLength;
		}
	}
}

} // End of namespace Video
