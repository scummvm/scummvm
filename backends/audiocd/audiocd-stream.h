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

#ifndef BACKENDS_AUDIOCD_AUDIOCD_STREAM_H
#define BACKENDS_AUDIOCD_AUDIOCD_STREAM_H

#include "audio/audiostream.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/timer.h"

class AudioCDStream : public Audio::SeekableAudioStream {
public:
	AudioCDStream();
	~AudioCDStream();

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	int getRate() const { return 44100; }
	bool endOfData() const;
	bool seek(const Audio::Timestamp &where);
	Audio::Timestamp getLength() const;

protected:
	virtual uint getStartFrame() const = 0;
	virtual uint getEndFrame() const = 0;
	virtual bool readFrame(int frame, int16 *buffer) = 0;

	void startTimer(bool fillBuffer = false);
	void stopTimer();

	enum {
		kBytesPerFrame = 2352,
		kSamplesPerFrame = kBytesPerFrame / 2
	};

	enum {
		kSecondsPerMinute = 60,
		kFramesPerSecond = 75
	};

	enum {
		// Keep about a second's worth of audio in the buffer
		kBufferThreshold = kFramesPerSecond
	};

private:
	int16 _buffer[kSamplesPerFrame];
	int _frame;
	uint _bufferPos;

	Common::Queue<int16 *> _bufferQueue;
	int _bufferFrame;
	Common::Mutex _mutex;

	bool _forceStop;
	bool shouldForceStop() const;
	void forceStop();
	Common::Mutex _forceStopMutex;

	bool readNextFrame();
	static void timerProc(void *refCon);
	void onTimer();
	void emptyQueue();
};

#endif
