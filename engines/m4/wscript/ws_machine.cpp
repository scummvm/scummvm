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
 * along with this program.  If not, see <http:// Www.gnu.org/licenses/>.
 *
 */

#include "m4/wscript/ws_machine.h"
#include "m4/wscript/ws_cruncher.h"
#include "m4/wscript/ws_hal.h"
#include "m4/wscript/wst_regs.h"
#include "m4/wscript/wscript.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/dbg/debug.h"
#include "m4/mem/mem.h"
#include "m4/platform/timer.h"
#include "m4/vars.h"
#include "m4/detection.h"

namespace M4 {

#define COND_FLAG		0x80000000
#define OP_COUNT		0x00007fff
#define OP_JUMP			3
#define OP_KILL			4

static void clear_msg_list(machine *m);
static void clear_persistent_msg_list(machine *m);

bool ws_Initialize(frac16 *theGlobals) {
	int32 i;

	_GWS(machineIDCount) = 0;
	_GWS(dataFormats) = ws_GetDataFormats();

	if (!theGlobals) {
		ws_LogErrorMsg(FL, "ws_Initialize() called without a valid global register array.");
		return false;
	}

	_GWS(ws_globals) = theGlobals;

	for (i = 0; i < GLB_SHARED_VARS; i++) {
		_GWS(ws_globals)[i] = 0;
	}

	_GWS(firstMachine) = nullptr;
	_GWS(nextXM) = nullptr;
	_GWS(myGlobalMessages) = nullptr;

	if (!ws_InitWSTimer()) {
		return false;
	}
	if (!ws_InitCruncher()) {
		return false;
	}

	if (!ws_InitHAL()) {
		return false;
	}

	_GWS(oldTime) = timer_read_60();
	_GWS(pauseTime) = 0;
	_GWS(enginesPaused) = false;

	return true;
}

void ws_Shutdown() {
	ws_KillTime();
	ws_KillCruncher();
	ws_KillMachines();
	ws_KillHAL();
}

static void dispose_msgRequest(msgRequest *msg) {
	if (msg)
		mem_free(msg);
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
	m->myPersistentMsgs = nullptr;

	// Clear the used persistent msgs
	nextMsg = m->usedPersistentMsgs;
	while (nextMsg) {
		freeMsg = nextMsg;
		nextMsg = nextMsg->nextMsg;
		dispose_msgRequest(freeMsg);
	}

	m->usedPersistentMsgs = nullptr;
}

static msgRequest *new_msgRequest() {
	msgRequest *newMsg;
	if ((newMsg = (msgRequest *)mem_alloc(sizeof(msgRequest), "msgRequest")) == nullptr) {
		ws_LogErrorMsg(FL, "Failed to mem_alloc() %d bytes.", sizeof(msgRequest));
	}
	return newMsg;
}

static void restore_persistent_msgs(machine *m) {
	msgRequest *lastMsg;

	// Check params...
	if ((!m) || (!m->usedPersistentMsgs)) {
		return;
	}

	// Loop to find the last used persistent msg
	lastMsg = m->usedPersistentMsgs;
	while (lastMsg->nextMsg) {
		lastMsg = lastMsg->nextMsg;
	}

	// Place the entire usedPersistentMsgs linked list at the front of the persistentMsgs list
	lastMsg->nextMsg = m->myPersistentMsgs;
	m->myPersistentMsgs = m->usedPersistentMsgs;
	m->usedPersistentMsgs = nullptr;
}

// CONDITIONAL OPs

static void op_AFTER(machine *m, int32 *pcOffset) {
	int32 myElapsedTime;

	if (!_GWS(myArg2)) {
		ws_Error(m, ERR_MACH, 0x0261, "functionality: after arg1 {...}");
	}

	if (_GWS(myArg3)) {
		myElapsedTime = (int32)imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3)) >> 16;
	} else {
		myElapsedTime = (int32)(*_GWS(myArg2)) >> 16;
	}

	ws_MakeOnTimeReq(_GWS(ws_globals)[GLB_TIME] + myElapsedTime, m, *pcOffset, (int32)*_GWS(myArg1) >> 14);
	*pcOffset += (int32)*_GWS(myArg1) >> 14;
}

static void op_ON_END_SEQ(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0260, "on_seq_end() failed.");
	}
	ws_OnEndSeqRequest(m->myAnim8, *pcOffset, *_GWS(myArg1) >> 14);
	*pcOffset += (int32)*_GWS(myArg1) >> 14;
}

/**
 * Message Requests are stored in a linked list, directly in the machine
 * A message is never actually received, but when another machine wishes to
 * send a message to this one, it checks this list to see if the message is
 * expected, and if this machine knows what to do.
 */
static void op_ON_MSG(machine *m, int32 *pcOffset) {
	msgRequest *myMsg;
	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0260, "on_msg() failed.");
	}

	if ((myMsg = new_msgRequest()) == nullptr) {
		ws_Error(m, ERR_MACH, 0x02fe, "on_msg() failed.");
		return;
	}

	if (_GWS(myArg2)) {
		myMsg->msgHash = *_GWS(myArg2);
	} else {
		myMsg->msgHash = 0;
	}
	if (_GWS(myArg3)) {
		myMsg->msgValue = *_GWS(myArg3);
	} else {
		myMsg->msgValue = 0;
	}

	myMsg->pcOffset = *pcOffset;
	myMsg->pcCount = (int32)*_GWS(myArg1) >> 14;
	myMsg->nextMsg = m->myMsgs;
	m->myMsgs = myMsg;
	*pcOffset += (int32)*_GWS(myArg1) >> 14;
}

/**
 * Same as op_ON_MSG() except these messages do not get cleared between states
 */
static void op_ON_P_MSG(machine *m, int32 *pcOffset) {
	msgRequest *myMsg, *prevMsg;
	uint32 msgHash;
	frac16 msgValue;

	if (!_GWS(myArg2)) {
		ws_Error(m, ERR_MACH, 0x0261, "functionality: on_p_msg arg1 {...}");
	}

	// Get the values for msgHash and msgValue from the args...
	msgHash = *_GWS(myArg2);
	if (_GWS(myArg3)) {
		msgValue = *_GWS(myArg3);
	} else {
		msgValue = 0;
	}

	// Since the message is persistent, it may have been satisfied earlier, check the used list
	prevMsg = nullptr;
	myMsg = m->usedPersistentMsgs;

	// Loop through all the used msgs, see if there is already a struct in place
	while (myMsg && ((myMsg->msgHash != msgHash) || (myMsg->msgValue != msgValue))) {
		prevMsg = myMsg;
		myMsg = myMsg->nextMsg;
	}

	// If a previous identical msg has already been requested, restore it
	if (myMsg) {
		// Remove it from the used msgs linked list
		// if myMsg is first in the list
		if (!prevMsg) {
			m->usedPersistentMsgs = myMsg->nextMsg;
		} else {
			// Else myMsg is in the middle of the list (after prevMsg)
			prevMsg->nextMsg = myMsg->nextMsg;
		}
	} else {
		// Else a new msg has to be created
		if ((myMsg = new_msgRequest()) == nullptr) {
			ws_Error(m, ERR_MACH, 0x02fe, "on_p_msg() failed.");
			return;
		}

		// Set the msg request values
		myMsg->msgHash = msgHash;
		myMsg->msgValue = msgValue;
	}

	// Since it may be a replacement msg, a new pcOffset may be set
	myMsg->pcOffset = *pcOffset;
	myMsg->pcCount = (int32)*_GWS(myArg1) >> 14;

	// Link it into the list
	myMsg->nextMsg = m->myPersistentMsgs;
	m->myPersistentMsgs = myMsg;

	// Update the pcOffset
	*pcOffset += (int32)*_GWS(myArg1) >> 14;
}

static void op_SWITCH_LT(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg3)) {
		ws_Error(m, ERR_MACH, 0x0262, "functionality: switch (arg1 < arg2) {...}");
	}
	if (*_GWS(myArg2) >= *_GWS(myArg3)) {
		*pcOffset += (int32)*_GWS(myArg1) >> 14;
	}
}

static void op_SWITCH_LE(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg3)) {
		ws_Error(m, ERR_MACH, 0x0262, "functionality: switch (arg1 <= arg2) {...}");
	}
	if (*_GWS(myArg2) > *_GWS(myArg3)) {
		*pcOffset += (int32)*_GWS(myArg1) >> 14;
	}
}

static void op_SWITCH_EQ(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg3)) {
		ws_Error(m, ERR_MACH, 0x0262, "functionality: switch (arg1 == arg2) {...}");
	}
	if (*_GWS(myArg2) != *_GWS(myArg3)) {
		*pcOffset += (int32)*_GWS(myArg1) >> 14;
	}
}

static void op_SWITCH_NE(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg3)) {
		ws_Error(m, ERR_MACH, 0x0262, "functionality: switch (arg1 != arg2) {...}");
	}
	if (*_GWS(myArg2) == *_GWS(myArg3)) {
		*pcOffset += (int32)*_GWS(myArg1) >> 14;
	}
}

static void op_SWITCH_GE(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg3)) {
		ws_Error(m, ERR_MACH, 0x0262, "functionality: switch (arg1 >= arg2) {...}");
	}
	if (*_GWS(myArg2) < *_GWS(myArg3)) {
		*pcOffset += (int32)*_GWS(myArg1) >> 14;
	}
}

static void op_SWITCH_GT(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg3)) {
		ws_Error(m, ERR_MACH, 0x0262, "functionality: switch (arg1 > arg2) {...}");
	}
	if (*_GWS(myArg2) <= *_GWS(myArg3)) {
		*pcOffset += (int32)*_GWS(myArg1) >> 14;
	}
}


// IMMEDIATE OPs

static bool op_DO_NOTHING(machine *m, int32 *pcOffset) {
	return true;
}

static bool op_GOTO(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: goto arg1");
	}
	m->curState = (*_GWS(myArg1)) >> 16;
	m->recurseLevel = 0;
	return false;
}

static bool op_JUMP(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: jump arg1");
	}

	*pcOffset += (int32)*_GWS(myArg1) >> 16;
	return true;
}

static bool op_TERMINATE(machine *m, int32 *pcOffset) {
	m->curState = -1;
	m->recurseLevel = 0;
	return false;
}

static bool op_START_SEQ(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: start_seq arg1");
	}

	// Here we check whether a program was previously running
	if (!m->myAnim8) {
		if ((m->myAnim8 = ws_AddAnim8ToCruncher(m, *_GWS(myArg1) >> 16)) == nullptr) {
			ws_Error(m, ERR_MACH, 0x02ff, "start_seq() failed.");
		}
	} else {
		if (!ws_ChangeAnim8Program(m, *_GWS(myArg1) >> 16)) {
			ws_Error(m, ERR_MACH, 0x02ff, "start_seq() failed.");
		}
	}

	// Inform the ws debugger of the new sequence
	dbg_LaunchSequence(m->myAnim8);

	return true;
}

static bool op_PAUSE_SEQ(machine *m, int32 *pcOffset) {
	ws_PauseAnim8(m->myAnim8);
	return true;
}

static bool op_STORE_VAL(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg2)) {
		ws_Error(m, ERR_MACH, 0x0264, "functionality: arg1 = arg2  or  arg1 = rand(arg2, arg3)");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		*_GWS(myArg1) = *_GWS(myArg2);
	}

	return true;
}

static bool op_SEND_MSG(machine *m, int32 *pcOffset) {
	frac16 msgValue;

	if (!_GWS(myArg2)) {
		ws_Error(m, ERR_MACH, 0x0264, "functionality: send to machine arg1, message arg2");
	}
	if (_GWS(myArg3)) {
		msgValue = *_GWS(myArg3);
	} else {
		msgValue = 0;
	}
	sendWSMessage(*_GWS(myArg2), msgValue, nullptr, *_GWS(myArg1) >> 16, m, 1);
	return true;
}

static bool op_SEND_GMSG(machine *m, int32 *pcOffset) {
	frac16 msgValue;

	if (!_GWS(myArg2)) {
		ws_Error(m, ERR_MACH, 0x0264, "functionality: send to to all machines of type arg1, message arg2");
	}
	if (_GWS(myArg3)) {
		msgValue = *_GWS(myArg3);
	} else {
		msgValue = 0;
	}

	sendWSMessage(*_GWS(myArg2), msgValue, nullptr, *_GWS(myArg1) >> 16, m, 0);
	return true;
}

static bool op_REPLY_MSG(machine *m, int32 *pcOffset) {
	frac16	msgValue;

	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: reply to sender with msg arg1");
	}
	if (_GWS(myArg2)) {
		msgValue = *_GWS(myArg2);
	} else {
		msgValue = 0;
	}

	sendWSMessage(*_GWS(myArg1), msgValue, m->msgReplyXM, 0, m, 1);
	return true;
}

static bool op_SYSTEM_MSG(machine *m, int32 *pcOffset) {
	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: send to 'C' callback function with msg arg1");
	}

	if (m->CintrMsg) {
		(m->CintrMsg)(*_GWS(myArg1), m);
	}

	return true;
}

static bool op_TRIG(machine *m, int32 *pcOffset) {
	int32 myCount, i;
	char tempStr[80];

	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: trigger mach arg1, arg2 instances");
	}

	if (_GWS(myArg2)) {
		if (_GWS(myArg3)) {
			myCount = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3)) >> 16;
		} else {
			myCount = (*_GWS(myArg2)) >> 16;
		}
	} else {
		myCount = 1;
	}

	Common::sprintf_s(tempStr, "*** TRIGGERED BY MACHINE: %d", m->myHash);
	for (i = 0; i < myCount; i++) {
		if (!TriggerMachineByHash(*_GWS(myArg1) >> 16, m->myAnim8, -1, -1, m->CintrMsg, false, tempStr)) {
			ws_Error(m, ERR_MACH, 0x0267, "trig() failed");
		}
	}

	return true;
}

static bool op_TRIG_W(machine *m, int32 *pcOffset) {
	int32 myHash, myCount = 0, minCount = 0, maxCount = 0, i, myInstruction;
	int32 myIndex, minIndex, maxIndex, myDataCount;
	int32 myDataHash;
	bool randFlag = false;
	char tempStr[80];
	uint32 *myPC, *oldPC, *machInstr;

	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "functionality: trigger mach arg1, arg2 instances");
	}

	myHash = (*_GWS(myArg1)) >> 16;
	if (_GWS(myArg2)) {
		if (_GWS(myArg3)) {
			randFlag = true;
			minCount = (*_GWS(myArg2)) >> 16;
			maxCount = (*_GWS(myArg3)) >> 16;
		} else {
			myCount = (*_GWS(myArg2)) >> 16;
		}
	} else {
		myCount = 1;
	}

	// This is a double length instruction - up to 128 bits, we must read in the next pCode
	machInstr = (uint32 *)((intptr)(*(m->machHandle)) + (uint32)m->machInstrOffset);
	myPC = (uint32 *)((intptr)machInstr + *pcOffset);
	oldPC = myPC;

	dbg_SetCurrMachInstr(m, *pcOffset, false);

	if ((myInstruction = ws_PreProcessPcode(&myPC, m->myAnim8)) < 0) {
		ws_Error(m, ERR_MACH, 0x0266, "trig_w() failed.");
	}

	dbg_EndCurrMachInstr();

	// Now find the new pcOffset
	*pcOffset += (byte *)myPC - (byte *)oldPC;

	if (!_GWS(myArg1)) {
		ws_Error(m, ERR_MACH, 0x0263, "trig_w instruction requires a data hash specified by a second pCode.");
	}

	myDataHash = (int32)(*_GWS(myArg1)) >> 16;
	myDataCount = ws_GetDATACount(myDataHash);
	if (_GWS(myArg2)) {
		if (_GWS(myArg3)) {
			minIndex = (*_GWS(myArg2)) >> 16;
			maxIndex = (*_GWS(myArg3)) >> 16;
		} else {
			minIndex = (*_GWS(myArg2)) >> 16;
			maxIndex = (*_GWS(myArg2)) >> 16;
		}
	} else {
		minIndex = 0;
		maxIndex = myDataCount;
	}

	if (myInstruction) {
		for (myIndex = minIndex; myIndex <= maxIndex; myIndex++) {
			if (randFlag) {
				myCount = imath_ranged_rand(minCount, maxCount);
			}
			for (i = 0; i < myCount; i++) {
				Common::sprintf_s(tempStr, "*** TRIGGERED BY MACHINE: %d", m->myHash);
				if (!TriggerMachineByHash(myHash, m->myAnim8, myDataHash, myIndex, m->CintrMsg, false, tempStr)) {
					ws_Error(m, ERR_MACH, 0x0267, "trig_w() failed");
				}
			}
		}
	} else {
		myIndex = imath_ranged_rand(minIndex, maxIndex);
		if (randFlag) {
			myCount = imath_ranged_rand(minCount, maxCount);
		}
		for (i = 0; i < myCount; i++) {
			Common::sprintf_s(tempStr, "*** TRIGGERED BY MACHINE: %d", m->myHash);
			if (!TriggerMachineByHash(myHash, m->myAnim8, myDataHash, myIndex, m->CintrMsg, false, tempStr)) {
				ws_Error(m, ERR_MACH, 0x0267, "trig_w() failed");
			}
		}
	}
	return true;
}

static bool op_CLEAR_REGS(machine *m, int32 *pcOffset) {
	Anim8 *myAnim8;
	int32 i;

	if (!m->myAnim8) {
		ws_Error(m, ERR_INTERNAL, 0x02f3, "clear_regs() failed.");
	}

	myAnim8 = m->myAnim8;
	for (i = 0; i < IDX_COUNT + myAnim8->numLocalVars; i++) {
		myAnim8->myRegs[i] = 0;
	}

	myAnim8->myRegs[IDX_S] = 0x10000;

	return true;
}

static bool op_RESUME_SEQ(machine *m, int32 *pcOffset) {
	if (!m->myAnim8) {
		ws_Error(m, ERR_INTERNAL, 0x02f3, "resume_seq() failed.");
	}

	ws_ResumeAnim8(m->myAnim8);
	return true;
}


bool (*immOpTable[])(machine *m, int32 *pcOffset) = {
	nullptr,					//0		***END
	&op_DO_NOTHING,				//1
	&op_GOTO,					//2
	&op_JUMP,					//3		don't forget the op_jump #define
	&op_TERMINATE,				//4
	&op_START_SEQ,				//5
	&op_PAUSE_SEQ,				//6
	&op_RESUME_SEQ,				//7
	&op_STORE_VAL,				//8
	&op_SEND_MSG,				//9
	&op_SEND_GMSG,				//10
	&op_REPLY_MSG,				//11
	&op_SYSTEM_MSG,				//12
	&op_TRIG,					//13
	&op_TRIG_W,					//14
	&op_CLEAR_REGS				//15
};

void (*condOpTable[])(machine *m, int32 *pcOffset) = {
	&op_AFTER,					//0
	&op_ON_END_SEQ,				//1
	&op_ON_MSG,					//2
	&op_ON_P_MSG,				//3
	&op_SWITCH_LT,				//4
	&op_SWITCH_LE,				//5
	&op_SWITCH_EQ,				//6
	&op_SWITCH_NE,				//7
	&op_SWITCH_GE,				//8
	&op_SWITCH_GT				//9
};

void pauseEngines(void) {
	_GWS(enginesPaused) = true;
}

void unpauseEngines(void) {
	_GWS(enginesPaused) = false;
}

void addPauseTime(int32 myTime) {
	_GWS(pauseTime) += myTime;
}

void cycleEngines(Buffer *cleanBackground, int16 *depth_table, Buffer *screenCodes,
		uint8 *myPalette, uint8 *ICT, bool updateVideo) {
	int32 clockTime;

	dbg_DebugNextCycle();
	clockTime = timer_read_60();

	if (_GWS(enginesPaused)) {
		_GWS(pauseTime) += clockTime - _GWS(oldTime);
		_GWS(oldTime) = clockTime;

	} else {
		_GWS(ws_globals)[GLB_WATCH_DOG] = clockTime - _GWS(pauseTime) - _GWS(ws_globals)[GLB_TIME];
		_GWS(ws_globals)[GLB_TIME] += _GWS(ws_globals)[GLB_WATCH_DOG];
		ws_CrunchAnim8s(depth_table);

		if (cleanBackground) {
			ws_DoDisplay(cleanBackground, depth_table, screenCodes, myPalette, ICT, updateVideo);
		}
		ws_CrunchEOSreqs();

		ws_CheckTimeReqs(_GWS(ws_globals)[GLB_TIME]);
	}
}

void ws_RefreshWoodscriptBuffer(Buffer *cleanBackground, int16 *depth_table,
		Buffer *screenCodes, uint8 *myPalette, uint8 *ICT) {
	ws_hal_RefreshWoodscriptBuffer(_GWS(myCruncher), cleanBackground, depth_table,
		screenCodes, myPalette, ICT);
}

static void cancelAllEngineReqs(machine *m) {
	globalMsgReq *myGMsg, *tempGMsg;

	if (m->machID == DEAD_MACHINE_ID) {
		return;
	}

	//---- CANCEL CRUNCHER REQS
	if (m->myAnim8) {
		ws_CancelOnEndSeq(m->myAnim8);
	}

	//---- Free all pending state message requests in this machine
	clear_msg_list(m);

	//---- Restore all persistent message requests in this machine
	restore_persistent_msgs(m);

	//---- Free all pending global messages requests in this machine
	if (_GWS(myGlobalMessages)) {
		myGMsg = _GWS(myGlobalMessages);
		while (myGMsg->next) {
			if (myGMsg->next->sendM == m) {
				tempGMsg = myGMsg->next;
				myGMsg->next = myGMsg->next->next;
				mem_free((void *)tempGMsg);
			} else myGMsg = myGMsg->next;
		}
	}

	//---- CANCEL TIMER REQS
	ws_CancelOnTimeReqs(m);
}


static void shutdownMachine(machine *m) {
	if (m->machID == DEAD_MACHINE_ID) {
		return;
	}

	dbg_RemoveWSMach(m);

	if (m->myAnim8) {
		ws_RemoveAnim8FromCruncher(m->myAnim8);
	}

	//---- Free all pending message requests in this machine
	clear_msg_list(m);
	clear_persistent_msg_list(m);

	// Fix nextXM so SendWSMessage doesn't break
	if (m == _GWS(nextXM)) {
		_GWS(nextXM) = _GWS(nextXM)->next;
	}

	// Clear any existing walk path
	DisposePath(m->walkPath);

	m->machID = DEAD_MACHINE_ID;

	if (m->machName) {
		m->machName[0] = '\0';
		mem_free((void *)m->machName);
		m->machName = nullptr;
	}
}

static machine *getValidNext(machine *currMachine) {
	machine *iterMachine = currMachine;
	if (iterMachine) {
		while ((iterMachine = iterMachine->next) != nullptr) {
			if (iterMachine->machID != DEAD_MACHINE_ID) {
				return iterMachine;
			}
		}
	}
	return nullptr;
}

void terminateMachinesByHash(uint32 machHash) {
	machine *curr, *next;

	curr = _GWS(firstMachine);					// Start at beginning of machine chain
	while (curr) {
		next = curr->next;						// Preserve next pointer against curr's dealloc
		if (curr->myHash == machHash) {			// is this one to delete?
			if (curr == _GWS(firstMachine)) {	//	maintain the beginning of machine chain
				_GWS(firstMachine) = next;
			}
			cancelAllEngineReqs(curr);			// cancel its requests
			shutdownMachine(curr);				// deallocate the whole ball'o'wax
		}
		curr = next;							// and pop aint32 the chain
	}
}

void terminateMachine(machine *myMachine) {
	if ((!myMachine) || (!verifyMachineExists(myMachine))) {
		return;
	}

	cancelAllEngineReqs(myMachine);
	shutdownMachine(myMachine);
}

void terminateMachineAndNull(machine *&m) {
	if (m)
		terminateMachine(m);
	m = nullptr;
}

bool verifyMachineExists(machine *m) {
	machine *tempM;

	// Parameter verification
	if (!m) {
		return false;
	}

	// Loop through the active machine list, looking for m
	tempM = _GWS(firstMachine);
	while (tempM && (tempM != m)) {
		tempM = getValidNext(tempM);
	}

	// If the end of the list was reached, and m was not found, false
	if (!tempM) {
		return false;
	}

	// Otherwise m was found, therefore machine exists
	return true;
}

int32 ws_KillMachines() {
	machine *myMachine;
	globalMsgReq *tempGlobalMsg;
	int32 myBytes = 0;

	// Deallocate all machines
	myMachine = _GWS(firstMachine);
	while (myMachine) {
		// get any next Machine here, not validNext
		_GWS(firstMachine) = _GWS(firstMachine)->next;

		if (myMachine->machID != DEAD_MACHINE_ID) {
			cancelAllEngineReqs(myMachine);
			shutdownMachine(myMachine);
		}

		mem_free((void *)myMachine);
		myBytes += sizeof(machine);
		myMachine = _GWS(firstMachine);
	}

	// Deallocate global messages
	tempGlobalMsg = _GWS(myGlobalMessages);
	while (tempGlobalMsg) {
		_GWS(myGlobalMessages) = _GWS(myGlobalMessages)->next;
		mem_free((void *)tempGlobalMsg);
		tempGlobalMsg = _GWS(myGlobalMessages);
	}

	return myBytes;
}

void ws_KillDeadMachines() {
	machine *myMachine;
	machine **priorNext = &_GWS(firstMachine);

	// Deallocate all machines that are dead
	while ((myMachine = *priorNext) != nullptr) {
		if (myMachine->machID == DEAD_MACHINE_ID) {
			// Shutdown the dead machine, and unlink it from the machine chain
			*priorNext = myMachine->next;
			mem_free(myMachine);

		} else {
			// Valid machine, skip over
			priorNext = &myMachine->next;
		}
	}
}

// This is the proc designed to evaluate the instructions of the state machine

static int32 StepAt(int32 *pcOffset, machine *m) {
	bool keepProcessing;
	int32 myInstruction;
	Anim8 *myAnim8;
	uint32 machID, *myPC, *oldPC, *machInstr;

	machID = m->machID;
	myAnim8 = m->myAnim8;

	// Find the current PC and process it to get the current instruction
	machInstr = (uint32 *)((intptr)(*(m->machHandle)) + m->machInstrOffset);
	myPC = (uint32 *)((intptr)(machInstr) + *pcOffset);
	oldPC = myPC;
	_GWS(pcOffsetOld) = *pcOffset;

	dbg_SetCurrMachInstr(m, *pcOffset, false);

	if ((myInstruction = ws_PreProcessPcode(&myPC, myAnim8)) < 0) {
		ws_Error(m, ERR_MACH, 0x0266, nullptr);
	}

	dbg_EndCurrMachInstr();

	// Now find the new pcOffset
	*pcOffset += (byte *)myPC - (byte *)oldPC;

	if (myInstruction >= 64) {
		condOpTable[myInstruction - 64](m, pcOffset);
	} else if (myInstruction > 0) {
		keepProcessing = immOpTable[myInstruction](m, pcOffset);

		if (!keepProcessing) {
			// Does the machine still exist
			if (m->machID == machID) {
				cancelAllEngineReqs(m);
				if (m->curState == -1) {
					shutdownMachine(m);
				} else {									// If machine hasn't terminated
					IntoTheState(m);						// recurse to kickstart next state
				}
			}
		}
	}

	return myInstruction;
}


void ws_StepWhile(machine *m, int32 pcOffset, int32 pcCount) {
	int32 myInstruction, oldPC;
	uint32 machID, recurseLevel;

	// We are executing machine instructions after a conditional has been satisfied.
	// Mark where we started
	oldPC = pcOffset;

	// Increment and remember the recurseLevel and the machine ID
	m->recurseLevel++;
	recurseLevel = m->recurseLevel;
	machID = m->machID;

	// Execute instructions until the conditional count has been reached.
	myInstruction = -1;
	while (myInstruction && (myInstruction != OP_KILL) &&
		(pcOffset >= oldPC) && (pcOffset - oldPC < pcCount) &&
		(m->machID == machID) && (m->recurseLevel == recurseLevel)) {
		myInstruction = StepAt(&pcOffset, m);
	}

	// The last instruction might have been a JUMP instruction.  This should be
	// a JUMP to reissue the conditional, Therefore, reexecute the conditional.
	if (myInstruction == OP_JUMP) {
		StepAt(&pcOffset, m);
	}

	// If the above loop executed without being modified (ie terminated) by a call to StepAt()
	if (myInstruction != OP_KILL) {
		if ((m->machID == machID) && (m->recurseLevel == recurseLevel)) {
			m->recurseLevel--;
		}
	}
}


// When a state machine enters a new state, every request and command is
// evaluated immediately.

void IntoTheState(machine *m) {
	int32 myInstruction;
	uint32 *stateTable, machID, recurseLevel;
	int32 pcOffset;

	if ((m->curState >= m->numOfStates) || (m->curState < 0)) {
		ws_Error(m, ERR_INTERNAL, 0x2f2, "IntoTheState() failed.");
	}

	stateTable = (uint32 *)((intptr)(*(m->machHandle)) + (intptr)m->stateTableOffset);
	pcOffset = FROM_LE_32(stateTable[m->curState]);

	// Increment and remember the recurseLevel and the machine ID
	m->recurseLevel++;
	recurseLevel = m->recurseLevel;
	machID = m->machID;

	// Execute all instruction until an instruction (ie. OP_END) signals execution to stop
	// by returning 0, or something has reset the recurseLevel (ie. op_GOTO)
	myInstruction = -1;
	while (myInstruction && (myInstruction != OP_KILL) &&
		((m->machID == machID) && (m->recurseLevel == recurseLevel))) {
		myInstruction = StepAt(&pcOffset, m);
	}

	if (myInstruction != OP_KILL) {
		// If the above loop executed without being modified (ie terminated) by a call to StepAt()
		if ((m->machID == machID) && (m->recurseLevel == recurseLevel)) {
			// Decriment the recurse counter
			m->recurseLevel--;
		}
	}
}

// This proc creates an instance of a machine based on the machine chunk

machine *TriggerMachineByHash(int32 myHash, Anim8 *parentAnim8, int32 dataHash, int32 dataRow, MessageCB CintrMsg, bool debug, const char *machName) {
	machine *m;

	if ((m = (machine *)mem_alloc(sizeof(machine), "machine")) == nullptr) {
		ws_LogErrorMsg(FL, "Out of memory - mem requested: %d.", sizeof(machine));
		ws_LogErrorMsg(FL, "Trying to trigger hash: %d, name: %s", myHash, machName);
		ws_Error(m, ERR_INTERNAL, 0x2fe, "TriggerMachineByHash() failed.");
		return nullptr;
	}

	// Initialize the identification fields
	_GWS(machineIDCount)++;
	if (_GWS(machineIDCount) == DEAD_MACHINE_ID) {
		_GWS(machineIDCount)++;
	}

	m->myHash = myHash;
	m->machID = _GWS(machineIDCount);
	m->machName = mem_strdup(machName);

	if ((m->machHandle = ws_GetMACH(myHash, &m->numOfStates, &m->stateTableOffset, &m->machInstrOffset)) == nullptr) {
		ws_LogErrorMsg(FL, "Trying to trigger hash: %d, name: %s", myHash, machName);
		return nullptr;
	}

	// Get the data handle and offset if requested
	if (dataHash >= 0) {
		m->dataHash = dataHash;
		if ((m->dataHandle = ws_GetDATA(dataHash, (uint32)dataRow, &m->dataOffset)) == nullptr) {
			ws_LogErrorMsg(FL, "Trying to trigger hash: %d, name: %s", myHash, machName);
			return nullptr;
		}
	} else {
		m->dataHash = -1;
		m->dataHandle = nullptr;
	}

	// Insert m into the list...
	m->next = _GWS(firstMachine);
	m->prev = nullptr;
	if (_GWS(firstMachine)) {
		_GWS(firstMachine)->prev = m;
	}
	_GWS(firstMachine) = m;

	m->recurseLevel = 0;
	m->curState = 0;
	m->myAnim8 = nullptr;
	m->parentAnim8 = parentAnim8;
	m->targetCount = 0;
	m->msgReplyXM = nullptr;
	m->CintrMsg = CintrMsg;
	m->myMsgs = nullptr;
	m->myPersistentMsgs = nullptr;
	m->usedPersistentMsgs = nullptr;
	m->walkPath = nullptr;

	dbg_DebugWSMach(m, debug);

	IntoTheState(m);
	return m;
}

machine *TriggerMachineByHash(MessageCB intrMsg, const char *machName) {
	return TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
		intrMsg, machName);
}

machine *TriggerMachineByHash(int32 val1, int32 val2, int32 val3, int32 val4, int32 val5, int32 val6,
	int32 x, int32 y, int32 scale, int32 layer, bool flag,
	MessageCB intrMsg, const char *machName) {
	_G(globals)[GLB_TEMP_1] = val1 << 24;
	_G(globals)[GLB_TEMP_2] = val2 << 16;
	_G(globals)[GLB_TEMP_3] = val3 << 16;
	_G(globals)[GLB_TEMP_4] = val4 << 16;

	_G(globals)[GLB_TEMP_5] = (val5 << 16) / 100;
	_G(globals)[GLB_TEMP_6] = val6 << 16;
	_G(globals)[GLB_TEMP_7] = x << 16;
	_G(globals)[GLB_TEMP_8] = y << 16;
	_G(globals)[GLB_TEMP_9] = (scale << 16) / 100;
	_G(globals)[GLB_TEMP_10] = layer << 16;
	_G(globals)[GLB_TEMP_11] = flag ? 0xffff0000 : 0x10000;

	return TriggerMachineByHash(40, nullptr, -1, -1, intrMsg, false, machName);
}

enum {
	REGULAR_MSG = 0,
	PERSISTENT_MSG
};


static bool SearchMsgList(uint32 msgHash, uint32 msgValue, machine *recvM, int32 whichList, machine *sendM) {
	bool found;
	msgRequest *myMsg = nullptr, *prevMsg;
	int32 pcOffset, pcCount;

	// Initialize search vars
	found = false;
	prevMsg = nullptr;

	// Find the first msg, based on which list is to be searched
	switch (whichList) {
	// Regular messages
	case REGULAR_MSG:
		myMsg = recvM->myMsgs;
		break;

	// Persistent msgs
	case PERSISTENT_MSG:
		myMsg = recvM->myPersistentMsgs;
		break;

	default:
		break;
	}

	// Search through the message list
	while (myMsg && (!found)) {
		// Check if we've found the msg we're looking for
		if ((myMsg->msgHash == msgHash) && ((uint32)myMsg->msgValue == msgValue)) {
			// Set found bool
			found = true;

			// Find out where to begin executing from
			pcOffset = myMsg->pcOffset;
			pcCount = myMsg->pcCount;

			// Remove the msg from the list, based on which list
			switch (whichList) {
			// Regular messages
			case REGULAR_MSG:
				// If myMsg was first in the list
				if (!prevMsg) {
					recvM->myMsgs = myMsg->nextMsg;
				} else {
					// Else it was in the middle
					prevMsg->nextMsg = myMsg->nextMsg;
				}

				// Dispose of the message
				dispose_msgRequest(myMsg);
				break;

			// Persistent messages
			case PERSISTENT_MSG:
				// If myMsg was first in the list
				if (!prevMsg) {
					recvM->myPersistentMsgs = myMsg->nextMsg;
				} else {
					// Else it was in the middle
					prevMsg->nextMsg = myMsg->nextMsg;
				}

				// Move the message to the inactive list
				myMsg->nextMsg = recvM->usedPersistentMsgs;
				recvM->usedPersistentMsgs = myMsg;
				break;

			default:
				break;
			}

			// Set up so the recv machine can reply to this message
			recvM->msgReplyXM = sendM;

			// Service the request
			ws_StepWhile(recvM, pcOffset, pcCount);
		} else {
			// Else check the next message
			prevMsg = myMsg;
			myMsg = myMsg->nextMsg;
		}
	}

	// Return whether a message was found or not
	return found;
}

// This proc is what allows a machine to send a message to another machine(s)

void sendWSMessage(uint32 msgHash, frac16 msgValue, machine *recvM,
		uint32 machHash, machine *sendM, int32 msgCount) {
	bool found, more_to_send;
	machine *currMachine;
	int32 myCount;
	bool sendToAll;
	globalMsgReq *myGlobalMsgs, *tempGlobalMsg;

	debugC(1, kDebugMessages, "Message %xh, %xh, %s, %xh, %s, %d",
		msgHash, msgValue, recvM ? recvM->machName : "NONE",
		machHash, sendM ? sendM->machName : "NONE", msgCount);

	// In this case we are sending to a specific machine: recvM
	if (recvM) {
		// Search first the regular message list, and if it was not found
		if (!SearchMsgList(msgHash, msgValue, recvM, REGULAR_MSG, sendM)) {
			// Search the persistent message list
			SearchMsgList(msgHash, msgValue, recvM, PERSISTENT_MSG, sendM);
		}

		// and return
		return;
	}

	// Otherwise...
	// Not sending to a specific machine, so send to <msgCount> machines with the given hash

	// Prepare a global message structure
	if ((tempGlobalMsg = (globalMsgReq *)mem_alloc(sizeof(globalMsgReq), "globalMsgReq")) == nullptr) {
		ws_LogErrorMsg(FL, "Out of memory - mem requested: %d.", sizeof(machine));
		ws_Error(nullptr, ERR_INTERNAL, 0x2fe, "SendWSMessage() failed.");
	}
	tempGlobalMsg->msgHash = msgHash;
	tempGlobalMsg->msgValue = msgValue;
	tempGlobalMsg->machHash = machHash;
	tempGlobalMsg->sendM = sendM;
	tempGlobalMsg->msgCount = msgCount;
	tempGlobalMsg->next = nullptr;

	// If we are in the middle of a "global send message", queue the request and exit
	// Question: is this a re-entrancy check?
	// Answer: not really.  If a machine sends out a "global send message", then we
	//         to completely process the "global send message" before any other
	//				"global send message" statements are executed.  Suppose machine:A
	//				accepted two different messages.  Machine:B sends out message:1
	//				Machine:C also receives message:1 sent by machine:B, and sends out
	//				message:2.  Since machine:C received message:1 before machine:A, now
	//				both message:1 and message:2 have been sent.  Which does machine:A
	//				respond to?  Queueing message:2 until message:1 has been completely
	//				processed ensures predictability.  ie.  In this case, machine:A
	//				will respond to message:1 before any machine responds to message:2.
	//

	// Check to see if we are already in the middle of processing global messages
	if (_GWS(myGlobalMessages)) {
		// Find the end of the global list
		myGlobalMsgs = _GWS(myGlobalMessages);
		while (myGlobalMsgs->next) {
			myGlobalMsgs = myGlobalMsgs->next;
		}

		// myGlobalMsgs is the last element, now tempGlobalMsg is.
		myGlobalMsgs->next = tempGlobalMsg;

		// Since we are already processing a global message, this one is now queued, and we return
		return;
	}

	// We are not currently processing another global message, therefore put this on the queue
	// To prevent future global requests from processing until this request is serviced
	_GWS(myGlobalMessages) = tempGlobalMsg;

	// Loop through and service all global requests.
	while (_GWS(myGlobalMessages)) {
		// Sending to all machines, or just a bunch of them?
		myCount = _GWS(myGlobalMessages)->msgCount;
		if (myCount <= 0) {
			sendToAll = true;
		} else {
			sendToAll = false;
		}

		// Search machine list
		more_to_send = true;
		currMachine = _GWS(firstMachine);
		while (currMachine && more_to_send) {
			// Set nextXM up in case this machine is deleted during the ws_StepWhile
			// nextXM will be maintained by ShutDownMachine()
			_GWS(nextXM) = getValidNext(currMachine);

			// Have we got a machine of the specified hash
			if (currMachine->myHash == _GWS(myGlobalMessages)->machHash) {
				// Search the machines regular list.
				found = SearchMsgList(msgHash, msgValue, currMachine, REGULAR_MSG, sendM);

				// If the message wasn't found in the regular list, search the persistent list
				if (!found) {
					found = SearchMsgList(msgHash, msgValue, currMachine, PERSISTENT_MSG, sendM);
				}

				// Check to see if found
				if (found) {
					myCount--;
					if ((!sendToAll) && (myCount <= 0)) {
						more_to_send = false;
					}
				}
			}

			currMachine = _GWS(nextXM);
		}

		// Note: ws_StepWhile could have added more messages to the proceedings
		// Discard a global message and queue up the next one:
		tempGlobalMsg = _GWS(myGlobalMessages);
		_GWS(myGlobalMessages) = _GWS(myGlobalMessages)->next;
		mem_free(tempGlobalMsg);
	}
}

void sendWSMessage(int32 val1, machine *recv, int32 series1, int32 val3, int32 val4,
	int32 trigger, int32 series2, int32 val6, int32 val7, int32 val8) {
	if (!trigger)
		trigger = -1;

	_G(globals)[GLB_TEMP_1] = val1 << 16;
	_G(globals)[GLB_TEMP_2] = series1 << 24;
	_G(globals)[GLB_TEMP_3] = val3 << 16;
	_G(globals)[GLB_TEMP_4] = val4 << 16;
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);
	_G(globals)[GLB_TEMP_6] = val6 << 16;
	_G(globals)[GLB_TEMP_7] = val7 << 16;
	_G(globals)[GLB_TEMP_8] = val8 << 16;
	_G(globals)[GLB_TEMP_9] = series2 << 24;

	sendWSMessage(0x10000, 0, recv, 0, nullptr, 1);
}

} // End of namespace M4
