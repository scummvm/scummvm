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

#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "tony/sched.h"

namespace Tony {

Scheduler *g_scheduler = 0;

//--------------------- FUNCTIONS ------------------------

Scheduler::Scheduler() {
	processList = NULL;
	pFreeProcesses = NULL;
	pCurrent = NULL;

#ifdef DEBUG
	// diagnostic process counters
	numProcs = 0;
	maxProcs = 0;
#endif

	pRCfunction = NULL;
	pidCounter = 0;

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
 * Continously makes a given process wait for another process to finish
 *
 * @param pid			Process identifier
 * @param duration		Duration in milliseconds
 * @param expired		Set to true if delay period expired
 */
void Scheduler::waitForSingleObject(CORO_PARAM, int pid, int duration, bool *expired) {
	if (!pCurrent)
		error("Called Scheduler::waitForSingleObject from the main process");

	CORO_BEGIN_CONTEXT;
		uint32 endTime;
		PROCESS *pProc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->endTime = (duration == INFINITE) ? INFINITE : g_system->getMillis() + duration;
	if (expired)
		*expired = false;

	// Outer loop for doing checks until expiry 
	while (g_system->getMillis() < _ctx->endTime) {
		// Check to see if a process with the given Id exists
		_ctx->pProc = active->pNext;
		while ((_ctx->pProc != NULL) && (_ctx->pProc->pid != pid))
			_ctx->pProc = _ctx->pProc->pNext;

		if (_ctx->pProc == NULL) {
			// No match process found, so it's okay to break out of loop
			if (expired)
				*expired = true;

			break;
		}

		// Sleep until the next cycle
		CORO_SLEEP(1);
	}

	CORO_END_CODE;
}

/**
 * Creates a new process.
 *
 * @param pid	process identifier
 * @param CORO_ADDR	coroutine start address
 * @param pParam	process specific info
 * @param sizeParam	size of process specific info
 */
uint32 Scheduler::createProcess(CORO_ADDR coroAddr, const void *pParam, int sizeParam) {
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
	pProc->pid = ++pidCounter;

	// set new process specific info
	if (sizeParam) {
		assert(sizeParam > 0 && sizeParam <= PARAM_SIZE);

		// set new process specific info
		memcpy(pProc->param, pParam, sizeParam);
	}

	// return created process
	return pProc->pid;
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

} // End of namespace Tony
