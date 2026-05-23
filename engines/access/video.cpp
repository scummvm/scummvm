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

#include "audio/decoders/raw.h"

#include "access/video.h"
#include "access/access.h"

namespace Access {

VideoPlayer::VideoPlayer(AccessEngine *vm) : Manager(vm), _videoData(nullptr),
_videoFrame(0), _soundFrame(0), _videoEnd(false), _soundFlag(false), _vidSurface(nullptr) {
}

VideoPlayer::~VideoPlayer() {
	closeVideo();
}

void VideoPlayer::setVideo(BaseSurface *vidSurface, const Common::Point &pt, const Common::Path &filename, int rate) {
	// Open up video stream
	_videoData = _vm->_files->loadRawFile(filename);

	_vidSurface = vidSurface;
	setVideo(pt);
	setRate(rate);
}

void VideoPlayer::setVideo(BaseSurface *vidSurface, const Common::Point &pt, const FileIdent &videoFile, int rate) {
	// Open up video stream
	_videoData = _vm->_files->loadFile(videoFile);

	_vidSurface = vidSurface;
	setVideo(pt);
	setRate(rate);
}

void VideoPlayer::closeVideo() {
	delete _videoData;
	_videoData = nullptr;
}

void VideoPlayer::playToEnd() {
	Common::CustomEventType action = kActionNone;
	while (!_vm->shouldQuit() && _videoData && !_videoEnd) {
		playVideo();
		_vm->_events->pollEvents();

		_vm->_events->getAction(action);
		if (action == kActionSkip) {
			_videoEnd = true;
			continue;
		}

		delayToNextFrame();
	}
}

////////////////////////////////////

VideoPlayer_v1::VideoPlayer_v1(AccessEngine *vm) : VideoPlayer(vm) {
	_vidSurface = nullptr;
	_startCoord = nullptr;

	_frameCount = 0;
	_xCount = 0;
	_scanCount = 0;
	_frameSize = 0;
	_rate = 0;

	_header._frameCount = 0;
	_header._width = _header._height = 0;
	_header._flags = VIDEOFLAG_NONE;
}


void VideoPlayer_v1::setVideo(const Common::Point &pt) {
	_vidSurface->_orgX1 = pt.x;
	_vidSurface->_orgY1 = pt.y;

	// Load in header
	_header._frameCount = _videoData->_stream->readUint16LE();
	_header._width = _videoData->_stream->readUint16LE();
	_header._height = _videoData->_stream->readUint16LE();
	_videoData->_stream->skip(1);
	_header._flags = (VideoFlags)_videoData->_stream->readByte();

	_startCoord = (byte *)_vidSurface->getBasePtr(pt.x, pt.y);
	_frameCount = _header._frameCount - 2;
	_xCount = _header._width;
	_scanCount = _header._height;
	_videoFrame = 0;
	_videoBounds = Common::Rect(pt.x, pt.y, pt.x + _header._width, pt.y + _header._height);

	getFrame();

	if (_header._flags == VIDEOFLAG_BG) {
		// Draw the background
		for (int y = 0; y < _scanCount; ++y) {
			byte *pDest = (byte *)_vidSurface->getBasePtr(pt.x, pt.y + y);
			_videoData->_stream->read(pDest, _xCount);
		}

		if (_vidSurface == _vm->_screen) {
			assert(pt.x >= 0 && pt.y >= 0 && pt.x < _vm->_screen->w && pt.y < _vm->_screen->h &&
				_xCount > 0 && _xCount <= _vm->_screen->w && _scanCount > 0 && _scanCount <= _vm->_screen->h);
			_vm->_newRects.push_back(Common::Rect(pt.x, pt.y, pt.x + _xCount, pt.y + _scanCount));
		}

		getFrame();
	}

	_videoEnd = false;
}

void VideoPlayer_v1::setRate(int rate) {
	_rate = rate;
	_vm->_timers[31]._timer = rate;
	_vm->_timers[31]._initTm = _rate;
}


void VideoPlayer_v1::getFrame() {
	_frameSize = _videoData->_stream->readUint16LE();
}

void VideoPlayer_v1::playVideo() {
	if (_vm->_timers[31]._flag)
		return;
	_vm->_timers[31]._flag = 1;

	byte *pDest = _startCoord;
	byte *pLine = _startCoord;
	uint32 frameEnd = _videoData->_stream->pos() + _frameSize;

	if (frameEnd > _videoData->_stream->size())
		error("VideoPlayer::playVideo: Frame end %d > stream size %d", frameEnd, (int)_videoData->_stream->size());

	while ((uint32)_videoData->_stream->pos() < frameEnd) {
		int count = _videoData->_stream->readByte();

		if (count & 0x80) {
			count &= 0x7f;

			// Skip count number of pixels
			// Loop across lines if necessary
			while (count >= (pLine + _xCount - pDest)) {
				count -= (pLine + _xCount - pDest);
				pLine += _vidSurface->pitch;
				pDest = pLine;
			}

			// Skip any remaining pixels in the new line
			pDest += count;
		} else {
			// Read count number of pixels

			// Load across lines if necessary
			while (count >= (pLine + _xCount - pDest)) {
				int lineCount = (pLine + _xCount - pDest);
				_videoData->_stream->read(pDest, lineCount);
				count -= lineCount;
				pLine += _vidSurface->pitch;
				pDest = pLine;
			}

			// Load remainder of pixels on line
			if (count > 0) {
				_videoData->_stream->read(pDest, count);
				pDest += count;
			}
		}
	}

	// If the video is playing on the screen surface, add a dirty rect
	if (_vidSurface == _vm->_screen)
		_vm->_screen->markAllDirty();

	_vm->_screen->dump("vidframe");

	getFrame();
	if (++_videoFrame == _frameCount) {
		closeVideo();
		_videoEnd = true;
	}
}

void VideoPlayer_v1::delayToNextFrame() {
	_vm->_events->delay(1000 / _rate - 20);
}

void VideoPlayer_v1::copyVideo() {
	// aka drawTalkVideoFrame
	_vm->_player->calcPlayer();

	// Figure out the dirty rect area for the video frame
	Common::Rect r = Common::Rect(_vm->_vidX - _vm->_screen->_bufferStart.x,
		_vm->_vidY - _vm->_screen->_bufferStart.y,
		_vm->_vidX - _vm->_screen->_bufferStart.x + _header._width,
		_vm->_vidY - _vm->_screen->_bufferStart.y + _header._height);
	if (_vm->_screen->clip(r))
		return;
	assert(r.left >= 0 && r.left < _vm->_screen->w && r.top >= 0 && r.top < _vm->_screen->h &&
		r.right > 0 && r.right <= _vm->_screen->w && r.bottom > 0 && r.bottom <= _vm->_screen->h);
	_vm->_newRects.push_back(r);

	// Draw the clipped video to the buffer
	int vh = r.height();
	int vw = r.width();
	int destIdx = r.left + r.top * _vm->_buffer2.pitch;
	int srcIdx = _vm->_screen->_leftSkip + _vm->_screen->_topSkip * _vm->_vidBuf.pitch;

	assert(srcIdx >= 0);
	assert(destIdx >= 0);

	const byte *srcP = (const byte *)_vm->_vidBuf.getPixels() + srcIdx;
	byte *destP = (byte *)_vm->_buffer2.getPixels() + destIdx;
	for (int i = 0; i < vh; i++) {
		Common::copy(srcP, srcP + vw, destP);
		srcP += _vm->_vidBuf.pitch;
		destP += _vm->_buffer2.pitch;
	}
}

//////////////////////////////////////////////////

VideoPlayer_v2::VideoPlayer_v2(AccessEngine *vm, bool setPal) : VideoPlayer(vm), _audioStream(nullptr),
_frame(nullptr), _nextFrameTime(0), _setPal(setPal), _startMs(0), _drawBorder(false) {
}

void VideoPlayer_v2::setVideo(const Common::Point &pt) {
	_header._id = _videoData->_stream->readUint32LE();
	_header._version = _videoData->_stream->readByte();
	_header._frameCount = _videoData->_stream->readUint16LE();
	_header._width = _videoData->_stream->readUint16LE();
	_header._height = _videoData->_stream->readUint16LE();
	_header._frameIncr = _videoData->_stream->readUint16LE();
	_header._unk = _videoData->_stream->readUint16LE();

	if (_header._id != 0x444956) // 'VID' (LE)
		warning("Video header for %s is not VID", _videoData->getFileName());

	_videoFrame = 0;
	_audioStream = NULL;
	_videoEnd = false;
	_vidSurface->_orgX1 = pt.x;
	_vidSurface->_orgY1 = pt.y;

	_frame = new BaseSurface();
	_frame->create(_header._width, _header._height, Graphics::PixelFormat::createFormatCLUT8());

	_startMs = _vm->_events->getPriorFrameTime();
	_delayTotal = 0;

	debugC(kDebugGraphics, "Load video V2: id = %d, version = %d, frameCount = %d, width = %d, height = %d, frameIncr = %d, unk = %d",
	  _header._id, _header._version, _header._frameCount, _header._width, _header._height, _header._frameIncr, _header._unk);

	playVideo();
}

void VideoPlayer_v2::playVideo() {
	if (_videoEnd || !_videoData)
		return;

	debugC(kDebugGraphics, "VideoPlayer_v2::handleChunk() %08X", (int)_videoData->_stream->pos());

	byte type = _videoData->_stream->readByte();

	debugC(kDebugGraphics, "VideoPlayer_v2::handleChunk() type = %d; _currFrame = %d; frameCount = %d", type, _videoFrame, _header._frameCount);

	switch (type) {
		case 0:
			// This may be a bug in our decoder - it seems to be
			// correctly handled if we just skip?
			//handleStraitChunk();
			break;
		case 1:
			// aka doDiff
			handleFrameChunk(true, false);
			break;
		case 2:
			handlePaletteChunk();
			break;
		case 3:
			// aka doComp
			handleFrameChunk(false, false);
			break;
		case 4:
			// aka doDiff2
			handleFrameChunk(true, true);
			break;
		case 0x7C:
			handleSoundChunk(true);
			break;
		case 0x7D:
			handleSoundChunk(false);
			break;
		case 0x14:
			closeVideo();
			_videoEnd = true;
			break;
		default:
			warning("VideoPlayer_v2::handleChunk() Unknown chunk type %d at %08X", type, (int)_videoData->_stream->pos() - 1);
			closeVideo();
			_videoEnd = true;
	}

}

void VideoPlayer_v2::handlePaletteChunk() {
	debugC(kDebugGraphics, "VideoPlayer_v2::handlePaletteChunk()");
	byte buf[768];
	_videoData->_stream->read(buf, 768);
	for (int i = 0; i < 768; i++)
		buf[i] *= 4;
	_pal = Graphics::Palette(buf, 256);
	if (_setPal) {
		setVideoPalNow();
		// only do it once?
		_setPal = false;
	}
}

void VideoPlayer_v2::setVideoPalNow() {
	_vm->_screen->setRawPalette(_pal);
	_vm->_screen->setPalette();
	_vm->_screen->copyRawPalToTempPal();
}

void VideoPlayer_v2::calcNextFrameTime(int delay) {
	if (delay)
		_delayTotal += delay;
	uint32 elapsedForNextFrame = _header._frameIncr * 1000 * (_videoFrame + 1) / 60;
	_nextFrameTime = _startMs + _delayTotal * 1000 / 60 + elapsedForNextFrame;
}

void VideoPlayer_v2::handleFrameChunk(bool delta, bool skipLines) {
	debugC(kDebugGraphics, "VideoPlayer_v2::handleFrameChunk(%d, %d)", delta, skipLines);

	uint32 frameSize = _header._width * _header._height;
	byte *dest;

	delayToNextFrame();

	uint32 frameDelay = _videoData->_stream->readUint16LE();
	debugC(kDebugGraphics, "frameDelay = %d", frameDelay);
	calcNextFrameTime(frameDelay);

	if (_drawBorder) {
		Common::Point pt(_vidSurface->_orgX1, _vidSurface->_orgY1);
		_vidSurface->BaseSurface::drawBox(pt.x - 1, pt.y - 1, pt.x + _vm->_video->getWidth(), pt.y + _vm->_video->getHeight(), 249);
		_vidSurface->BaseSurface::drawBox(pt.x - 2, pt.y - 2, pt.x + _vm->_video->getWidth() + 1, pt.y + _vm->_video->getHeight() + 1, 248);
		_vidSurface->BaseSurface::drawBox(pt.x - 3, pt.y - 3, pt.x + _vm->_video->getWidth() + 2, pt.y + _vm->_video->getHeight() + 2, 247);
		_drawBorder = false;
	}

	const Common::Rect frameBounds(Common::Point(_vidSurface->_orgX1, _vidSurface->_orgY1), _frame->w, _frame->h);
	if (_videoFrame == 0 && delta) {
		// If it's the first frame, grab the background
		// (in case the first video frame happens to be a delta frame)
		_frame->blitFrom(*_vidSurface, frameBounds, Common::Point());
	}

	if (skipLines) {
		uint16 lineStart = _videoData->_stream->readUint16LE();
		debugC(kDebugGraphics, "lineStart = %d", lineStart);
		dest = (byte*)_frame->getBasePtr(0, lineStart);
		frameSize -= lineStart * _frame->pitch;
	} else {
		dest = (byte*)_frame->getBasePtr(0, 0);
	}

	while (frameSize > 0) {
		byte count = _videoData->_stream->readByte();
		if (delta && count == 0)
			break;
		if (count & 0x80) {
			count &= 0x7F;
			if (!delta) {
				// repeat next byte 'count & 0x7F' times
				byte value = _videoData->_stream->readByte();
				memset(dest, value, count);
			} // else skip 'count & 0x7F' bytes in output buffer
		} else {
			// copy next 'count' bytes
			_videoData->_stream->read(dest, count);
		}
		dest += count;
		frameSize -= count;
	}

	// Draw the video frame
	_vidSurface->blitFrom(*_frame, Common::Point(_vidSurface->_orgX1, _vidSurface->_orgY1));
	_vidSurface->addDirtyRect(frameBounds);

	_videoFrame++;
	if (_videoFrame >= _header._frameCount) {
		_videoEnd = true;
		closeVideo();
	}
}

void VideoPlayer_v2::handleStraitChunk() {
	debugC(kDebugGraphics, "VideoPlayer_v2::handleStrait()");

	uint32 frameDelay = _videoData->_stream->readUint16LE();
	debugC(kDebugGraphics, "frameDelay = %d", frameDelay);
	calcNextFrameTime(frameDelay);

	byte *dest = (byte*)_frame->getBasePtr(0, 0);
	for (int y = 0; y < _frame->h; y++) {
		_videoData->_stream->read(dest, _frame->w);
		dest += _frame->pitch;
	}

	// Draw the video frame
	const Common::Rect frameBounds(Common::Point(_vidSurface->_orgX1, _vidSurface->_orgY1), _frame->w, _frame->h);
	_vidSurface->blitFrom(*_frame, Common::Point(_vidSurface->_orgX1, _vidSurface->_orgY1));
	_vidSurface->addDirtyRect(frameBounds);

	_videoFrame++;
	if (_videoFrame >= _header._frameCount) {
		_videoEnd = true;
		closeVideo();
	}
}

void VideoPlayer_v2::handleSoundChunk(bool init) {
	debugC(kDebugGraphics, "VideoPlayer_v2::handleSoundChunk(%d)", init);

	if (init) {
		uint16 time = _videoData->_stream->readUint16LE();
		byte frequencyDivisor = _videoData->_stream->readByte();
		int frequency = 1000000 / (256 - frequencyDivisor);
		debugC(kDebugGraphics, "time = %d; freqencyDivisor = %d; freqency = %d", time, frequencyDivisor, frequency);

		_audioStream = Audio::makeQueuingAudioStream(frequency, false);
		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, _audioStream);

	}

	uint16 soundSize = _videoData->_stream->readUint16LE();
	byte *soundData = (byte *)malloc(soundSize);

	_videoData->_stream->read(soundData, soundSize);

	if (!_audioStream) {
		warning("Got sound block size %d before sound init! Skipping.", soundSize);
		return;
	}

	_audioStream->queueBuffer(soundData, soundSize, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	debugC(kDebugGraphics, "soundSize = %d", soundSize);
}


void VideoPlayer_v2::closeVideo() {
	if (_audioStream) {
		_audioStream->finish();
		_vm->_mixer->stopHandle(_audioStreamHandle);
	}
	delete _frame;
	_frame = nullptr;
	VideoPlayer::closeVideo();
}


void VideoPlayer_v2::delayToNextFrame() {
	if (!_nextFrameTime)
		return;

	while (_nextFrameTime > g_system->getMillis() && !_vm->shouldQuit()) {
		_vm->_events->pollEventsAndWait();
	}
}

} // End of namespace Access
