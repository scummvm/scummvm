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

#include "stdafx.h"
#include "scumm.h"

#include <exec/memory.h>
#include <exec/semaphores.h>
#include <dos/dostags.h>
#include <emul/emulinterface.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>

#include "morphos.h"
#include "timer.h"

static TagItem TimerServiceTags[] = { { NP_Entry,    0 },
									  { NP_Name, 	 (ULONG)"ScummVM Timer Service" },
									  { NP_Priority, 20 },
									  { TAG_DONE,    0 }
									};

Timer::Timer(Scumm * system)
{
	static EmulFunc ThreadEmulFunc;

	InitSemaphore(&TimerServiceSemaphore);

	ThreadEmulFunc.Trap = TRAP_FUNC;
	ThreadEmulFunc.Address = (ULONG) &TimerService;
	ThreadEmulFunc.StackSize = 16000;
	ThreadEmulFunc.Extension = 0;
	ThreadEmulFunc.Arg1 = (ULONG) this;
	TimerServiceTags[0].ti_Data = (ULONG) &ThreadEmulFunc;
	TimerServiceThread = CreateNewProc(TimerServiceTags);
}

Timer::~Timer()
{
	if (TimerServiceThread)
	{
		Signal((Task *) TimerServiceThread, SIGBREAKF_CTRL_C);
		ObtainSemaphore(&TimerServiceSemaphore);
		ReleaseSemaphore(&TimerServiceSemaphore);
	}
}

bool Timer::init()
{
	return TimerServiceThread != NULL;
}

void Timer::release()
{
}

bool Timer::installProcedure(int ((*procedure)(int)), int32 interval)
{
	return SendMsg(TSM_MSGID_ADDTIMER, procedure, interval);
}

void Timer::releaseProcedure(int ((*procedure)(int)))
{
	SendMsg(TSM_MSGID_REMTIMER, procedure, 0);
}

bool Timer::SendMsg(ULONG msg_id, int ((*procedure)(int)), LONG interval)
{
	if (TimerServiceThread == NULL)
		return false;

	TimerServiceMessage *tmsg = (TimerServiceMessage *) AllocVec(sizeof (TimerServiceMessage), MEMF_PUBLIC | MEMF_CLEAR);
	if (tmsg == NULL)
		return false;

	MsgPort *reply_port = CreateMsgPort();
	if (reply_port == NULL)
	{
		FreeVec(tmsg);
		return false;
	}

	tmsg->tsm_Message.mn_Node.ln_Type = NT_MESSAGE;
	tmsg->tsm_Message.mn_ReplyPort = reply_port;
	tmsg->tsm_Message.mn_Length = sizeof (TimerServiceMessage);
	tmsg->tsm_MsgID = msg_id;
	tmsg->tsm_Callback = procedure;
	tmsg->tsm_Interval = interval;
	PutMsg(&TimerServiceThread->pr_MsgPort, tmsg);
	WaitPort(reply_port);
	GetMsg(reply_port);
	
	FreeVec(tmsg);
	DeleteMsgPort(reply_port);

	return true;
}

void Timer::TimerService(Timer *this_ptr)
{
	MsgPort *port = &((Process *) FindTask(NULL))->pr_MsgPort;
	ULONG port_bit = 1 << port->mp_SigBit;
	ULONG signal_mask = SIGBREAKF_CTRL_C | port_bit;
	ULONG timer_bits = 0, signals;
	ULONG interval, t;
	timeval start_callback, end_callback;

	ULONG timers = 0;
	TimerSlot timer_slots[MAX_TIMERS];

	ObtainSemaphore(&this_ptr->TimerServiceSemaphore);

	for (;;)
	{
		signals = Wait(signal_mask);

		GetSysTime(&start_callback);
		
		if (signals & port_bit)
		{
			TimerServiceMessage *tmsg;

			while (tmsg = (TimerServiceMessage *) GetMsg(port))
			{
				if (tmsg->tsm_Message.mn_Length == sizeof (TimerServiceMessage))
				{
					switch (tmsg->tsm_MsgID)
					{
						case TSM_MSGID_ADDTIMER:
							if (timers < MAX_TIMERS)
							{
								ULONG unit = UNIT_MICROHZ;

								if (tmsg->tsm_Interval > 1000)
									unit = UNIT_VBLANK;
								OSystem_MorphOS::OpenATimer(&timer_slots[timers].ts_Port, (IORequest **) &timer_slots[timers].ts_IORequest, unit);
								timer_slots[timers].ts_Callback = tmsg->tsm_Callback;
								timer_slots[timers].ts_Interval = tmsg->tsm_Interval;
								timer_slots[timers].ts_SignalBit = 1 << timer_slots[timers].ts_Port->mp_SigBit;

								signal_mask |= timer_slots[timers].ts_SignalBit;
								timer_bits |= timer_slots[timers].ts_SignalBit;

								timerequest *req = timer_slots[timers].ts_IORequest;
								interval = timer_slots[timers].ts_Interval;
								req->tr_node.io_Command  = TR_ADDREQUEST;
								req->tr_time.tv_secs  = interval/1000;
								req->tr_time.tv_micro = (interval%1000)*1000;
								SendIO(req);

								timers++;
							}
							break;

						case TSM_MSGID_REMTIMER:
						{
							for (t = 0; t < timers; t++)
							{
								if (timer_slots[t].ts_Callback == tmsg->tsm_Callback)
								{
									AbortIO((IORequest *) timer_slots[t].ts_IORequest);
									WaitIO((IORequest *) timer_slots[t].ts_IORequest);
									signal_mask &= ~timer_slots[t].ts_SignalBit;
									timer_bits &= ~timer_slots[t].ts_SignalBit;
									DeleteIORequest((IORequest *) timer_slots[t].ts_IORequest);
									DeleteMsgPort(timer_slots[t].ts_Port);
									if (t < timers-1)
										memmove(&timer_slots[t], &timer_slots[t+1], sizeof (TimerSlot)*(timers-t-1));
									timers--;
									continue;
								}
							}
							break;
						}

						default:
							warning("MorphOS TimerService received message of unknown type.");
					}
				}

				ReplyMsg((Message *) tmsg);
			}
		}

		if (signals & SIGBREAKF_CTRL_C)
			break;

		if (signals & timer_bits)
		{
			for (t = 0; t < timers; t++)
			{
				if (signals & timer_slots[t].ts_SignalBit)
				{
					timerequest *req = timer_slots[t].ts_IORequest;
					WaitIO((IORequest *) req);
					interval = timer_slots[t].ts_Interval;
					(*timer_slots[t].ts_Callback)(interval);
					GetSysTime(&end_callback);
					SubTime(&end_callback, &start_callback);
					interval -= end_callback.tv_sec*1000+end_callback.tv_micro/1000+20;

					req->tr_node.io_Command  = TR_ADDREQUEST;
					req->tr_time.tv_secs  = interval/1000;
					req->tr_time.tv_micro = (interval%1000)*1000;
					SendIO(req);
				}
			}
		}
	}

	for (t = 0; t < timers; t++)
	{
		AbortIO((IORequest *) timer_slots[t].ts_IORequest);
		WaitIO((IORequest *) timer_slots[t].ts_IORequest);
		DeleteIORequest((IORequest *) timer_slots[t].ts_IORequest);
		DeleteMsgPort(timer_slots[t].ts_Port);
	}

	ReleaseSemaphore(&this_ptr->TimerServiceSemaphore);
	RemTask(NULL);
}

