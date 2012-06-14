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
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/scummsys.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "tony/tony.h"
#include "tony/mpal/lzo.h"	
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpaldll.h"

namespace Tony {

namespace MPAL {

#define GETARG(type)   va_arg(v, type)

/****************************************************************************\
*       Copyright
\****************************************************************************/

const char *mpalCopyright =
	"\n\nMPAL - MultiPurpose Adventure Language for Windows 95\n"
	"Copyright 1997-98 Giovanni Bajo and Luca Giusti\n"
	"ALL RIGHTS RESERVED\n"
	"\n"
	"\n";

/****************************************************************************\
*       Internal functions
\****************************************************************************/

/**
 * Locks the variables for access
 */
void lockVar(void) {
	GLOBALS._lpmvVars = (LPMPALVAR)globalLock(GLOBALS._hVars);
}

/**
 * Unlocks variables after use
 */
void unlockVar(void) {
	globalUnlock(GLOBALS._hVars);
}

/**
 * Locks the messages for access
 */
static void LockMsg(void) {
#ifdef NEED_LOCK_MSGS
	GLOBALS._lpmmMsgs = (LPMPALMSG)globalLock(GLOBALS._hMsgs);
#endif
}


/**
 * Unlocks the messages after use
 */
static void UnlockMsg(void) {
#ifdef NEED_LOCK_MSGS
	globalUnlock(GLOBALS._hMsgs);
#endif
}


/**
 * Locks the dialogs for access
 */
static void lockDialogs(void) {
	GLOBALS._lpmdDialogs = (LPMPALDIALOG)globalLock(GLOBALS._hDialogs);
}


/**
 * Unlocks the dialogs after use
 */
static void unlockDialogs(void) {
	globalUnlock(GLOBALS._hDialogs);
}


/**
 * Locks the location data structures for access
 */
static void lockLocations(void) {
	GLOBALS._lpmlLocations = (LPMPALLOCATION)globalLock(GLOBALS._hLocations);
}


/**
 * Unlocks the location structures after use
 */
static void unlockLocations(void) {
	globalUnlock(GLOBALS._hLocations);
}


/**
 * Locks the items structures for use
 */
static void lockItems(void) {
	GLOBALS._lpmiItems = (LPMPALITEM)globalLock(GLOBALS._hItems);
}


/**
 * Unlocks the items structures after use
 */
static void unlockItems(void) {
	globalUnlock(GLOBALS._hItems);
}


/**
 * Locks the script data structures for use
 */
static void LockScripts(void) {
	GLOBALS._lpmsScripts = (LPMPALSCRIPT)globalLock(GLOBALS._hScripts);
}


/**
 * Unlocks the script data structures after use
 */
static void unlockScripts(void) {
	globalUnlock(GLOBALS._hScripts);
}


/**
 * Returns the current value of a global variable
 *
 * @param lpszVarName		Name of the variable
 * @returns		Current value
 * @remarks		Before using this method, you must call lockVar() to
 * lock the global variablves for use. Then afterwards, you will
 * need to remember to call UnlockVar()
 */
int32 varGetValue(const char *lpszVarName) {
	int i;
	LPMPALVAR v = GLOBALS._lpmvVars;

	for (i = 0; i < GLOBALS._nVars; v++, i++)
		if (strcmp(lpszVarName, v->lpszVarName) == 0)
			return v->dwVal;

	GLOBALS._mpalError = 1;
	return 0;
}


/**
 * Sets the value of a MPAL global variable
 * @param lpszVarName       Name of the variable
 * @param val				Value to set
 */
void varSetValue(const char *lpszVarName, int32 val) {
	uint i;
	LPMPALVAR v = GLOBALS._lpmvVars;

	for (i = 0; i < GLOBALS._nVars; v++, i++)
		if (strcmp(lpszVarName, v->lpszVarName) == 0) {
			v->dwVal = val;
			if (GLOBALS._lpiifCustom != NULL && strncmp(v->lpszVarName, "Pattern.", 8) == 0) {
				i = 0;
				sscanf(v->lpszVarName, "Pattern.%u", &i);
				GLOBALS._lpiifCustom(i, val, -1);
			} else if (GLOBALS._lpiifCustom != NULL && strncmp(v->lpszVarName, "Status.", 7) == 0) {
				i = 0;
				sscanf(v->lpszVarName,"Status.%u", &i);
				GLOBALS._lpiifCustom(i, -1, val);
			}
			return;
		}

	GLOBALS._mpalError = 1;
	return;
}


/**
 * Find the index of a location within the location array. Remember to call LockLoc() beforehand.
 *
 * @param nLoc				Location number to search for
 * @returns		Index, or -1 if the location is not present
 * @remarks		This function requires the location list to have
 * first been locked with a call to LockLoc().
 */
static int locGetOrderFromNum(uint32 nLoc) {
	int i;
	LPMPALLOCATION loc = GLOBALS._lpmlLocations;

	for (i = 0; i < GLOBALS._nLocations; i++, loc++)
		if (loc->nObj == nLoc)
			return i;

	return -1;
}


/**
 * Find the index of a message within the messages array
 * @param nMsg				Message number to search for
 * @returns		Index, or -1 if the message is not present
 * @remarks		This function requires the message list to have
 * first been locked with a call to LockMsg()
 */
static int msgGetOrderFromNum(uint32 nMsg) {
	int i;
	LPMPALMSG msg = GLOBALS._lpmmMsgs;

	for (i = 0; i < GLOBALS._nMsgs; i++, msg++)
		if (msg->_wNum == nMsg)
			return i;

	return -1;
}

/**
 * Find the index of an item within the items array
 * @param nItem				Item number to search for
 * @returns		Index, or -1 if the item is not present
 * @remarks		This function requires the item list to have
 * first been locked with a call to LockItems()
 */
static int itemGetOrderFromNum(uint32 nItem) {
	int i;
	LPMPALITEM item = GLOBALS._lpmiItems;

	for (i = 0; i < GLOBALS._nItems; i++, item++)
		if (item->nObj == nItem)
			return i;

	return -1;
}


/**
 * Find the index of a script within the scripts array
 * @param nScript			Script number to search for
 * @returns		Index, or -1 if the script is not present
 * @remarks		This function requires the script list to have
 * first been locked with a call to LockScripts()
 */
static int scriptGetOrderFromNum(uint32 nScript) {
	int i;
	LPMPALSCRIPT script = GLOBALS._lpmsScripts;

	for (i = 0; i < GLOBALS._nScripts; i++, script++)
		if (script->nObj == nScript)
			return i;

	return -1;
}


/**
 * Find the index of a dialog within the dialogs array
 * @param nDialog			Dialog number to search for
 * @returns		Index, or -1 if the dialog is not present
 * @remarks		This function requires the dialog list to have
 * first been locked with a call to LockDialogs()
 */
static int dialogGetOrderFromNum(uint32 nDialog) {
	int i;
	LPMPALDIALOG dialog = GLOBALS._lpmdDialogs;

	for (i = 0; i < GLOBALS._nDialogs; i++, dialog++)
		if (dialog->nObj == nDialog)
			return i;

	return -1;
}


/**
 * Duplicates a message
 * @param nMsgOrd			Index of the message inside the messages array
 * @returns		Pointer to the duplicated message.
 * @remarks		Remember to free the duplicated message when done with it.
 */
static char *DuplicateMessage(uint32 nMsgOrd) {
	const char *origmsg;
	char *clonemsg;
	int j;

	if (nMsgOrd == (uint32)-1)
		return NULL;

	origmsg = (const char *)globalLock(GLOBALS._lpmmMsgs[nMsgOrd]._hText);

	j = 0;
	while (origmsg[j] != '\0' || origmsg[j + 1] != '\0')
		j++;
	j += 2;

	clonemsg = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, j);
	if (clonemsg == NULL)
		return NULL;

	copyMemory(clonemsg, origmsg, j);
	globalUnlock(GLOBALS._lpmmMsgs[nMsgOrd]._hText);

	return clonemsg;
}


/**
 * Duplicate a sentence of a dialog
 * @param nDlgOrd			Index of the dialog in the dialogs array
 * @param nPeriod           Sentence number to be duplicated.
 * @returns		Pointer to the duplicated phrase. Remember to free it
 * when done with it.
 */
static char *duplicateDialogPeriod(uint32 nPeriod) {
	const char *origmsg;
	char *clonemsg;
	LPMPALDIALOG dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;
	int i, j;

	for (j = 0; dialog->_periods[j] != NULL; j++)
		if (dialog->_periodNums[j] == nPeriod) {
			/* Found the phrase, it should be duplicated */
			origmsg = (const char *)globalLock(dialog->_periods[j]);

			/* Calculate the length and allocate memory */
			i = 0;
			while (origmsg[i] != '\0') i++;

			clonemsg = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, i + 1);
			if (clonemsg == NULL)
				return NULL;

			copyMemory(clonemsg, origmsg, i);

			globalUnlock(dialog->_periods[j]);

			return clonemsg;
		}

	return NULL;
}


/**
 * Load a resource from the MPR file
 *
 * @param dwId				ID of the resource to load
 * @returns		Handle to the loaded resource
 */
HGLOBAL resLoad(uint32 dwId) {
	int i;
	HGLOBAL h;
	char head[4];
	uint32 nBytesRead;
	uint32 nSizeComp, nSizeDecomp;
	byte *temp, *buf;

	for (i = 0; i < GLOBALS._nResources; i++)
		if (GLOBALS._lpResources[i * 2] == dwId) {
			GLOBALS._hMpr.seek(GLOBALS._lpResources[i * 2 + 1]);
			nBytesRead = GLOBALS._hMpr.read(head, 4);
			if (nBytesRead != 4)
				return NULL;
			if (head[0] != 'R' || head[1] != 'E' || head[2] != 'S' || head[3] != 'D')
				return NULL;

			nSizeDecomp = GLOBALS._hMpr.readUint32LE();
			if (GLOBALS._hMpr.err())
				return NULL;

			nSizeComp = GLOBALS._hMpr.readUint32LE();
			if (GLOBALS._hMpr.err())
				return NULL;

			h = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, nSizeDecomp + (nSizeDecomp / 1024) * 16);
			buf = (byte *)globalLock(h);
			temp = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT,nSizeComp);

			nBytesRead = GLOBALS._hMpr.read(temp, nSizeComp);
			if (nBytesRead != nSizeComp)
				return NULL;

			lzo1x_decompress(temp, nSizeComp, buf, &nBytesRead);
			if (nBytesRead != nSizeDecomp)
				return NULL;

			globalDestroy(temp);
			globalUnlock(h);
			return h;
		}

	return NULL;
}

static uint32 *getSelectList(uint32 i) {
	uint32 *sl;
	int j, k, num;
	LPMPALDIALOG dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	/* Count how many are active selects */
	num = 0;
	for (j = 0; dialog->_choice[i]._select[j].dwData != 0; j++)
		if (dialog->_choice[i]._select[j].curActive)
			num++;

	/* If there are 0, it's a mistake */
	if (num == 0)
		return NULL;

	sl= (uint32 *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(uint32) * (num + 1));
	if (sl == NULL)
		return NULL;

	/* Copy all the data inside the active select list */
	k = 0;
	for (j = 0; dialog->_choice[i]._select[j].dwData != 0; j++)
		if (dialog->_choice[i]._select[j].curActive)
			sl[k++] = dialog->_choice[i]._select[j].dwData;

	sl[k] = (uint32)NULL;
	return sl;
}

static uint32 *GetItemList(uint32 nLoc) {
	uint32 *il;
	uint32 num,i,j;
	LPMPALVAR v = GLOBALS._lpmvVars;

	num = 0;
	for (i = 0; i < GLOBALS._nVars; i++, v++) {
		if (strncmp(v->lpszVarName,"Location",8) == 0 && v->dwVal == nLoc)
			num++;
	}

	il = (uint32 *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(uint32) * (num + 1));
	if (il == NULL)
		return NULL;

	v = GLOBALS._lpmvVars;
	j = 0;
	for (i = 0; i < GLOBALS._nVars; i++, v++) {
		if (strncmp(v->lpszVarName, "Location", 8) == 0 && v->dwVal == nLoc) {
			sscanf(v->lpszVarName, "Location.%u", &il[j]);
			j++;
		}
	}

	il[j] = (uint32)NULL;
	return il;
}

static LPITEM getItemData(uint32 nOrdItem) {
	LPMPALITEM curitem = GLOBALS._lpmiItems + nOrdItem;
	LPITEM ret;
	HGLOBAL hDat;
	char *dat;
	int i, j;
	char *patlength;
	uint32 dim;

	// Zeroing out the allocated memory is required!!!
	ret = (LPITEM)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(ITEM));
	if (ret == NULL)
		return NULL;
	ret->_speed = 150;

	hDat = resLoad(curitem->dwRes);
	dat = (char *)globalLock(hDat);

	if (dat[0] == 'D' && dat[1] == 'A' && dat[2] == 'T') {
		i = dat[3];			// For version 1.0!!
		dat += 4;

		if (i >= 0x10) {	// From 1.0, there's a destination point for each object
			ret->_destX = (int16)READ_LE_UINT16(dat);
			ret->_destY = (int16)READ_LE_UINT16(dat + 2);
			dat += 4;
		}

		if (i >= 0x11) {	// From 1.1, there's animation speed
			ret->_speed = READ_LE_UINT16(dat);
			dat += 2;
		} else
			ret->_speed = 150;
	}

	ret->_numframe = *dat++;
	ret->_numpattern = *dat++;
	ret->_destZ = *dat++;

	// Upload the left & top co-ordinates of each frame
	for (i = 0; i < ret->_numframe; i++) {
		ret->_frameslocations[i].left = (int16)READ_LE_UINT16(dat);
		ret->_frameslocations[i].top = (int16)READ_LE_UINT16(dat + 2);
		dat += 4;
	}

	// Upload the size of each frame and calculate the right & bottom
	for (i = 0; i < ret->_numframe; i++) {
		ret->_frameslocations[i].right = (int16)READ_LE_UINT16(dat) + ret->_frameslocations[i].left;
		ret->_frameslocations[i].bottom = (int16)READ_LE_UINT16(dat + 2) + ret->_frameslocations[i].top;
		dat += 4;
	}

	// Upload the bounding boxes of each frame
	for (i = 0; i < ret->_numframe; i++) {
		ret->_bbox[i].left = (int16)READ_LE_UINT16(dat);
		ret->_bbox[i].top = (int16)READ_LE_UINT16(dat + 2);
		ret->_bbox[i].right = (int16)READ_LE_UINT16(dat + 4);
		ret->_bbox[i].bottom = (int16)READ_LE_UINT16(dat + 6);
		dat += 8;
	}

	// Load the animation pattern
	patlength = dat;
	dat += ret->_numpattern;

	for (i = 1; i < ret->_numpattern; i++) {
		for (j = 0; j < patlength[i]; j++)
			ret->_pattern[i][j] = dat[j];
		ret->_pattern[i][(int)patlength[i]] = 255;   // Terminate pattern
		dat += patlength[i];
	}

	// Upload the individual frames of animations
	for (i = 1; i < ret->_numframe; i++) {
		dim = (uint32)(ret->_frameslocations[i].right - ret->_frameslocations[i].left) *
			(uint32)(ret->_frameslocations[i].bottom - ret->_frameslocations[i].top);
		ret->_frames[i] = (char *)globalAlloc(GMEM_FIXED,dim);
   
		if (ret->_frames[i] == NULL)
			return NULL;
		copyMemory(ret->_frames[i], dat, dim);
		dat += dim;
	}

	// Check if we've got to the end of the file
	i = READ_LE_UINT16(dat);
	if (i != 0xABCD)
		return NULL;

	globalUnlock(hDat);
	globalFree(hDat);

	return ret;
}


/** 
 * Thread that calls a custom function. It is used in scripts, so that each script
 * function is executed without delaying the others.
 *
 * @param param				pointer to a pointer to the structure that defines the call.
 * @remarks		The passed structure is freed when the process finishes.
 */
void CustomThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		LPCFCALL p;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->p = *(LPCFCALL *)param;

	CORO_INVOKE_4(GLOBALS._lplpFunctions[_ctx->p->_nCf], _ctx->p->_arg1, _ctx->p->_arg2, _ctx->p->_arg3, _ctx->p->_arg4);

	globalFree(_ctx->p);

	CORO_END_CODE;
}


/**
 * Main process for running a script.
 *
 * @param param				Pointer to a pointer to a structure containing the script data.
 * @remarks		The passed structure is freed when the process finishes.
 */
void ScriptThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		uint i, j, k;
		uint32 dwStartTime;
		uint32 dwCurTime;
		uint32 dwId;
		int numHandles;
		LPCFCALL p;
	CORO_END_CONTEXT(_ctx);

	static uint32 cfHandles[MAX_COMMANDS_PER_MOMENT];
	LPMPALSCRIPT s = *(const LPMPALSCRIPT *)param;

	CORO_BEGIN_CODE(_ctx);

	_ctx->dwStartTime = _vm->getTime();
	_ctx->numHandles = 0;

// debugC(DEBUG_BASIC, kTonyDebugMPAL, "PlayScript(): Moments: %u\n",s->nMoments);
	for (_ctx->i = 0; _ctx->i < s->nMoments; _ctx->i++) {
		// Sleep for the required time
		if (s->Moment[_ctx->i].dwTime == -1) {
			CORO_INVOKE_4(CoroScheduler.waitForMultipleObjects, _ctx->numHandles, cfHandles, true, CORO_INFINITE);
			_ctx->dwStartTime = _vm->getTime();
		} else {
			_ctx->dwCurTime = _vm->getTime();
			if (_ctx->dwCurTime < _ctx->dwStartTime + (s->Moment[_ctx->i].dwTime * 100)) {
  //     debugC(DEBUG_BASIC, kTonyDebugMPAL, "PlayScript(): Sleeping %lums\n",_ctx->dwStartTime+(s->Moment[_ctx->i].dwTime*100)-_ctx->dwCurTime);
				CORO_INVOKE_1(CoroScheduler.sleep, _ctx->dwStartTime+(s->Moment[_ctx->i].dwTime * 100) - _ctx->dwCurTime);
			}
		}

		_ctx->numHandles = 0;
		for (_ctx->j = 0; _ctx->j < s->Moment[_ctx->i].nCmds; _ctx->j++) {
			_ctx->k = s->Moment[_ctx->i].CmdNum[_ctx->j];

			if (s->_command[_ctx->k].type == 1) {
				_ctx->p = (LPCFCALL)globalAlloc(GMEM_FIXED, sizeof(CFCALL));
				if (_ctx->p == NULL) {
					GLOBALS._mpalError = 1;

					CORO_KILL_SELF();
					return;
				}

				_ctx->p->_nCf  = s->_command[_ctx->k]._nCf;
				_ctx->p->_arg1 = s->_command[_ctx->k]._arg1;
				_ctx->p->_arg2 = s->_command[_ctx->k]._arg2;
				_ctx->p->_arg3 = s->_command[_ctx->k]._arg3;
				_ctx->p->_arg4 = s->_command[_ctx->k]._arg4;

					 // !!! New process management
				if ((cfHandles[_ctx->numHandles++] = CoroScheduler.createProcess(CustomThread, &_ctx->p, sizeof(LPCFCALL))) == 0) {
					GLOBALS._mpalError = 1;

					CORO_KILL_SELF();
					return;
				}
			} else if (s->_command[_ctx->k].type == 2) {
				lockVar();
				varSetValue(
					s->_command[_ctx->k].lpszVarName,
					evaluateExpression(s->_command[_ctx->k].expr)
				);
				unlockVar();

			} else {
				GLOBALS._mpalError = 1;
				globalFree(s);

				CORO_KILL_SELF();
				return;
			}
		}
	}

	globalFree(s);

	CORO_KILL_SELF();

	CORO_END_CODE;
}


/**
 * Thread that performs an action on an item. the thread always executes the action, 
 * so it should create a new item in which the action is the one required.
 * Furthermore, the expression is not checked, but it is always performed the action.
 *
 * @param param				Pointer to a pointer to a structure containing the action.
 */
void ActionThread(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int j, k;
	CORO_END_CONTEXT(_ctx);

	const LPMPALITEM item = *(const LPMPALITEM *)param;

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._mpalError = 0;
	for (_ctx->j = 0; _ctx->j < item->Action[item->dwRes].nCmds; _ctx->j++) {
		_ctx->k = item->Action[item->dwRes].CmdNum[_ctx->j];

		if (item->_command[_ctx->k].type == 1) {
			// Custom function
			debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d Call=%s params=%d,%d,%d,%d",
				CoroScheduler.getCurrentPID(), GLOBALS._lplpFunctionStrings[item->_command[_ctx->k]._nCf].c_str(),
				item->_command[_ctx->k]._arg1, item->_command[_ctx->k]._arg2,
				item->_command[_ctx->k]._arg3, item->_command[_ctx->k]._arg4
			);

			CORO_INVOKE_4(GLOBALS._lplpFunctions[item->_command[_ctx->k]._nCf],
				item->_command[_ctx->k]._arg1,
				item->_command[_ctx->k]._arg2,
				item->_command[_ctx->k]._arg3,
				item->_command[_ctx->k]._arg4

			);
		} else if (item->_command[_ctx->k].type == 2) {
			// Variable assign
			debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d Variable=%s",
				CoroScheduler.getCurrentPID(), item->_command[_ctx->k].lpszVarName);

			lockVar();
			varSetValue(item->_command[_ctx->k].lpszVarName, evaluateExpression(item->_command[_ctx->k].expr));
			unlockVar();

		} else {
			GLOBALS._mpalError = 1;
			break;
		}
	}

	globalDestroy(item);
	
	debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d ended", CoroScheduler.getCurrentPID());

	CORO_END_CODE;
}

/**
 * This thread monitors a created action to detect when it ends.
 * @remarks		Since actions can spawn sub-actions, this needs to be a
 * separate thread to determine when the outer action is done
 */
void ShutUpActionThread(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int slotNumber;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	GLOBALS._bExecutingAction = false;

	if (_vm->_initialLoadSlotNumber != -1) {
		_ctx->slotNumber = _vm->_initialLoadSlotNumber;
		_vm->_initialLoadSlotNumber = -1;

		CORO_INVOKE_1(_vm->loadState, _ctx->slotNumber);
	}


	CORO_END_CODE;
}


/**
 * Polls one location (starting point of a process)
 *
 * @param param				Pointer to an index in the array of polling locations.
 */
void LocationPollThread(CORO_PARAM, const void *param) {
	typedef struct {
		uint32 nItem, nAction;

		uint16 wTime;
		byte perc;
		HGLOBAL when;
		byte nCmds;
		uint16 CmdNum[MAX_COMMANDS_PER_ACTION];

		uint32 dwLastTime;
	} MYACTION;

	typedef struct {
		uint32 nItem;
		uint32 hThread;
	} MYTHREAD;

	CORO_BEGIN_CONTEXT;
		uint32 *il;
		int i, j, k;
		int numitems;
		int nRealItems;
		LPMPALITEM curItem,newItem;
		int nIdleActions;
		uint32 curTime;
		uint32 dwSleepTime;
		uint32 dwId;
		int ord;
		bool delayExpired;
		bool expired;

		MYACTION *MyActions;
		MYTHREAD *MyThreads;
	CORO_END_CONTEXT(_ctx);

	uint32 id = *((const uint32 *)param);

	CORO_BEGIN_CODE(_ctx);

	/* To begin with, we need to request the item list from the location */
	_ctx->il = mpalQueryItemList(GLOBALS._nPollingLocations[id]);

	/* Count the items */
	for (_ctx->numitems = 0; _ctx->il[_ctx->numitems] != 0; _ctx->numitems++)
		;

	/* We look for items without idle actions, and eliminate them from the list */
	lockItems();
	_ctx->nIdleActions = 0;
	_ctx->nRealItems = 0;
	for (_ctx->i = 0; _ctx->i < _ctx->numitems; _ctx->i++) {
		_ctx->ord = itemGetOrderFromNum(_ctx->il[_ctx->i]);

		if (_ctx->ord == -1) continue;
	 
		_ctx->curItem = GLOBALS._lpmiItems + _ctx->ord;

		_ctx->k = 0;
		for (_ctx->j = 0; _ctx->j < _ctx->curItem->nActions; _ctx->j++)
			if (_ctx->curItem->Action[_ctx->j].num == 0xFF)
				_ctx->k++;

		_ctx->nIdleActions += _ctx->k;

		if (_ctx->k == 0)
			/* We can remove this item from the list */
			_ctx->il[_ctx->i] = (uint32)NULL;
		else
			_ctx->nRealItems++;
	}
	unlockItems();

	/* If there is nothing left, we can exit */
	if (_ctx->nRealItems == 0) {
		globalDestroy(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	_ctx->MyThreads = (MYTHREAD *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _ctx->nRealItems * sizeof(MYTHREAD));
	if (_ctx->MyThreads == NULL) {
		globalDestroy(_ctx->il);
		CORO_KILL_SELF();
		return;
	}


	/* We have established that there is at least one item that contains idle actions.
	   Now we created the mirrored copies of the idle actions. */
	_ctx->MyActions = (MYACTION *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _ctx->nIdleActions * sizeof(MYACTION));
	if (_ctx->MyActions == NULL) {
		globalDestroy(_ctx->MyThreads);
		globalDestroy(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	lockItems();
	_ctx->k = 0;

	for (_ctx->i = 0; _ctx->i < _ctx->numitems; _ctx->i++) {
		if (_ctx->il[_ctx->i] == 0)
			continue;

		_ctx->curItem = GLOBALS._lpmiItems + itemGetOrderFromNum(_ctx->il[_ctx->i]);

		for (_ctx->j = 0; _ctx->j < _ctx->curItem->nActions; _ctx->j++) {
			if (_ctx->curItem->Action[_ctx->j].num == 0xFF) {
				_ctx->MyActions[_ctx->k].nItem = _ctx->il[_ctx->i];
				_ctx->MyActions[_ctx->k].nAction = _ctx->j;

				_ctx->MyActions[_ctx->k].wTime = _ctx->curItem->Action[_ctx->j].wTime;
				_ctx->MyActions[_ctx->k].perc = _ctx->curItem->Action[_ctx->j].perc;
				_ctx->MyActions[_ctx->k].when = _ctx->curItem->Action[_ctx->j].when;
				_ctx->MyActions[_ctx->k].nCmds = _ctx->curItem->Action[_ctx->j].nCmds;
				copyMemory(_ctx->MyActions[_ctx->k].CmdNum, _ctx->curItem->Action[_ctx->j].CmdNum,
				MAX_COMMANDS_PER_ACTION * sizeof(uint16));

				_ctx->MyActions[_ctx->k].dwLastTime = _vm->getTime();
				_ctx->k++;
			}
		}
	}

	unlockItems();

	/* We don't need the item list anymore */
	globalDestroy(_ctx->il);


	/* Here's the main loop */
	while (1) {
		/* Cerchiamo tra tutte le idle actions quella a cui manca meno tempo per
			l'esecuzione */
		_ctx->curTime = _vm->getTime();
		_ctx->dwSleepTime = (uint32)-1L;

		for (_ctx->k = 0;_ctx->k<_ctx->nIdleActions;_ctx->k++)
			if (_ctx->curTime >= _ctx->MyActions[_ctx->k].dwLastTime + _ctx->MyActions[_ctx->k].wTime) {
				_ctx->dwSleepTime = 0;
				break;
		     } else
				_ctx->dwSleepTime = MIN(_ctx->dwSleepTime, _ctx->MyActions[_ctx->k].dwLastTime + _ctx->MyActions[_ctx->k].wTime - _ctx->curTime);

		/* We fall alseep, but always checking that the event is set when prompted for closure */
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, GLOBALS._hEndPollingLocations[id], _ctx->dwSleepTime, &_ctx->expired);

		//if (_ctx->k == WAIT_OBJECT_0)
		if (!_ctx->expired)
			break;

		for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
			if (_ctx->MyThreads[_ctx->i].nItem != 0) {
				CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _ctx->MyThreads[_ctx->i].hThread, 0, &_ctx->delayExpired);

				// if result == WAIT_OBJECT_0)
				if (!_ctx->delayExpired)
					_ctx->MyThreads[_ctx->i].nItem = 0;
			}

		_ctx->curTime = _vm->getTime();

		/* Loop through all the necessary idle actions */
		for (_ctx->k = 0; _ctx->k < _ctx->nIdleActions; _ctx->k++)
			if (_ctx->curTime >= _ctx->MyActions[_ctx->k].dwLastTime + _ctx->MyActions[_ctx->k].wTime) {
				_ctx->MyActions[_ctx->k].dwLastTime += _ctx->MyActions[_ctx->k].wTime;

			   /* It's time to check to see if fortune is on the side of the idle action */
				byte randomVal = (byte)_vm->_randomSource.getRandomNumber(99);
				if (randomVal < _ctx->MyActions[_ctx->k].perc) {
					/* Check if there is an action running on the item */
					if ((GLOBALS._bExecutingAction) && (GLOBALS._nExecutingAction == _ctx->MyActions[_ctx->k].nItem))
						continue;

					/* Check to see if there already another idle funning running on the item */
					for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
						if (_ctx->MyThreads[_ctx->i].nItem == _ctx->MyActions[_ctx->k].nItem)
							break;

					if (_ctx->i < _ctx->nRealItems)
						continue;

					/* Ok, we are the only ones :) */
					lockItems();
					_ctx->curItem = GLOBALS._lpmiItems + itemGetOrderFromNum(_ctx->MyActions[_ctx->k].nItem);

					/* Check if there is a WhenExecute expression */
					_ctx->j=_ctx->MyActions[_ctx->k].nAction;
					if (_ctx->curItem->Action[_ctx->j].when != NULL)
						if (!evaluateExpression(_ctx->curItem->Action[_ctx->j].when)) {
							unlockItems();
							continue;
						}

					/* Ok, we can perform the action. For convenience, we do it in a new process */
					_ctx->newItem = (LPMPALITEM)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALITEM));
					if (_ctx->newItem == false) {
						globalDestroy(_ctx->MyThreads);
						globalDestroy(_ctx->MyActions);
						
						CORO_KILL_SELF();
						return;
					}

					copyMemory(_ctx->newItem,_ctx->curItem, sizeof(MPALITEM));
					unlockItems();

					/* We copy the action in #0 */
//					_ctx->newItem->Action[0].nCmds = _ctx->curItem->Action[_ctx->j].nCmds;
//					copyMemory(_ctx->newItem->Action[0].CmdNum,_ctx->curItem->Action[_ctx->j].CmdNum,_ctx->newItem->Action[0].nCmds*sizeof(_ctx->newItem->Action[0].CmdNum[0]));
					_ctx->newItem->dwRes=_ctx->j;

					/* We will create an action, and will provide the necessary details */
					for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
						if (_ctx->MyThreads[_ctx->i].nItem == 0)
							break;

					_ctx->MyThreads[_ctx->i].nItem = _ctx->MyActions[_ctx->k].nItem;

					// Create the process
					if ((_ctx->MyThreads[_ctx->i].hThread = CoroScheduler.createProcess(ActionThread, &_ctx->newItem, sizeof(LPMPALITEM))) == CORO_INVALID_PID_VALUE) {
					//if ((_ctx->MyThreads[_ctx->i].hThread = (void*)_beginthread(ActionThread, 10240,(void *)_ctx->newItem))= = (void*)-1)
						globalDestroy(_ctx->newItem);
						globalDestroy(_ctx->MyThreads);
						globalDestroy(_ctx->MyActions);
						
						CORO_KILL_SELF();
						return;
					}

					/* Skip all idle actions of the same item */
				}
			}
	}


	// Set idle skip on
	CORO_INVOKE_4(GLOBALS._lplpFunctions[200], 0, 0, 0, 0);

	for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++)
		if (_ctx->MyThreads[_ctx->i].nItem != 0) {
			CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _ctx->MyThreads[_ctx->i].hThread, 5000, &_ctx->delayExpired);

/*
			//if (result != WAIT_OBJECT_0)
			if (_ctx->delayExpired)
				TerminateThread(_ctx->MyThreads[_ctx->i].hThread, 0);
*/
			CoroScheduler.killMatchingProcess(_ctx->MyThreads[_ctx->i].hThread);
		}

	// Set idle skip off
	CORO_INVOKE_4(GLOBALS._lplpFunctions[201], 0, 0, 0, 0);

	/* We're finished */
	globalDestroy(_ctx->MyThreads);
	globalDestroy(_ctx->MyActions);
	
	CORO_KILL_SELF();

	CORO_END_CODE;
}


/**
 * Wait for the end of the dialog execution thread, and then restore global 
 * variables indicating that the dialogue has finished.
 *
 * @param param				Pointer to a handle to the dialog 
 * @remarks		This additional process is used, instead of clearing variables
 * within the same dialog thread, because due to the recursive nature of a dialog,
 * it would be difficult to know within it when the dialog is actually ending.
 */
void ShutUpDialogThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	GLOBALS._bExecutingDialog = false;
	GLOBALS._nExecutingDialog = 0;
	GLOBALS._nExecutingChoice = 0;

	CoroScheduler.setEvent(GLOBALS._hAskChoice);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

void doChoice(CORO_PARAM, uint32 nChoice);


/**
 * Executes a group of the current dialog. Can 'be the Starting point of a process.
 * @parm nGroup				Number of the group to perform
 */
void GroupThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		LPMPALDIALOG dialog;
		int i, j, k;
		int type;
	CORO_END_CONTEXT(_ctx);

	uint32 nGroup = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	// Lock the _ctx->dialog
	lockDialogs();

	// Find the pointer to the current _ctx->dialog
	_ctx->dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	// Search inside the group requesting the _ctx->dialog
	for (_ctx->i = 0; _ctx->dialog->_group[_ctx->i].num != 0; _ctx->i++) {
		if (_ctx->dialog->_group[_ctx->i].num == nGroup) {
			// Cycle through executing the commands of the group
			for (_ctx->j = 0; _ctx->j < _ctx->dialog->_group[_ctx->i].nCmds; _ctx->j++) {
				_ctx->k = _ctx->dialog->_group[_ctx->i].CmdNum[_ctx->j];

				_ctx->type = _ctx->dialog->_command[_ctx->k].type;
				if (_ctx->type == 1) {
					// Call custom function
					CORO_INVOKE_4(GLOBALS._lplpFunctions[_ctx->dialog->_command[_ctx->k]._nCf],
						_ctx->dialog->_command[_ctx->k]._arg1, 
						_ctx->dialog->_command[_ctx->k]._arg2,
						_ctx->dialog->_command[_ctx->k]._arg3, 
						_ctx->dialog->_command[_ctx->k]._arg4
					);

				} else if (_ctx->type == 2) {
					// Set a variable
					lockVar();
					varSetValue(_ctx->dialog->_command[_ctx->k].lpszVarName, evaluateExpression(_ctx->dialog->_command[_ctx->k].expr));
					unlockVar();
					
				} else if (_ctx->type == 3) {
					// DoChoice: call the chosen function
					CORO_INVOKE_1(doChoice, (uint32)_ctx->dialog->_command[_ctx->k].nChoice);
					
				} else {
					GLOBALS._mpalError = 1;
					unlockDialogs();
					
					CORO_KILL_SELF();
					return;
				}
			}

			/* The gruop is finished, so we can return to the calling function.
			 * If the group was the first called, then the process will automatically 
			 * end. Otherwise it returns to the caller method
			 */
			return;
		}
	}

	/* If we are here, it means that we have not found the requested group */
	GLOBALS._mpalError = 1;
	unlockDialogs();
	
	CORO_KILL_SELF();

	CORO_END_CODE;
}


/**
 * Make a choice in the current dialog.
 *
 * @param nChoice			Number of choice to perform
 */
void doChoice(CORO_PARAM, uint32 nChoice) {
	CORO_BEGIN_CONTEXT;
		LPMPALDIALOG dialog;
		int i, j, k;
		uint32 nGroup;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	/* Lock the dialogs */
	lockDialogs();

	/* Get a pointer to the current dialog */
	_ctx->dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	/* Search the choice between those required in the dialog */
	for (_ctx->i = 0; _ctx->dialog->_choice[_ctx->i].nChoice != 0; _ctx->i++)
		if (_ctx->dialog->_choice[_ctx->i].nChoice == nChoice)
			break;

	/* If nothing has been found, exit with an error */
	if (_ctx->dialog->_choice[_ctx->i].nChoice == 0) {
		/* If we're here, we did not find the required choice */
		GLOBALS._mpalError = 1;
		unlockDialogs();

		CORO_KILL_SELF();
		return;
	}

	/* We've found the requested choice. Remember what in global variables */
	GLOBALS._nExecutingChoice = _ctx->i;

	while (1) {
		GLOBALS._nExecutingChoice = _ctx->i;

		_ctx->k = 0;
		/* Calculate the expression of each selection, to see if they're active or inactive */
		for (_ctx->j = 0; _ctx->dialog->_choice[_ctx->i]._select[_ctx->j].dwData != 0; _ctx->j++)
			if (_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].when == NULL) {
				_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].curActive = 1;
				_ctx->k++;
			} else if (evaluateExpression(_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].when)) {
				_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].curActive = 1;
				_ctx->k++;
			} else
				_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].curActive = 0;

		/* If there are no choices activated, then the dialog is finished. */
		if (_ctx->k == 0) {
			unlockDialogs();
			break;
		}

		/* There are choices available to the user, so wait for them to make one */
		CoroScheduler.resetEvent(GLOBALS._hDoneChoice);
		CoroScheduler.setEvent(GLOBALS._hAskChoice);
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._hDoneChoice, CORO_INFINITE);

		/* Now that the choice has been made, we can run the groups associated with the choice tbontbtitq
		*/
		_ctx->j = GLOBALS._nSelectedChoice;
		for (_ctx->k = 0; _ctx->dialog->_choice[_ctx->i]._select[_ctx->j].wPlayGroup[_ctx->k] != 0; _ctx->k++) {
			_ctx->nGroup = _ctx->dialog->_choice[_ctx->i]._select[_ctx->j].wPlayGroup[_ctx->k];
			CORO_INVOKE_1(GroupThread, &_ctx->nGroup);
		}

		/* Control attribute */
		if (_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].attr & (1 << 0)) {
			/* Bit 0 set: the end of the choice */
			unlockDialogs();
			break;
		}

		if (_ctx->dialog->_choice[_ctx->i]._select[_ctx->j].attr & (1 << 1)) {
			/* Bit 1 set: the end of the dialog */
			unlockDialogs();
			
			CORO_KILL_SELF();
			return;
		}

		/* End of choic ewithout attributes. We must do it again */
	}

	// If we're here, we found an end choice. Return to the caller group
	return;

	CORO_END_CODE;
}


/**
 * Perform an action on a certain item.
 *
 * @param nAction			Action number
 * @param ordItem           Index of the item in the items list
 * @param dwParam			Any parameter for the action.
 * @returns		Id of the process that was launched to perform the action, or
 * CORO_INVALID_PID_VALUE if the action was not defined, or the item was inactive.
 * @remarks		You can get the index of an item from its number by using
 * the itemGetOrderFromNum() function. The items list must first be locked
 * by calling LockItem().
 */
static uint32 doAction(uint32 nAction, uint32 ordItem, uint32 dwParam) {
	LPMPALITEM item = GLOBALS._lpmiItems;
	int i;
	LPMPALITEM newitem;
	uint32 h;

	item+=ordItem;
	Common::String buf = Common::String::format("Status.%u", item->nObj);
	if (varGetValue(buf.c_str()) <= 0)
		return CORO_INVALID_PID_VALUE;

	for (i = 0; i < item->nActions; i++) {
		if (item->Action[i].num != nAction)
			continue;

		if (item->Action[i].wParm != dwParam)
			continue;

		if (item->Action[i].when != NULL) {
			if (!evaluateExpression(item->Action[i].when))
				continue;
		}

		// Now we find the right action to be performed
		// Duplicate the item and copy the current action in #i into #0
		newitem = (LPMPALITEM)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALITEM));
		if (newitem == NULL)
			return CORO_INVALID_PID_VALUE;

		// In the new version number of the action in writing dwRes
		Common::copy((byte *)item, (byte *)item + sizeof(MPALITEM), (byte *)newitem);
/*   newitem->Action[0].nCmds=item->Action[i].nCmds;
   copyMemory(newitem->Action[0].CmdNum,item->Action[i].CmdNum,newitem->Action[0].nCmds*sizeof(newitem->Action[0].CmdNum[0]));
*/
		newitem->dwRes = i;

		// And finally we can laucnh the process that will execute the action,
		// and a second process to free up the memory when the action is finished.

		// !!! New process management
		if ((h = CoroScheduler.createProcess(ActionThread, &newitem, sizeof(LPMPALITEM))) == CORO_INVALID_PID_VALUE)
			return CORO_INVALID_PID_VALUE;

		if (CoroScheduler.createProcess(ShutUpActionThread, &h, sizeof(uint32)) == CORO_INVALID_PID_VALUE)
			return CORO_INVALID_PID_VALUE;

		GLOBALS._nExecutingAction = item->nObj;
		GLOBALS._bExecutingAction = true;

		return h;
	}

	return CORO_INVALID_PID_VALUE;
}

/**
 * Shows a dialog in a separate process.
 *
 * @param nDlgOrd				The index of the dialog in the dialog list
 * @param nGroup				Number of the group to perform
 * @returns						The process Id of the process running the dialog
 *								or CORO_INVALID_PID_VALUE on error
 * @remarks						The dialogue runs in a thread created on purpose, 
 * so that must inform through an event and when 'necessary to you make a choice. 
 * The data on the choices may be obtained through various queries.
 */
static uint32 doDialog(uint32 nDlgOrd, uint32 nGroup) {
	uint32 h;

	// Store the running dialog in a global variable
	GLOBALS._nExecutingDialog = nDlgOrd;

	// Enables the flag to indicate that there is' a running dialogue
	GLOBALS._bExecutingDialog = true;

	CoroScheduler.resetEvent(GLOBALS._hAskChoice);
	CoroScheduler.resetEvent(GLOBALS._hDoneChoice);

	// Create a thread that performs the dialogue group

	// Create the process
	if ((h = CoroScheduler.createProcess(GroupThread, &nGroup, sizeof(uint32))) == CORO_INVALID_PID_VALUE)
		return CORO_INVALID_PID_VALUE;

	// Create a thread that waits until the end of the dialog process, and will restore the global variables
	if (CoroScheduler.createProcess(ShutUpDialogThread, &h, sizeof(uint32)) == CORO_INVALID_PID_VALUE) {
		// Something went wrong, so kill the previously started dialog process
		CoroScheduler.killMatchingProcess(h);
		return CORO_INVALID_PID_VALUE;
	}

	return h;
}


/**
 * Takes note of the selection chosen by the user, and warns the process that was running 
 * the box that it can continue.
 *
 * @param nChoice           Number of choice that was in progress
 * @param dwData			Since combined with select selection
 * @returns		True if everything is OK, false on failure
 */
bool doSelection(uint32 i, uint32 dwData) {
	LPMPALDIALOG dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;
	int j;

	for (j = 0; dialog->_choice[i]._select[j].dwData != 0; j++)
		if (dialog->_choice[i]._select[j].dwData == dwData && dialog->_choice[i]._select[j].curActive != 0)
			break;

	if (dialog->_choice[i]._select[j].dwData == 0)
		return false;

	GLOBALS._nSelectedChoice = j;
	CoroScheduler.setEvent(GLOBALS._hDoneChoice);
	return true;
}


/**
 * @defgroup Exported functions
 */

/**
 * Initialises the MPAL library and opens the .MPC file, which will be used for all queries.
 *
 * @param lpszMpcFileName   Name of the MPC file
 * @param lpszMprFileName   Name of the MPR file
 * @param lplpcfArray		Array of pointers to custom functions.
 * @returns		True if everything is OK, false on failure
 */
bool mpalInit(const char *lpszMpcFileName, const char *lpszMprFileName, 
			  LPLPCUSTOMFUNCTION lplpcfArray, Common::String *lpcfStrings) {
	Common::File hMpc;
	byte buf[5];
	uint32 nBytesRead;
	bool bCompress;
	uint32 dwSizeDecomp, dwSizeComp;
	byte *cmpbuf;

 //printf("Item: %lu\n", sizeof(MPALITEM));
 //printf("Script: %lu\n", sizeof(MPALSCRIPT));
 //printf("Dialog: %lu\n", sizeof(MPALDIALOG));

	/* Save the array of custom functions */
	GLOBALS._lplpFunctions = lplpcfArray;
	GLOBALS._lplpFunctionStrings = lpcfStrings;

	/* OPen the MPC file for reading */
	if (!hMpc.open(lpszMpcFileName))
		return false;

	/* Read and check the header */
	nBytesRead = hMpc.read(buf, 5);
	if (nBytesRead != 5)
		return false;

	if (buf[0] != 'M' || buf[1] != 'P' || buf[2] != 'C' || buf[3] != 0x20)
		return false;

	bCompress = buf[4];

	/* Reads the size of the uncompressed file, and allocate memory */
	dwSizeDecomp = hMpc.readUint32LE();
	if (hMpc.err())
		return false;

	byte *lpMpcImage = (byte *)globalAlloc(GMEM_FIXED, dwSizeDecomp + 16);
	if (lpMpcImage == NULL)
		return false;

	if (bCompress) {
		/* Get the compressed size and read the data in */
		dwSizeComp = hMpc.readUint32LE();
		if (hMpc.err())
			return false;

		cmpbuf = (byte *)globalAlloc(GMEM_FIXED, dwSizeComp);
		if (cmpbuf == NULL)
			return false;

		nBytesRead = hMpc.read(cmpbuf, dwSizeComp);
		if (nBytesRead != dwSizeComp)
			return false;

		/* Decompress the data */
		lzo1x_decompress(cmpbuf, dwSizeComp, lpMpcImage, &nBytesRead);
		if (nBytesRead != dwSizeDecomp)
			return false;

		globalDestroy(cmpbuf);
	} else {
		/* If the file is not compressed, we directly read in the data */
		nBytesRead = hMpc.read(lpMpcImage, dwSizeDecomp);
		if (nBytesRead != dwSizeDecomp)
			return false;
	}

	/* Close the file */
	hMpc.close();

	/* Process the data */
	if (ParseMpc(lpMpcImage) == false)
		return false;

	globalDestroy(lpMpcImage);

	/* Calculate memory usage */
	/*
 {
	 char errbuf[256];
	 wsprintf(errbuf,"Utilizzo in RAM: VAR %lu, MSG %lu, DLG %lu, ITM %lu, LOC %lu, SCR %lu",
	   GLOBALS.nVars*sizeof(MPALVAR),
		 GLOBALS.nMsgs*sizeof(MPALMSG),
		 GLOBALS.nDialogs*sizeof(MPALDIALOG),
		 GLOBALS.nItems*sizeof(MPALITEM),
		 GLOBALS.nLocations*sizeof(MPALLOCATION),
		 GLOBALS.nScripts*sizeof(MPALSCRIPT));
	 MessageBox(NULL,errbuf,"Dump",MB_OK);
 }
*/

	/* Open the MPR file */
	if (!GLOBALS._hMpr.open(lpszMprFileName))
		return false;

	/* Seek to the end of the file to read overall information */
	GLOBALS._hMpr.seek(-12, SEEK_END);

	dwSizeComp = GLOBALS._hMpr.readUint32LE();
	if (GLOBALS._hMpr.err())
		return false;

	GLOBALS._nResources = GLOBALS._hMpr.readUint32LE();
	if (GLOBALS._hMpr.err())
		return false;

	nBytesRead = GLOBALS._hMpr.read(buf, 4);
	if (GLOBALS._hMpr.err())
		return false;

	if (buf[0] !='E' || buf[1] != 'N' || buf[2] != 'D' || buf[3] != '0')
		return false;

	/* Move to the start of the resources header */
	GLOBALS._hMpr.seek(-(12 + (int)dwSizeComp), SEEK_END);

	GLOBALS._lpResources = (uint32 *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, GLOBALS._nResources * 8);
	if (GLOBALS._lpResources == NULL)
		return false;

	cmpbuf = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSizeComp);
	if (cmpbuf == NULL)
		return false;

	nBytesRead = GLOBALS._hMpr.read(cmpbuf, dwSizeComp);
	if (nBytesRead != dwSizeComp)
		return false;

	lzo1x_decompress((const byte *)cmpbuf, dwSizeComp, (byte *)GLOBALS._lpResources, (uint32 *)&nBytesRead);
	if (nBytesRead != (uint32)GLOBALS._nResources * 8)
		return false;

	globalDestroy(cmpbuf);

	/* Reset back to the start of the file, leaving it open */
	GLOBALS._hMpr.seek(0, SEEK_SET);

	/* There is no action or dialog running by default */
	GLOBALS._bExecutingAction = false;
	GLOBALS._bExecutingDialog = false;

	/* There's no polling location */
	Common::fill(GLOBALS._nPollingLocations, GLOBALS._nPollingLocations + MAXPOLLINGLOCATIONS, 0);

	/* Create the event that will be used to co-ordinate making choices and choices finishing */
	GLOBALS._hAskChoice = CoroScheduler.createEvent(true, false);
	GLOBALS._hDoneChoice = CoroScheduler.createEvent(true, false);

	return true;
}

/**
 * Frees resources allocated by the MPAL subsystem
 */
void mpalFree() {
	// Free the resource list
	globalDestroy(GLOBALS._lpResources);
}

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that returns numeric results.
 */
uint32 mpalQueryDWORD(uint16 wQueryType, ...) {
	int x, y;
	Common::String buf;
	uint32 dwRet = 0;
	char *n;

	va_list v;
	va_start(v, wQueryType);

	GLOBALS._mpalError = OK;

	if (wQueryType == MPQ_VERSION) {

		/*
		 *  uint32 mpalQuery(MPQ_VERSION);
		 */
		dwRet = HEX_VERSION;

	} else if (wQueryType == MPQ_GLOBAL_VAR) {
		/*
		 *  uint32 mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName);
		 */
		lockVar();
		dwRet = (uint32)varGetValue(GETARG(char *));
		unlockVar();

	} else if (wQueryType == MPQ_MESSAGE) {
		/*
		 *  char * mpalQuery(MPQ_MESSAGE, uint32 nMsg);
		 */
		error("mpalQuery(MPQ_MESSAGE, uint32 nMsg) used incorrect method variant");

		
	} else if (wQueryType == MPQ_ITEM_PATTERN) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem);
		 */
		lockVar();
		buf = Common::String::format("Pattern.%u", GETARG(uint32));
		dwRet = (uint32)varGetValue(buf.c_str());
		unlockVar();
		
	} else if (wQueryType == MPQ_LOCATION_SIZE) {
		/*
		 *  uint32 mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord);
		 */
		lockLocations();
		x = locGetOrderFromNum(GETARG(uint32));
		y = GETARG(uint32);
		if (x != -1) {
			if (y == MPQ_X)
				dwRet = GLOBALS._lpmlLocations[x].dwXlen;
			else if (y == MPQ_Y)
				dwRet = GLOBALS._lpmlLocations[x].dwYlen;
			else
				GLOBALS._mpalError = 1;
		} else
			GLOBALS._mpalError = 1;

		unlockLocations();
		
	} else if (wQueryType == MPQ_LOCATION_IMAGE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc);
		 */
		error("mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc) used incorrect variant");

	} else if (wQueryType == MPQ_RESOURCE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_RESOURCE, uint32 dwRes);
		 */
		error("mpalQuery(MPQ_RESOURCE, uint32 dwRes) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_LIST) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_LIST, uint32 nLoc);
		 */
		error("mpalQuery(MPQ_ITEM_LIST, uint32 nLoc) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_DATA) {
		/*
		 *  LPITEM mpalQuery(MPQ_ITEM_DATA, uint32 nItem);
		 */
		error("mpalQuery(MPQ_ITEM_DATA, uint32 nItem) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_IS_ACTIVE) {
		/*
		 *  bool mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem);
		 */
		lockVar();
		x = GETARG(uint32);
		buf = Common::String::format("Status.%u", x);
		if (varGetValue(buf.c_str()) <= 0)
			dwRet = (uint32)false;
		else
			dwRet = (uint32)true;

		unlockVar();

	} else if (wQueryType == MPQ_ITEM_NAME) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_NAME, uint32 nItem, char * lpszName);
		 */
		lockVar();
		x = GETARG(uint32);
		n = GETARG(char *);
		buf = Common::String::format("Status.%u", x);
		if (varGetValue(buf.c_str()) <= 0)
			n[0]='\0';
		else {
			lockItems();
			y = itemGetOrderFromNum(x);
			copyMemory(n, (char *)(GLOBALS._lpmiItems + y)->lpszDescribe, MAX_DESCRIBE_SIZE);
			unlockItems();
		}

		unlockVar();

	} else if (wQueryType == MPQ_DIALOG_PERIOD) {
		/*
		 *  char *mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod);
		 */
		error("mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		error("mpalQuery(MPQ_DIALOG_WAITFORCHOICE) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_SELECTLIST) {
		/*
		 *  uint32 *mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice);
		 */
		error("mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_SELECTION) {
		/*
		 *  bool mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData);
		 */
		lockDialogs();
		x = GETARG(uint32);
		y = GETARG(uint32);
		dwRet = (uint32)doSelection(x, y);

		unlockDialogs();

	} else if (wQueryType == MPQ_DO_ACTION) {
		/*
		 *  int mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam);
		 */
		lockItems();
		lockVar();
		x = GETARG(uint32);
		int z = GETARG(uint32);
		y = itemGetOrderFromNum(z);
		if (y != -1) {
			dwRet = doAction(x, y, GETARG(uint32));
		} else {
			dwRet = CORO_INVALID_PID_VALUE;
			GLOBALS._mpalError = 1;
		}

		unlockVar();
		unlockItems();

	} else if (wQueryType == MPQ_DO_DIALOG) {
		/*
		 *  int mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup);
		 */
		if (!GLOBALS._bExecutingDialog) {
			lockDialogs();

			x = dialogGetOrderFromNum(GETARG(uint32));
			y = GETARG(uint32);
			dwRet = doDialog(x, y);
			unlockDialogs();
		}
	} else {
		/*
		 *  DEFAULT -> ERROR
		 */
		GLOBALS._mpalError = 1;
	}

	va_end(v);
	return dwRet;
}

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that returns a pointer or handle.
 */
HANDLE mpalQueryHANDLE(uint16 wQueryType, ...) {
	int x, y;
	char *n;
	Common::String buf;
	va_list v;
	va_start(v, wQueryType);
	void *hRet = NULL;

	GLOBALS._mpalError = OK;

	if (wQueryType == MPQ_VERSION) {
		/*
		 *  uint32 mpalQuery(MPQ_VERSION);
		 */
		error("mpalQuery(MPQ_VERSION) used incorrect variant");

	} else if (wQueryType == MPQ_GLOBAL_VAR) {
		/*
		 *  uint32 mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName);
		 */
		error("mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName) used incorrect variant");

	} else if (wQueryType == MPQ_MESSAGE) {
		/*
		 *  char * mpalQuery(MPQ_MESSAGE, uint32 nMsg);
		 */
		LockMsg();
		hRet = DuplicateMessage(msgGetOrderFromNum(GETARG(uint32)));
		UnlockMsg();
		
	} else if (wQueryType == MPQ_ITEM_PATTERN) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem);
		 */
		error("mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem) used incorrect variant");
		
	} else if (wQueryType == MPQ_LOCATION_SIZE) {
		/*
		 *  uint32 mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord);
		 */
		error("mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord) used incorrect variant");
		
	} else if (wQueryType == MPQ_LOCATION_IMAGE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc);
		 */
		lockLocations();
		x = locGetOrderFromNum(GETARG(uint32));
		hRet = resLoad(GLOBALS._lpmlLocations[x].dwPicRes);
		unlockLocations();

	} else if (wQueryType == MPQ_RESOURCE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_RESOURCE, uint32 dwRes);
		 */
		hRet = resLoad(GETARG(uint32));

	} else if (wQueryType == MPQ_ITEM_LIST) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_LIST, uint32 nLoc);
		 */
		lockVar();
		hRet = GetItemList(GETARG(uint32));
		lockVar();

	} else if (wQueryType == MPQ_ITEM_DATA) {
		/*
		 *  LPITEM mpalQuery(MPQ_ITEM_DATA, uint32 nItem);
		 */
		lockItems();
		hRet = getItemData(itemGetOrderFromNum(GETARG(uint32)));
		unlockItems();

	} else if (wQueryType == MPQ_ITEM_IS_ACTIVE) {
		/*
		 *  bool mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem);
		 */
		error("mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_NAME) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_NAME, uint32 nItem, char *lpszName);
		 */
		lockVar();
		x = GETARG(uint32);
		n = GETARG(char *);
		buf = Common::String::format("Status.%u", x);
		if (varGetValue(buf.c_str()) <= 0)
			n[0] = '\0';
		else {
			lockItems();
			y = itemGetOrderFromNum(x);
			copyMemory(n, (char *)(GLOBALS._lpmiItems + y)->lpszDescribe, MAX_DESCRIBE_SIZE);
			unlockItems();
		}

		unlockVar();

	} else if (wQueryType == MPQ_DIALOG_PERIOD) {
		/*
		 *  char * mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod);
		 */
		lockDialogs();
		y = GETARG(uint32);
		hRet = duplicateDialogPeriod(y);
		unlockDialogs();

	} else if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		error("mpalQuery(MPQ_DIALOG_WAITFORCHOICE) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_SELECTLIST) {
		/*
		 *  uint32 *mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice);
		 */
		lockDialogs();
		hRet = getSelectList(GETARG(uint32));
		unlockDialogs();

	} else if (wQueryType == MPQ_DIALOG_SELECTION) {
		/*
		 *  bool mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData);
		 */
		error("mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData) used incorrect variant");

	} else if (wQueryType == MPQ_DO_ACTION) {
		/*
		 *  int mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam);
		 */
		error("mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam) used incorrect variant");

	} else if (wQueryType == MPQ_DO_DIALOG) {
		/*
		 *  int mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup);
		 */
		error("mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup) used incorrect variant");
	} else {
		/*
		 *  DEFAULT -> ERROR
		 */
		GLOBALS._mpalError = 1;
	}

	va_end(v);
	return hRet;
}


/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that needs to run within a co-routine context.
 */
void mpalQueryCORO(CORO_PARAM, uint16 wQueryType, uint32 *dwRet, ...) {
	CORO_BEGIN_CONTEXT;
		uint32 dwRet;
	CORO_END_CONTEXT(_ctx);

	va_list v;
	va_start(v, dwRet);

	CORO_BEGIN_CODE(_ctx);

	if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._hAskChoice, CORO_INFINITE);

		CoroScheduler.resetEvent(GLOBALS._hAskChoice);

		if (GLOBALS._bExecutingDialog)
			*dwRet = (uint32)GLOBALS._nExecutingChoice;
		else
			*dwRet = (uint32)((int)-1);
	} else {
		error("mpalQueryCORO called with unsupported query type");
	}

	CORO_END_CODE;

	va_end(v);
}


/**
 * Returns the current MPAL error code
 *
 * @returns		Error code
 */
uint32 mpalGetError(void) {
	return GLOBALS._mpalError;
}


/**
 * Execute a script. The script runs on multitasking by a thread.
 *
 * @param nScript			Script number to run
 * @returns		TRUE if the script 'was launched, FALSE on failure
 */
bool mpalExecuteScript(int nScript) {
	int n;
	LPMPALSCRIPT s;

	LockScripts();
	n = scriptGetOrderFromNum(nScript);
	s = (LPMPALSCRIPT)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALSCRIPT));
	if (s == NULL)
		return false;

	copyMemory(s, GLOBALS._lpmsScripts + n, sizeof(MPALSCRIPT));
	unlockScripts();

	// !!! New process management
	if (CoroScheduler.createProcess(ScriptThread, &s, sizeof(LPMPALSCRIPT)) == CORO_INVALID_PID_VALUE)
 		return false;

	return true;
}


/**
 * Install a custom routine That will be called by MPAL every time the pattern 
 * of an item has been changed.
 *
 * @param lpiifCustom		Custom function to install
 */
void mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCus) {
	GLOBALS._lpiifCustom = lpiifCus;
}


/**
 * Process the idle actions of the items on one location.
 *
 * @param nLoc				Number of the location whose items must be processed 
 * for idle actions.
 * @returns		TRUE if all OK, and FALSE if it exceeded the maximum limit.
 * @remarks		The maximum number of locations that can be polled
 * simultaneously is defined defined by MAXPOLLINGFUNCIONS
 */
bool mpalStartIdlePoll(int nLoc) {
	uint32 i;

	for (i = 0; i < MAXPOLLINGLOCATIONS; i++)
		if (GLOBALS._nPollingLocations[i] == (uint32)nLoc)
			return false;

	for (i = 0; i < MAXPOLLINGLOCATIONS; i++) {
		if (GLOBALS._nPollingLocations[i] == 0) {
			GLOBALS._nPollingLocations[i] = nLoc;

			GLOBALS._hEndPollingLocations[i] = CoroScheduler.createEvent(true, false);
// !!! New process management
			if ((GLOBALS._pollingThreads[i] = CoroScheduler.createProcess(LocationPollThread, &i, sizeof(uint32))) == CORO_INVALID_PID_VALUE)
//			 if ((GLOBALS.hEndPollingLocations[i] = (void*)_beginthread(LocationPollThread, 10240,(void *)i))= = (void*)-1)
				return false;

			return true;
		}
	}

	return false;
}


/**
 * Stop processing the idle actions of the items on one location.
 *
 * @param nLo				Number of the location
 * @returns		TRUE if all OK, FALSE if the specified location was not
 * in the process of polling
 */
void mpalEndIdlePoll(CORO_PARAM, int nLoc, bool *result) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	for (_ctx->i = 0; _ctx->i < MAXPOLLINGLOCATIONS; _ctx->i++) {
		if (GLOBALS._nPollingLocations[_ctx->i] == (uint32)nLoc) {
			CoroScheduler.setEvent(GLOBALS._hEndPollingLocations[_ctx->i]);

			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._pollingThreads[_ctx->i], CORO_INFINITE);

			CoroScheduler.closeEvent(GLOBALS._hEndPollingLocations[_ctx->i]);
			GLOBALS._nPollingLocations[_ctx->i] = 0;

			if (result)
				*result = true;
			return;
		}
	}

	if (result)
		*result = false;

	CORO_END_CODE;
}


/**
 * Retrieve the length of a save state
 *
 * @returns		Length in bytes
 */
int mpalGetSaveStateSize(void) {
	return GLOBALS._nVars * sizeof(MPALVAR) + 4;
}


/**
 * Store the save state into a buffer. The buffer must be
 * length at least the size specified with mpalGetSaveStateSize
 *
 * @param buf				Buffer where to store the state
 */
void mpalSaveState(byte *buf) {
	lockVar();
	WRITE_LE_UINT32(buf, GLOBALS._nVars);
	copyMemory(buf + 4, (byte *)GLOBALS._lpmvVars, GLOBALS._nVars * sizeof(MPALVAR));
	unlockVar();	
}


/**
 * Load a save state from a buffer.
 *
 * @param buf				Buffer where to store the state
 * @returns		Length of the state buffer in bytes
 */
int mpalLoadState(byte *buf) {
	// We must destroy and recreate all the variables
	globalFree(GLOBALS._hVars);

	GLOBALS._nVars = READ_LE_UINT32(buf);
	
	GLOBALS._hVars = globalAllocate(GMEM_ZEROINIT | GMEM_MOVEABLE, GLOBALS._nVars * sizeof(MPALVAR));
	lockVar();
	copyMemory((byte *)GLOBALS._lpmvVars, buf + 4, GLOBALS._nVars * sizeof(MPALVAR));
	unlockVar();

	return GLOBALS._nVars * sizeof(MPALVAR) + 4;
}

bool bDontOutput;

struct MsgCommentsStruct {
	uint16 wStart;
	uint16 wEnd;
	const char *pComment;	
};
const MsgCommentsStruct MsgComments[] = {
	{ 10, 16, "###" },
	{ 560, 563, "@@@ BUTCH & DUDLEY:" },
	{ 551, 553, "@@@ JACK'S LETTER (JACK'S VOICE):" },
	{ 679, 679, "@@@ OFF-SCREEN VOICE:" },
	{ 799, 799, "###" },
	{ 830, 838, "RE-HASHING (FROM MACBETH):" },
	{ 890, 894, "@@@ BEARDED LADY FROM WITHIN HER ROOM:" },
	{ 1175, 1175, "###" },
	{ 1210, 1210, "###" },
	{ 1347, 1349, "###" },
	{ 1175, 1175, "###" },
	{ 1342, 1343, "###" },
	{ 1742, 1742, "@@@ OFF-SCREEN VOICE:" },
	{ 1749, 1749, "###" },
	{ 1759, 1759, "###" },
	{ 2165, 2166, "@@@ MORTIMER:" },
	{ 2370, 2372, "@@@ ELECTRONIC VOICE FROM AN AUTOMATIC PICTURE MACHINE:" },
	{ 2580, 2589, "@@@ BIFF:" },
	{ 2590, 2593, "@@@ BARTENDER:" },
	{ 2596, 2596, "@@@ SAD PIRATE:" },
	{ 2760, 2767, "@@@ EGGHEAD:" },
	{ 2959, 2959, "@@@ MONSTROUS VOICE FROM BEHIND A LOCKED DOOR:" },
	{ 3352, 3352, "@@@ POLLY:" },
	{ 3378, 3379, "@@@ POLLY:" },
	{ 3461, 3469, "@@@ RANDALL:" },
	{ 3571, 3574, "@@@ CAPTAIN'S JOURNAL (CAPTAIN'S VOICE):" },
	{ 3646, 3646, "NOTE: THIS SENTENCE ENDS THE STORY TOLD IN SENTENCES 03640 - 03643:" },
	{ 3647, 3648, "TONY SPEAKS TRYING TO IMITATE CAPTAIN CORNELIUS' VOICE:" },
	{ 3670, 3671, "###" },
	{ 3652, 3652, "###" },
	{ 3656, 3657, "@@@ GATEKEEPER:" },
	{ 3658, 3659, "@@@ GATEKEEPER (FAR AWAY):" },
	{ 3790, 3795, "@@@ GATEKEEPER:" },
	{ 3798, 3799, "@@@ OFF-SCREEN VOICE:" },
	{ 4384, 4384, "###" },
	{ 4394, 4395, "###" },
	{ 4780, 4780, "###" },
	{ 5089, 5089, "TONY PLAYING SOMEONE ELSE, WITH A DEEPER TONE:" },
	{ 5090, 5090, "NORMAL TONY:" },
	{ 5091, 5091, "TONY PLAYING SOMEONE ELSE, WITH A DEEPER TONE:" },
	{ 5262, 5262, "@@@ OFF-SCREEN VOICE" },
	{ 5276, 5277, "###" },
	{ 5326, 5326, "###" },
	{ 5472, 5472, "LYRICS FROM THE SONG \"I AM ONE\", BY SMASHING PUMPKINS:" },
	{ 5488, 5488, "LYRICS FROM THE SONG \"I AM ONE\", BY SMASHING PUMPKINS:" },
	{ 5652, 5653, "###" },
//bernie	{ 11000, 15000, "###" },
	{ 11000, 11111, "###" },


	{ 0, 0, NULL }
};

void outputStartMsgComment(uint16 wNum, Common::OutSaveFile *f) {
	int i;

	for (i = 0; MsgComments[i].wStart != 0; i++)
		if (MsgComments[i].wStart == wNum) {
			debugC(DEBUG_BASIC, kTonyDebugMPAL, "Start: %d\n", wNum);

			f->writeString("</TABLE>\n<P>\n<P>\n");

			if (strcmp(MsgComments[i].pComment, "###") != 0 && strncmp(MsgComments[i].pComment, "@@@", 3) != 0) {
				f->writeString(Common::String::format("%s\n", MsgComments[i].pComment));
				f->writeString("<P>\n<P>\n<TABLE WIDTH = 100%% BORDER = 1>\n");
			} else
				bDontOutput = true;
			return;
		}
}

void OutputEndMsgComment(uint16 wNum, Common::OutSaveFile *f) {
	int i;

	for (i = 0; MsgComments[i].wEnd != 0; i++)
		if (MsgComments[i].wEnd == wNum) {
			debugC(DEBUG_BASIC, kTonyDebugMPAL, "End: %d\n", wNum);

			if (strcmp(MsgComments[i].pComment, "###") != 0 && strncmp(MsgComments[i].pComment, "@@@", 3) != 0) {
				f->writeString("</TABLE>\n<P>\n");
			} else
				bDontOutput = false;
	
			f->writeString("<P>\n<P>\n<TABLE WIDTH = 100%% BORDER = 1>\n");
			return;
		}
}


int OutputStartOther(uint16 wNum, Common::OutSaveFile *f) {
	int i;
	
	for (i = 0; MsgComments[i].wStart != 0; i++)
		if (MsgComments[i].wStart <= wNum && MsgComments[i].wEnd >= wNum) {
			if (strncmp(MsgComments[i].pComment, "@@@", 3) == 0) {
				if (MsgComments[i].wStart == wNum) {
					f->writeString(Common::String::format("%s\n", MsgComments[i].pComment + 4));
					f->writeString("<P>\n<P>\n<TABLE WIDTH = 100%% BORDER = 1>\n");
				}
				
				return 1;				
			}
		}

	return 0;
}


void outputEndOther(uint16 wNum, Common::OutSaveFile *f) {
	int i;

	for (i = 0; MsgComments[i].wStart != 0; i++)
		if (MsgComments[i].wEnd == wNum && strncmp(MsgComments[i].pComment, "@@@", 3) == 0) {
			f->writeString("</TABLE>\n<P>\n");	
			break;
		}
}


void mpalDumpMessages(void) {
	int i, j;
	char *lpMessage;
	char *p;
	char *lpPeriods[30];
	char fname[64];
	char frase[2048];
	int nPeriods;
	Common::OutSaveFile *f, *v1;

	v1 = g_system->getSavefileManager()->openForSaving("voicelist.txt");

	LockMsg();

	bDontOutput = false;

	debugC(DEBUG_BASIC, kTonyDebugMPAL, "Dumping MESSAGES.HTM...\n");
	
	f = g_system->getSavefileManager()->openForSaving("Messages.htm");
	f->writeString("<HTML>\n<BODY>\n<TABLE WIDTH = 100%% BORDER = 1>\n");

	for (i = 0; i < GLOBALS._nMsgs; i++) {
		lpMessage = (char *)globalLock(GLOBALS._lpmmMsgs[i]._hText);
		if (*lpMessage != '\0') {
			// bernie: debug
			/*if (GLOBALS.lpmmMsgs[i].wNum == 1950) {
				int a = 1;
			}*/

			nPeriods = 1;
			p = lpPeriods[0] = lpMessage;

			outputStartMsgComment(GLOBALS._lpmmMsgs[i]._wNum, f);

			while (1) {
				// Find the end of the current period
				while (*p != '\0')
					p++;

				// If there is another '\0' at the end of the message, then finish
				p++;
				if (*p == '\0')
					break;

				// Otherwise there is another line, so remember the next one's start
				lpPeriods[nPeriods++] = p;
			}

			// Now make a loop over all the periods
			for (j = 0; j < nPeriods; j++) {
				if (nPeriods == 1)
					sprintf(fname, "000-%05d.WAV", GLOBALS._lpmmMsgs[i]._wNum);
				else
					sprintf(fname, "000-%05d-%02d.WAV", GLOBALS._lpmmMsgs[i]._wNum,j);
			
				strcpy(frase, lpPeriods[j]);

				while ((p = strchr(frase,'^')) != NULL)
					*p = '\"';

				p = frase;
				while (*p == ' ')
					p++;
				if (*p == '\0')
					continue;

				if (!bDontOutput) {
					v1->writeString(Common::String::format("%s\n", fname));
					f->writeString("\t<TR>\n");
					f->writeString(Common::String::format("\t\t<TD WIDTH=20%%> %s </B></TD>\n", fname));
					f->writeString(Common::String::format("\t\t<TD> %s </TD>\n", frase));
					f->writeString("\t</TR>\n");
				}
			}

			OutputEndMsgComment(GLOBALS._lpmmMsgs[i]._wNum, f);

			globalUnlock(GLOBALS._lpmmMsgs[i]._hText);
		}
	}

	f->writeString("</TABLE>\n</BODY>\n</HTML>\n");

	f->finalize();
	v1->finalize();
	delete f;
	delete v1;

	UnlockMsg();
}



void mpalDumpOthers(void) {
	int i,j;
	char *lpMessage;
	char *p;
	char *lpPeriods[30];
	char fname[64];
	char frase[2048];
	int nPeriods;

	Common::OutSaveFile *f, *v1;

	v1 = g_system->getSavefileManager()->openForSaving("voicelist.txt");
	f = g_system->getSavefileManager()->openForSaving("Others.htm");
	LockMsg();
	
	bDontOutput = false;
	
	debugC(DEBUG_BASIC, kTonyDebugMPAL, "Dumping OTHERS.HTM...\n");

	f->writeString("<HTML>\n<BODY>\n");
	
	for (i = 0; i < GLOBALS._nMsgs; i++) {
		lpMessage = (char *)globalLock(GLOBALS._lpmmMsgs[i]._hText);
		if (*lpMessage != '\0') {
			nPeriods = 1;
			p = lpPeriods[0] = lpMessage;
			
			if (OutputStartOther(GLOBALS._lpmmMsgs[i]._wNum, f)) {	
				while (1) {
					// Find the end of the current period
					while (*p != '\0')
						p++;
					
					// If there is another '0' at the end, then the message is finished
					p++;
					if (*p == '\0')
						break;
					
					// Remember the start of the next line
					lpPeriods[nPeriods++] = p;
				}
				
				// Now loop over all the periods
				for (j = 0; j < nPeriods; j++) {
					if (nPeriods == 1)
						sprintf(fname, "000-%05d.WAV", GLOBALS._lpmmMsgs[i]._wNum);
					else
						sprintf(fname, "000-%05d-%02d.WAV", GLOBALS._lpmmMsgs[i]._wNum,j);				

					strcpy(frase,lpPeriods[j]);
					
					while ((p = strchr(frase, '^')) != NULL)
						*p = '\"';

					p = frase;
					while (*p == ' ')
						p++;
					if (*p == '\0')
						continue;		
					
					if (!bDontOutput) {
						v1->writeString(Common::String::format("%s\n", fname));
						f->writeString("\t<TR>\n");
						f->writeString(Common::String::format("\t\t<TD WIDTH=20%%> %s </B></TD>\n", fname));
						f->writeString(Common::String::format("\t\t<TD> %s </TD>\n", frase));
						f->writeString("\t</TR>\n");
					}
				}
			}

			outputEndOther(GLOBALS._lpmmMsgs[i]._wNum, f);
			
			globalUnlock(GLOBALS._lpmmMsgs[i]._hText);
		}
	}
	
	f->writeString("</BODY>\n</HTML>\n");
	
	f->finalize();
	v1->finalize();

	delete f;
	delete v1;
	UnlockMsg();
}


#if 0 // English names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Old lady 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Convict", NULL, NULL, "Jack (with microphone)", "Old lady 1", NULL };
const char *DLG90[] = { "Tony", "Bearded lady", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Piranha", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Snowman", NULL };
const char *DLG160[] = { "Tony", "Shmiley", NULL };
const char *DLG161[] = { "Tony", "Shmiley", NULL };
const char *DLG162[] = { "Tony", "Shmiley", NULL };
const char *DLG163[] = { "Tony", "Shmiley", NULL };
const char *DLG180[] = { "Tony", "Beast", NULL };
const char *DLG190[] = { "Tony", "Beast", NULL };
const char *DLG191[] = { "Tony", "Beast", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer", NULL };
const char *DLG211[] = { "Tony", "Mortimer", NULL };
const char *DLG212[] = { "Tony", "Mortimer", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Bartender", "Sad pirate", "Anchorman", "Biff", NULL };
const char *DLG251[] = { "Tony", "Bartender", "Sad pirate", "Anchorman", "Biff", NULL };
const char *DLG260[] = { "Tony", "Captain", "Captain (tale)", NULL };
const char *DLG270[] = { "Tony", "Egghead", NULL };
const char *DLG271[] = { "Tony", "Egghead", NULL };
const char *DLG272[] = { "Tony", "Egghead", NULL };
const char *DLG290[] = { "Tony", "Old lady 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Polly", "Captain (off scene)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Gatekeeper", NULL };
const char *DLG371[] = { "Tony", "Gatekeeper", NULL };
const char *DLG372[] = { "Tony", "Gatekeeper", NULL };
const char *DLG373[] = { "Tony", "Gatekeeper", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };	
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Mr. Wishing Well", "Tony (from the top of the well)", NULL  };
const char *DLG560[] = { "Tony", "Superintendent", NULL };
const char *DLG590[] = { "Tony", "Pantagruel", NULL };
const char *DLG600[] = { "Tony", "Jack", "Jack", NULL, "Jack", NULL, NULL, NULL, "Storyteller", "Mr. Wishing Well", NULL };
#endif

#if 0 // Polish names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Staruszka 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Skazaniec", NULL, NULL, "£ebster (przez mikrofon)", "Staruszka 1", NULL };
const char *DLG90[] = { "Tony", "Kobieta z Brod¹", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Pirania", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Ba³wan", NULL };
const char *DLG160[] = { "Tony", "Œmiechozol", NULL };
const char *DLG161[] = { "Tony", "Œmiechozol", NULL };
const char *DLG162[] = { "Tony", "Œmiechozol", NULL };
const char *DLG163[] = { "Tony", "Œmiechozol", NULL };
const char *DLG180[] = { "Tony", "Wycz", NULL };
const char *DLG190[] = { "Tony", "Wycz", NULL };
const char *DLG191[] = { "Tony", "Wycz", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer (Okropny)", NULL };
const char *DLG211[] = { "Tony", "Mortimer (Okropny)", NULL };
const char *DLG212[] = { "Tony", "Mortimer (Okropny)", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Barman", "Smutny Pirat", "Wodzirej", "Biff", NULL };
const char *DLG251[] = { "Tony", "Barman", "Smutny Pirat", "Wodzirej", "Biff", NULL };
const char *DLG260[] = { "Tony", "Kapitan", "Captain (opowieœæ)", NULL };
const char *DLG270[] = { "Tony", "Jajog³owy", NULL };
const char *DLG271[] = { "Tony", "Jajog³owy", NULL };
const char *DLG272[] = { "Tony", "Jajog³owy", NULL };
const char *DLG290[] = { "Tony", "Staruszka 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Polly", "Kapitan (zza sceny)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Stra¿nik", NULL };
const char *DLG371[] = { "Tony", "Stra¿nik", NULL };
const char *DLG372[] = { "Tony", "Stra¿nik", NULL };
const char *DLG373[] = { "Tony", "Stra¿nik", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Gospodyni (zza sceny)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Gospodyni", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Pan Studnia ¯yczeñ", "Tony (nad studni¹)", NULL  };
const char *DLG560[] = { "Tony", "Inspektor", NULL };
const char *DLG590[] = { "Tony", "Pantaloniarz", NULL };
const char *DLG600[] = { "Tony", "£ebster", "£ebster", NULL, "£ebster", NULL, NULL, NULL, "Narrator", "Pan Studnia ¯yczeñ", NULL };
#endif // Polish


#if 0 // Russian
const char *DLG10[] = { "Òîíè", NULL };
const char *DLG51[] = { "Òîíè", "Áó÷", "Äàäëè" };
const char *DLG52[] = { "Òîíè", NULL };
const char *DLG61[] = { "Òîíè", "Ñòàðóøêà 1", NULL };
const char *DLG71[] = { "Òîíè", "Òèìîòè", "Îñóæäåííûé", NULL, NULL, "Äæåê (ñ ìèêðîôîíîì)", "Ñòàðóøêà 1", NULL };
const char *DLG90[] = { "Òîíè", "Áîðîäàòàÿ æåíùèíà", NULL };
const char *DLG110[] = { "Òîíè", "Ëîðåíö", NULL };
const char *DLG111[] = { "Òîíè", "Ëîðåíö", NULL };
const char *DLG130[] = { "Òîíè", "Ïèðàíüÿ", NULL };
const char *DLG150[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG151[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG152[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG153[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG154[] = { "Òîíè", "Ðóôóñ", "Ñíåãîâèê", NULL };
const char *DLG160[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG161[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG162[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG163[] = { "Òîíè", "Øìàéëè", NULL };
const char *DLG180[] = { "Òîíè", "×óäîâèùå", NULL };
const char *DLG190[] = { "Òîíè", "×óäîâèùå", NULL };
const char *DLG191[] = { "Òîíè", "×óäîâèùå", NULL };
const char *DLG201[] = { "Òîíè", NULL };
const char *DLG210[] = { "Òîíè", "Ìîðòèìåð", NULL };
const char *DLG211[] = { "Òîíè", "Ìîðòèìåð", NULL };
const char *DLG212[] = { "Òîíè", "Ìîðòèìåð", NULL };
const char *DLG240[] = { "Òîíè", "Èçàáåëëà", NULL };
const char *DLG250[] = { "Òîíè", "Áàðìåí", "Ãðóñòíûé ïèðàò", "Âåäóùèé", "Áèôô", NULL };
const char *DLG251[] = { "Òîíè", "Áàðìåí", "Ãðóñòíûé ïèðàò", "Âåäóùèé", "Áèôô", NULL };
const char *DLG260[] = { "Òîíè", "Êàïèòàí", "Êàïèòàí (ðàññêàç)", NULL };
const char *DLG270[] = { "Òîíè", "ßéöåãîëîâûé", NULL };
const char *DLG271[] = { "Òîíè", "ßéöåãîëîâûé", NULL };
const char *DLG272[] = { "Òîíè", "ßéöåãîëîâûé", NULL };
const char *DLG290[] = { "Òîíè", "Ñòàðóøêà 2", NULL };
const char *DLG310[] = { "Òîíè", "Óîëëè", NULL };
const char *DLG330[] = { "Òîíè", "Ïîëëè", "Êàïèòàí (çà ñöåíîé)", NULL };
const char *DLG340[] = { "Òîíè", "Ðýíäàë", NULL };
const char *DLG360[] = { "Òîíè", NULL };
const char *DLG361[] = { "Òîíè", NULL };
const char *DLG370[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG371[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG372[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG373[] = { "Òîíè", "Ïðèâðàòíèê", NULL };
const char *DLG380[] = { "Òîíè", NULL };
const char *DLG410[] = { "Òîíè", "Ãâåíäåëü", NULL };
const char *DLG430[] = { "Òîíè", "Ãàðîëüä", "×àê", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Òîíè", NULL };
const char *DLG470[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Òîíè", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Òîíè", "Pin-up", NULL };
const char *DLG490[] = { "Òîíè", "Ãâåíäåëü", NULL };	
const char *DLG530[] = { "Òîíè", "Ãàðîëüä", "×àê", NULL };
const char *DLG550[] = { "Òîíè", "Ãîñïîäèí Êîëîäåö æåëàíèé", "Òîíè (ñ âåðøèíû êîëîäöà)", NULL  };
const char *DLG560[] = { "Òîíè", "Íà÷àëüíèê îõðàíû", NULL };
const char *DLG590[] = { "Òîíè", "Ïàíòàãðþýëü", NULL };
const char *DLG600[] = { "Òîíè", "Äæåê", "Äæåê", NULL, "Äæåê", NULL, NULL, NULL, "Ðàññêàç÷èê", "Ãîñïîäèí Êîëîäåö æåëàíèé", NULL };
#endif // Russian


#if 0 // Czech names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Stará paní 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Trestanec", NULL, NULL, "Jack (s mikrofonem)", "Stará paní 1", NULL };
const char *DLG90[] = { "Tony", "Vousatá žena", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Piraòa", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Snìhulák", NULL };
const char *DLG160[] = { "Tony", "Shmiley", NULL };
const char *DLG161[] = { "Tony", "Shmiley", NULL };
const char *DLG162[] = { "Tony", "Shmiley", NULL };
const char *DLG163[] = { "Tony", "Shmiley", NULL };
const char *DLG180[] = { "Tony", "Zvíøe", NULL };
const char *DLG190[] = { "Tony", "Zvíøe", NULL };
const char *DLG191[] = { "Tony", "Zvíøe", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer", NULL };
const char *DLG211[] = { "Tony", "Mortimer", NULL };
const char *DLG212[] = { "Tony", "Mortimer", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Barman", "Smutný pirát", "Moderátor", "Biff", NULL };
const char *DLG251[] = { "Tony", "Barman", "Smutný pirát", "Moderátor", "Biff", NULL };
const char *DLG260[] = { "Tony", "Kapitán", "Kapitán (pøíbìh)", NULL };
const char *DLG270[] = { "Tony", "Intelektuál", NULL };
const char *DLG271[] = { "Tony", "Intelektuál", NULL };
const char *DLG272[] = { "Tony", "Intelektuál", NULL };
const char *DLG290[] = { "Tony", "Stará paní 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Lóra", "Kapitán (mimo scénu)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Strážný", NULL };
const char *DLG371[] = { "Tony", "Strážný", NULL };
const char *DLG372[] = { "Tony", "Strážný", NULL };
const char *DLG373[] = { "Tony", "Strážný", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };	
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Pan Studna pøání", "Tony (z vrcholu studny)", NULL  };
const char *DLG560[] = { "Tony", "Správce", NULL };
const char *DLG590[] = { "Tony", "Pantagruel", NULL };
const char *DLG600[] = { "Tony", "Jack", "Jack", NULL, "Jack", NULL, NULL, NULL, "Vypravìè", "Pan Studna pøání", NULL };
#endif // Czech names

#if 1 // Deutsch names
const char *DLG10[] = { "Tony", NULL };
const char *DLG51[] = { "Tony", "Butch", "Dudley" };
const char *DLG52[] = { "Tony", NULL };
const char *DLG61[] = { "Tony", "Alte Dame 1", NULL };
const char *DLG71[] = { "Tony", "Timothy", "Sträfling", NULL, NULL, "Jack (mit Mikrofon)", "Alte Dame 1", NULL };
const char *DLG90[] = { "Tony", "Bärtige Dame", NULL };
const char *DLG110[] = { "Tony", "Lorenz", NULL };
const char *DLG111[] = { "Tony", "Lorenz", NULL };
const char *DLG130[] = { "Tony", "Piranha", NULL };
const char *DLG150[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG151[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG152[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG153[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG154[] = { "Tony", "Rufus", "Schneemann", NULL };
const char *DLG160[] = { "Tony", "Shmiley", NULL };
const char *DLG161[] = { "Tony", "Shmiley", NULL };
const char *DLG162[] = { "Tony", "Shmiley", NULL };
const char *DLG163[] = { "Tony", "Shmiley", NULL };
const char *DLG180[] = { "Tony", "Biest", NULL };
const char *DLG190[] = { "Tony", "Biest", NULL };
const char *DLG191[] = { "Tony", "Biest", NULL };
const char *DLG201[] = { "Tony", NULL };
const char *DLG210[] = { "Tony", "Mortimer", NULL };
const char *DLG211[] = { "Tony", "Mortimer", NULL };
const char *DLG212[] = { "Tony", "Mortimer", NULL };
const char *DLG240[] = { "Tony", "Isabella", NULL };
const char *DLG250[] = { "Tony", "Barmann", "Trauriger Pirat", "Chefanimateur", "Biff", NULL };
const char *DLG251[] = { "Tony", "Barmann", "Trauriger Pirat", "Chefanimateur", "Biff", NULL };
const char *DLG260[] = { "Tony", "Kapitän", "Kapitän (Erzählung)", NULL };
const char *DLG270[] = { "Tony", "Eierkopf", NULL };
const char *DLG271[] = { "Tony", "Eierkopf", NULL };
const char *DLG272[] = { "Tony", "Eierkopf", NULL };
const char *DLG290[] = { "Tony", "Alte Dame 2", NULL };
const char *DLG310[] = { "Tony", "Wally", NULL };
const char *DLG330[] = { "Tony", "Polly", "Kapitän (im Off)", NULL };
const char *DLG340[] = { "Tony", "Randall", NULL };
const char *DLG360[] = { "Tony", NULL };
const char *DLG361[] = { "Tony", NULL };
const char *DLG370[] = { "Tony", "Pförtner", NULL };
const char *DLG371[] = { "Tony", "Pförtner", NULL };
const char *DLG372[] = { "Tony", "Pförtner", NULL };
const char *DLG373[] = { "Tony", "Pförtner", NULL };
const char *DLG380[] = { "Tony", NULL };
const char *DLG410[] = { "Tony", "Gwendel", NULL };
const char *DLG430[] = { "Tony", "Harold", "Chuck", "Pigeons", "Housekeeper (off scene)", NULL };
const char *DLG460[] = { "Tony", NULL };
const char *DLG470[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG471[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG472[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG473[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG474[] = { "Tony", "Housekeeper", "Mirror", NULL };
const char *DLG480[] = { "Tony", "Pin-up", NULL };
const char *DLG490[] = { "Tony", "Gwendel", NULL };	
const char *DLG530[] = { "Tony", "Harold", "Chuck", NULL };
const char *DLG550[] = { "Tony", "Herr Wunschbrunnen", "Tony (über dem Brunnen)", NULL  };
const char *DLG560[] = { "Tony", "Verwalter", NULL };
const char *DLG590[] = { "Tony", "Pantagruel", NULL };
const char *DLG600[] = { "Tony", "Jack", "Jack", NULL, "Jack", NULL, NULL, NULL, "Erzähler", "Herr Wunschbrunnen", NULL };
#endif


#define HANDLE_DIALOG(num)	\
case num:	\
	if (nPers >= (int)(sizeof(DLG##num) / sizeof(const char *)) || DLG##num[nPers] == NULL)	\
	{	\
		warning("ERROR: The character #%d does not exist in dialog %d!\n", nPers, nDlg);	\
		return "ERROR";	\
	}	\
	else	\
		return DLG##num[nPers];


const char *getPersonName(uint16 nDlg, int nPers) {
	switch (nDlg) {
	HANDLE_DIALOG(10);
	HANDLE_DIALOG(51);
	HANDLE_DIALOG(52);
	HANDLE_DIALOG(61);
	HANDLE_DIALOG(71);
	HANDLE_DIALOG(90);
	HANDLE_DIALOG(110);
	HANDLE_DIALOG(111);
	HANDLE_DIALOG(130);
	HANDLE_DIALOG(150);
	HANDLE_DIALOG(151);
	HANDLE_DIALOG(152);
	HANDLE_DIALOG(153);
	HANDLE_DIALOG(154);
	HANDLE_DIALOG(160);
	HANDLE_DIALOG(161);
	HANDLE_DIALOG(162);
	HANDLE_DIALOG(163);
	HANDLE_DIALOG(180);
	HANDLE_DIALOG(190);
	HANDLE_DIALOG(191);
	HANDLE_DIALOG(201);
	HANDLE_DIALOG(210);
	HANDLE_DIALOG(211);
	HANDLE_DIALOG(212);
	HANDLE_DIALOG(240);
	HANDLE_DIALOG(250);
	HANDLE_DIALOG(251);
	HANDLE_DIALOG(260);
	HANDLE_DIALOG(270);
	HANDLE_DIALOG(271);
	HANDLE_DIALOG(272);
	HANDLE_DIALOG(290);
	HANDLE_DIALOG(310);
	HANDLE_DIALOG(330);
	HANDLE_DIALOG(340);
	HANDLE_DIALOG(360);
	HANDLE_DIALOG(361);
	HANDLE_DIALOG(370);
	HANDLE_DIALOG(371);
	HANDLE_DIALOG(372);
	HANDLE_DIALOG(373);
	HANDLE_DIALOG(380);
	HANDLE_DIALOG(410);
	HANDLE_DIALOG(430);
	HANDLE_DIALOG(460);
	HANDLE_DIALOG(470);
	HANDLE_DIALOG(471);
	HANDLE_DIALOG(472);
	HANDLE_DIALOG(473);
	HANDLE_DIALOG(474);
	HANDLE_DIALOG(480);
	HANDLE_DIALOG(490);
	HANDLE_DIALOG(530);
	HANDLE_DIALOG(550);
	HANDLE_DIALOG(560);
	HANDLE_DIALOG(590);
	HANDLE_DIALOG(600);

	default:
		warning("ERROR: Dialog %d does not exist!", (int)nDlg);
		return "ERROR";
	}
}

void mpalDumpDialog(LPMPALDIALOG dlg) {
	char dfn[64];
	char fname[64];
	int g,c,j;
	struct command* curCmd;
	char *frase; char *p;
	char copia[2048];
	bool bAtLeastOne;
	Common::OutSaveFile *f, *v1;

	v1 = g_system->getSavefileManager()->openForSaving("voicelist.txt");
	
	sprintf(dfn,"DIALOG%03d.HTM", dlg->nObj);
	warning("Dumping %s...\n", dfn);

	f = g_system->getSavefileManager()->openForSaving(dfn);

	f->writeString("<HTML>\n<BODY>\n");

	for (g = 0; dlg->_group[g].num != 0; g++) {
		bAtLeastOne = false;

		for (c = 0; c<dlg->_group[g].nCmds; c++) {
			curCmd = &dlg->_command[dlg->_group[g].CmdNum[c]];
			if (curCmd->type == 1 && curCmd->_nCf == 71) {
				bAtLeastOne = true;
				break;
			}
		}
		
		if (!bAtLeastOne)
			continue;
		
		f->writeString(Common::String::format("<P>\n<H3>Group %d</H3>\n<P>\n", g));
		f->writeString("<TABLE WIDTH = 100%% BORDER = 1>\n");

		for (c = 0; c < dlg->_group[g].nCmds; c++) {
			curCmd = &dlg->_command[dlg->_group[g].CmdNum[c]];

			// If it's a custom function, call SendDialogMessage(nPers, nMsg)
			if (curCmd->type == 1 && curCmd->_nCf == 71) {
				sprintf(fname, "%03d-%05d.WAV", dlg->nObj, curCmd->_arg2);
				
				for (j = 0; dlg->_periods[j] != NULL; j++)
					if (dlg->_periodNums[j] == curCmd->_arg2)
						break;
						
				if (dlg->_periods[j] == NULL)
					warning("ERROR: Dialog %d, Period %d not found!", (int)dlg->nObj, (int)curCmd->_arg2);
				else {	
					frase = (char *)globalLock(dlg->_periods[j]);
					strcpy(copia, frase);
					globalUnlock(dlg->_periods[j]);

					while ((p = strchr(copia,'^')) != NULL)
						*p = '\"';

					p = frase;
					while (*p == ' ')
						p++;
					if (*p == '\0')
						continue;				

					v1->writeString(Common::String::format("%s\n", fname));
					f->writeString("\t<TR>\n");
					f->writeString(Common::String::format("\t\t<TD WIDTH=20%%> %s </TD>\n", fname));
					f->writeString(Common::String::format("\t\t<TD WIDTH = 13%%> <B> %s </B> </TD>\n", 
						getPersonName(dlg->nObj, curCmd->_arg1)));
					f->writeString(Common::String::format("\t\t<TD> %s </TD>\n",copia));
					f->writeString("\t</TR>\n");
					//fprintf(f, "(%s) <%s> %s\n", fname, GetPersonName(dlg->nObj, curCmd->arg1), copia);
				}
			}
		}

		f->writeString("</TABLE><P>\n");
		//fprintf(f,"\n\n\n\n");
	}

	f->finalize();
	v1->finalize();
	delete f;
	delete v1;
}

void mpalDumpDialogs(void) {
	int i;

	lockDialogs();

	for (i = 0; i < GLOBALS._nDialogs; i++)
		mpalDumpDialog(&GLOBALS._lpmdDialogs[i]);

	unlockDialogs();
}

} // end of namespace MPAL

} // end of namespace Tony
