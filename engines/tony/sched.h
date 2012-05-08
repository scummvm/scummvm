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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Data structures used by the process scheduler
 */

#ifndef TONY_SCHED_H
#define TONY_SCHED_H

#include "common/list.h"
#include "tony/coroutine.h"

namespace Tony {

// the size of process specific info
#define	PARAM_SIZE	32

// the maximum number of processes
#define	NUM_PROCESS	100
#define MAX_PROCESSES 100

#define INFINITE 0xffffffff
#define INVALID_PID_VALUE 0

typedef void (*CORO_ADDR)(CoroContext &, const void *);

/** process structure */
struct PROCESS {
	PROCESS *pNext;		///< pointer to next process in active or free list
	PROCESS *pPrevious;	///< pointer to previous process in active or free list

	CoroContext state;		///< the state of the coroutine
	CORO_ADDR  coroAddr;	///< the entry point of the coroutine

	int sleepTime;		///< number of scheduler cycles to sleep
	uint32 pid;			///< process ID
	bool waiting;		///< process is currently in a waiting state
	char param[PARAM_SIZE];	///< process specific info
};
typedef PROCESS *PPROCESS;

struct INT_CONTEXT;

/** Event structure */
struct EVENT {
	uint32 pid;
	bool manualReset;
	bool signalled;
};

/**
 * Create and manage "processes" (really coroutines).
 */
class Scheduler {
public:
	/** Pointer to a function of the form "void function(PPROCESS)" */
	typedef void (*VFPTRPP)(PROCESS *);

private:

	/** list of all processes */
	PROCESS *processList;

	/** active process list - also saves scheduler state */
	PROCESS *active;

	/** pointer to free process list */
	PROCESS *pFreeProcesses;

	/** the currently active process */
	PROCESS *pCurrent;

	/** Auto-incrementing process Id */
	int pidCounter;

	/** Event list */
	Common::List<EVENT *> _events;

#ifdef DEBUG
	// diagnostic process counters
	int numProcs;
	int maxProcs;

	void CheckStack();
#endif

	/**
	 * Called from killProcess() to enable other resources
	 * a process may be allocated to be released.
	 */
	VFPTRPP pRCfunction;

	PROCESS *getProcess(uint32 pid);
	EVENT *getEvent(uint32 pid);
public:

	Scheduler();
	~Scheduler();

	void reset();

	#ifdef	DEBUG
	void printStats();
	#endif

	void schedule();
	void rescheduleAll();
	void reschedule(PPROCESS pReSchedProc = NULL);
	void giveWay(PPROCESS pReSchedProc = NULL);
	void waitForSingleObject(CORO_PARAM, int pid, uint32 duration, bool *expired = NULL);
	void waitForMultipleObjects(CORO_PARAM, int nCount, uint32 *pidList, bool bWaitAll, 
			uint32 duration, bool *expired = NULL);
	void Scheduler::sleep(CORO_PARAM, uint32 duration);

	uint32 createProcess(CORO_ADDR coroAddr, const void *pParam, int sizeParam);
	uint32 createProcess(CORO_ADDR coroAddr, const void *pParam) {
		return createProcess(coroAddr, &pParam, sizeof(void *));
	}
	void killProcess(PROCESS *pKillProc);

	PROCESS *getCurrentProcess();
	int getCurrentPID() const;
	int killMatchingProcess(uint32 pidKill, int pidMask = -1);

	void setResourceCallback(VFPTRPP pFunc);

	/* Event methods */
	uint32 createEvent(bool bManualReset, bool bInitialState);
	void closeEvent(uint32 pidEvent);
	void setEvent(uint32 pidEvent);
	void resetEvent(uint32 pidEvent);
	void pulseEvent(uint32 pidEvent);
};

extern Scheduler *g_scheduler;	// FIXME: Temporary global var, to be used until everything has been OOifyied

} // End of namespace Tony

#endif		// TONY_SCHED_H
