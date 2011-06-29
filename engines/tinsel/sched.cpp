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
 * Process scheduler.
 */

#include "tinsel/handle.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/sched.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

Scheduler *g_scheduler = 0;

#include "common/pack-start.h"	// START STRUCT PACKING

struct PROCESS_STRUC {
	uint32 processId;		// ID of process
	SCNHANDLE hProcessCode;	// handle to actor script
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static uint32 numSceneProcess;
static SCNHANDLE hSceneProcess;

static uint32 numGlobalProcess;
static PROCESS_STRUC *pGlobalProcess;

//--------------------- FUNCTIONS ------------------------

Scheduler::Scheduler() {
	processList = 0;
	pFreeProcesses = 0;
	pCurrent = 0;

#ifdef DEBUG
	// diagnostic process counters
	numProcs = 0;
	maxProcs = 0;
#endif

	pRCfunction = 0;

	active = new PROCESS;
	active->pPrevious = NULL;
	active->pNext = NULL;

	g_scheduler = this;	// FIXME HACK
}

Scheduler::~Scheduler() {
	// Kill all running processes (i.e. free memory allocated for their state).
	PROCESS *pProc = active->pNext;
	while (pProc != NULL) {
		delete pProc->state;
		pProc->state = 0;
		pProc = pProc->pNext;
	}

	free(processList);
	processList = NULL;

	delete active;
	active = 0;
}

/**
 * Kills all processes and places them on the free list.
 */
void Scheduler::reset() {

#ifdef DEBUG
	// clear number of process in use
	numProcs = 0;
#endif

	if (processList == NULL) {
		// first time - allocate memory for process list
		processList = (PROCESS *)calloc(MAX_PROCESSES, sizeof(PROCESS));

		// make sure memory allocated
		if (processList == NULL) {
			error("Cannot allocate memory for process data");
		}

		// fill with garbage
		memset(processList, 'S', MAX_PROCESSES * sizeof(PROCESS));
	}

	// Kill all running processes (i.e. free memory allocated for their state).
	PROCESS *pProc = active->pNext;
	while (pProc != NULL) {
		delete pProc->state;
		pProc->state = 0;
		pProc = pProc->pNext;
	}

	// no active processes
	pCurrent = active->pNext = NULL;

	// place first process on free list
	pFreeProcesses = processList;

	// link all other processes after first
	for (int i = 1; i <= NUM_PROCESS; i++) {
		processList[i - 1].pNext = (i == NUM_PROCESS) ? NULL : processList + i;
		processList[i - 1].pPrevious = (i == 1) ? active : processList + (i - 2);
	}
}


#ifdef	DEBUG
/**
 * Shows the maximum number of process used at once.
 */
void Scheduler::printStats() {
	debug("%i process of %i used", maxProcs, NUM_PROCESS);
}
#endif

#ifdef DEBUG
/**
 * Checks both the active and free process list to insure all the links are valid,
 * and that no processes have been lost
 */
void Scheduler::CheckStack() {
	Common::List<PROCESS *> pList;

	// Check both the active and free process lists
	for (int i = 0; i < 2; ++i) {
		PROCESS *p = (i == 0) ? active : pFreeProcesses;

		if (p != NULL) {
			// Make sure the linkages are correct
			while (p->pNext != NULL) {
				assert(p->pNext->pPrevious == p);
				pList.push_back(p);
				p = p->pNext;
			}
			pList.push_back(p);
		}
	}

	// Make sure all processes are accounted for
	for (int idx = 0; idx < NUM_PROCESS; idx++) {
		bool found = false;
		for (Common::List<PROCESS *>::iterator i = pList.begin(); i != pList.end(); ++i) {
			PROCESS *pTemp = *i;
			if (*i == &processList[idx]) {
				found = true;
				break;
			}
		}

		assert(found);
	}
}
#endif

/**
 * Give all active processes a chance to run
 */
void Scheduler::schedule() {
	// start dispatching active process list
	PROCESS *pNext;
	PROCESS *pProc = active->pNext;
	while (pProc != NULL) {
		pNext = pProc->pNext;

		if (--pProc->sleepTime <= 0) {
			// process is ready for dispatch, activate it
			pCurrent = pProc;
			pProc->coroAddr(pProc->state, pProc->param);

			if (!pProc->state || pProc->state->_sleep <= 0) {
				// Coroutine finished
				pCurrent = pCurrent->pPrevious;
				killProcess(pProc);
			} else {
				pProc->sleepTime = pProc->state->_sleep;
			}

			// pCurrent may have been changed
			pNext = pCurrent->pNext;
			pCurrent = NULL;
		}

		pProc = pNext;
	}
}

/**
 * Reschedules all the processes to run again this query
 */
void Scheduler::rescheduleAll() {
	assert(pCurrent);

	// Unlink current process
	pCurrent->pPrevious->pNext = pCurrent->pNext;
	if (pCurrent->pNext)
		pCurrent->pNext->pPrevious = pCurrent->pPrevious;

	// Add process to the start of the active list
	pCurrent->pNext = active->pNext;
	active->pNext->pPrevious = pCurrent;
	active->pNext = pCurrent;
	pCurrent->pPrevious = active;
}

/**
 * If the specified process has already run on this tick, make it run
 * again on the current tick.
 */
void Scheduler::reschedule(PPROCESS pReSchedProc) {
	// If not currently processing the schedule list, then no action is needed
	if (!pCurrent)
		return;

	if (!pReSchedProc)
		pReSchedProc = pCurrent;

	PPROCESS pEnd;

	// Find the last process in the list.
	// But if the target process is down the list from here, do nothing
	for (pEnd = pCurrent; pEnd->pNext != NULL; pEnd = pEnd->pNext) {
		if (pEnd->pNext == pReSchedProc)
			return;
	}

	assert(pEnd->pNext == NULL);

	// Could be in the middle of a KillProc()!
	// Dying process was last and this process was penultimate
	if (pReSchedProc->pNext == NULL)
		return;

	// If we're moving the current process, move it back by one, so that the next
	// schedule() iteration moves to the now next one
	if (pCurrent == pReSchedProc)
		pCurrent = pCurrent->pPrevious;

	// Unlink the process, and add it at the end
	pReSchedProc->pPrevious->pNext = pReSchedProc->pNext;
	pReSchedProc->pNext->pPrevious = pReSchedProc->pPrevious;
	pEnd->pNext = pReSchedProc;
	pReSchedProc->pPrevious = pEnd;
	pReSchedProc->pNext = NULL;
}

/**
 * Moves the specified process to the end of the dispatch queue
 * allowing it to run again within the current game cycle.
 * @param pGiveProc		Which process
 */
void Scheduler::giveWay(PPROCESS pReSchedProc) {
	// If not currently processing the schedule list, then no action is needed
	if (!pCurrent)
		return;

	if (!pReSchedProc)
		pReSchedProc = pCurrent;

	// If the process is already at the end of the queue, nothing has to be done
	if (!pReSchedProc->pNext)
		return;

	PPROCESS pEnd;

	// Find the last process in the list.
	for (pEnd = pCurrent; pEnd->pNext != NULL; pEnd = pEnd->pNext)
		;
	assert(pEnd->pNext == NULL);


	// If we're moving the current process, move it back by one, so that the next
	// schedule() iteration moves to the now next one
	if (pCurrent == pReSchedProc)
		pCurrent = pCurrent->pPrevious;

	// Unlink the process, and add it at the end
	pReSchedProc->pPrevious->pNext = pReSchedProc->pNext;
	pReSchedProc->pNext->pPrevious = pReSchedProc->pPrevious;
	pEnd->pNext = pReSchedProc;
	pReSchedProc->pPrevious = pEnd;
	pReSchedProc->pNext = NULL;
}

/**
 * Creates a new process.
 *
 * @param pid	process identifier
 * @param CORO_ADDR	coroutine start address
 * @param pParam	process specific info
 * @param sizeParam	size of process specific info
 */
PROCESS *Scheduler::createProcess(int pid, CORO_ADDR coroAddr, const void *pParam, int sizeParam) {
	PROCESS *pProc;

	// get a free process
	pProc = pFreeProcesses;

	// trap no free process
	assert(pProc != NULL); // Out of processes

#ifdef DEBUG
	// one more process in use
	if (++numProcs > maxProcs)
		maxProcs = numProcs;
#endif

	// get link to next free process
	pFreeProcesses = pProc->pNext;
	if (pFreeProcesses)
		pFreeProcesses->pPrevious = NULL;

	if (pCurrent != NULL) {
		// place new process before the next active process
		pProc->pNext = pCurrent->pNext;
		if (pProc->pNext)
			pProc->pNext->pPrevious = pProc;

		// make this new process the next active process
		pCurrent->pNext = pProc;
		pProc->pPrevious = pCurrent;

	} else {	// no active processes, place process at head of list
		pProc->pNext = active->pNext;
		pProc->pPrevious = active;

		if (pProc->pNext)
			pProc->pNext->pPrevious = pProc;
		active->pNext = pProc;

	}

	// set coroutine entry point
	pProc->coroAddr = coroAddr;

	// clear coroutine state
	pProc->state = 0;

	// wake process up as soon as possible
	pProc->sleepTime = 1;

	// set new process id
	pProc->pid = pid;

	// set new process specific info
	if (sizeParam) {
		assert(sizeParam > 0 && sizeParam <= PARAM_SIZE);

		// set new process specific info
		memcpy(pProc->param, pParam, sizeParam);
	}

	// return created process
	return pProc;
}

/**
 * Kills the specified process.
 *
 * @param pKillProc	which process to kill
 */
void Scheduler::killProcess(PROCESS *pKillProc) {
	// make sure a valid process pointer
	assert(pKillProc >= processList && pKillProc <= processList + NUM_PROCESS - 1);

	// can not kill the current process using killProcess !
	assert(pCurrent != pKillProc);

#ifdef DEBUG
	// one less process in use
	--numProcs;
	assert(numProcs >= 0);
#endif

	// Free process' resources
	if (pRCfunction != NULL)
		(pRCfunction)(pKillProc);

	delete pKillProc->state;
	pKillProc->state = 0;

	// Take the process out of the active chain list
	pKillProc->pPrevious->pNext = pKillProc->pNext;
	if (pKillProc->pNext)
		pKillProc->pNext->pPrevious = pKillProc->pPrevious;

	// link first free process after pProc
	pKillProc->pNext = pFreeProcesses;
	if (pFreeProcesses)
		pKillProc->pNext->pPrevious = pKillProc;
	pKillProc->pPrevious = NULL;

	// make pKillProc the first free process
	pFreeProcesses = pKillProc;
}



/**
 * Returns a pointer to the currently running process.
 */
PROCESS *Scheduler::getCurrentProcess() {
	return pCurrent;
}

/**
 * Returns the process identifier of the specified process.
 *
 * @param pProc	which process
 */
int Scheduler::getCurrentPID() const {
	PROCESS *pProc = pCurrent;

	// make sure a valid process pointer
	assert(pProc >= processList && pProc <= processList + NUM_PROCESS - 1);

	// return processes PID
	return pProc->pid;
}

/**
 * Kills any process matching the specified PID. The current
 * process cannot be killed.
 *
 * @param pidKill	process identifier of process to kill
 * @param pidMask	mask to apply to process identifiers before comparison
 * @return The number of processes killed is returned.
 */
int Scheduler::killMatchingProcess(int pidKill, int pidMask) {
	int numKilled = 0;
	PROCESS *pProc, *pPrev;	// process list pointers

	for (pProc = active->pNext, pPrev = active; pProc != NULL; pPrev = pProc, pProc = pProc->pNext) {
		if ((pProc->pid & pidMask) == pidKill) {
			// found a matching process

			// dont kill the current process
			if (pProc != pCurrent) {
				// kill this process
				numKilled++;

				// Free the process' resources
				if (pRCfunction != NULL)
					(pRCfunction)(pProc);

				delete pProc->state;
				pProc->state = 0;

				// make prev point to next to unlink pProc
				pPrev->pNext = pProc->pNext;
				if (pProc->pNext)
					pPrev->pNext->pPrevious = pPrev;

				// link first free process after pProc
				pProc->pNext = pFreeProcesses;
				pProc->pPrevious = NULL;
				pFreeProcesses->pPrevious = pProc;

				// make pProc the first free process
				pFreeProcesses = pProc;

				// set to a process on the active list
				pProc = pPrev;
			}
		}
	}

#ifdef DEBUG
	// adjust process in use
	numProcs -= numKilled;
	assert(numProcs >= 0);
#endif

	// return number of processes killed
	return numKilled;
}

/**
 * Set pointer to a function to be called by killProcess().
 *
 * May be called by a resource allocator, the function supplied is
 * called by killProcess() to allow the resource allocator to free
 * resources allocated to the dying process.
 *
 * @param pFunc	Function to be called by killProcess()
 */
void Scheduler::setResourceCallback(VFPTRPP pFunc) {
	pRCfunction = pFunc;
}

/**************************************************************************\
|***********    Stuff to do with scene and global processes    ************|
\**************************************************************************/

/**
 * The code for for restored scene processes.
 */
static void RestoredProcessProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	_ctx->pic = *(const PINT_CONTEXT *)param;

	_ctx->pic = RestoreInterpretContext(_ctx->pic);
	AttachInterpret(_ctx->pic, g_scheduler->getCurrentProcess());

	CORO_INVOKE_1(Interpret, _ctx->pic);

	CORO_END_CODE;
}

/**
 * Process Tinsel Process
 */
static void ProcessTinselProcess(CORO_PARAM, const void *param) {
	const PINT_CONTEXT *pPic = (const PINT_CONTEXT *)param;

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	CORO_INVOKE_1(Interpret, *pPic);

	CORO_KILL_SELF();
	CORO_END_CODE;
}


/**************************************************************************\
|*****************    Stuff to do with scene processes    *****************|
\**************************************************************************/

/**
 * Called to restore a scene process.
 */
void RestoreSceneProcess(INT_CONTEXT *pic) {
	uint32 i;
	PROCESS_STRUC	*pStruc;

	pStruc = (PROCESS_STRUC *)LockMem(hSceneProcess);
	for (i = 0; i < numSceneProcess; i++) {
		if (FROM_LE_32(pStruc[i].hProcessCode) == pic->hCode) {
			g_scheduler->createProcess(PID_PROCESS + i, RestoredProcessProcess,
					 &pic, sizeof(pic));
			break;
		}
	}

	assert(i < numSceneProcess);
}

/**
 * Run a scene process with the given event.
 */
void SceneProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape,
						bool *result) {
	uint32 i;		// Loop counter
	if (result) *result = false;

	CORO_BEGIN_CONTEXT;
		PROCESS_STRUC *pStruc;
		PPROCESS pProc;
		PINT_CONTEXT pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->pStruc = (PROCESS_STRUC *)LockMem(hSceneProcess);
	for (i = 0; i < numSceneProcess; i++) {
		if (FROM_LE_32(_ctx->pStruc[i].processId) == procID) {
			assert(_ctx->pStruc[i].hProcessCode);		// Must have some code to run

			_ctx->pic = InitInterpretContext(GS_PROCESS,
				FROM_LE_32(_ctx->pStruc[i].hProcessCode),
				event,
				NOPOLY,			// No polygon
				0,			// No actor
				NULL,			// No object
				myEscape);
			if (_ctx->pic == NULL)
				return;

			_ctx->pProc = g_scheduler->createProcess(PID_PROCESS + i, ProcessTinselProcess,
				&_ctx->pic, sizeof(_ctx->pic));
			AttachInterpret(_ctx->pic, _ctx->pProc);
			break;
		}
	}

	if (i == numSceneProcess)
		return;

	if (bWait) {
		CORO_INVOKE_2(WaitInterpret, _ctx->pProc, result);
	}

	CORO_END_CODE;
}

/**
 * Kill all instances of a scene process.
 */
void KillSceneProcess(uint32 procID) {
	uint32 i;		// Loop counter
	PROCESS_STRUC	*pStruc;

	pStruc = (PROCESS_STRUC *) LockMem(hSceneProcess);
	for (i = 0; i < numSceneProcess; i++) {
		if (FROM_LE_32(pStruc[i].processId) == procID) {
			g_scheduler->killMatchingProcess(PID_PROCESS + i, -1);
			break;
		}
	}
}

/**
 * Register the scene processes in a scene.
 */
void SceneProcesses(uint32 numProcess, SCNHANDLE hProcess) {
	numSceneProcess = numProcess;
	hSceneProcess = hProcess;
}


/**************************************************************************\
|*****************    Stuff to do with global processes    ****************|
\**************************************************************************/

/**
 * Called to restore a global process.
 */
void RestoreGlobalProcess(INT_CONTEXT *pic) {
	uint32 i;		// Loop counter

	for (i = 0; i < numGlobalProcess; i++) {
		if (pGlobalProcess[i].hProcessCode == pic->hCode) {
			g_scheduler->createProcess(PID_GPROCESS + i, RestoredProcessProcess,
					 &pic, sizeof(pic));
			break;
		}
	}

	assert(i < numGlobalProcess);
}

/**
 * Kill them all (restore game).
 */
void KillGlobalProcesses() {

	for (uint32 i = 0; i < numGlobalProcess; ++i)	{
		g_scheduler->killMatchingProcess(PID_GPROCESS + i, -1);
	}
}

/**
 * Run a global process with the given event.
 */
bool GlobalProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape) {
	CORO_BEGIN_CONTEXT;
		PINT_CONTEXT	pic;
		PPROCESS	pProc;
	CORO_END_CONTEXT(_ctx);

	bool result = false;

	CORO_BEGIN_CODE(_ctx);

	uint32	i;		// Loop counter
	_ctx->pProc = NULL;

	for (i = 0; i < numGlobalProcess; ++i)	{
		if (pGlobalProcess[i].processId == procID) {
			assert(pGlobalProcess[i].hProcessCode);		// Must have some code to run

			_ctx->pic = InitInterpretContext(GS_GPROCESS,
				pGlobalProcess[i].hProcessCode,
				event,
				NOPOLY,			// No polygon
				0,			// No actor
				NULL,			// No object
				myEscape);

			if (_ctx->pic != NULL) {

				_ctx->pProc = g_scheduler->createProcess(PID_GPROCESS + i, ProcessTinselProcess,
					&_ctx->pic, sizeof(_ctx->pic));
				AttachInterpret(_ctx->pic, _ctx->pProc);
			}
			break;
		}
	}

	if ((i == numGlobalProcess) || (_ctx->pic == NULL))
		result = false;
	else if (bWait)
		CORO_INVOKE_ARGS_V(WaitInterpret, false, (CORO_SUBCTX, _ctx->pProc, &result));

	CORO_END_CODE;
	return result;
}

/**
 * Kill all instances of a global process.
 */
void xKillGlobalProcess(uint32 procID) {
	uint32 i;		// Loop counter

	for (i = 0; i < numGlobalProcess; ++i) {
		if (pGlobalProcess[i].processId == procID) {
			g_scheduler->killMatchingProcess(PID_GPROCESS + i, -1);
			break;
		}
	}
}

/**
 * Register the global processes list
 */
void GlobalProcesses(uint32 numProcess, byte *pProcess) {
	pGlobalProcess = new PROCESS_STRUC[numProcess];
	numGlobalProcess = numProcess;
	byte *p = pProcess;

	for (uint i = 0; i < numProcess; ++i, p += 8) {
		pGlobalProcess[i].processId = READ_LE_UINT32(p);
		pGlobalProcess[i].hProcessCode = READ_LE_UINT32(p + 4);
	}
}

/**
 * Frees the global processes list
 */
void FreeGlobalProcesses() {
	delete[] pGlobalProcess;
	pGlobalProcess = 0;
	numGlobalProcess = 0;
}

} // End of namespace Tinsel
