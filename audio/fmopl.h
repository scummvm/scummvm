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

#ifndef AUDIO_FMOPL_H
#define AUDIO_FMOPL_H

#include "audio/chip.h"

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
 * A representation of a Yamaha OPL chip.
 */
class OPL : virtual public Audio::Chip {
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

	using Audio::Chip::start;
	void start(TimerCallback *callback) { start(callback, kDefaultCallbackFrequency); }

	enum {
		/**
		 * The default callback frequency that start() uses
		 */
		kDefaultCallbackFrequency = 250
	};

protected:
	/**
	 * Initializes an OPL3 chip for emulating dual OPL2.
	 * 
	 * @param oplType The type of OPL configuration that the engine expects.
	 * If this is not DualOpl2, this function will do nothing.
	 */
	void initDualOpl2OnOpl3(Config::OplType oplType);

	/**
	 * Processes the specified register write so it will be correctly handled
	 * for emulating dual OPL2 on an OPL3 chip.
	 * 
	 * @param r The register that is written to.
	 * @param v The value written to the register.
	 * @param oplType The type of OPL configuration that the engine expects.
	 * If this is not DualOpl2, this function will do nothing.
	 * @return True if the register write can be processed normally; false if
	 * it should be discarded. In this case, a new call to writeReg is usually
	 * performed by this function to replace the discarded register write.
	*/
	bool emulateDualOpl2OnOpl3(int r, int v, Config::OplType oplType);

	bool _rhythmMode;
	int _connectionFeedbackValues[3];
};

/** @} */
} // End of namespace OPL

#endif
