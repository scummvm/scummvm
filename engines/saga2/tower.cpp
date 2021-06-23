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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/debug.h"

#include "saga2/saga2.h"
#include "saga2/tower.h"

namespace Saga2 {

// enable the following to display startup states
#define DEBUG_INIT 0

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern int maxInitState;
extern TowerLayer tower[];
void SystemEventLoop(void);

/* ===================================================================== *
   Locals
 * ===================================================================== */

static int initializationState;

/* ===================================================================== *
   Code
 * ===================================================================== */

// ------------------------------------------------------------------------
// Find the appropriate init/term pair

static int32 getTowerLayer(int is) {
	for (int32 i = 0; i < maxInitState; i++)
		if (tower[i].ord == is)
			return (i);
	return 0;
}

static bool initTowerLayer(int is) {
	bool r;

	r = tower[is].init();

	return r;
}

static void termTowerLayer(int is) {
	tower[is].term();
}



// ------------------------------------------------------------------------
// Tower Builder

INITIALIZER(programInit) {
	for (initializationState = 0; initializationState < maxInitState;) {
		int32 tLevel = getTowerLayer(initializationState);
		bool r = false;
		SystemEventLoop();
		r = initTowerLayer(tLevel);

		if (r) {
			setInitState(initializationState + 1);
		} else {
			error("Tower Initialization Step %d Failed (record %d)", initializationState, tLevel);
			return false;
		}
	}
	return true;
}

// ------------------------------------------------------------------------
// Tower Destroyer

TERMINATOR(programTerm) {
	while (initializationState > 0) {
		setInitState(initializationState - 1);
		int32 tLevel = getTowerLayer(initializationState);
		termTowerLayer(tLevel);
	}
}


// ------------------------------------------------------------------------
// Null Builder

INITIALIZER(initTowerBase) {
	return true;
}

// ------------------------------------------------------------------------
// Null Destroyer

TERMINATOR(termTowerBase) {
}

// ------------------------------------------------------------------------
// This pair should be the first thing in the tower list

INITIALIZER(initErrorManagers) {
	return true;
}

TERMINATOR(termErrorManagers) {
}

// ------------------------------------------------------------------------
// This pair should be used before the display is initialized

INITIALIZER(initDelayedErrors) {
	return true;
}

TERMINATOR(termDelayedErrors) {
}

// ------------------------------------------------------------------------
// This pair should be used once everything is working

INITIALIZER(initActiveErrors) {
	return true;
}

TERMINATOR(termActiveErrors) {
}


// ------------------------------------------------------------------------
// State transition hooks

int initState(void) {
	return initializationState;
}

void setInitState(int i) {
	initializationState = i;

	debugC(1, kDebugInit, "Init state %d", i);
	if (i == maxInitState)
		debugC(1, kDebugInit, "INITIALIZATION COMPLETE");
}

} // end of namespace Saga2
