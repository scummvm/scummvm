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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

namespace Bolt {

bool XpLib::initSound() {
	_audioStream = Audio::makeQueuingAudioStream(22050, false);
	_nextSoundDeadlineMs = 0;
	_sndPlayState = 0;
	_sndQueued = 0;
	_sndPaused = false;
	_sndCompletedCount = 0;
	_sndSampleRate = 22050;

	g_system->getMixer()->playStream(
		Audio::Mixer::kSFXSoundType,
		&_soundHandle,
		_audioStream,
		-1, 255, 0,
		DisposeAfterUse::NO);

	return true;
}

bool XpLib::pollSound(byte **outData) {
	if (outData)
		*outData = nullptr;

	if (_sndQueued == 0 || _sndPaused)
		return false;

	if (!_audioStream)
		return false;

	bool ready = false;

	int activeStreams = _audioStream->numQueuedStreams();
	int completed = _sndQueued - activeStreams;
	if (completed > (int)_sndCompletedCount)
		_sndCompletedCount = completed;

	uint32 now = _bolt->_system->getMillis();

	debug(5, "pollSound: queued=%d active=%d completed=%d state=%d deadline=%u now=%u",
		  _sndQueued, activeStreams, _sndCompletedCount, _sndPlayState,
		  _sndNextDeadline, now);

	if (_sndCompletedCount > 0) {
		if (_sndPlayState == 0) {
			_sndPlayState = 1;
		} else if (_sndPlayState == 2) {
			_sndPlayState = 0;
			ready = true;
		}
	}

	if (!ready) {
		if (now >= _sndNextDeadline) {
			if (_sndPlayState == 0) {
				_sndPlayState = 2;
			} else if (_sndPlayState == 1) {
				_sndPlayState = 0;
				ready = true;
			}
		}
	}

	if (!ready)
		return false;

	// Buffer completed!
	byte *sourcePtr = nullptr;
	if (!_bufferSourceQueue.empty()) {
		sourcePtr = _bufferSourceQueue.front();
		_bufferSourceQueue.pop();
	}

	if (outData)
		*outData = sourcePtr;

	_sndCompletedCount--;
	_sndQueued--;

	if (!_durationQueue.empty())
		_durationQueue.pop();

	if (_sndQueued == 0)
		return true;

	// Deadline adjustment for next buffer...
	uint32 nextDurationMs = 0;
	if (!_durationQueue.empty())
		nextDurationMs = _durationQueue.front();

	int32 elapsed = (int32)(_sndNextDeadline - _sndBufferQueueTime);

	if (elapsed > 0) {
		elapsed /= 50;
		int32 maxAdj = (int32)nextDurationMs / 10;
		if (maxAdj < elapsed)
			elapsed = maxAdj;
		_sndNextDeadline += (nextDurationMs - elapsed);
	} else {
		_sndNextDeadline = _sndBufferQueueTime + nextDurationMs;
	}

	_sndBufferQueueTime = _bolt->_system->getMillis();

	return true;
}

bool XpLib::playSound(byte *data, uint32 size, int16 sampleRate) {
	if (!_audioStream)
		return false;

	if (sampleRate != 22050)
		return false;

	// Drain completed buffers if full...
	int retries = 0;
	while (_audioStream->numQueuedStreams() >= 50) {
		byte *eventData;
		uint32 dummy = 0;
		if (pollSound(&eventData))
			postEvent(etSound, dummy, eventData);

		_bolt->_system->delayMillis(1);
		if (++retries > 5000) {
			warning("XpLib::playSound(): drain timeout");
			return false;
		}
	}

	byte *buf = (byte *)malloc(size);
	if (!buf)
		return false;

	memcpy(buf, data, size);
	_audioStream->queueBuffer(buf, size, DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);

	uint32 durationMs = (size * 1000) / (uint32)sampleRate;

	// First buffer: set initial deadline...
	if (_sndQueued == 0) {
		uint32 now = _bolt->_system->getMillis();
		_sndNextDeadline = now + durationMs;
		_sndBufferQueueTime = now;

		if (_sndPaused)
			_pauseTimeMs = now;
	}

	_sndQueued++;
	_durationQueue.push(durationMs);
	_bufferSourceQueue.push(data);

	return true;
}

bool XpLib::pauseSound() {
	if (_sndPaused)
		return true;

	_sndPaused = true;
	_pauseTimeMs = _bolt->_system->getMillis();
	g_system->getMixer()->pauseHandle(_soundHandle, true);
	return true;
}

bool XpLib::resumeSound() {
	if (_sndQueued > 0) {
		uint32 now = _bolt->_system->getMillis();
		uint32 pauseDuration = now - _pauseTimeMs;
		_sndNextDeadline += pauseDuration;
	}

	_sndPaused = false;
	g_system->getMixer()->pauseHandle(_soundHandle, false);
	return true;
}

bool XpLib::stopSound() {
	g_system->getMixer()->stopHandle(_soundHandle);

	_sndQueued = 0;
	_sndPaused = false;
	_sndCompletedCount = 0;
	_sndPlayState = 0;
	_nextSoundDeadlineMs = 0;
	_pauseTimeMs = 0;

	while (!_durationQueue.empty())
		_durationQueue.pop();

	while (!_bufferSourceQueue.empty())
		_bufferSourceQueue.pop();

	uint32 dummy;
	while (getEvent(etSound, &dummy) == etSound);

	// Recreate stream for next use...
	_audioStream = Audio::makeQueuingAudioStream(22050, false);
	g_system->getMixer()->playStream(
		Audio::Mixer::kSFXSoundType,
		&_soundHandle,
		_audioStream,
		-1, 255, 0,
		DisposeAfterUse::NO);

	return true;
}

void XpLib::shutdownSound() {
	g_system->getMixer()->stopHandle(_soundHandle);
	_audioStream = nullptr;
}

} // End of namespace Bolt
