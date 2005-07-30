/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM Team
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

#include "common/stdafx.h"
#include "base/engine.h"

#include <exec/memory.h>
#include <exec/semaphores.h>
#include <dos/dostags.h>
#include <emul/emulinterface.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>

#include "morphos.h"
#include "timer.h"

Timer::Timer(Engine * engine)
{
	if ((TimerServicePort = CreateMsgPort()))
	{
		TimerServiceStartup.mn_Node.ln_Type = NT_MESSAGE;
		TimerServiceStartup.mn_ReplyPort = TimerServicePort;
		TimerServiceStartup.mn_Length = sizeof(TimerServiceStartup);

		TimerServiceThread = CreateNewProcTags(NP_Entry, 	 (ULONG) TimerService,
															NP_CodeType, CODETYPE_PPC,
															NP_Name,  	 (ULONG) "ScummVM Timer Service",
															NP_Priority, 0,
															NP_StartupMsg, &TimerServiceStartup,
															NP_PPC_Arg1, (ULONG) this,
															NP_PPC_Arg2, (ULONG) engine,
															TAG_DONE
														  );
	}
}

Timer::~Timer()
{
	if (TimerServiceThread)
	{
		Signal((Task *) TimerServiceThread, SIGBREAKF_CTRL_C);
		WaitPort(TimerServicePort);
		DeleteMsgPort(TimerServicePort);
		TimerServiceThread = NULL;
	}
}

bool Timer::init()
{
	return TimerServiceThread != NULL;
}

void Timer::release()
{
}

bool Timer::installTimerProc(TimerProc procedure, int32 interval)
{
	return SendMsg(TSM_MSGID_ADDTIMER, procedure, interval);
}

void Timer::removeTimerProc(TimerProc procedure)
{
	SendMsg(TSM_MSGID_REMTIMER, procedure, 0);
}

bool Timer::SendMsg(ULONG msg_id, TimerProc procedure, LONG interval)
{
	if (TimerServiceThread == NULL)
		return false;

	TimerServiceMessage *tmsg = (TimerServiceMessage *) AllocVec(sizeof (TimerServiceMessage), MEMF_PUBLIC | MEMF_CLEAR);
	if (tmsg == NULL)
		return false;

	tmsg->tsm_Message.mn_Node.ln_Type = NT_MESSAGE;
	tmsg->tsm_Message.mn_ReplyPort = NULL;
	tmsg->tsm_Message.mn_Length = sizeof (TimerServiceMessage);
	tmsg->tsm_MsgID = msg_id;
	tmsg->tsm_Callback = procedure;
	tmsg->tsm_Interval = interval;
	PutMsg(&TimerServiceThread->pr_MsgPort, (Message*) tmsg);

	return true;
}

void Timer::TimerService(Timer *this_ptr, Engine *engine)
{
	MsgPort *port = &((Process *) FindTask(NULL))->pr_MsgPort;
	ULONG port_bit = 1 << port->mp_SigBit;
	ULONG signal_mask = SIGBREAKF_CTRL_C | port_bit;
	ULONG timer_bits = 0, signals;
	ULONG interval, t;
	timeval start_callback, end_callback;

	ULONG timers = 0;
	TimerSlot timer_slots[MAX_TIMERS];

	for (;;)
	{
		signals = Wait(signal_mask);

		GetSysTime(&start_callback);

		if (signals & port_bit)
		{
			TimerServiceMessage *tmsg;

			while ((tmsg = (TimerServiceMessage *) GetMsg(port)))
			{
				if (tmsg->tsm_Message.mn_Length == sizeof (TimerServiceMessage))
				{
					switch (tmsg->tsm_MsgID)
					{
						case TSM_MSGID_ADDTIMER:
							if (timers < MAX_TIMERS)
							{
								ULONG unit = UNIT_MICROHZ;

								if (tmsg->tsm_Interval >= 1000000)
									unit = UNIT_VBLANK;
								if (OSystem_MorphOS::OpenATimer(&timer_slots[timers].ts_Port, (IORequest **) &timer_slots[timers].ts_IORequest, unit))
								{
									timer_slots[timers].ts_Callback = tmsg->tsm_Callback;
									timer_slots[timers].ts_Interval = tmsg->tsm_Interval;
									timer_slots[timers].ts_SignalBit = 1 << timer_slots[timers].ts_Port->mp_SigBit;

									signal_mask |= timer_slots[timers].ts_SignalBit;
									timer_bits |= timer_slots[timers].ts_SignalBit;

									timerequest *req = timer_slots[timers].ts_IORequest;
									interval = timer_slots[timers].ts_Interval;
									req->tr_node.io_Command  = TR_ADDREQUEST;
									req->tr_time.tv_secs  = interval/1000000;
									req->tr_time.tv_micro = interval%1000000;
									SendIO((IORequest*) req);

									timers++;
								}
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
									CloseDevice((IORequest *) timer_slots[t].ts_IORequest);
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

				if (tmsg->tsm_Message.mn_ReplyPort)
					ReplyMsg((Message *) tmsg);
				else
					FreeVec((Message *) tmsg);
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
					(*timer_slots[t].ts_Callback)(engine);
					GetSysTime(&end_callback);
					SubTime(&end_callback, &start_callback);
					interval -= end_callback.tv_sec*1000000+end_callback.tv_micro/1000000+40000;
					if (interval < 0)
						interval = 0;

					req->tr_node.io_Command  = TR_ADDREQUEST;
					req->tr_time.tv_secs  = interval/1000000;
					req->tr_time.tv_micro = interval%1000000;
					SendIO((IORequest*) req);
				}
			}
		}
	}

	for (t = 0; t < timers; t++)
	{
		AbortIO((IORequest *) timer_slots[t].ts_IORequest);
		WaitIO((IORequest *) timer_slots[t].ts_IORequest);
		CloseDevice((IORequest *) timer_slots[t].ts_IORequest);
		DeleteIORequest((IORequest *) timer_slots[t].ts_IORequest);
		DeleteMsgPort(timer_slots[t].ts_Port);
	}
}

