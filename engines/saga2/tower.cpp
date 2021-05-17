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

#include "saga2/std.h"
#include "saga2/tower.h"
#include "saga2/osexcept.h"
#include "saga2/messager.h"

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

#if !DEBUG
	OSExceptBlk {
#endif

		r = tower[is].init();

#if !DEBUG
	}
	OSExcepTrap {
		r = FALSE;
		OSExceptHnd;
	}
#endif


	return r;

}

static bool cleanupErr = FALSE;

static void termTowerLayer(int is) {
#if !DEBUG
	OSExceptBlk {
#endif

		tower[is].term();

#if !DEBUG
	}
	OSExcepTrap {
		cleanupErr = TRUE;
		OSExceptHnd;
	}
#endif
}



// ------------------------------------------------------------------------
// Tower Builder

INITIALIZER(programInit) {
	for (initializationState = 0; initializationState < maxInitState;) {
		int32 tLevel = getTowerLayer(initializationState);
		bool r = FALSE;
		SystemEventLoop();
		r = initTowerLayer(tLevel);

		if (r) {
			setInitState(initializationState + 1);
		} else {
			SystemError se = SystemError(
			                     tower[tLevel].onFail,
			                     "Tower Initialization Step %d Failed (record %d)",
			                     initializationState,
			                     tLevel);
			se.notify();
			return FALSE;
		}
	}
	return TRUE;
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
	return TRUE;
}

// ------------------------------------------------------------------------
// Null Destroyer

TERMINATOR(termTowerBase) {
}

// ------------------------------------------------------------------------
// This pair should be the first thing in the tower list

INITIALIZER(initErrorManagers) {
	//initErrorHandlers();
	return SetFatalMode(fhmSimple);
}

// defining VERIFY_EXIT will give you a message box when the program has
//    been sucessfully cleaned up


#ifdef VERIFY_EXIT
#include "saga2/program.h"
#endif

TERMINATOR(termErrorManagers) {
#ifdef VERIFY_EXIT
	extern HWND hWndMain;
#endif
	SetFatalMode(fhmNone);
#ifdef VERIFY_EXIT
	MessageBox(hWndMain, "Cleanup complete", PROGRAM_FULL_NAME, MB_APPLMODAL | MB_OK);
#endif
}

// ------------------------------------------------------------------------
// This pair should be used before the display is initialized

INITIALIZER(initDelayedErrors) {
	return SetFatalMode(fhmHold);
}

TERMINATOR(termDelayedErrors) {
	SetFatalMode(fhmSimple);
}

// ------------------------------------------------------------------------
// This pair should be used once everything is working

INITIALIZER(initActiveErrors) {
	return SetFatalMode(fhmGUI);
}

TERMINATOR(termActiveErrors) {
	SetFatalMode(fhmHold);
}


// ------------------------------------------------------------------------
// State transition hooks

int initState(void) {
	return initializationState;
}

void setInitState(int i) {
#if DEBUG_INIT
	MonoMessager mm = MonoMessager("GameInit");
#else
	NullMessager mm;
#endif

	initializationState = i;

#if DEBUG_INIT
	WriteStatusF(12, "Init State %d", i);
	mm("Init state %d", i);
	if (i == maxInitState) {
		WriteStatusF(13, "INITIALIZATION COMPLETE");
		mm("INITIALIZATION COMPLETE");
	}
#endif
}

} // end of namespace Saga2
