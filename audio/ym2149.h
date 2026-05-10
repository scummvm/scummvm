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

#ifndef AUDIO_YM2149_H
#define AUDIO_YM2149_H

#include "audio/chip.h"
#include "common/scummsys.h"

namespace YM2149 {

class YM2149;

class Config {
public:
	/**
	 * Creates a YM2149 driver.
	 */
	static YM2149 *create();
};

class YM2149 : virtual public Audio::Chip {
private:
	static bool _hasInstance;

public:
	YM2149();
	virtual ~YM2149();

	/**
	 * Initializes the YM2149 emulator.
	 *
	 * @return		true on success, false on failure
	 */
	virtual bool init() = 0;

	/**
	 * Reinitializes the YM2149 emulator
	 */
	virtual void reset() = 0;

	/**
	 * Function to directly write to a specific YM2149 register.
	 *
	 * @param r		hardware register number to write to
	 * @param v		value, which will be written
	 */
	virtual void writeReg(int r, uint8 v) = 0;
};

} // End of namespace YM2149

#endif
