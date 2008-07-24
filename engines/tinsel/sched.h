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
 * $URL$
 * $Id$
 *
 * Data structures used by the process scheduler
 */

#ifndef TINSEL_SCHED_H     // prevent multiple includes
#define TINSEL_SCHED_H

#include "tinsel/dw.h"	// new data types
#include "tinsel/coroutine.h"

namespace Tinsel {

// the size of process specific info
#define	PARAM_SIZE	32

// the maximum number of processes
#define	NUM_PROCESS	64

typedef void (*CORO_ADDR)(CoroContext &, const void *);


struct PROCESS;

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
	
#ifdef DEBUG
	// diagnostic process counters
	int numProcs;
	int maxProcs;
#endif
	
	/**
	 * Called from killProcess() to enable other resources
	 * a process may be allocated to be released.
	 */
	VFPTRPP pRCfunction;


public:

	Scheduler();
	~Scheduler();

	void reset();
	
	#ifdef	DEBUG
	void printStats();
	#endif
	
	void schedule();
	
	PROCESS *createProcess(int pid, CORO_ADDR coroAddr, const void *pParam, int sizeParam);
	void killProcess(PROCESS *pKillProc);
	
	PROCESS *getCurrentProcess();
	int getCurrentPID() const;
	int killMatchingProcess(int pidKill, int pidMask);
	
	
	void setResourceCallback(VFPTRPP pFunc);

};

extern Scheduler *g_scheduler;	// FIXME: Temporary global var, to be used until everything has been OOifyied

} // end of namespace Tinsel

#endif		// TINSEL_SCHED_H
