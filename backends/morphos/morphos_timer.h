/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
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
#define TSM_MSGID_REMTIMER    1

struct TimerServiceMessage
{
	Message tsm_Message;
	ULONG tsm_MsgID;
	int ((*tsm_Callback)(int));
	LONG tsm_Interval;
};

class Timer
{
	public:
	   Timer(Scumm * system);
	   ~Timer();

		bool init();
		void release();
		bool installProcedure(int ((*procedure)(int)), int32 interval);
		void releaseProcedure(int ((*procedure)(int)));

	protected:
		bool SendMsg(ULONG MsgID, int ((*procedure)(int)), LONG interval);
		static void TimerService(Timer *);

		Process *TimerServiceThread;
		SignalSemaphore TimerServiceSemaphore;

		struct TimerSlot
		{
			MsgPort *ts_Port;
			timerequest *ts_IORequest;
			ULONG ts_SignalBit;
			int ((*ts_Callback)(int));
			LONG ts_Interval;
		};
};

#endif

