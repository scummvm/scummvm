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

#include <devices/timer.h>

#include "stdafx.h"
#include "scumm.h"

#include <dos/dos.h>
#include <exec/memory.h>
#include <devices/ahi.h>
#include <devices/amidi.h>

#define NO_PPCINLINE_STDARG
#define NO_PPCINLINE_VARARGS
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/ahi.h>

extern int GetTicks();
extern Scumm scumm;
extern IMuse sound;
extern SOUND_DRIVER_TYPE snd_driv;

#define AHI_BUF_SIZE	 (8*1024)

struct SignalSemaphore ScummMusicThreadRunning;

static struct MsgPort     *ahiPort    		 = NULL;
static struct AHIRequest  *ahiReq[ 2 ]		 = { NULL, NULL };
static UWORD					ahiCurBuf		 = 0;
static BOOL						ahiReqSent[ 2 ] = { FALSE, FALSE };
static BYTE						ahiDevice		 = -1;
		 UBYTE					ahiUnit  		 = AHI_DEFAULT_UNIT;
static char					  *ahiBuf[ 2 ]		 = { NULL, NULL };

static struct MsgPort 		 *ScummMidiPort = NULL;
		 struct IOMidiRequest *ScummMidiRequest = NULL;

bool init_morphos_sound( ULONG MidiUnit, bool NoMusic )
{
	if( !NoMusic )
	{
		ScummMidiPort = CreateMsgPort();
		if( ScummMidiPort )
		{
			ScummMidiRequest = (struct IOMidiRequest *)CreateIORequest( ScummMidiPort, sizeof( struct IOMidiRequest ) );
			if( ScummMidiRequest )
			{
				ScummMidiRequest->amr_Version = 2;
				if( OpenDevice( "amidi.device", MidiUnit, (struct IORequest *)ScummMidiRequest, AMIDIF_MIDISERVER ) )
				{
					DeleteIORequest( (struct IORequest *)ScummMidiRequest );
					DeleteMsgPort( ScummMidiPort );
					ScummMidiRequest = NULL;
					ScummMidiPort = NULL;
				}
			}
			else
			{
				DeleteMsgPort( ScummMidiPort );
				ScummMidiPort = NULL;
			}
		}

		if( !ScummMidiRequest )
			warning( "Could not open AMidi - music will not play" );
	}

	if( !(ahiPort = CreateMsgPort()) )
		return false;

	if( !(ahiReq[ 0 ] = (struct AHIRequest *)CreateIORequest( ahiPort, sizeof( struct AHIRequest ) )) )
	{
		DeleteMsgPort( ahiPort );
		ahiPort = NULL;
		return false;
	}

	if( !(ahiReq[ 1 ] = (struct AHIRequest *)AllocVec( sizeof( struct AHIRequest ), MEMF_ANY | MEMF_PUBLIC )) )
	{
		DeleteIORequest( ahiReq[ 0 ] );
		DeleteMsgPort( ahiPort );
		ahiReq[ 0 ] = NULL;
		ahiPort = NULL;
		return false;
	}

	if( !(ahiBuf[ 0 ] = (char *)AllocVec( 2*AHI_BUF_SIZE, MEMF_ANY | MEMF_PUBLIC )) )
	{
		FreeVec( ahiReq[ 1 ] );
		DeleteIORequest( ahiReq[ 0 ] );
		DeleteMsgPort( ahiPort );
		ahiReq[ 0 ] = NULL;
		ahiReq[ 1 ] = NULL;
		ahiPort = NULL;
		return false;
	}
	ahiBuf[ 1 ] = &ahiBuf[ 0 ][ AHI_BUF_SIZE ];

	ahiReq[ 0 ]->ahir_Version = 4;
	if( ahiDevice = OpenDevice( AHINAME, 0, (struct IORequest *)ahiReq[ 0 ], 0 ) )
	{
		FreeVec( ahiBuf[ 0 ] );
		FreeVec( ahiReq[ 1 ] );
		DeleteIORequest( ahiReq[ 0 ] );
		DeleteMsgPort( ahiPort );
		ahiBuf[ 0 ] = NULL;
		ahiReq[ 0 ] = NULL;
		ahiReq[ 1 ] = NULL;
		ahiPort = NULL;
		return false;
	}

	CopyMem( ahiReq[ 0 ], ahiReq[ 1 ], sizeof( struct AHIRequest ) );

	return true;
}


void exit_morphos_sound()
{
	if( ScummMidiRequest )
	{
		CloseDevice( (struct IORequest *)ScummMidiRequest );
		DeleteIORequest( (struct IORequest *)ScummMidiRequest );
		DeleteMsgPort( ScummMidiPort );
	}

	if( ahiReq[ 1 ] )
		FreeVec( ahiReq[ 1 ] );

	if( ahiReq[ 0 ] )
	{
		CloseDevice( (struct IORequest *)ahiReq[ 0 ] );
		DeleteIORequest( ahiReq[ 0 ] );
	}

	if( ahiBuf[ 0 ] )
		FreeVec( (APTR)ahiBuf[ 0 ] );

	if( ahiPort )
		DeleteMsgPort( ahiPort );
}


int morphos_music_thread( Scumm *s, ULONG MidiUnit, bool NoMusic )
{
	int  old_time, cur_time;
	bool initialized;
   bool TimerAvailable = false;
	struct MsgPort     *TimerMsgPort;
	struct timerequest *TimerIORequest;

	ObtainSemaphore( &ScummMusicThreadRunning );

	initialized = init_morphos_sound( MidiUnit, NoMusic );
	if( !initialized )
		warning( "Sound could not be initialized" );

	TimerMsgPort = CreateMsgPort();
	if( TimerMsgPort )
	{
		TimerIORequest = (struct timerequest *)CreateIORequest( TimerMsgPort, sizeof( struct timerequest ) );
		if( TimerIORequest )
		{
			if( OpenDevice( "timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIORequest, 0 ) == 0 )
				TimerAvailable = true;
			else
			{
				DeleteIORequest( (struct IORequest *)TimerIORequest );
				DeleteMsgPort( TimerMsgPort );
			}
		}
		else
			DeleteMsgPort( TimerMsgPort );
	}

	if( !TimerAvailable )
	{
		warning( "ScummVM Music Thread: no timer available! Sound and music will be disabled" );
		Wait( SIGBREAKF_CTRL_F );
	}
	else
	{
		old_time = GetTicks();

		for(;;)
		{
			if( CheckSignal( SIGBREAKF_CTRL_F ) )
			{
				if( ahiReqSent[ ahiCurBuf ] )
				{
					AbortIO( (struct IORequest *)ahiReq[ ahiCurBuf ] );
					WaitIO ( (struct IORequest *)ahiReq[ ahiCurBuf ] );
				}
				break;
			}

			if( !snd_driv.wave_based() )
			{
				cur_time = GetTicks();
				while( old_time < cur_time )
				{
					old_time += 10;
					sound.on_timer();
				}
/*				  TimerIORequest->tr_time.tv_micro = (old_time-cur_time)*1000;
				if( TimerIORequest->tr_time.tv_micro == 0 )
					TimerIORequest->tr_time.tv_micro = 100;*/
				TimerIORequest->tr_time.tv_micro = 10000;
			}
			else
				TimerIORequest->tr_time.tv_micro = 10000;

			TimerIORequest->tr_node.io_Command  = TR_ADDREQUEST;
			TimerIORequest->tr_time.tv_secs  = 0;
			DoIO( (struct IORequest *)TimerIORequest );

			if( !initialized )
				continue;

			if( !ahiReqSent[ ahiCurBuf ] || CheckIO( (struct IORequest *)ahiReq[ ahiCurBuf ] ) )
			{
				struct AHIRequest *req = ahiReq[ ahiCurBuf ];
				UWORD ahiOtherBuf = !ahiCurBuf;

				if( ahiReqSent[ ahiCurBuf ] )
					WaitIO( (struct IORequest *)req );

				if( CheckSignal( SIGBREAKF_CTRL_F ) )
					break;

				scumm.mixWaves( (int16 *)ahiBuf[ ahiCurBuf ], AHI_BUF_SIZE >> 1 );

				req->ahir_Std.io_Message.mn_Node.ln_Pri = 0;
				req->ahir_Std.io_Command = CMD_WRITE;
				req->ahir_Std.io_Data    = ahiBuf[ ahiCurBuf ];
				req->ahir_Std.io_Length  = AHI_BUF_SIZE;
				req->ahir_Type				 = AHIST_M16S;
				req->ahir_Frequency		 = SAMPLES_PER_SEC;
				req->ahir_Position  		 = 0x8000;
				req->ahir_Volume			 = 0x10000;
				req->ahir_Link				 = (ahiReqSent[ ahiOtherBuf ] && !CheckIO( (struct IORequest *)ahiReq[ ahiOtherBuf ] )) ? ahiReq[ ahiOtherBuf ] : NULL;
				SendIO( (struct IORequest *)req );

				ahiReqSent[ ahiCurBuf ] = TRUE;
				ahiCurBuf = ahiOtherBuf;
			}
		}
	}

	if( TimerAvailable )
	{
		CloseDevice( (struct IORequest *)TimerIORequest );
		DeleteIORequest( (struct IORequest *)TimerIORequest );
		DeleteMsgPort( TimerMsgPort );
	}

	exit_morphos_sound();

	ReleaseSemaphore( &ScummMusicThreadRunning );
	return 0;
}

