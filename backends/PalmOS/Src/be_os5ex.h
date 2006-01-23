/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * $Header$
 *
 */

#ifndef BE_OS5EX_H
#define BE_OS5EX_H

#include "be_os5.h"

#ifndef PALMOS_ARM
// Not usable with 68k mode (?), so ...
#define OSystem_PalmOS5Ex	OSystem_PalmOS5
#else

typedef struct {
	UInt32 __r9;
	UInt32 __r10;
	UInt32 timerID;
	UInt32 ticks;

	TimerPtr timer;
} TimerExType, *TimerExPtr;

class OSystem_PalmOS5Ex : public OSystem_PalmOS5 {
private:
	void timer_handler() {};
	void sound_handler() {};
	void int_quit();

	SndStreamVariableBufferCallback sound_callback();

public:
	OSystem_PalmOS5Ex();
	static OSystem *create();

	void setTimerCallback(TimerProc callback, int interval);

	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);
};
#endif

#endif
