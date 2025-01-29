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

#include "common/debug.h"
#include "audio/audiostream.h"

#include "groovie/video/player.h"
#include "groovie/groovie.h"
#include "audio/mixer.h"
#include "common/debug-channels.h"

namespace Groovie {

VideoPlayer::VideoPlayer(GroovieEngine *vm) :
	_vm(vm), _syst(vm->_system), _file(nullptr), _audioStream(nullptr), _fps(0), _overrideSpeed(false), _flags(0),
	_begunPlaying(false), _millisBetweenFrames(0), _lastFrameTime(0), _frameTimeDrift(0) {

	_startTime = _syst->getMillis();

	int16 h = g_system->getOverlayHeight();

	_subtitles.setBBox(Common::Rect(20, h - 120, g_system->getOverlayWidth() - 20, h - 20));
	_subtitles.setColor(0xff, 0xff, 0xff);
	_subtitles.setFont("LiberationSans-Regular.ttf");
}

bool VideoPlayer::load(Common::SeekableReadStream *file, uint16 flags) {
	_file = file;
	_flags = flags;
	_overrideSpeed = false;
	_startTime = _syst->getMillis();

	stopAudioStream();
	_fps = loadInternal();

	if (_fps != 0) {
		setOverrideSpeed(_overrideSpeed);
		_begunPlaying = false;
		return true;
	} else {
		_file = nullptr;
		return false;
	}
}

void VideoPlayer::setOverrideSpeed(bool isOverride) {
	_overrideSpeed = isOverride;
	if (_fps != 0) {
		if (isOverride)
			_millisBetweenFrames = 1000.0f / 26.0f;
		else
			_millisBetweenFrames = 1000.0f / float(_fps);
	}
}

void VideoPlayer::fastForward() {
	_millisBetweenFrames = 0;
	_frameTimeDrift = 0;
	stopAudioStream();
}

bool VideoPlayer::isFastForwarding() {
	return DebugMan.isDebugChannelEnabled(kDebugFast) || _millisBetweenFrames <= 0;
}

bool VideoPlayer::playFrame() {
	bool end = true;

	// Process the next frame while the file is open
	if (_file) {
		end = playFrameInternal();

		_subtitles.drawSubtitle(_lastFrameTime - _startTime);
	}

	// The file has been completely processed
	if (end) {
		_file = nullptr;

		// Wait for pending audio
		if (_audioStream) {
			if (_audioStream->endOfData() || isFastForwarding()) {
				// Mark the audio stream as finished (no more data will be appended)
				_audioStream->finish();
				_audioStream = nullptr;
			} else {
				// Don't end if there's still audio playing
				end = false;
			}
		}

		unloadSubtitles();
	}

	return end;
}

void VideoPlayer::unloadSubtitles() {
	if (_subtitles.isLoaded()) {
		_subtitles.close();
		g_system->hideOverlay();
	}
}

void VideoPlayer::waitFrame() {
	if (isFastForwarding()) {
		return;
	}
	uint32 currTime = _syst->getMillis();
	if (!_begunPlaying) {
		_begunPlaying = true;
		_lastFrameTime = currTime;
		_frameTimeDrift = 0.0f;

		if (_subtitles.isLoaded()) {
			g_system->showOverlay(false);
			g_system->clearOverlay();
		}
	} else {
		uint32 millisDiff = currTime - _lastFrameTime;
		float fMillis = _millisBetweenFrames + _frameTimeDrift;
		// use floorf instead of roundf, because delayMillis often slightly over-sleeps
		uint32 millisSleep = MAX(0.0f, floorf(fMillis) - float(millisDiff));

		if (millisSleep > 0) {
			debugC(7, kDebugVideo, "Groovie::Player: Delaying %d (currTime=%d, _lastFrameTime=%d, millisDiff=%d, _millisBetweenFrame=%.2f, _frameTimeDrift=%.2f)",
				   millisSleep, currTime, _lastFrameTime, millisDiff, _millisBetweenFrames, _frameTimeDrift);
			_syst->delayMillis(millisSleep);
			currTime = _syst->getMillis();
			debugC(7, kDebugVideo, "Groovie::Player: Finished delay at %d", currTime);
		}

		_frameTimeDrift = fMillis - float(currTime - _lastFrameTime);
		if (abs(_frameTimeDrift) >= _millisBetweenFrames) {
			_frameTimeDrift = 0;
		}
		debugC(6, kDebugVideo, "Groovie::Player: Frame displayed at %d (%f FPS), _frameTimeDrift=%.2f", currTime, 1000.0 / (currTime - _lastFrameTime), _frameTimeDrift);
		_lastFrameTime = currTime;
	}
}

} // End of Groovie namespace
