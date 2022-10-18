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

#ifndef AUDIO_CMS_H
#define AUDIO_CMS_H

#include "common/func.h"
#include "common/ptr.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/softsynth/cms.h"

namespace CMS {

class CMS;

class Config {
public:
	/**
	 * Creates a CMS driver.
	 */
	static CMS *create();
};

/**
 * The type of the CMS timer callback functor.
 */
typedef Common::Functor0<void> TimerCallback;

class CMS {
private:
	static bool _hasInstance;

public:
	// The default number of timer callbacks per second.
	static const int DEFAULT_CALLBACK_FREQUENCY = 250;

	CMS();
	virtual ~CMS();

	/**
	 * Initializes the CMS emulator.
	 *
	 * @return		true on success, false on failure
	 */
	virtual bool init() = 0;

	/**
	 * Reinitializes the CMS emulator
	 */
	virtual void reset() = 0;

	/**
	 * Writes a byte to the given I/O port. CMS responds to 2 sets of 2 ports:
	 * 0x220/0x222 - value for the 1st/2nd chip (channels 0-5/6-B)
	 * 0x221/0x223 - register for the 1st/2nd chip
	 *
	 * @param a		port address
	 * @param v		value, which will be written
	 */
	virtual void write(int a, int v) = 0;

	/**
	 * Function to directly write to a specific CMS register. We allow writing
	 * to secondary CMS chip registers by using register values >= 0x100.
	 *
	 * @param r		hardware register number to write to
	 * @param v		value, which will be written
	 */
	virtual void writeReg(int r, int v) = 0;

	/**
	 * Start the CMS with callbacks.
	 */
	void start(TimerCallback *callback, int timerFrequency = DEFAULT_CALLBACK_FREQUENCY);

	/**
	 * Stop the CMS
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
 * A CMS that represents an emulated CMS.
 *
 * This will send callbacks based on the number of samples
 * decoded in readBuffer().
 */
class EmulatedCMS : public CMS, protected Audio::AudioStream {
protected:
	static const int FIXP_SHIFT = 16;

public:
	EmulatedCMS();
	virtual ~EmulatedCMS();

	// CMS API
	void setCallbackFrequency(int timerFrequency) override;

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) override;
	int getRate() const override;
	bool endOfData() const override;
	bool isStereo() const override;

protected:
	// CMS API
	void startCallbacks(int timerFrequency) override;
	void stopCallbacks() override;

	/**
	 * Read up to 'length' samples.
	 *
	 * Data will be in native endianess, 16 bit per sample, signed. buffer will
	 * be filled with interleaved left and right channel samples, starting with
	 * a left sample. The requested number of samples is stereo samples, so if
	 * you request 2 samples, you will get a total of two left channel and two
	 * right channel samples.
	 */
	virtual void generateSamples(int16 *buffer, int numSamples) = 0;

private:
	int _baseFreq;

	int _nextTick;
	int _samplesPerTick;

	Audio::SoundHandle *_handle;
};

/**
 * A CMS that represents the DOSBox CMS emulator.
 */
class DOSBoxCMS : public EmulatedCMS {
public:
	DOSBoxCMS();
	~DOSBoxCMS() override;

	bool init() override;
	void reset() override;
	void write(int a, int v) override;
	void writeReg(int r, int v) override;

protected:
	void generateSamples(int16 *buffer, int numSamples) override;

private:
	CMSEmulator *_cms;
};

} // End of namespace CMS

#endif
