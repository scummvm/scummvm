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
#include "voyeur/staticres.h"
#include "common/memstream.h"
#include "common/system.h"
#include "audio/decoders/raw.h"
#include "graphics/surface.h"

namespace Video {

RL2Decoder::RL2Decoder(Audio::Mixer::SoundType soundType) : _soundType(soundType) {
}

RL2Decoder::~RL2Decoder() {
	close();
}

bool RL2Decoder::loadVideo(int videoId) {
	Common::String filename = Common::String::format("%s.rl2", 
		::Voyeur::SZ_FILENAMES[videoId * 2]);
	return loadFile(filename);
}

bool RL2Decoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	// Load basic file information
	_header.load(stream);

	// Check RL2 magic number
	if (!_header.isValid()) {
		warning("RL2Decoder::loadStream(): attempted to load non-RL2 data (0x%08X)", _header._signature);
		return false;
	}

	// Add an audio track if sound is present
	RL2AudioTrack *audioTrack = NULL;
	if (_header._soundRate) {
		audioTrack = new RL2AudioTrack(_header, stream, _soundType);
		addTrack(audioTrack);
	}

	// Create a video track
	addTrack(new RL2VideoTrack(_header, audioTrack, stream));

	return true;
}

const Common::List<Common::Rect> *RL2Decoder::getDirtyRects() const {
	const Track *track = getTrack(1);

	if (track)
		return ((const RL2VideoTrack *)track)->getDirtyRects();

	return 0;
}

void RL2Decoder::clearDirtyRects() {
	Track *track = getTrack(1);

	if (track)
		((RL2VideoTrack *)track)->clearDirtyRects();
}

void RL2Decoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	Track *track = getTrack(1);

	if (track)
		((RL2VideoTrack *)track)->copyDirtyRectsToBuffer(dst, pitch);
}

Graphics::Surface *RL2Decoder::getVideoSurface() {
	Track *track = getTrack(1);
	assert(track);

	return ((RL2VideoTrack *)track)->getSurface();
}

/*------------------------------------------------------------------------*/

RL2Decoder::RL2FileHeader::RL2FileHeader() {
	_frameOffsets = NULL;
	_frameSoundSizes = NULL;
}

RL2Decoder::RL2FileHeader::~RL2FileHeader() {
	delete[] _frameOffsets;
	delete[] _frameSoundSizes;
}

void RL2Decoder::RL2FileHeader::load(Common::SeekableReadStream *stream) {
	stream->seek(0);

	_form = stream->readUint32LE();
	_backSize = stream->readUint32LE();
	_signature = stream->readUint32LE();

	if (!isValid())
		return;

	_dataSize = stream->readUint32LE();
	_numFrames = stream->readUint32LE();
	_method = stream->readUint16LE();
	_soundRate = stream->readUint16LE();
	_rate = stream->readUint16LE();
	_channels = stream->readUint16LE();
	_defSoundSize = stream->readUint16LE();
	_videoBase = stream->readUint16LE();
	_colorCount = stream->readUint32LE();
	assert(_colorCount <= 256);

	stream->read(_palette, 768);

	// Skip over background frame, if any, and the list of overall frame sizes (which we don't use)
	stream->skip(_backSize + 4 * _numFrames);

	// Load frame offsets
	_frameOffsets = new uint32[_numFrames];
	for (int i = 0; i < _numFrames; ++i)
		_frameOffsets[i] = stream->readUint32LE();

	// Load the size of the sound portion of each frame
	_frameSoundSizes = new int[_numFrames];
	for (int i = 0; i < _numFrames; ++i)
		_frameSoundSizes[i] = stream->readUint32LE() & 0xffff;
}

bool RL2Decoder::RL2FileHeader::isValid() const {
	return _signature == MKTAG('R','L','V','2') || _signature != MKTAG('R','L','V','3');
}

/*------------------------------------------------------------------------*/

RL2Decoder::RL2VideoTrack::RL2VideoTrack(const RL2FileHeader &header, RL2AudioTrack *audioTrack, 
		Common::SeekableReadStream *stream): 
		_header(header), _audioTrack(audioTrack), _fileStream(stream) {

	// Calculate the frame rate
	int fps = (header._soundRate > 0) ? header._rate / header._defSoundSize : 11025 / 1103;
	_frameDelay = 1000 / fps;

	_surface = new Graphics::Surface();
	_surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	if (header._backSize == 0 || !strncmp((char *)&header._signature, "RLV3", 4)) {
		_backSurface = NULL;
	} else {
		_backSurface = new Graphics::Surface();
		_backSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	}

	_videoBase = header._videoBase;
	_dirtyPalette = true;

	_curFrame = 0;
	_nextFrameStartTime = 0;
}

RL2Decoder::RL2VideoTrack::~RL2VideoTrack() {
	delete _fileStream;

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

		Common::copy((byte *)_surface->getPixels(), (byte *)_surface->getPixels() + (320 * 200), 
			(byte *)_backSurface->getPixels());
		_dirtyRects.push_back(Common::Rect(0, 0, _surface->w, _surface->h));
	}

	// Move to the next frame data
	_fileStream->seek(_header._frameOffsets[_curFrame]);

	// If there's any sound data, pass it to the audio track
	_fileStream->seek(_header._frameSoundSizes[_curFrame], SEEK_CUR);

	// Decode the graphic data
	if (_backSurface) {
		if (_curFrame > 0)
			Common::copy((byte *)_backSurface->getPixels(), (byte *)_backSurface->getPixels() + (320 * 200), 
				(byte *)_surface->getPixels());

		rl2DecodeFrameWithBackground();
	} else {
		rl2DecodeFrameWithoutBackground();
	}

	_curFrame++;
	_nextFrameStartTime += _frameDelay;

	return _surface;
}

void RL2Decoder::RL2VideoTrack::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	for (Common::List<Common::Rect>::const_iterator it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
		for (int y = (*it).top; y < (*it).bottom; ++y) {
			const int x = (*it).left;
			memcpy(dst + y * pitch + x, (byte *)_surface->getPixels() + y * getWidth() + x, (*it).right - x);
		}
	}

	clearDirtyRects();
}

void RL2Decoder::RL2VideoTrack::copyFrame(uint8 *data) {
	memcpy((byte *)_surface->getPixels(), data, getWidth() * getHeight());

	// Redraw
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(0, 0, getWidth(), getHeight()));
}

void RL2Decoder::RL2VideoTrack::rl2DecodeFrameWithoutBackground(int screenOffset) {
	if (screenOffset == -1)
		screenOffset = _videoBase;
	byte *destP = (byte *)_surface->getPixels() + screenOffset;
	int frameSize = _surface->w * _surface->h - screenOffset;

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
	byte *src = (byte *)_backSurface->getPixels();
	byte *dest = (byte *)_surface->getPixels();

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

/*------------------------------------------------------------------------*/

RL2Decoder::RL2AudioTrack::RL2AudioTrack(const RL2FileHeader &header, Common::SeekableReadStream *stream, Audio::Mixer::SoundType soundType): 
		_header(header), _soundType(soundType) {
	_audStream = createAudioStream();

	// Add all the sound data for all the frames at once to avoid stuttering
	for (int frameNumber = 0; frameNumber < header._numFrames; ++frameNumber) {
		int offset = _header._frameOffsets[frameNumber];
		int size = _header._frameSoundSizes[frameNumber];

		byte *data = (byte *)malloc(size);
		stream->seek(offset);
		stream->read(data, size);
		Common::MemoryReadStream *memoryStream = new Common::MemoryReadStream(data, size,
			DisposeAfterUse::YES);

		_audStream->queueAudioStream(Audio::makeRawStream(memoryStream, _header._rate, 
			Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), DisposeAfterUse::YES);
	}
}

RL2Decoder::RL2AudioTrack::~RL2AudioTrack() {
	delete _audStream;
}

void RL2Decoder::RL2AudioTrack::queueSound(Common::SeekableReadStream *stream, int size) {
	if (_audStream) {
		// Queue the sound data
		byte *data = (byte *)malloc(size);
		stream->read(data, size);
		Common::MemoryReadStream *memoryStream = new Common::MemoryReadStream(data, size,
			DisposeAfterUse::YES);

		_audStream->queueAudioStream(Audio::makeRawStream(memoryStream, _header._rate, 
			Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), DisposeAfterUse::YES);
		//		_audioTrack->queueSound(_fileStream, _header._frameSoundSizes[_curFrame]);

	} else {
		delete stream;
	}
}

Audio::AudioStream *RL2Decoder::RL2AudioTrack::getAudioStream() const {
	return _audStream;
}

Audio::QueuingAudioStream *RL2Decoder::RL2AudioTrack::createAudioStream() {
	return Audio::makeQueuingAudioStream(_header._rate, _header._channels == 2);
}

} // End of namespace Video
