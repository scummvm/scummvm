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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "voyeur/animation.h"
#include "voyeur/staticres.h"
#include "voyeur/voyeur.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "audio/decoders/raw.h"
#include "graphics/surface.h"

namespace Voyeur {

// Number of audio frames to keep audio track topped up when playing back video
#define SOUND_FRAMES_READAHEAD 3

RL2Decoder::RL2Decoder() {
	_paletteStart = 0;
	_fileStream = nullptr;
	_soundFrameNumber = -1;

	_audioTrack = nullptr;
	_videoTrack = nullptr;
}

RL2Decoder::~RL2Decoder() {
	close();
}

bool RL2Decoder::loadVideo(int videoId) {
	Common::String filename = Common::String::format("%s.rl2",
		::Voyeur::SZ_FILENAMES[videoId * 2]);
	return loadRL2File(filename, false);
}

bool RL2Decoder::loadRL2File(const Common::String &file, bool palFlag) {
	bool result = VideoDecoder::loadFile(file);
	_paletteStart = palFlag ? 0 : 128;
	return result;
}

bool RL2Decoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	// Load basic file information
	_fileStream = stream;
	_header.load(stream);
	_paletteStart = 0;

	// Check RL2 magic number
	if (!_header.isValid()) {
		warning("RL2Decoder::loadStream(): attempted to load non-RL2 data (0x%08X)", _header._signature);
		return false;
	}

	// Add an audio track if sound is present
	_audioTrack = nullptr;
	if (_header._soundRate) {
		_audioTrack = new RL2AudioTrack(_header, stream, getSoundType());
		addTrack(_audioTrack);
	}

	// Create a video track
	_videoTrack = new RL2VideoTrack(_header, _audioTrack, stream);
	addTrack(_videoTrack);

	// Load the offset/sizes of the video's audio data
	_soundFrames.reserve(_header._numFrames);
	for (int frameNumber = 0; frameNumber < _header._numFrames; ++frameNumber) {
		int offset = _header._frameOffsets[frameNumber];
		int size = _header._frameSoundSizes[frameNumber];

		_soundFrames.push_back(SoundFrame(offset, size));
	}

	return true;
}

const Common::List<Common::Rect> *RL2Decoder::getDirtyRects() const {
	if (_videoTrack)
		return _videoTrack->getDirtyRects();

	return nullptr;
}

void RL2Decoder::clearDirtyRects() {
	if (_videoTrack)
		_videoTrack->clearDirtyRects();
}

void RL2Decoder::copyDirtyRectsToBuffer(uint8 *dst, uint pitch) {
	if (_videoTrack)
		_videoTrack->copyDirtyRectsToBuffer(dst, pitch);
}

void RL2Decoder::readNextPacket() {
	int frameNumber = getCurFrame();
	RL2AudioTrack *audioTrack = getRL2AudioTrack();

	// Handle queueing sound data
	if (_soundFrameNumber == -1)
		_soundFrameNumber = (frameNumber == -1) ? 0 : frameNumber;

	while (audioTrack->numQueuedStreams() < SOUND_FRAMES_READAHEAD &&
			(_soundFrameNumber < (int)_soundFrames.size())) {
		_fileStream->seek(_soundFrames[_soundFrameNumber]._offset);
		audioTrack->queueSound(_fileStream, _soundFrames[_soundFrameNumber]._size);
		++_soundFrameNumber;
	}
}

bool RL2Decoder::seekIntern(const Audio::Timestamp &where) {
	_soundFrameNumber = -1;
	return VideoDecoder::seekIntern(where);
}

void RL2Decoder::close() {
	VideoDecoder::close();
	delete _fileStream;
	_fileStream = nullptr;
	_soundFrameNumber = -1;
}

/*------------------------------------------------------------------------*/

RL2Decoder::SoundFrame::SoundFrame(int offset, int size) {
	_offset = offset;
	_size = size;
}

/*------------------------------------------------------------------------*/

RL2Decoder::RL2FileHeader::RL2FileHeader() {
	_frameOffsets = nullptr;
	_frameSoundSizes = nullptr;

	_channels = 0;
	_colorCount = 0;
	_numFrames = 0;
	_rate = 0;
	_soundRate = 0;
	_videoBase = 0;
	_backSize = 0;
	_signature = MKTAG('N', 'O', 'N', 'E');
	_form = 0;
	_dataSize = 0;
	_method = 0;
	_defSoundSize = 0;
}

RL2Decoder::RL2FileHeader::~RL2FileHeader() {
	delete[] _frameOffsets;
	delete[] _frameSoundSizes;
}

void RL2Decoder::RL2FileHeader::load(Common::SeekableReadStream *stream) {
	stream->seek(0);

	_form = stream->readUint32LE();
	_backSize = stream->readUint32LE();
	_signature = stream->readUint32BE();

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
	delete[] _frameOffsets;
	_frameOffsets = new uint32[_numFrames];
	for (int i = 0; i < _numFrames; ++i)
		_frameOffsets[i] = stream->readUint32LE();

	// Load the size of the sound portion of each frame
	delete[] _frameSoundSizes;
	_frameSoundSizes = new int[_numFrames];
	for (int i = 0; i < _numFrames; ++i)
		_frameSoundSizes[i] = stream->readUint32LE() & 0xffff;
}

bool RL2Decoder::RL2FileHeader::isValid() const {
	return _signature == MKTAG('R','L','V','2') || _signature == MKTAG('R','L','V','3');
}

Common::Rational RL2Decoder::RL2FileHeader::getFrameRate() const {
	return (_soundRate > 0) ? Common::Rational(_rate, _defSoundSize) :
		Common::Rational(11025, 1103);
}

/*------------------------------------------------------------------------*/

RL2Decoder::RL2VideoTrack::RL2VideoTrack(const RL2FileHeader &header, RL2AudioTrack *audioTrack,
		Common::SeekableReadStream *stream): _header(header), _fileStream(stream) {

	_frameOffsets = nullptr;

	// Set up surfaces
	_surface = new Graphics::Surface();
	_surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_backSurface = nullptr;

	_hasBackFrame = header._backSize != 0;
	if (_hasBackFrame)
		initBackSurface();

	_videoBase = header._videoBase;
	_dirtyPalette = header._colorCount > 0;

	_curFrame = -1;
	_initialFrame = true;
}

RL2Decoder::RL2VideoTrack::~RL2VideoTrack() {
	// Free surfaces
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

bool RL2Decoder::RL2VideoTrack::seek(const Audio::Timestamp &time) {
	int frame = getFrameAtTime(time);

	if (frame < 0 || frame >= _header._numFrames)
		return false;

	_curFrame = frame;
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
	if (_initialFrame && _hasBackFrame) {
		// Read in the initial background frame
		_fileStream->seek(0x324);
		rl2DecodeFrameWithoutTransparency(0);

		Common::copy((byte *)_surface->getPixels(), (byte *)_surface->getPixels() + (320 * 200),
			(byte *)_backSurface->getPixels());
		_dirtyRects.push_back(Common::Rect(0, 0, _surface->w, _surface->h));
		_initialFrame = false;
	}

	// Move to the next frame data
	_fileStream->seek(_header._frameOffsets[++_curFrame]);

	// If there's any sound data, pass it to the audio track
	_fileStream->seek(_header._frameSoundSizes[_curFrame], SEEK_CUR);

	// Decode the graphic data using the appropriate method depending on whether the animation
	// has a background or just raw frames without any background transparency
	if (_backSurface) {
		rl2DecodeFrameWithTransparency(_videoBase);
	} else {
		rl2DecodeFrameWithoutTransparency(_videoBase);
	}

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
			int runLength = _fileStream->readByte();
			runLength = MIN(runLength, frameSize);

			Common::fill(destP, destP + runLength, nextByte & 0x7f);
			destP += runLength;
			frameSize -= runLength;
		} else {
			// Follow byte run length for zeroes. If zero, indicates end of image
			int runLength = _fileStream->readByte();
			if (runLength == 0)
				break;

			runLength = MIN(runLength, frameSize);
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
	int frameSize = _surface->w * _surface->h - screenOffset;
	byte *refP = (byte *)_backSurface->getPixels();
	byte *destP = (byte *)_surface->getPixels();

	// If there's a screen offset, copy unchanged initial pixels from reference surface
	if (screenOffset > 0)
		Common::copy(refP, refP + screenOffset, destP);

	// Main decode loop
	while (frameSize > 0) {
		byte nextByte = _fileStream->readByte();

		if (nextByte == 0) {
			// Move one single byte from reference surface
			assert(frameSize > 0);
			destP[screenOffset] = refP[screenOffset];
			++screenOffset;
			--frameSize;
		} else if (nextByte < 0x80) {
			// Single 7-bit pixel to output (128-255)
			assert(frameSize > 0);
			destP[screenOffset] = nextByte | 0x80;
			++screenOffset;
			--frameSize;
		} else if (nextByte == 0x80) {
			int runLength = _fileStream->readByte();
			if (runLength == 0)
				return;

			// Run length of transparency (i.e. pixels to copy from reference frame)
			runLength = MIN(runLength, frameSize);

			Common::copy(refP + screenOffset, refP + screenOffset + runLength, destP + screenOffset);
			screenOffset += runLength;
			frameSize -= runLength;
		} else {
			// Run length of a single pixel value
			int runLength = _fileStream->readByte();
			runLength = MIN(runLength, frameSize);

			Common::fill(destP + screenOffset, destP + screenOffset + runLength, nextByte);
			screenOffset += runLength;
			frameSize -= runLength;
		}
	}

	// If there's a remaining section of the screen not covered, copy it from reference surface
	if (screenOffset < (_surface->w * _surface->h))
		Common::copy(refP + screenOffset, refP + (_surface->w * _surface->h), destP + screenOffset);
}

Graphics::Surface *RL2Decoder::RL2VideoTrack::getBackSurface() {
	if (!_backSurface)
		initBackSurface();

	return _backSurface;
}

/*------------------------------------------------------------------------*/

RL2Decoder::RL2AudioTrack::RL2AudioTrack(const RL2FileHeader &header, Common::SeekableReadStream *stream, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType),
		_header(header) {
	// Create audio straem for the audio track
	_audStream = Audio::makeQueuingAudioStream(_header._rate, _header._channels == 2);
}

RL2Decoder::RL2AudioTrack::~RL2AudioTrack() {
	delete _audStream;
}

void RL2Decoder::RL2AudioTrack::queueSound(Common::SeekableReadStream *stream, int size) {
	// Queue the sound data
	byte *data = (byte *)malloc(size);
	stream->read(data, size);
	Common::MemoryReadStream *memoryStream = new Common::MemoryReadStream(data, size,
		DisposeAfterUse::YES);

	_audStream->queueAudioStream(Audio::makeRawStream(memoryStream, _header._rate,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), DisposeAfterUse::YES);
}

Audio::AudioStream *RL2Decoder::RL2AudioTrack::getAudioStream() const {
	return _audStream;
}

void RL2Decoder::play(VoyeurEngine *vm, int resourceOffset,
		byte *frames, byte *imgPos) {
	vm->flipPageAndWait();
	int paletteStart = getPaletteStart();
	int paletteCount = getPaletteCount();

	PictureResource videoFrame(getRL2VideoTrack()->getBackSurface());
	int picCtr = 0;
	while (!vm->shouldQuit() && !endOfVideo() && !vm->_eventsManager->_mouseClicked) {
		if (hasDirtyPalette()) {
			const byte *palette = getPalette();

			vm->_screen->setPalette128(palette, paletteStart, paletteCount);
		}

		if (needsUpdate()) {
			if (frames) {
				// If reached a point where a new background is needed, load it
				// and copy over to the video decoder
				if (getCurFrame() >= READ_LE_UINT16(frames + picCtr * 4)) {
					PictureResource *newPic = vm->_bVoy->boltEntry(0x302 + picCtr)._picResource;
					Common::Point pt(READ_LE_UINT16(imgPos + 4 * picCtr) - 32,
						READ_LE_UINT16(imgPos + 4 * picCtr + 2) - 20);

					vm->_screen->sDrawPic(newPic, &videoFrame, pt);
					++picCtr;
				}
			}

			// Decode the next frame and display
			const Graphics::Surface *frame = decodeNextFrame();
			vm->_screen->blitFrom(*frame);
		}

		vm->_eventsManager->getMouseInfo();
		g_system->delayMillis(10);
	}
}

} // End of namespace Voyeur
