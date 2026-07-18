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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/file.h"
#include "common/system.h"

#include "video/bink_decoder.h"

#include "engines/nancy/video.h"
#include "engines/nancy/commontypes.h"

#include "engines/nancy/movieplayer.h"

namespace Nancy {

MoviePlayer::MoviePlayer() {}

MoviePlayer::~MoviePlayer() {}

bool MoviePlayer::loadFile(const Common::Path &name, bool bidirectionalCache) {
	freeFrameCache();

	const Common::Path avfPath = name.append(".avf");
	const Common::Path bikPath = name.append(".bik");

	// Detect the format from which file exists (AVF wins if both do).
	if (Common::File::exists(avfPath)) {
		_videoType = kVideoPlaytypeAVF;
		_decoder.reset(new AVFDecoder(bidirectionalCache ? AVFDecoder::kLoadBidirectional : AVFDecoder::kLoadForward));
	} else if (Common::File::exists(bikPath)) {
		_videoType = kVideoPlaytypeBink;
		_decoder.reset(new Video::BinkDecoder());
	} else {
		_decoder.reset();
		return false;
	}

	_currentSurface = nullptr;

	if (!_decoder->loadFile(_videoType == kVideoPlaytypeAVF ? avfPath : bikPath)) {
		_decoder.reset();
		return false;
	}

	// The AVF decoder caches frames itself, so only the Bink path needs ours.
	_useFrameCache = bidirectionalCache && _videoType == kVideoPlaytypeBink;
	if (_useFrameCache) {
		_frameCache.resize(_decoder->getFrameCount());
	}

	return true;
}

bool MoviePlayer::isVideoLoaded() const {
	return _decoder && _decoder->isVideoLoaded();
}

void MoviePlayer::close() {
	_currentSurface = nullptr;
	freeFrameCache();
	if (_decoder) {
		_decoder->close();
	}
}

void MoviePlayer::freeFrameCache() {
	for (Graphics::Surface &surf : _frameCache) {
		surf.free();
	}
	_frameCache.clear();
	_useFrameCache = false;
}

void MoviePlayer::start()					{ if (_decoder) _decoder->start(); }
void MoviePlayer::stop()					{ if (_decoder) _decoder->stop(); }
void MoviePlayer::pauseVideo(bool pause)	{ if (_decoder) _decoder->pauseVideo(pause); }
bool MoviePlayer::isPlaying() const			{ return _decoder && _decoder->isPlaying(); }
bool MoviePlayer::needsUpdate() const		{ return _decoder && _decoder->needsUpdate(); }

bool MoviePlayer::endOfVideo() const {
	if (!_decoder) {
		return true;
	}
	// AVF has its own stricter end check (accounts for playback time / reversal).
	if (_videoType == kVideoPlaytypeAVF) {
		return ((AVFDecoder *)_decoder.get())->atEnd();
	}
	return _decoder->endOfVideo();
}

void MoviePlayer::seekToFrame(uint frame)				{ if (_decoder) _decoder->seekToFrame(frame); }
void MoviePlayer::seek(const Audio::Timestamp &time)	{ if (_decoder) _decoder->seek(time); }
void MoviePlayer::rewind()								{ if (_decoder) _decoder->rewind(); }
void MoviePlayer::setRate(const Common::Rational &rate)	{ _decoder->setRate(rate); }
Common::Rational MoviePlayer::getRate() const			{ return _decoder->getRate(); }
void MoviePlayer::setReverse(bool reverse)				{ _decoder->setReverse(reverse); }

int MoviePlayer::getCurFrame() const		{ return _decoder ? _decoder->getCurFrame() : -1; }
int MoviePlayer::getFrameCount() const		{ return _decoder ? _decoder->getFrameCount() : 0; }
Audio::Timestamp MoviePlayer::getDuration() const	{ return _decoder->getDuration(); }
uint16 MoviePlayer::getWidth() const		{ return _decoder->getWidth(); }
uint16 MoviePlayer::getHeight() const		{ return _decoder->getHeight(); }

void MoviePlayer::addFrameTime(uint16 timeToAdd) {
	if (_decoder && _videoType == kVideoPlaytypeAVF) {
		((AVFDecoder *)_decoder.get())->addFrameTime(timeToAdd);
	}
}

const Graphics::Surface *MoviePlayer::decodeNextFrame(int frameNr) {
	if (frameNr < 0) {
		return _decoder->decodeNextFrame();
	}

	if (_videoType == kVideoPlaytypeAVF) {
		AVFDecoder *decoder = (AVFDecoder *)_decoder.get();
		const Graphics::Surface *frame = decoder->decodeFrame(frameNr);
		decoder->seek(frameNr);	// seek to take advantage of caching
		return frame;
	}

	// Bink path.
	if (_useFrameCache && (uint)frameNr < _frameCache.size() && _frameCache[frameNr].getPixels()) {
		return &_frameCache[frameNr];
	}

	// seekToFrame() re-decodes from the previous keyframe, so only seek when the
	// frame can't be reached by decoding forward one step.
	if (_decoder->getCurFrame() + 1 != frameNr) {
		_decoder->seekToFrame(frameNr);
	}

	const Graphics::Surface *frame = _decoder->decodeNextFrame();

	// The decoder reuses one surface per frame, so cache a copy.
	if (_useFrameCache && frame && (uint)frameNr < _frameCache.size()) {
		_frameCache[frameNr].copyFrom(*frame);
		return &_frameCache[frameNr];
	}

	return frame;
}

// --- Simple frame-range player ------------------------------------------

void MoviePlayer::storeCurrentFrame() {
	_currentSurface = _decoder->decodeNextFrame();
}

void MoviePlayer::goToFrame(int frameNr) {
	if (!_decoder) {
		return;
	}

	if (!_decoder->isPlaying()) {
		_decoder->start();
	}
	_rangePlaying = false;
	_decoder->setReverse(false);
	_decoder->seekToFrame(frameNr);
	storeCurrentFrame();

	int frameCount = _decoder->getFrameCount();
	uint32 durationMs = _decoder->getDuration().msecs();
	_frameDelayMs = (frameCount > 0 && durationMs > 0) ? (durationMs / frameCount) : 66;
}

void MoviePlayer::playRange(int first, int last) {
	if (!_decoder) {
		return;
	}

	if (!_decoder->isPlaying()) {
		_decoder->start();
	}
	_step = (last >= first) ? 1 : -1;
	_rangeLast = last;
	_decoder->setReverse(_step < 0);
	_decoder->seekToFrame(first);
	storeCurrentFrame();
	_rangePlaying = (first != last);
	_lastFrameTime = g_system->getMillis();
}

bool MoviePlayer::update() {
	if (!_rangePlaying || !_decoder) {
		return false;
	}

	// Own the frame timing (one frame per delay, never skipping), so playback
	// starts immediately and stays smooth even when the decoder's own timing
	// would stall after a seek.
	uint32 now = g_system->getMillis();
	if (now - _lastFrameTime < _frameDelayMs) {
		return false;
	}
	_lastFrameTime = now;

	if (_decoder->endOfVideo() ||
			(_step > 0 && _decoder->getCurFrame() >= _rangeLast) ||
			(_step < 0 && _decoder->getCurFrame() <= _rangeLast)) {
		_rangePlaying = false;
		return false;
	}

	storeCurrentFrame();
	return true;
}

void MoviePlayer::drawFrame(Graphics::ManagedSurface &dst, const Common::Point &pos) const {
	if (_currentSurface) {
		dst.blitFrom(*_currentSurface, pos);
	}
}

} // End of namespace Nancy
