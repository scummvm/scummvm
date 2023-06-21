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

#include "m4/wscript/ws_machine.h"
#include "m4/wscript/ws_crnch.h"
#include "m4/wscript/wscript.h"
#include "m4/core/errors.h"
#include "m4/globals.h"

namespace M4 {

bool ws_Initialize(frac16 *theGlobals) {
	int32 i;

	_G(machineIDCount) = 0;

	_G(dataFormats) = ws_GetDataFormats();

	if (!theGlobals) {
		ws_LogErrorMsg(FL, "ws_Initialize() called without a valid global register array.");
		return false;
	}

	_G(ws_globals) = theGlobals;

	for (i = 0; i < GLB_SHARED_VARS; i++) {
		_G(ws_globals)[i] = 0;
	}

	_G(firstMachine) = NULL;
	_G(nextXM) = NULL;
	_G(myGlobalMessages) = NULL;
#ifdef TODO
	if (!ws_InitWSTimer()) {
		return false;
	}
	if (!ws_InitCruncher()) {
		return false;
	}
	if (!ws_InitHAL()) {
		return false;
	}

	_G(oldTime) = timer_read_60();
#endif
	_G(pauseTime) = 0;
	_G(enginesPaused) = false;

	return true;
}

void ws_Shutdown() {
#ifdef TODO
	ws_KillTime();
	ws_KillCruncher();
	ws_KillMachines();
	ws_KillHAL();
#endif
}

void TerminateMachinesByHash(int32 machHash) {
	warning("TODO: TerminateMachinesByHash");
}

} // End of namespace M4
