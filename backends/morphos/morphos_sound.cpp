/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 Rüdiger Hanke
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * MorphOS sound support
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "engine.h"

#include <dos/dos.h>
#include <exec/memory.h>
#include <devices/ahi.h>
#include <devices/etude.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/ahi.h>

#include "morphos.h"
#include "morphos_sound.h"

#define AHI_BUF_SIZE	 (8*1024)

SignalSemaphore ScummMusicThreadRunning;
SignalSemaphore ScummSoundThreadRunning;

static MsgPort    *ahiPort    	  = NULL;
static AHIRequest *ahiReq[2]	   = { NULL, NULL };
static UWORD		ahiCurBuf		 = 0;
static bool			ahiReqSent[2] = { false, false };
static BYTE			ahiDevice		 = -1;
		 UBYTE		ahiUnit  		 = AHI_DEFAULT_UNIT;
static char			*ahiBuf[2]	   = { NULL, NULL };

static MsgPort 	   *ScummMidiPort = NULL;
		 IOMidiRequest *ScummMidiRequest = NULL;

		 Device        *EtudeBase = NULL;

bool etude_available()
{
	bool avail = init_morphos_music(ScummMidiUnit, ETUDEF_DIRECT);
	if (avail)
		exit_morphos_music();
	return avail;
}

bool init_morphos_music(ULONG MidiUnit, ULONG DevFlags)
{
	ScummMidiPort = CreateMsgPort();
	if (ScummMidiPort)
	{
		ScummMidiRequest = (IOMidiRequest *) CreateIORequest(ScummMidiPort, sizeof (IOMidiRequest));
		if (ScummMidiRequest)
		{
			ScummMidiRequest->emr_Version = 1;
			if (OpenDevice(ETUDENAME, MidiUnit, (IORequest *) ScummMidiRequest, DevFlags))
			{
				DeleteIORequest((IORequest *) ScummMidiRequest);
				DeleteMsgPort(ScummMidiPort);
				ScummMidiRequest = NULL;
				ScummMidiPort = NULL;
			}
			else
				EtudeBase = ScummMidiRequest->emr_Std.io_Device;
		}
		else
		{
			DeleteMsgPort(ScummMidiPort);
			ScummMidiPort = NULL;
		}
	}

	if (!ScummMidiRequest)
		return false;

	return true;
}


void exit_morphos_music()
{
	if (ScummMidiRequest)
	{
		CloseDevice((IORequest *) ScummMidiRequest);
		DeleteIORequest((IORequest *) ScummMidiRequest);
		DeleteMsgPort(ScummMidiPort);
		ScummMidiRequest = NULL;
		ScummMidiPort = NULL;
		EtudeBase = NULL;
	}
}


static bool init_morphos_sound()
{
	if (!(ahiPort = CreateMsgPort()))
		return false;

	if (!(ahiReq[0] = (AHIRequest *) CreateIORequest(ahiPort, sizeof (AHIRequest))))
	{
		DeleteMsgPort(ahiPort);
		ahiPort = NULL;
		return false;
	}

	if (!(ahiReq[1] = (AHIRequest *) AllocVec(sizeof (AHIRequest), MEMF_PUBLIC)))
	{
		DeleteIORequest(ahiReq[0]);
		DeleteMsgPort(ahiPort);
		ahiReq[0] = NULL;
		ahiPort = NULL;
		return false;
	}

	if (!(ahiBuf[0] = (char *) AllocVec(2*AHI_BUF_SIZE, MEMF_PUBLIC)))
	{
		FreeVec(ahiReq[1]);
		DeleteIORequest(ahiReq[0]);
		DeleteMsgPort(ahiPort);
		ahiReq[0] = NULL;
		ahiReq[1] = NULL;
		ahiPort = NULL;
		return false;
	}
	ahiBuf[1] = &ahiBuf[0][AHI_BUF_SIZE];

	ahiReq[0]->ahir_Version = 4;
	if ((ahiDevice = OpenDevice(AHINAME, 0, (IORequest *) ahiReq[0], 0)))
	{
		FreeVec(ahiBuf[0]);
		FreeVec(ahiReq[1]);
		DeleteIORequest(ahiReq[0]);
		DeleteMsgPort(ahiPort);
		ahiBuf[0] = NULL;
		ahiReq[0] = NULL;
		ahiReq[1] = NULL;
		ahiPort = NULL;
		return false;
	}

	CopyMem(ahiReq[0], ahiReq[1], sizeof (AHIRequest));

	ahiCurBuf = 0;
	ahiReqSent[0] = FALSE;
	ahiReqSent[1] = FALSE;

	return true;
}


static void exit_morphos_sound()
{
	if (ahiReq[1])
		FreeVec(ahiReq[1]);

	if (ahiReq[0])
	{
		CloseDevice((IORequest *) ahiReq[0]);
		DeleteIORequest(ahiReq[0]);
	}

	if (ahiBuf[0])
		FreeVec((APTR) ahiBuf[0]);

	if (ahiPort)
		DeleteMsgPort(ahiPort);
}

int morphos_sound_thread(OSystem_MorphOS *syst, ULONG SampleType)
{
	ULONG signals;
	bool  initialized;

	initialized = init_morphos_sound();
	if (!initialized)
	{
		warning("Sound could not be initialized. The game may hang at some point (press Ctrl-z then).");
		Wait(SIGBREAKF_CTRL_C);
	}
	else
	{
		for (;;)
		{
			while (!ahiReqSent[ahiCurBuf] || CheckIO((IORequest *) ahiReq[ahiCurBuf]))
			{
				AHIRequest *req = ahiReq[ahiCurBuf];
				UWORD ahiOtherBuf = !ahiCurBuf;

				if (ahiReqSent[ahiCurBuf])
					WaitIO((IORequest *) req);

				syst->fill_sound((byte *) ahiBuf[ahiCurBuf], AHI_BUF_SIZE);
	
				req->ahir_Std.io_Message.mn_Node.ln_Pri = 0;
				req->ahir_Std.io_Command = CMD_WRITE;
				req->ahir_Std.io_Data    = ahiBuf[ahiCurBuf];
				req->ahir_Std.io_Length  = AHI_BUF_SIZE;
				req->ahir_Type				 = SampleType;
				req->ahir_Frequency		 = SAMPLES_PER_SEC;
				req->ahir_Position  		 = 0x8000;
				req->ahir_Volume			 = 0x10000;
				req->ahir_Link				 = (ahiReqSent[ahiOtherBuf] && !CheckIO((IORequest *) ahiReq[ahiOtherBuf])) ? ahiReq[ahiOtherBuf] : NULL;
				SendIO((IORequest *)req);

				ahiReqSent[ahiCurBuf] = true;
				ahiCurBuf = ahiOtherBuf;
			}

			signals = Wait(SIGBREAKF_CTRL_C | (1 << ahiPort->mp_SigBit));

			if (signals & SIGBREAKF_CTRL_C)
				break;
		}

		if (ahiReqSent[ahiCurBuf])
		{
			AbortIO((IORequest *) ahiReq[ahiCurBuf]);
			WaitIO((IORequest *) ahiReq[ahiCurBuf]);
			ahiReqSent[ahiCurBuf] = false;
		}

		if (ahiReqSent[!ahiCurBuf])
		{
			AbortIO((IORequest *) ahiReq[!ahiCurBuf]);
			WaitIO((IORequest *) ahiReq[!ahiCurBuf]);
			ahiReqSent[!ahiCurBuf] = false;
		}
	}

	exit_morphos_sound();

	return 0;
}

