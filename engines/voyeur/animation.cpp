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

RL2Decoder::RL2VideoTrack *RL2Decoder::getVideoTrack() {
	Track *track = getTrack(1);
	assert(track);

	return (RL2VideoTrack *)track;
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
	_isRLV3 = !strncmp((const char *)&_signature, "RLV3", 4);

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

	_hasBackFrame = header._backSize != 0;

	_backSurface = NULL;
	if (_hasBackFrame)
		initBackSurface();

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

void RL2Decoder::RL2VideoTrack::initBackSurface() {
	_backSurface = new Graphics::Surface();
	_backSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
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
	if (_curFrame == 0 && _hasBackFrame) {
		// Read in the initial background frame
		_fileStream->seek(0x324);
		rl2DecodeFrameWithoutTransparency(0);

		initBackSurface();
		Common::copy((byte *)_surface->getPixels(), (byte *)_surface->getPixels() + (320 * 200), 
			(byte *)_backSurface->getPixels());
		_dirtyRects.push_back(Common::Rect(0, 0, _surface->w, _surface->h));
	}

	// Move to the next frame data
	_fileStream->seek(_header._frameOffsets[_curFrame]);

	// If there's any sound data, pass it to the audio track
	_fileStream->seek(_header._frameSoundSizes[_curFrame], SEEK_CUR);

	// Decode the graphic data using the appropriate method depending on whether the animation
	// has a background or just raw frames without any background transparency
	if (_hasBackFrame) {
		rl2DecodeFrameWithTransparency(_videoBase);
	} else {
		rl2DecodeFrameWithoutTransparency(_videoBase);
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

void RL2Decoder::RL2VideoTrack::rl2DecodeFrameWithoutTransparency(int screenOffset) {
	if (screenOffset == -1)
		screenOffset = _videoBase;
	int frameSize = _surface->w * _surface->h - screenOffset;
	byte *destP = (byte *)_surface->getPixels();

	// Main frame decode loop
	byte nextByte;
	for (;;) {
		nextByte = _fileStream->readByte();

		if (nextByte < 0x80) {
			// Simple byte to copy to output
			assert(frameSize > 0);
			*destP++ = nextByte;
			--frameSize;
		} else if (nextByte > 0x80) {
			// Lower 7 bits a run length for the following byte
			byte runLength = _fileStream->readByte();
			assert(frameSize >= runLength);
			Common::fill(destP, destP + runLength, nextByte & 0x7f);
			destP += runLength;
			frameSize -= runLength;
		} else {
			// Follow byte run length for zeroes. If zero, indicates end of image
			byte runLength = _fileStream->readByte();
			if (runLength == 0)
				break;

			assert(frameSize >= runLength);
			Common::fill(destP, destP + runLength, 0);
			destP += runLength;
			frameSize -= runLength;
		}
	}

	// If there's any remaining screen area, zero it out
	byte *endP = (byte *)_surface->getPixels() + _surface->w * _surface->h;
	if (destP != endP)
		Common::fill(destP, endP, 0);
}

void RL2Decoder::RL2VideoTrack::rl2DecodeFrameWithTransparency(int screenOffset) {
	int frameSize = _surface->w * _surface->h;
	byte *refP = (byte *)_backSurface->getPixels();
	byte *destP = (byte *)_surface->getPixels();

	// If there's a screen offset, copy unchanged initial pixels from reference surface
	if (screenOffset > 0)
		Common::copy(refP, refP + screenOffset, destP);

	// Main decode loop
	for (;;) {
		byte nextByte = _fileStream->readByte();

		if (nextByte == 0) {
			// Move one single byte from reference surface
			assert(frameSize > 0);
			destP[screenOffset] = refP[screenOffset];
			++screenOffset;
			--frameSize;
		} else if (nextByte < 0x80) {
			// Raw byte to copy to output
			assert(frameSize > 0);
			destP[screenOffset] = nextByte;
			++screenOffset;
			--frameSize;
		} else if (nextByte == 0x80) {
			byte runLength = _fileStream->readByte();
			if (runLength == 0)
				return;

			assert(frameSize >= runLength);
			Common::copy(refP + screenOffset, refP + screenOffset + runLength, destP + screenOffset);
			screenOffset += runLength;
			frameSize -= runLength;
		} else {
			// Run length of a single pixel value
			byte runLength = _fileStream->readByte();
			nextByte &= 0x7f;

			assert(frameSize >= runLength);
			Common::fill(destP + screenOffset, destP + screenOffset + runLength, nextByte);
			screenOffset += runLength;
			frameSize -= runLength;
		}
	}

	// If there's a remaining section of the screen not covered, copy it from reference surface
	if (screenOffset < (_surface->w * _surface->h))
		Common::copy(refP + screenOffset, refP + (_surface->w * _surface->h), destP + screenOffset);
}

void RL2Decoder::RL2VideoTrack::setupBackSurface(Graphics::Surface *surface) {
	if (!_backSurface)
		initBackSurface();

	assert(surface->w == _backSurface->w && surface->h == _backSurface->h);
	const byte *srcP = (const byte *)surface->getPixels();
	byte *destP = (byte *)_backSurface->getPixels();
	Common::copy(srcP, srcP + surface->w * surface->h, destP);
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
