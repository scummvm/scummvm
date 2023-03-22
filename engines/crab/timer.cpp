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
#include "timer.h"

Timer::Timer() {
	// Initialize the variables
	start_ticks = 0;
	paused_ticks = 0;
	target_ticks = 0;
	target_valid = false;
	paused = false;
	started = false;
}

void Timer::Load(rapidxml::xml_node<char> *node, const std::string &name, const bool &echo) {
	target_valid = LoadNum(target_ticks, name, node, echo);
}

void Timer::Start() {
	// Start the timer
	started = true;

	// Resume the timer
	paused = false;

	// Get the current clock time
	start_ticks = SDL_GetTicks();
}

void Timer::Stop() {
	// Stop the timer
	started = false;

	// Resume the timer
	paused = false;
}

Uint32 Timer::Ticks() {
	// If the timer is running
	if (started == true) {
		// If the timer is paused
		if (paused == true) {
			// Return the number of ticks when the timer was paused
			return paused_ticks;
		} else {
			// Return the current time minus the start time
			return SDL_GetTicks() - start_ticks;
		}
	}
	// If the timer isn't running
	return 0;
}

void Timer::Pause() {
	// If the timer is running and isn't already paused
	if ((started == true) && (paused == false)) {
		// Pause the timer
		paused = true;

		// Calculate the paused ticks
		paused_ticks = SDL_GetTicks() - start_ticks;
	}
}

void Timer::Resume() {
	// If the timer is paused
	if (paused == true) {
		// Resume the timer
		paused = false;

		// Reset the starting ticks
		start_ticks = SDL_GetTicks() - paused_ticks;

		// Reset the paused ticks
		paused_ticks = 0;
	}
}

bool Timer::TargetReached(const float &factor) {
	if (!started)
		Start();

	if (target_valid && Ticks() >= (target_ticks / factor))
		return true;

	return false;
}
