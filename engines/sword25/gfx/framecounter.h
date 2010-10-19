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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_FRAMECOUNTER_H
#define SWORD25_FRAMECOUNTER_H

// Includes
#include "sword25/kernel/common.h"

namespace Sword25 {

/**
 * A simple class that implements a frame counter
 */
class Framecounter {
private:

	// TODO: This class should be rewritten based on Audio::Timestamp,
	// which provides higher accuracy and avoids using 64 bit data types.
	typedef unsigned long long uint64;
	typedef signed long long int64;

	enum {
		DEFAULT_UPDATE_FREQUENCY = 10
	};

public:
	/**
	 * Creates a new BS_Framecounter object
	 * @param UpdateFrequency   Specifies how often the frame counter should be updated in a sceond.
	 * The default value is 10.
	 */
	Framecounter(int updateFrequency = DEFAULT_UPDATE_FREQUENCY);

	/**
	 * Determines how often the frame counter should be updated in a second.
	 * @param UpdateFrequency   Specifies how often the frame counter should be updated in a second.
	 */
	inline void setUpdateFrequency(int updateFrequency);

	/**
	 * This method must be called once per frame.
	 */
	void update();

	/**
	 * Returns the current FPS value.
	 */
	int getFPS() const {
		return _FPS;
	}

private:
	int _FPS;
	int _FPSCount;
	int64 _lastUpdateTime;
	uint64 _updateDelay;
};

// Inlines
void Framecounter::setUpdateFrequency(int updateFrequency) {
	// Frequency in time (converted to microseconds)
	_updateDelay = 1000000 / updateFrequency;
}

} // End of namespace Sword25

#endif
