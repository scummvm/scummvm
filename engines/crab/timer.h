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
#pragma once
#include "common_header.h"

#include "loaders.h"

class Timer {
private:
	// The clock time when the timer started
	Uint32 start_ticks;

	// The ticks stored when the timer was paused
	Uint32 paused_ticks;

	// Since most timers usually only check one value, we might as well as store it here
	Uint32 target_ticks;

	// See if we have a target loaded or set
	bool target_valid;

	// The timer status
	bool paused;
	bool started;

public:
	// Initialize variables
	Timer();
	void Target(const Uint32 &val) {
		target_valid = true;
		target_ticks = val;
	}

	void Load(rapidxml::xml_node<char> *node, const std::string &name, const bool &echo = true);

	// The various clock actions
	void Start();
	void Stop();
	void Pause();
	void Resume();

	// Gets the timer's time
	Uint32 Ticks();

	// Get the time remaining
	Uint32 RemainingTicks() { return target_ticks - Ticks(); }

	// Have we reached the target yet?
	bool TargetReached(const float &factor = 1.0f);

	// Checks the status of the timer
	const bool Started() { return started; }
	const bool Paused() { return paused; }
	const bool TargetValid() { return target_valid; }
};
