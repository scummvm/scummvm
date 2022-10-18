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
 * Data structures used by the process scheduler
 */

#ifndef TINSEL_SCHED_H     // prevent multiple includes
#define TINSEL_SCHED_H

#include "common/coroutines.h"
#include "tinsel/dw.h"	// new data types
#include "tinsel/events.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

struct INT_CONTEXT;

struct PROCESS_STRUC {
	uint32 processId;       // ID of process
	SCNHANDLE hProcessCode; // handle to actor script
};

//----------------- FUNCTION PROTOTYPES --------------------

void SceneProcesses(uint32 numProcess, SCNHANDLE hProcess);
void KillSceneProcess(uint32 procID);
void SceneProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait,
					   int myEscape, bool *result = NULL);
void RestoreSceneProcess(INT_CONTEXT *pic);

void GlobalProcesses(uint32 numProcess, byte *pProcess);
void xKillGlobalProcess(uint32 procID);
bool GlobalProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape);
void RestoreGlobalProcess(INT_CONTEXT *pic);
void KillGlobalProcesses();
void FreeGlobalProcesses();

} // End of namespace Tinsel

#endif		// TINSEL_SCHED_H
