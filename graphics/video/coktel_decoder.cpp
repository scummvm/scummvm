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

#include "graphics/video/coktel_decoder.h"

#ifdef GRAPHICS_VIDEO_COKTELDECODER_H

#include "sound/audiostream.h"

namespace Graphics {

CoktelDecoder::State::State() : flags(0), speechId(0) {
}


CoktelDecoder::CoktelDecoder(Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) :
	_mixer(&mixer), _soundType(soundType), _width(0), _height(0), _x(0), _y(0),
	_defaultX(0), _defaultY(0), _features(0), _frameCount(0), _paletteDirty(false),
	_ownSurface(true), _frameRate(12), _hasSound(false), _soundEnabled(false),
	_soundStage(kSoundNone), _audioStream(0) {

	memset(_palette, 0, 768);
}

CoktelDecoder::~CoktelDecoder() {
}

void CoktelDecoder::setSurfaceMemory(void *mem, uint16 width, uint16 height, uint8 bpp) {
	freeSurface();

	assert((width > 0) && (height > 0));
	assert(bpp == getPixelFormat().bytesPerPixel);

	_surface.w             = width;
	_surface.h             = height;
	_surface.pitch         = width * bpp;
	_surface.pixels        = mem;
	_surface.bytesPerPixel = bpp;

	_ownSurface = false;
}

void CoktelDecoder::setSurfaceMemory() {
	freeSurface();
	createSurface();

	_ownSurface = true;
}

bool CoktelDecoder::hasSurface() {
	return _surface.pixels != 0;
}

void CoktelDecoder::createSurface() {
	if (hasSurface())
		return;

	if ((_width > 0) && (_height > 0))
		_surface.create(_width, _height, getPixelFormat().bytesPerPixel);

	_ownSurface = true;
}

void CoktelDecoder::freeSurface() {
	if (!_ownSurface) {
		_surface.w             = 0;
		_surface.h             = 0;
		_surface.pitch         = 0;
		_surface.pixels        = 0;
		_surface.bytesPerPixel = 0;
	} else
		_surface.free();

	_ownSurface = true;
}

void CoktelDecoder::setXY(uint16 x, uint16 y) {
	_x = x;
	_y = y;
}

void CoktelDecoder::setXY() {
	setXY(_defaultX, _defaultY);
}

void CoktelDecoder::setFrameRate(Common::Rational frameRate) {
	_frameRate = frameRate;
}

uint16 CoktelDecoder::getDefaultX() const {
	return _defaultX;
}

uint16 CoktelDecoder::getDefaultY() const {
	return _defaultY;
}

const Common::List<Common::Rect> &CoktelDecoder::getDirtyRects() const {
	return _dirtyRects;
}

bool CoktelDecoder::hasSound() const {
	return _hasSound;
}

bool CoktelDecoder::isSoundEnabled() const {
	return _soundEnabled;
}

bool CoktelDecoder::isSoundPlaying() const {
	return _audioStream && _mixer->isSoundHandleActive(_audioHandle);
}

void CoktelDecoder::enableSound() {
	if (!hasSound() || isSoundEnabled())
		return;

	// Sanity check
	if (_mixer->getOutputRate() == 0)
		return;

	// Only possible on the first frame
	if (_curFrame > -1)
		return;

	_soundEnabled = true;
}

void CoktelDecoder::disableSound() {
	if (_audioStream) {

		if (_soundStage == kSoundPlaying) {
			_audioStream->finish();
			_mixer->stopHandle(_audioHandle);
		} else
			delete _audioStream;

	}

	_soundEnabled = false;
	_soundStage   = kSoundNone;

	_audioStream  = 0;
}

void CoktelDecoder::close() {
	disableSound();
	freeSurface();

	_x = 0;
	_y = 0;

	_defaultX = 0;
	_defaultY = 0;

	_features = 0;

	_frameCount = 0;

	_hasSound = false;
}

uint16 CoktelDecoder::getWidth() const {
	return _width;
}

uint16 CoktelDecoder::getHeight() const {
	return _height;
}

uint32 CoktelDecoder::getFrameCount() const {
	return _frameCount;
}

byte *CoktelDecoder::getPalette() {
	return _palette;
}

bool CoktelDecoder::hasDirtyPalette() const {
	return (_features & kFeaturesPalette) && _paletteDirty;
}

Common::Rational CoktelDecoder::getFrameRate() const {
	return _frameRate;
}

inline void CoktelDecoder::unsignedToSigned(byte *buffer, int length) {
	while (length-- > 0) *buffer++ ^= 0x80;
}


PreIMDDecoder::PreIMDDecoder(uint16 width, uint16 height,
	Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType),
	_stream(0), _videoBuffer(0), _videoBufferSize(0) {

	_width  = width;
	_height = height;
}

PreIMDDecoder::~PreIMDDecoder() {
	close();
}

bool PreIMDDecoder::seek(int32 frame, int whence, bool restart) {
	if (!isVideoLoaded())
		// Nothing to do
		return false;

	// Find the frame to which to seek
	if      (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _frameCount - frame - 1;
	else if (whence == SEEK_SET)
		frame--;
	else
		return false;

	if ((frame < -1) || (((uint32) frame) >= _frameCount))
		// Out of range
		return false;

	if (frame == _curFrame)
		// Nothing to do
		return true;

	// Run through the frames
	_curFrame = -1;
	_stream->seek(2);
	while (_curFrame != frame) {
		uint16 frameSize = _stream->readUint16LE();

		_stream->skip(frameSize + 2);

		_curFrame++;
	}

	return true;
}

bool PreIMDDecoder::load(Common::SeekableReadStream &stream) {
	// Since PreIMDs don't have any width and height values stored,
	// we need them to be specified in the constructor
	assert((_width > 0) && (_height > 0));

	close();

	_stream = &stream;

	_stream->seek(0);

	_frameCount = _stream->readUint16LE();

	_videoBufferSize = _width * _height;
	_videoBuffer     = new byte[_videoBufferSize];

	memset(_videoBuffer, 0, _videoBufferSize);

	return true;
}

void PreIMDDecoder::close() {
	reset();

	CoktelDecoder::close();

	delete _stream;

	delete[] _videoBuffer;

	_stream = 0;

	_videoBuffer     = 0;
	_videoBufferSize = 0;
}

bool PreIMDDecoder::isVideoLoaded() const {
	return _stream != 0;
}

Surface *PreIMDDecoder::decodeNextFrame() {
	if (!isVideoLoaded() || endOfVideo())
		return 0;

	createSurface();

	processFrame();
	renderFrame();

	if (_curFrame == 0)
		_startTime = g_system->getMillis();

	return &_surface;
}

void PreIMDDecoder::processFrame() {
	uint16 frameSize = _stream->readUint16LE();

	uint32 nextFramePos = _stream->pos() + frameSize + 2;

	byte cmd;

	cmd = _stream->readByte();
	frameSize--;

	if (cmd == 0) {
		// Palette. Ignored by Fascination, though

		_stream->skip(768);

		frameSize -= 769;

		cmd = _stream->readByte();
	}

	if (cmd != 2) {
		// Partial frame data

		uint32 fSize   = frameSize;
		uint32 vidSize = _videoBufferSize;

		byte *vidBuffer = _videoBuffer;

		while ((fSize > 0) && (vidSize > 0)) {
			uint32 n = _stream->readByte();
			fSize--;

			if ((n & 0x80) != 0) {
				// Data

				n = MIN<uint32>((n & 0x7F) + 1, MIN(fSize, vidSize));

				_stream->read(vidBuffer, n);

				vidBuffer += n;
				vidSize   -= n;
				fSize     -= n;

			} else {
				// Skip

				n = MIN<uint32>(n + 1, vidSize);

				vidBuffer += n;
				vidSize   -= n;
			}
		}

	} else {
		// Full direct frame

		uint32 vidSize = MIN<uint32>(_videoBufferSize, frameSize);

		_stream->read(_videoBuffer, vidSize);
	}

	_stream->seek(nextFramePos);

	_curFrame++;
}

void PreIMDDecoder::renderFrame() {
	_dirtyRects.clear();

	uint16 w = CLIP<int32>(_surface.w - _x, 0, _width);
	uint16 h = CLIP<int32>(_surface.h - _y, 0, _height);

	const byte *src = _videoBuffer;
	      byte *dst = (byte *) _surface.pixels + (_y * _surface.pitch) + _x;

	uint32 frameDataSize = _videoBufferSize;

	while (h-- > 0) {
		uint32 n = MIN<uint32>(w, frameDataSize);

		memcpy(dst, src, n);

		src += _width;
		dst += _surface.pitch;

		frameDataSize -= n;
	}

	_dirtyRects.push_back(Common::Rect(_x, _y, _x + _width, _y + _height));
}

PixelFormat PreIMDDecoder::getPixelFormat() const {
	return PixelFormat::createFormatCLUT8();
}


IMDDecoder::IMDDecoder(Audio::Mixer &mixer, Audio::Mixer::SoundType soundType) : CoktelDecoder(mixer, soundType),
	_stream(0), _version(0), _stdX(-1), _stdY(-1), _stdWidth(-1), _stdHeight(-1),
	_flags(0), _firstFramePos(0), _framePos(0), _frameCoords(0),
	_frameData(0), _frameDataSize(0), _frameDataLen(0),
	_videoBuffer(0), _videoBufferSize(0),
	_soundFlags(0), _soundFreq(0), _soundSliceSize(0), _soundSlicesCount(0) {

}

IMDDecoder::~IMDDecoder() {
	close();
}

bool IMDDecoder::seek(int32 frame, int whence, bool restart) {
	if (!isVideoLoaded())
		// Nothing to do
		return false;

	// Find the frame to which to seek
	if      (whence == SEEK_CUR)
		frame += _curFrame;
	else if (whence == SEEK_END)
		frame = _frameCount - frame - 1;
	else if (whence == SEEK_SET)
		frame--;
	else
		return false;

	if ((frame < -1) || (((uint32) frame) >= _frameCount))
		// Out of range
		return false;

	if (frame == _curFrame)
		// Nothing to do
		return true;

	// Try every possible way to find a file offset to that frame
	uint32 framePos = 0;
	if (frame == -1) {

		framePos = _firstFramePos;

	} else if (frame == 0) {

		framePos = _firstFramePos;
		_stream->seek(framePos);
		framePos += _stream->readUint16LE() + 4;

	} else if (_framePos) {

		framePos = _framePos[frame + 1];

	} else if (restart && (_soundStage == kSoundNone)) {

		for (int i = ((frame > _curFrame) ? _curFrame : 0); i <= frame; i++)
			processFrame();

		return true;

	} else {
		warning("Imd::seek(): Frame %d is not directly accessible", frame + 1);
		return false;
	}

	// Seek
	_stream->seek(framePos);
	_curFrame = frame;

	return true;
}

bool IMDDecoder::load(Common::SeekableReadStream &stream) {
	close();

	_stream = &stream;

	uint16 handle;

	handle   = _stream->readUint16LE();
	_version = _stream->readByte();

	// Version checking
	if ((handle != 0) || (_version < 2)) {
		warning("IMDDecoder::load(): Version incorrect (%d, 0x%X)", handle, _version);
		close();
		return false;
	}

	// Rest header
	_features      = _stream->readByte();
	_frameCount    = _stream->readUint16LE();
	_defaultX      = _stream->readSint16LE();
	_defaultY      = _stream->readSint16LE();
	_width         = _stream->readSint16LE();
	_height        = _stream->readSint16LE();
	_flags         = _stream->readUint16LE();
	_firstFramePos = _stream->readUint16LE();

	_x = _defaultX;
	_y = _defaultY;

	// IMDs always have video
	_features |= kFeaturesVideo;
	// IMDs always have palettes
	_features |= kFeaturesPalette;

	// Palette
	_stream->read((byte *) _palette, 768);

	_paletteDirty = true;

	if (!loadCoordinates()) {
		close();
		return false;
	}

	uint32 framePosPos, frameCoordsPos;
	if (!loadFrameTableOffsets(framePosPos, frameCoordsPos)) {
		close();
		return false;
	}

	if (!assessAudioProperties()) {
		close();
		return false;
	}

	if (!assessVideoProperties()) {
		close();
		return false;
	}

	if (!loadFrameTables(framePosPos, frameCoordsPos)) {
		close();
		return false;
	}

	// Seek to the first frame
	_stream->seek(_firstFramePos);

	return true;
}

bool IMDDecoder::loadCoordinates() {
	// Standard coordinates
	if (_version >= 3) {
		uint16 count = _stream->readUint16LE();
		if (count > 1) {
			warning("IMD: More than one standard coordinate quad found (%d)", count );
			return false;
		}

		if (count != 0) {
			_stdX      = _stream->readSint16LE();
			_stdY      = _stream->readSint16LE();
			_stdWidth  = _stream->readSint16LE();
			_stdHeight = _stream->readSint16LE();
			_features |= kFeaturesStdCoords;
		} else
			_stdX = _stdY = _stdWidth = _stdHeight = -1;

	} else
		_stdX = _stdY = _stdWidth = _stdHeight = -1;

	return true;
}

bool IMDDecoder::loadFrameTableOffsets(uint32 &framePosPos, uint32 &frameCoordsPos) {
	framePosPos    = 0;
	frameCoordsPos = 0;

	// Frame positions
	if (_version >= 4) {
		framePosPos = _stream->readUint32LE();
		if (framePosPos != 0) {
			_framePos  = new uint32[_frameCount];
			_features |= kFeaturesFramePos;
		}
	}

	// Frame coordinates
	if (_features & kFeaturesFrameCoords)
		frameCoordsPos = _stream->readUint32LE();

	return true;
}

bool IMDDecoder::assessVideoProperties() {
	// Sizes of the frame data and extra video buffer
	if (_features & kFeaturesDataSize) {
		_frameDataSize = _stream->readUint16LE();
		if (_frameDataSize == 0) {
			_frameDataSize   = _stream->readUint32LE();
			_videoBufferSize = _stream->readUint32LE();
		} else
			_videoBufferSize = _stream->readUint16LE();
	} else {
		_frameDataSize = _width * _height + 500;
		if (!(_flags & 0x100) || (_flags & 0x1000))
			_videoBufferSize = _frameDataSize;
	}

	// Allocating working memory
	_frameData = new byte[_frameDataSize + 500];
	memset(_frameData, 0, _frameDataSize + 500);

	_videoBuffer = new byte[_videoBufferSize + 500];
	memset(_videoBuffer, 0, _videoBufferSize + 500);

	return true;
}

bool IMDDecoder::assessAudioProperties() {
	if (_features & kFeaturesSound) {
		_soundFreq        = _stream->readSint16LE();
		_soundSliceSize   = _stream->readSint16LE();
		_soundSlicesCount = _stream->readSint16LE();

		if (_soundFreq < 0)
			_soundFreq = -_soundFreq;

		if (_soundSlicesCount < 0)
			_soundSlicesCount = -_soundSlicesCount - 1;

		if (_soundSlicesCount > 40) {
			warning("IMDDecoder::assessAudioProperties(): More than 40 sound slices found (%d)", _soundSlicesCount);
			return false;
		}

		_frameRate = Common::Rational(_soundFreq) / _soundSliceSize;

		_hasSound     = true;
		_soundEnabled = true;
		_soundStage   = kSoundLoaded;

		_audioStream = Audio::makeQueuingAudioStream(_soundFreq, false);
	}

	return true;
}

bool IMDDecoder::loadFrameTables(uint32 framePosPos, uint32 frameCoordsPos) {
	// Positions table
	if (_framePos) {
		_stream->seek(framePosPos);
		for (uint32 i = 0; i < _frameCount; i++)
			_framePos[i] = _stream->readUint32LE();
	}

	// Coordinates table
	if (_features & kFeaturesFrameCoords) {
		_stream->seek(frameCoordsPos);
		_frameCoords = new Coord[_frameCount];
		assert(_frameCoords);
		for (uint32 i = 0; i < _frameCount; i++) {
			_frameCoords[i].left   = _stream->readSint16LE();
			_frameCoords[i].top    = _stream->readSint16LE();
			_frameCoords[i].right  = _stream->readSint16LE();
			_frameCoords[i].bottom = _stream->readSint16LE();
		}
	}

	return true;
}

void IMDDecoder::close() {
	reset();

	CoktelDecoder::close();

	delete _stream;

	delete[] _framePos;
	delete[] _frameCoords;

	delete[] _frameData;

	delete[] _videoBuffer;

	_stream = 0;

	_version = 0;

	_stdX      = -1;
	_stdY      = -1;
	_stdWidth  = -1;
	_stdHeight = -1;

	_flags         = 0;

	_firstFramePos = 0;
	_framePos      = 0;
	_frameCoords   = 0;

	_frameData     = 0;
	_frameDataSize = 0;
	_frameDataLen  = 0;

	_videoBuffer     = 0;
	_videoBufferSize = 0;

	_soundFlags       = 0;
	_soundFreq        = 0;
	_soundSliceSize   = 0;
	_soundSlicesCount = 0;

	_hasSound     = false;
	_soundEnabled = false;
	_soundStage   = kSoundNone;
}

bool IMDDecoder::isVideoLoaded() const {
	return _stream != 0;
}

Surface *IMDDecoder::decodeNextFrame() {
	if (!isVideoLoaded() || endOfVideo())
		return 0;

	createSurface();

	processFrame();
	renderFrame();

	if (_curFrame == 0)
		_startTime = g_system->getMillis();

	return &_surface;
}

void IMDDecoder::processFrame() {
	_curFrame++;

	_dirtyRects.clear();

	_paletteDirty = false;

	uint32 cmd = 0;
	bool hasNextCmd = false;
	bool startSound = false;

	do {
		calcFrameCoords(_curFrame);

		cmd = _stream->readUint16LE();

		if ((cmd & kCommandBreakMask) == kCommandBreak) {
			// Flow control

			if (cmd == kCommandBreak) {
				_stream->skip(2);
				cmd = _stream->readUint16LE();
			}

			// Break
			if (cmd == kCommandBreakSkip0) {
				continue;
			} else if (cmd == kCommandBreakSkip16) {
				cmd = _stream->readUint16LE();
				_stream->skip(cmd);
				continue;
			} else if (cmd == kCommandBreakSkip32) {
				cmd = _stream->readUint32LE();
				_stream->skip(cmd);
				continue;
			}
		}

		// Audio
		if (_soundStage != kSoundNone) {
			if (cmd == kCommandNextSound) {

				nextSoundSlice(hasNextCmd);
				cmd = _stream->readUint16LE();

			} else if (cmd == kCommandStartSound) {

				startSound = initialSoundSlice(hasNextCmd);
				cmd = _stream->readUint16LE();

			} else
				emptySoundSlice(hasNextCmd);
		}

		// Set palette
		if (cmd == kCommandPalette) {
			_stream->skip(2);

			_paletteDirty = true;

			_stream->read(_palette, 768);
			cmd = _stream->readUint16LE();
		}

		hasNextCmd = false;

		if (cmd == kCommandJump) {
			// Jump to frame

			int16 frame = _stream->readSint16LE();
			if (_framePos) {
				_curFrame = frame - 1;
				_stream->seek(_framePos[frame]);

				hasNextCmd = true;
			}

		} else if (cmd == kCommandVideoData) {
			videoData(_stream->readUint32LE() + 2);

		} else if (cmd != 0)
			videoData(cmd + 2);
		else
			_dirtyRects.pop_back();

	} while (hasNextCmd);

	if (startSound && _soundEnabled) {
		_mixer->playStream(_soundType, &_audioHandle, _audioStream);
		_soundStage = kSoundPlaying;
	}

	if ((_curFrame >= (int32)(_frameCount - 1)) && (_soundStage == kSoundPlaying)) {
		_audioStream->finish();
		_mixer->stopHandle(_audioHandle);
		_audioStream = 0;
		_soundStage  = kSoundNone;
	}

}

void IMDDecoder::calcFrameCoords(uint32 frame) {
	if (frame == 0)
		_dirtyRects.push_back(Common::Rect(_x, _y, _x + _width, _y + _height));
	else if (_frameCoords && ((_frameCoords[frame].left != -1)))
		_dirtyRects.push_back(Common::Rect(_frameCoords[frame].left     , _frameCoords[frame].top,
		                                   _frameCoords[frame].right + 1, _frameCoords[frame].bottom + 1));
	else if (_stdX != -1)
		_dirtyRects.push_back(Common::Rect(_stdX, _stdY, _stdX + _stdWidth, _stdY + _stdHeight));
	else
		_dirtyRects.push_back(Common::Rect(_x, _y, _x + _width, _y + _height));
}

void IMDDecoder::videoData(uint32 size) {
	_stream->read(_frameData, size);
	_frameDataLen = size;

	renderFrame();
}

void IMDDecoder::renderFrame() {
	// TODO
}

void IMDDecoder::nextSoundSlice(bool hasNextCmd) {
	if (hasNextCmd || !_soundEnabled) {
		_stream->skip(_soundSliceSize);
		return;
	}

	byte *soundBuf = (byte *)malloc(_soundSliceSize);

	_stream->read(soundBuf, _soundSliceSize);
	unsignedToSigned(soundBuf, _soundSliceSize);

	_audioStream->queueBuffer(soundBuf, _soundSliceSize, DisposeAfterUse::YES, 0);
}

bool IMDDecoder::initialSoundSlice(bool hasNextCmd) {
	int dataLength = _soundSliceSize * _soundSlicesCount;

	if (hasNextCmd || !_soundEnabled) {
		_stream->skip(dataLength);
		return false;
	}

	byte *soundBuf = (byte *)malloc(dataLength);

	_stream->read(soundBuf, dataLength);
	unsignedToSigned(soundBuf, dataLength);

	_audioStream->queueBuffer(soundBuf, dataLength, DisposeAfterUse::YES, 0);

	return _soundStage == kSoundLoaded;
}

void IMDDecoder::emptySoundSlice(bool hasNextCmd) {
	if (hasNextCmd || !_soundEnabled)
		return;

	byte *soundBuf = (byte *)malloc(_soundSliceSize);

	memset(soundBuf, 0, _soundSliceSize);

	_audioStream->queueBuffer(soundBuf, _soundSliceSize, DisposeAfterUse::YES, 0);
}

PixelFormat IMDDecoder::getPixelFormat() const {
	return PixelFormat::createFormatCLUT8();
}

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELDECODER_H
