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

#include "audio/chip.h"

namespace CMS {

class CMS;

class Config {
public:
	/**
	 * Creates a CMS driver.
	 */
	static CMS *create();
};

class CMS : virtual public Audio::Chip {
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

	using Audio::Chip::start;
	void start(TimerCallback *callback) { start(callback, DEFAULT_CALLBACK_FREQUENCY); }
};

} // End of namespace CMS

#endif
