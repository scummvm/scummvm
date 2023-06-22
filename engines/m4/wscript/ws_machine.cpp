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
#include "m4/wscript/ws_cruncher.h"
#include "m4/wscript/wscript.h"
#include "m4/core/errors.h"
#include "m4/platform/timer.h"
#include "m4/globals.h"

namespace M4 {

static int32 ws_KillMachines();
static void clear_msg_list(machine *m);
static void clear_persistent_msg_list(machine *m);

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
	_G(pauseTime) = 0;
	_G(enginesPaused) = false;

	return true;
}

void ws_Shutdown() {
	ws_KillTime();
	ws_KillCruncher();
	ws_KillMachines();
	ws_KillHAL();
}

void TerminateMachinesByHash(int32 machHash) {
	warning("TODO: TerminateMachinesByHash");
}

static int32 ws_KillMachines() {
	machine *myMachine;
	globalMsgReq *tempGlobalMsg;
	int32 myBytes = 0;

	// Deallocate all machines
	myMachine = _G(firstMachine);
	while (myMachine) {
		_G(firstMachine) = _G(firstMachine)->next;

		clear_msg_list(myMachine);
		clear_persistent_msg_list(myMachine);

		mem_free(myMachine);
		myBytes += sizeof(machine);
		myMachine = _G(firstMachine);
	}

	// Deallocate global messages
	tempGlobalMsg = _G(myGlobalMessages);
	while (tempGlobalMsg) {
		_G(myGlobalMessages) = _G(myGlobalMessages)->next;
		mem_free((void *)tempGlobalMsg);
		tempGlobalMsg = _G(myGlobalMessages);
	}

	return myBytes;
}

static void dispose_msgRequest(msgRequest *msg) {
	if (msg) {
		mem_free(msg);
	}
}

static void clear_msg_list(machine *m) {
	msgRequest *freeMsg, *nextMsg;

	nextMsg = m->myMsgs;
	while (nextMsg) {
		freeMsg = nextMsg;
		nextMsg = nextMsg->nextMsg;
		dispose_msgRequest(freeMsg);
	}

	m->myMsgs = nullptr;
}

static void clear_persistent_msg_list(machine *m) {
	msgRequest *freeMsg, *nextMsg;

	// Clear the active persistent msgs
	nextMsg = m->myPersistentMsgs;
	while (nextMsg) {
		freeMsg = nextMsg;
		nextMsg = nextMsg->nextMsg;
		dispose_msgRequest(freeMsg);
	}
	m->myPersistentMsgs = NULL;

	// Clear the used persistent msgs
	nextMsg = m->usedPersistentMsgs;
	while (nextMsg) {
		freeMsg = nextMsg;
		nextMsg = nextMsg->nextMsg;
		dispose_msgRequest(freeMsg);
	}
	m->usedPersistentMsgs = NULL;
}

} // End of namespace M4
