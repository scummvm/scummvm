/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
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
 */

#ifndef MORPHOS_TIMER_H
#define MORPHOS_TIMER_H

#ifndef TIMER_H
#include "timer.h"	// for MAX_TIMER
#endif

#ifndef EXEC_PORTS_H
#include <exec/ports.h>
#endif

#ifndef EXEC_IO_H
#include <exec/io.h>
#endif

#ifndef EXEC_SEMAPHORES_H
#include <exec/semaphores.h>
#endif

#ifndef DOS_DOSEXTENS_H
#include <dos/dosextens.h>
#endif

class OSystem;

#define TSM_MSGID_ADDTIMER	  0
#define TSM_MSGID_REMTIMER   1

struct TimerServiceMessage
{
	Message tsm_Message;
	ULONG tsm_MsgID;
	TimerProc tsm_Callback;
	LONG tsm_Interval;
};

class Timer
{
	public:
		Timer(Engine * engine);
	   ~Timer();

		bool init();
		void release();
		bool installTimerProc(TimerProc procedure, int32 interval);
		void removeTimerProc(TimerProc procedure);

	protected:
		bool SendMsg(ULONG MsgID, TimerProc procedure, LONG interval);
		static void TimerService(Timer *, Engine *);

		Process *TimerServiceThread;
		MsgPort *TimerServicePort;
		Message TimerServiceStartup;

		struct TimerSlot
		{
			MsgPort *ts_Port;
			timerequest *ts_IORequest;
			ULONG ts_SignalBit;
			TimerProc ts_Callback;
			LONG ts_Interval;
		};
};

#endif

