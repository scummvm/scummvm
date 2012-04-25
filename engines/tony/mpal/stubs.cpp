e/* ScummVM - Graphic Adventure Engine
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

#ifndef TONY_MPAL_STUBS
#define TONY_MPAL_STUBS

/****************************************************************************\
*       This file contains stubs and mappings for things used by the MPAL
*		library that are handled differently under ScummVM
\****************************************************************************/

#include "common/algorithm.h"
#include "common/system.h"
#include "engines/engine.h"
#include "tony/tony.h"
#include "stubs.h"

namespace Tony {

namespace MPAL {

/**
 * Allocates a memory block
 */
byte *GlobalAlloc(uint16 flags, int size) {
	byte *result = (byte *)malloc(size);

	if (flags & GMEM_ZEROINIT)
		Common::fill(result, result + size, 0);

	return result;
}

/**
 * Lock a global handle
 * @param h		Global handle
 * @remarks		Since HGLOBALs are directly representing the pointers anyway,
 *				simply return it
 */
void *GlobalLock(HGLOBAL h) {
	return h;
}

/**
 * Unlock a global handle
 * @param h		Global handle
 * @remarks		Since HGLOBALs are directly representing the pointers anyway,
 *				the unlock method doesn't need to do anything
 */
void GlobalUnlock(HGLOBAL h) {
}

/**
 * Free a globally allocated memory block
 * @param h		Global handle
 */
void GlobalFree(HGLOBAL h) {
	free(h);
}

/**
 * Display a message
 * @param msg		Message to display
 */
void MessageBox(const Common::String &msg) {

	_vm->GUIError(msg);
}

/**
 * Gets the current time in milliseconds
 */
uint32 timeGetTime() {
	return g_system->getMillis();
}

HANDLE CreateThread(void *lpThreadAttributes, size_t dwStackSize, 
					LPTHREAD_START_ROUTINE lpStartAddress, void *lpParameter,
					uint32 dwCreationFlags, uint32 *lpThreadId) {
	*lpThreadId = 0;
	return 0;
}

void ExitThread(HANDLE ThreadId) {
}

void TerminateThread(HANDLE ThreadId, uint32 dwExitCode) {

}

void CloseHandle(HANDLE ThreadId) {

}

void Sleep(uint32 time) {
}

int WaitForSingleObject(HANDLE ThreadId, uint32 dwSleepTime) {
	return 0;
}

uint32 WaitForMultipleObjects(uint32 nCount, const HANDLE *lpHandles, bool bWaitAll, uint32 dwMilliseconds) {
	return 0;
}

HANDLE CreateEvent(void *lpEventAttributes, bool bManualReset, bool bInitialState, const char *lpName) {
	return 0;
}

void SetEvent(HANDLE hEvent) {
}

void ResetEvent(HANDLE hEvent) {
}

} // end of namespace MPAL

} // end of namespace Tony

#endif
