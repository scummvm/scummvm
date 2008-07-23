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

typedef void (*CORO_ADDR)(CoroContext &);


// process structure

struct PROCESS {
	PROCESS *pNext;	// pointer to next process in active or free list

	CoroContext state;		// the state of the coroutine
	CORO_ADDR  coroAddr;	// the entry point of the coroutine 

	int sleepTime;		// number of scheduler cycles to sleep
	int pid;		// process ID
	char param[PARAM_SIZE];	// process specific info
};


/*----------------------------------------------------------------------*\
|*			Scheduler Function Prototypes			*|
\*----------------------------------------------------------------------*/

void InitScheduler(void);	// called to init scheduler - kills all processes and places them on free list

void FreeProcessList(void);

#ifdef	DEBUG
void ProcessStats(void);	// Shows the maximum number of process used at once
#endif

void Scheduler(void);		// called to start process dispatching

PROCESS *CoroutineInstall(int pid, CORO_ADDR coroAddr, const void *pParam, int sizeParam);

void ProcessKill(		// kill a process
	PROCESS *pKillProc);	// which process to kill (must be different from current one)

PROCESS *CurrentProcess(void);	// Returns a pointer to the currently running process

int ProcessGetPID(		// Returns the process identifier of the specified process
	PROCESS *pProc);	// which process

char *ProcessGetParamsSelf();

int KillMatchingProcess(	// kill any process matching the pid parameters
	int pidKill,		// process identifier of process to kill
	int pidMask);		// mask to apply to process identifiers before comparison


// Pointer to a function of the form "void function(PPROCESS)"
typedef void (*VFPTRPP)(PROCESS *);

void SetResourceCallback(VFPTRPP pFunc);	// May be called by a resource allocator,
				// the function supplied is called by ProcessKill()
				// to allow the resource allocator to free resources
				// allocated to the dying process.


} // end of namespace Tinsel

#endif		// TINSEL_SCHED_H
