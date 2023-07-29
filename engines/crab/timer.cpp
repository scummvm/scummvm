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
// Purpose:  Timer functions
//=============================================================================
#include "crab/timer.h"

namespace Crab {

Timer::Timer() {
	// Initialize the variables
	_startTicks = 0;
	_pausedTicks = 0;
	_targetTicks = 0;
	_targetValid = false;
	_paused = false;
	_started = false;
}

void Timer::load(rapidxml::xml_node<char> *node, const Common::String &name, const bool &echo) {
	_targetValid = loadNum(_targetTicks, name, node, echo);
}

void Timer::start() {
	// Start the timer
	_started = true;

	// Resume the timer
	_paused = false;

	// Get the current clock time
	_startTicks = g_system->getMillis();
}

void Timer::stop() {
	// Stop the timer
	_started = false;

	// Resume the timer
	_paused = false;
}

uint32 Timer::ticks() {
	// If the timer is running
	if (_started == true) {
		// If the timer is paused
		if (_paused == true) {
			// Return the number of ticks when the timer was paused
			return _pausedTicks;
		} else {
			// Return the current time minus the start time
			return g_system->getMillis() - _startTicks;
		}
	}
	// If the timer isn't running
	return 0;
}

void Timer::pause() {
	// If the timer is running and isn't already paused
	if ((_started == true) && (_paused == false)) {
		// Pause the timer
		_paused = true;

		// Calculate the paused ticks
		_pausedTicks = g_system->getMillis() - _startTicks;
	}
}

void Timer::resume() {
	// If the timer is paused
	if (_paused == true) {
		// Resume the timer
		_paused = false;

		// Reset the starting ticks
		_startTicks = g_system->getMillis() - _pausedTicks;
		// Reset the paused ticks
		_pausedTicks = 0;
	}
}

bool Timer::targetReached(const float &factor) {
	if (!_started)
		start();

	if (_targetValid && ticks() >= (_targetTicks / factor))
		return true;

	return false;
}

} // End of namespace Crab
