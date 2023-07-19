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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

//=============================================================================
// Author:   Arvind
// Purpose:  Timer class
//=============================================================================
#ifndef CRAB_TIMER_H
#define CRAB_TIMER_H

#include "crab/crab.h"
#include "crab/common_header.h"
#include "crab/loaders.h"

namespace Crab {

class Timer {
private:
	// The clock time when the timer started
	uint32 _startTicks;

	// The ticks stored when the timer was paused
	uint32 _pausedTicks;

	// Since most timers usually only check one value, we might as well as store it here
	uint32 _targetTicks;

	// See if we have a target loaded or set
	bool _targetValid;

	// The timer status
	bool _paused;
	bool _started;

public:
	// Initialize variables
	Timer();

	void target(const uint32 &val) {
		_targetValid = true;
		_targetTicks = val;
	}

	void load(rapidxml::xml_node<char> *node, const Common::String &name, const bool &echo = true);

	// The various clock actions
	void start();
	void stop();
	void pause();
	void resume();

	// Gets the timer's time
	uint32 ticks();

	// Get the time remaining
	uint32 remainingTicks() {
		return _targetTicks - ticks();
	}

	// Have we reached the target yet?
	bool targetReached(const float &factor = 1.0f);

	// Checks the status of the timer
	bool started() {
		return _started;
	}

	bool paused() {
		return _paused;
	}

	bool targetValid() {
		return _targetValid;
	}
};

} // End of namespace Crab

#endif // CRAB_TIMER_H
