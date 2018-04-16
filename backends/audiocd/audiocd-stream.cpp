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
 * Original license header:
 *
 * Cabal - Legacy Game Implementations
 *
 * Cabal is the legal property of its developers, whose names
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

#include "backends/audiocd/audiocd-stream.h"
#include "common/textconsole.h"

AudioCDStream::AudioCDStream() : _buffer(), _frame(0), _bufferPos(0), _bufferFrame(0), _forceStop(false) {
}

AudioCDStream::~AudioCDStream() {
	// Stop the timer; the subclass needs to do this,
	// so this is just a last resort.
	stopTimer();

	// Clear any buffered frames
	emptyQueue();
}

int AudioCDStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;

	// See if any data is left first
	while (_bufferPos < kSamplesPerFrame && samples < numSamples)
		buffer[samples++] = _buffer[_bufferPos++];

	// Bail out if done
	if (endOfData())
		return samples;

	while (samples < numSamples && !endOfData()) {
		if (!readNextFrame())
			return samples;

		// Copy the samples over
		for (_bufferPos = 0; _bufferPos < kSamplesPerFrame && samples < numSamples;)
			buffer[samples++] = _buffer[_bufferPos++];
	}

	return samples;
}

bool AudioCDStream::readNextFrame() {
	// Fetch a frame from the queue
	int16 *buffer;

	{
		Common::StackLock lock(_mutex);

		// Nothing we can do if it's empty
		if (_bufferQueue.empty())
			return false;

		buffer = _bufferQueue.pop();
	}

	memcpy(_buffer, buffer, kSamplesPerFrame * 2);
	delete[] buffer;
	_frame++;
	return true;
}

bool AudioCDStream::endOfData() const {
	return !shouldForceStop() && getStartFrame() + _frame >= getEndFrame() && _bufferPos == kSamplesPerFrame;
}

bool AudioCDStream::seek(const Audio::Timestamp &where) {
	// Stop the timer
	stopTimer();

	// Clear anything out of the queue
	emptyQueue();

	// Convert to the frame number
	// Really not much else needed
	_bufferPos = kSamplesPerFrame;
	_frame = where.convertToFramerate(kFramesPerSecond).totalNumberOfFrames();
	_bufferFrame = _frame;

	// Start the timer again
	startTimer();
	return true;
}

Audio::Timestamp AudioCDStream::getLength() const {
	return Audio::Timestamp(0, getEndFrame() - getStartFrame(), kFramesPerSecond);
}

void AudioCDStream::timerProc(void *refCon) {
	static_cast<AudioCDStream *>(refCon)->onTimer();
}

void AudioCDStream::onTimer() {
	// The goal here is to do as much work in this timer instead
	// of doing it in the readBuffer() call, which is the mixer.

	// If we're done, bail.
	if (shouldForceStop() || getStartFrame() + _bufferFrame >= getEndFrame())
		return;

	// Get a quick count of the number of items in the queue
	// We don't care that much; we only need a quick estimate
	_mutex.lock();
	uint32 queueCount = _bufferQueue.size();
	_mutex.unlock();

	// If we have enough audio buffered, bail out
	if (queueCount >= kBufferThreshold)
		return;

	while (!shouldForceStop() && queueCount < kBufferThreshold && getStartFrame() + _bufferFrame < getEndFrame()) {
		int16 *buffer = new int16[kSamplesPerFrame];

		// Figure out the MSF of the frame we're looking for
		int frame = _bufferFrame + getStartFrame();

		// Request to read that frame
		if (!readFrame(frame, buffer)) {
			warning("Failed to read CD audio");
			forceStop();
			return;
		}

		_bufferFrame++;

		// Now push the buffer onto the queue
		Common::StackLock lock(_mutex);
		_bufferQueue.push(buffer);
		queueCount = _bufferQueue.size();
	}
}

void AudioCDStream::startTimer(bool fillBuffer) {
	_forceStop = false;
	if (fillBuffer) {
		onTimer();
	}
	g_system->getTimerManager()->installTimerProc(timerProc, 10 * 1000, this, "AudioCDStream");
}

void AudioCDStream::stopTimer() {
	forceStop();
	g_system->getTimerManager()->removeTimerProc(timerProc);
}

void AudioCDStream::emptyQueue() {
	while (!_bufferQueue.empty())
		delete[] _bufferQueue.pop();
}

bool AudioCDStream::shouldForceStop() const {
	Common::StackLock lock(_forceStopMutex);
	return _forceStop;
}

void AudioCDStream::forceStop() {
	Common::StackLock lock(_forceStopMutex);
	_forceStop = true;
}
