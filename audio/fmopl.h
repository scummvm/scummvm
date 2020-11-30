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

#ifndef AUDIO_FMOPL_H
#define AUDIO_FMOPL_H

#include "audio/audiostream.h"

#include "common/func.h"
#include "common/ptr.h"
#include "common/scummsys.h"

namespace Audio {
class SoundHandle;
}

namespace Common {
class String;
}

namespace OPL {

class OPL;

/**
 * @defgroup audio_fmopl OPL emulation
 * @ingroup audio
 *
 * @brief OPL class for managing an OPS emulator.
 * @{
 */

class Config {
public:
	enum OplFlags {
		kFlagOpl2		= (1 << 0),
		kFlagDualOpl2	= (1 << 1),
		kFlagOpl3		= (1 << 2)
	};

	/**
	 * OPL type to emulate.
	 */
	enum OplType {
		kOpl2,
		kDualOpl2,
		kOpl3
	};

	typedef int8 DriverId;
	struct EmulatorDescription {
		const char *name;
		const char *description;

		DriverId id;	// A unique ID for each driver
		uint32 flags;	// Capabilities of this driver
	};

	/**
	 * Get a list of all available OPL emulators.
	 * @return list of all available OPL emulators, terminated by a zero entry
	 */
	static const EmulatorDescription *getAvailable() { return _drivers; }

	/**
	 * Returns the driver id of a given name.
	 */
	static DriverId parse(const Common::String &name);

	/**
	 * @return The driver description for the given id or 0 in case it is not
	 *         available.
	 */
	static const EmulatorDescription *findDriver(DriverId id);

	/**
	 * Detects a driver for the specific type.
	 *
	 * @return Returns a valid driver id on success, -1 otherwise.
	 */
	static DriverId detect(OplType type);

	/**
	 * Creates the specific driver with a specific type setup.
	 */
	static OPL *create(DriverId driver, OplType type);

	/**
	 * Wrapper to easily init an OPL chip, without specifing an emulator.
	 * By default it will try to initialize an OPL2 emulator, thus an AdLib card.
	 */
	static OPL *create(OplType type = kOpl2);

private:
	static const EmulatorDescription _drivers[];
};

/**
 * The type of the OPL timer callback functor.
 */
typedef Common::Functor0<void> TimerCallback;

/**
 * A representation of a Yamaha OPL chip.
 */
class OPL {
private:
	static bool _hasInstance;
public:
	OPL();
	virtual ~OPL() { _hasInstance = false; }

	/**
	 * Initializes the OPL emulator.
	 *
	 * @return		true on success, false on failure
	 */
	virtual bool init() = 0;

	/**
	 * Reinitializes the OPL emulator
	 */
	virtual void reset() = 0;

	/**
	 * Writes a byte to the given I/O port.
	 *
	 * @param a		port address
	 * @param v		value, which will be written
	 */
	virtual void write(int a, int v) = 0;

	/**
	 * Reads a byte from the given I/O port.
	 *
	 * @param a		port address
	 * @return		value read
	 */
	virtual byte read(int a) = 0;

	/**
	 * Function to directly write to a specific OPL register.
	 * This writes to *both* chips for a Dual OPL2. We allow
	 * writing to secondary OPL registers by using register
	 * values >= 0x100.
	 *
	 * @param r		hardware register number to write to
	 * @param v		value, which will be written
	 */
	virtual void writeReg(int r, int v) = 0;

	/**
	 * Start the OPL with callbacks.
	 */
	void start(TimerCallback *callback, int timerFrequency = kDefaultCallbackFrequency);

	/**
	 * Stop the OPL
	 */
	void stop();

	/**
	 * Change the callback frequency. This must only be called from a
	 * timer proc.
	 */
	virtual void setCallbackFrequency(int timerFrequency) = 0;

	enum {
		/**
		 * The default callback frequency that start() uses
		 */
		kDefaultCallbackFrequency = 250
	};

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
 * An OPL that represents a real OPL, as opposed to an emulated one.
 *
 * This will use an actual timer instead of using one calculated from
 * the number of samples in an AudioStream::readBuffer call.
 */
class RealOPL : public OPL {
public:
	RealOPL();
	virtual ~RealOPL();

	// OPL API
	void setCallbackFrequency(int timerFrequency);

protected:
	// OPL API
	void startCallbacks(int timerFrequency);
	void stopCallbacks();

private:
	static void timerProc(void *refCon);
	void onTimer();

	uint _baseFreq;
	uint _remainingTicks;

	enum {
		kMaxFreq = 100
	};
};

/**
 * An OPL that represents an emulated OPL.
 *
 * This will send callbacks based on the number of samples
 * decoded in readBuffer().
 */
class EmulatedOPL : public OPL, protected Audio::AudioStream {
public:
	EmulatedOPL();
	virtual ~EmulatedOPL();

	// OPL API
	void setCallbackFrequency(int timerFrequency);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	int getRate() const;
	bool endOfData() const { return false; }

protected:
	// OPL API
	void startCallbacks(int timerFrequency);
	void stopCallbacks();

	/**
	 * Read up to 'length' samples.
	 *
	 * Data will be in native endianess, 16 bit per sample, signed.
	 * For stereo OPL, buffer will be filled with interleaved
	 * left and right channel samples, starting with a left sample.
	 * Furthermore, the samples in the left and right are summed up.
	 * So if you request 4 samples from a stereo OPL, you will get
	 * a total of two left channel and two right channel samples.
	 */
	virtual void generateSamples(int16 *buffer, int numSamples) = 0;

private:
	int _baseFreq;

	enum {
		FIXP_SHIFT = 16
	};

	int _nextTick;
	int _samplesPerTick;

	Audio::SoundHandle *_handle;
};
/** @} */
} // End of namespace OPL

#endif
