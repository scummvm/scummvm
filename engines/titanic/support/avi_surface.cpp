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

#include "common/system.h"
#include "graphics/pixelformat.h"
#include "video/avi_decoder.h"
#include "titanic/support/avi_surface.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "titanic/titanic.h"

namespace Titanic {

Video::AVIDecoder::AVIVideoTrack &AVIDecoder::getVideoTrack(uint idx) {
	assert(idx < _videoTracks.size());
	AVIVideoTrack *track = static_cast<AVIVideoTrack *>(_videoTracks[idx].track);
	return *track;
}

AVISurface::AVISurface(const CResourceKey &key) : _movieName(key.getString()) {
	_videoSurface = nullptr;
	_streamCount = 0;
	_movieFrameSurface[0] = _movieFrameSurface[1] = nullptr;
	_framePixels = nullptr;
	_priorFrameTime = 0;

	// Reset current frame. We need to keep track of frames separately from the decoder,
	// since it needs to be able to go beyond the frame count or to negative to allow
	// correct detection of when range playbacks have finished
	_currentFrame = -1;
	_priorFrame = -1;
	_isReversed = false;

	// Create a decoder
	_decoder = new AVIDecoder(Audio::Mixer::kPlainSoundType);
	if (!_decoder->loadFile(_movieName))
		error("Could not open video - %s", key.getString().c_str());

	_streamCount = _decoder->videoTrackCount();

	_soundManager = nullptr;
	_hasAudio = false;
	_frameRate = 0.0;
}

AVISurface::~AVISurface() {
	if (_videoSurface)
		_videoSurface->_flipVertically = false;
	delete _framePixels;
	delete _movieFrameSurface[0];
	delete _movieFrameSurface[1];
	delete _decoder;
}

bool AVISurface::play(uint flags, CGameObject *obj) {
	if (flags & MOVIE_REVERSE)
		return play(_decoder->getFrameCount() - 1, 0, flags, obj);
	else
		return play(0, _decoder->getFrameCount() - 1, flags, obj);
}

bool AVISurface::play(int startFrame, int endFrame, uint flags, CGameObject *obj) {
	if (flags & MOVIE_STOP_PREVIOUS)
		stop();

	return play(startFrame, endFrame, -1, flags, obj);
}

bool AVISurface::play(int startFrame, int endFrame, int initialFrame, uint flags, CGameObject *obj) {
	CMovieRangeInfo *info = new CMovieRangeInfo();
	info->_startFrame = startFrame;
	info->_endFrame = endFrame;
	info->_isReversed = endFrame < startFrame;
	info->_initialFrame = 0;
	info->_isRepeat = flags & MOVIE_REPEAT;

	if (obj) {
		CMovieEvent *me = new CMovieEvent();
		me->_type = MET_MOVIE_END;
		me->_startFrame = startFrame;
		me->_endFrame = endFrame;
		me->_initialFrame = 0;
		me->_gameObject = obj;

		info->addEvent(me);
	}

	_movieRangeInfo.push_back(info);

	if (_movieRangeInfo.size() == 1) {
		// First play call, so start the movie playing
		setReversed(info->_isReversed);
		return startAtFrame(initialFrame);
	} else {
		return true;
	}
}

void AVISurface::stop() {
	_decoder->stop();
	_movieRangeInfo.destroyContents();
}

void AVISurface::pause() {
	_decoder->pauseVideo(true);
}

void AVISurface::resume() {
	if (_decoder->isPaused())
		_decoder->pauseVideo(false);
}

bool AVISurface::startAtFrame(int frameNumber) {
	if (isPlaying())
		// If it's already playing, then don't allow it
		return false;

	if (frameNumber == -1)
		// Default to starting frame of first movie range
		frameNumber = _movieRangeInfo.front()->_startFrame;
	if (_isReversed && frameNumber == (int)_decoder->getFrameCount())
		--frameNumber;

	// Start the playback
	_decoder->start();

	// Seek to the starting frame
	seekToFrame(frameNumber);

	// If we're in reverse playback, set the decoder to play in reverse
	if (_isReversed)
		_decoder->setRate(Common::Rational(-1));

	renderFrame();

	return true;
}

void AVISurface::seekToFrame(uint frameNumber) {
	if (_isReversed && frameNumber == _decoder->getFrameCount())
		--frameNumber;

	if ((int)frameNumber != _currentFrame) {
		_decoder->seekToFrame(frameNumber);
		_currentFrame = _priorFrame = (int)frameNumber;
	}
}

void AVISurface::setReversed(bool isReversed) {
	_isReversed = isReversed;
}

bool AVISurface::handleEvents(CMovieEventList &events) {
	if (!isPlaying())
		return true;

	CMovieRangeInfo *info = _movieRangeInfo.front();
	_priorFrame = _currentFrame;
	_currentFrame += _isReversed ? -1 : 1;

	int newFrame = _currentFrame;
	if ((info->_isReversed && newFrame < info->_endFrame) ||
		(!info->_isReversed && newFrame > info->_endFrame)) {
		if (info->_isRepeat) {
			newFrame = info->_startFrame;
		} else {
			info->getMovieEnd(events);
			_movieRangeInfo.remove(info);
			delete info;

			if (_movieRangeInfo.empty()) {
				// No more ranges, so stop playback
				stop();
			} else {
				// Not empty, so move onto new first one
				info = _movieRangeInfo.front();
				newFrame = info->_startFrame;
				setReversed(info->_isReversed);
			}
		}
	}

	if (isPlaying()) {
		if (newFrame != getFrame()) {
			// The frame has been changed, so move to new position
			seekToFrame(newFrame);
			renderFrame();
		}

		// Get any events for the given position
		info->getMovieFrame(events, newFrame);
		return renderFrame();
	} else {
		return false;
	}
}

void AVISurface::setVideoSurface(CVideoSurface *surface) {
	_videoSurface = surface;

	// Handling for secondary video stream
	if (_streamCount == 2) {
		const Common::String &streamName = _decoder->getVideoTrack(1).getName();

		if (streamName == "mask0") {
			_videoSurface->_transparencyMode = TRANS_MASK0;
		} else if (streamName == "mask255") {
			_videoSurface->_transparencyMode = TRANS_MASK255;
		} else if (streamName == "alpha0") {
			_videoSurface->_transparencyMode = TRANS_ALPHA0;
		} else if (streamName == "alpha255") {
			_videoSurface->_transparencyMode = TRANS_ALPHA255;
		}
	}

	setupDecompressor();
}

void AVISurface::setupDecompressor() {
	if (!_decoder)
		return;

	for (int idx = 0; idx < _streamCount; ++idx) {
		Graphics::PixelFormat format = _decoder->getVideoTrack(idx).getPixelFormat();
		int decoderPitch = _decoder->getWidth() * format.bytesPerPixel;
		bool flag = false;

		if (idx == 0 && _videoSurface && _videoSurface->getPitch() == decoderPitch) {
			const uint bitCount = _decoder->getVideoTrack(0).getBitCount();
			const int vDepth = _videoSurface->getPixelDepth();

			switch (bitCount) {
			case 15:
				flag = vDepth == 1;
				break;

			case 16:
				flag = vDepth == 1 || vDepth == 2;
				break;

			case 24:
				flag = vDepth == 3;
				break;

			default:
				break;
			}
		}

		if (!flag) {
			_framePixels = new Graphics::ManagedSurface(_decoder->getWidth(), _decoder->getHeight(),
				_decoder->getVideoTrack(0).getPixelFormat());
		} else if (idx == 0) {
			// The original developers used a vertical flipped playback to indicate
			// an incompatibility between source video and dest surface bit-depths,
			// which would result in poor playback performance
			_videoSurface->_flipVertically = true;
		}
	}
}

void AVISurface::copyMovieFrame(const Graphics::Surface &src, Graphics::ManagedSurface &dest) {
	// WORKAROUND: Handle rare cases where frame sizes don't match the video size
	Common::Rect copyRect(0, 0, MIN(src.w, dest.w), MIN(src.h, dest.h));

	if (src.format.bytesPerPixel == 1) {
		// Paletted 8-bit, so convert to 16-bit and copy over
		const byte *palette = _decoder->getPalette();
		if (palette) {
			Graphics::Surface *s = src.convertTo(dest.format, palette);
			dest.blitFrom(*s, copyRect, Common::Point(0, 0));
			s->free();
			delete s;
		}
	} else if (src.format.bytesPerPixel == 2) {
		// Source is already 16-bit, with no alpha, so do a straight copy
		dest.blitFrom(src, copyRect, Common::Point(0, 0));
	} else {
		// Source is 32-bit which may have transparent pixels. Copy over each
		// pixel, replacing transparent pixels with the special transparency color
		byte a, r, g, b;
		assert(src.format.bytesPerPixel == 4 && dest.format.bytesPerPixel == 2);
		uint16 transPixel = _videoSurface->getTransparencyColor();

		for (uint y = 0; y < MIN(src.h, dest.h); ++y) {
			const uint32 *pSrc = (const uint32 *)src.getBasePtr(0, y);
			uint16 *pDest = (uint16 *)dest.getBasePtr(0, y);

			for (uint x = 0; x < MIN(src.w, dest.w); ++x, ++pSrc, ++pDest) {
				src.format.colorToARGB(*pSrc, a, r, g, b);
				assert(a == 0 || a == 0xff);

				*pDest = (a == 0) ? transPixel : dest.format.RGBToColor(r, g, b);
			}
		}
	}
}

uint AVISurface::getWidth() const {
	return _decoder->getWidth();
}

uint AVISurface::getHeight() const {
	return _decoder->getHeight();
}

void AVISurface::setFrame(int frameNumber) {
	// If playback was in process, stop it
	if (isPlaying())
		stop();

	// Ensure the frame number is valid
	if (frameNumber >= (int)_decoder->getFrameCount())
		frameNumber = _decoder->getFrameCount() - 1;

	seekToFrame(frameNumber);
	renderFrame();
}

bool AVISurface::isNextFrame() {
	if (!_decoder->endOfVideo())
		return _decoder->getTimeToNextFrame() == 0;

	// We're at the end of the video, so we need to manually
	// keep track of frame delays. Hardcoded at the moment for 15FPS
	const uint FRAME_TIME = 1000 / 15;
	uint32 currTime = g_system->getMillis();
	if (currTime >= (_priorFrameTime + FRAME_TIME)) {
		_priorFrameTime = currTime;
		return true;
	}

	return false;
}

bool AVISurface::renderFrame() {
	// Check there's a frame ready for display
	if (!_decoder->needsUpdate())
		return false;

	// Make a copy of each decoder's video frame
	for (int idx = 0; idx < _streamCount; ++idx) {
		const Graphics::Surface *frame;

		if (idx == 0) {
			frame = _decoder->decodeNextFrame();
			if (!_movieFrameSurface[0])
				_movieFrameSurface[0] = new Graphics::ManagedSurface(_decoder->getWidth(), _decoder->getHeight(),
					g_system->getScreenFormat());

			copyMovieFrame(*frame, *_movieFrameSurface[0]);
		} else {
			frame = _decoder->decodeNextTransparency();
			if (!_movieFrameSurface[1])
				_movieFrameSurface[1] = new Graphics::ManagedSurface(_decoder->getWidth(), _decoder->getHeight(),
					Graphics::PixelFormat::createFormatCLUT8());

			_movieFrameSurface[1]->blitFrom(*frame);
		}
	}

	if (!_framePixels) {
		if (_videoSurface->lock()) {
			// Blit the frame directly to the video surface
			assert(_streamCount == 1);
			_videoSurface->blitFrom(Point(0, 0), &_movieFrameSurface[0]->rawSurface());

			_videoSurface->unlock();
		}
	} else {
		const Graphics::Surface &frameSurface = _movieFrameSurface[0]->rawSurface();
		_videoSurface->lock();

		if (frameSurface.format.bytesPerPixel == 1) {
			// For paletted 8-bit surfaces, we need to convert it to 16-bit,
			// since the blitting method we're using doesn't support palettes
			Graphics::Surface *s = frameSurface.convertTo(g_system->getScreenFormat(),
				_decoder->getPalette());

			_videoSurface->getRawSurface()->blitFrom(*s);
			s->free();
			delete s;
		} else {
			_videoSurface->getRawSurface()->blitFrom(frameSurface);
		}

		_videoSurface->unlock();
	}

	return false;
}

bool AVISurface::addEvent(int *frameNumber, CGameObject *obj) {
	if (!_movieRangeInfo.empty()) {
		CMovieRangeInfo *tail = _movieRangeInfo.back();
		assert(frameNumber);
		if (*frameNumber == -1)
			*frameNumber = tail->_startFrame;

		CMovieEvent *me = new CMovieEvent();
		me->_type = MET_FRAME;
		me->_startFrame = 0;
		me->_endFrame = 0;
		me->_initialFrame = *frameNumber;
		me->_gameObject = obj;
		tail->addEvent(me);

		return _movieRangeInfo.size() == 1 && *frameNumber == getFrame();
	}

	return false;
}

void AVISurface::setFrameRate(double rate) {
	// Convert rate from fps to relative to 1.0 (normal speed)
	const int PRECISION = 10000;
	double playRate = rate / 15.0;	// Standard 15 FPS
	Common::Rational pRate((int)(playRate * PRECISION), PRECISION);

	_decoder->setRate(pRate);
}

Graphics::ManagedSurface *AVISurface::getSecondarySurface() {
	return _streamCount <= 1 ? nullptr : _movieFrameSurface[1];
}

Graphics::ManagedSurface *AVISurface::duplicateTransparency() const {
	if (_streamCount <= 1) {
		return nullptr;
	} else {
		Graphics::ManagedSurface *dest = new Graphics::ManagedSurface(_movieFrameSurface[1]->w,
			_movieFrameSurface[1]->h, Graphics::PixelFormat::createFormatCLUT8());
		dest->blitFrom(*_movieFrameSurface[1]);
		return dest;
	}
}

void AVISurface::playCutscene(const Rect &r, uint startFrame, uint endFrame) {
	bool isDifferent = _movieFrameSurface[0]->w != r.width() ||
		_movieFrameSurface[0]->h != r.height();

	startAtFrame(startFrame);
	_currentFrame = startFrame;

	while (_currentFrame < (int)endFrame && !g_vm->shouldQuit()) {
		if (isNextFrame()) {
			renderFrame();
			++_currentFrame;

			if (isDifferent) {
				// Clear the destination area, and use the transBlitFrom method,
				// which supports arbitrary scaling, to reduce to the desired size
				g_vm->_screen->fillRect(r, 0);
				g_vm->_screen->transBlitFrom(*_movieFrameSurface[0],
					Common::Rect(0, 0, _movieFrameSurface[0]->w, _movieFrameSurface[0]->h), r);
			} else {
				g_vm->_screen->blitFrom(*_movieFrameSurface[0], Common::Point(r.left, r.top));
			}

			g_vm->_screen->update();
			g_vm->_events->pollEvents();
		}

		// Brief wait, and check at the same time for clicks to abort the clip
		if (g_vm->_events->waitForPress(10))
			break;
	}

	stop();
}

uint AVISurface::getBitDepth() const {
	return _decoder->getVideoTrack(0).getBitCount();
}

} // End of namespace Titanic
