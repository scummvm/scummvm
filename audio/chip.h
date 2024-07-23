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

#ifndef AUDIO_CHIP_H
#define AUDIO_CHIP_H

#include "common/func.h"
#include "common/ptr.h"

#include "audio/audiostream.h"

namespace Audio {
class SoundHandle;

class Chip {
public:
	virtual ~Chip() {}

	/**
	 * The type of the timer callback functor.
	 */
	typedef Common::Functor0<void> TimerCallback;

	/**
	 * Start the sound chip with callbacks.
	 */
	void start(TimerCallback *callback, int timerFrequency);

	/**
	 * Stop the sound chip
	 */
	void stop();

	/**
	 * Change the callback frequency. This must only be called from a
	 * timer proc.
	 */
	virtual void setCallbackFrequency(int timerFrequency) = 0;

protected:
	/**
	 * Start the callbacks.
	 */
	virtual void startCallbacks(int timerFrequency) = 0;

	/**
	 * Stop the callbacks.
	 */
	virtual void stopCallbacks() = 0;

	/**
	 * The functor for callbacks.
	 */
	Common::ScopedPtr<TimerCallback> _callback;
};

/**
 * A Chip that represents a real sound chip, as opposed to an emulated one.
 *
 * This will use an actual timer instead of using one calculated from
 * the number of samples in an AudioStream::readBuffer call.
 */
class RealChip : virtual public Chip {
public:
	RealChip();
	virtual ~RealChip();

	// Chip API
	void setCallbackFrequency(int timerFrequency);

protected:
	// Chip API
	void startCallbacks(int timerFrequency);
	void stopCallbacks();
	virtual void onTimer();

private:
	static void timerProc(void *refCon);

	uint _baseFreq;
	uint _remainingTicks;

	enum {
		kMaxFreq = 100
	};
};

/**
 * A Chip that represents an emulated sound chip.
 *
 * This will send callbacks based on the number of samples
 * decoded in readBuffer().
 */
class EmulatedChip : virtual public Chip, protected Audio::AudioStream {
protected:
	static const int FIXP_SHIFT = 16;

public:
	EmulatedChip();
	virtual ~EmulatedChip();

	// Chip API
	void setCallbackFrequency(int timerFrequency) override;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) override;
	int getRate() const override;
	bool endOfData() const override { return false; }

protected:
	// Chip API
	void startCallbacks(int timerFrequency) override final;
	void stopCallbacks() override final;

	/**
	 * Read up to 'length' samples.
	 *
	 * Data will be in native endianess, 16 bit per sample, signed.
	 * For stereo chips, buffer will be filled with interleaved
	 * left and right channel samples, starting with a left sample.
	 * Furthermore, the samples in the left and right are summed up.
	 * So if you request 4 samples from a stereo chip, you will get
	 * a total of two left channel and two right channel samples.
	 */
	virtual void generateSamples(int16 *buffer, int numSamples) = 0;

private:
	int _baseFreq;

	int _nextTick;
	int _samplesPerTick;

	Audio::SoundHandle *_handle;
};

} // End of namespace Audio

#endif
