/* Copyright (C) 1994-2003 Revolution Software Ltd
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

//--------------------------------------------------------------------------------------
//								BROKEN SWORD 2
//
//	SOUND.CPP		Contains the sound engine, fx & music functions
//					Some very 'sound' code in here ;)
//
//	(16Dec96 JEL)
//
//--------------------------------------------------------------------------------------

#include <stdio.h>

#include "stdafx.h"
//#include "src\driver96.h"
#include "console.h"
#include "defs.h"	// for RESULT
#include "interpreter.h"
#include "protocol.h"	// for FetchObjectName() for debugging FN_play_fx
#include "resman.h"
#include "sound.h"
#include "sword2.h"

//--------------------------------------------------------------------------------------
typedef struct
{
	uint32	resource;		// resource id of sample
	uint32	fetchId;		// Id of resource in PSX CD queue. :)
	uint16	delay;			// cycles to wait before playing (or 'random chance' if FX_RANDOM)
	uint8	volume;			// 0..16
	int8	pan;			// -16..16
	uint8	type;			// FX_SPOT, FX_RANDOM or FX_LOOP
} _fxq_entry;

#define FXQ_LENGTH 32		// max number of fx in queue at once [DO NOT EXCEED 255]

_fxq_entry fxq[FXQ_LENGTH];

//--------------------------------------------------------------------------------------

uint32 looping_music_id=0;	// used to store id of tunes that loop, for save & restore
char musicDirectory[120];

//--------------------------------------------------------------------------------------
// local function prototypes

void Trigger_fx (uint8 j);

//--------------------------------------------------------------------------------------
// initialise the fxq by clearing all the entries

void Init_fx_queue(void)
{
	uint8 j;


	for (j=0; j < FXQ_LENGTH; j++)	// scan the queue
	{
		fxq[j].resource = 0;		// 0 resource means 'empty' slot
		fxq[j].fetchId	= 0;		// Not being fetched.
	}
}

//--------------------------------------------------------------------------------------
// process the fxq once every game cycle

void Process_fx_queue(void)
{
	uint8 j;	// assuming FXQ_LENGTH is 255 or less


	for (j=0; j < FXQ_LENGTH; j++)				// scan the queue
	{
		if (fxq[j].resource)					// if this entry isn't empty
		{
			if (fxq[j].type == FX_RANDOM)		// if it's type FX_RANDOM
			{
				if (rand()%(fxq[j].delay)==0)	// 1 in 'delay' chance of this fx occurring
				{	
					Trigger_fx(j);				// play it
				}

			}
			else if(fxq[j].type == FX_SPOT)
			{
				if (fxq[j].delay)				// if delay is above 0
					fxq[j].delay--;				// decrement delay countdown
				else							// if zero delay remaining
				{
					Trigger_fx(j);				// play it
					fxq[j].type = FX_SPOT2;
				}
			}
			else if (fxq[j].type == FX_SPOT2)
			{
				if (g_sword2->_sound->IsFxOpen(j+1))
					fxq[j].resource = 0;		// Once the Fx has finished remove it from the queue.
			}
		}
	}
}

//--------------------------------------------------------------------------------------
void Trigger_fx(uint8 j)	// called from Process_fx_queue only
{
	uint8 *data;
	int32 id;
	uint32 rv;

	id = (uint32)j+1;	// because 0 is not a valid id

	if (fxq[j].type == FX_SPOT)
	{
		data = res_man.Res_open(fxq[j].resource);						// load in the sample
		data += sizeof(_standardHeader);
		rv = g_sword2->_sound->PlayFx( id, data, fxq[j].volume, fxq[j].pan, RDSE_FXSPOT );		// wav data gets copied to sound memory
		res_man.Res_close(fxq[j].resource);								// release the sample
//		fxq[j].resource = 0;											// clear spot fx from queue
	}
	else	// random & looped fx are already loaded into sound memory by FN_play_fx()
	{		// - to be referenced by 'j', so pass NULL data

		if (fxq[j].type == FX_RANDOM)
			rv = g_sword2->_sound->PlayFx( id, NULL, fxq[j].volume, fxq[j].pan, RDSE_FXSPOT );	// not looped
		else			// FX_LOOP
			rv = g_sword2->_sound->PlayFx( id, NULL, fxq[j].volume, fxq[j].pan, RDSE_FXLOOP );	// looped
	}

	#ifdef _SWORD2_DEBUG
	if (rv)
		Zdebug("SFX ERROR: PlayFx() returned %.8x (%s line %u)", rv, __FILE__, __LINE__);
	#endif
}

//--------------------------------------------------------------------------------------
int32 FN_play_fx(int32 *params)		// called from script only
{
	// params:	0 sample resource id
	//			1 type		(FX_SPOT, FX_RANDOM, FX_LOOP)
	//			2 delay		(0..65535)
	//			3 volume	(0..16)
	//			4 pan		(-16..16)

	// example script:	FN_play_fx (FXWATER, FX_LOOP, 0, 10, 15);
	//					fx_water = result;	// fx_water is just a local script flag
	//					.
	//					.
	//					.
	//					FN_stop_fx (fx_water);

	uint8	j=0;
	uint8	*data;
	uint32	id;
	uint32 rv;

	//----------------------------------
	#ifdef _SWORD2_DEBUG

	_standardHeader *header;
	char type[10];


	if (wantSfxDebug)
	{
		switch (params[1])	// 'type'
		{
			case FX_SPOT:
				strcpy(type,"SPOT");
				break;

			case FX_LOOP:
				strcpy(type,"LOOPED");
				break;

			case FX_RANDOM:
				strcpy(type,"RANDOM");
				break;

			default:
				strcpy(type,"INVALID");
		}

		Zdebug("SFX (sample=\"%s\", vol=%d, pan=%d, delay=%d, type=%s)", FetchObjectName(params[0]), params[3], params[4], params[2], type);
	}

	#endif	//_SWORD2_DEBUG
	//----------------------------------

	while ((j < FXQ_LENGTH) && (fxq[j].resource != 0))
		j++;

	if (j==FXQ_LENGTH)
	{
		return (IR_CONT);
//		Con_fatal_error("ERROR: Sound queue overflow in FN_play_fx() (%s line %u)",__FILE__,__LINE__);
	}
	else
	{
		fxq[j].resource	= params[0];			// wav resource id
		fxq[j].type		= params[1];			// FX_SPOT, FX_LOOP or FX_RANDOM

		if (fxq[j].type == FX_RANDOM)			// FX_RANDOM:
			fxq[j].delay = params[2] * 12 + 1;	//  'delay' param is the intended average no. seconds between playing this effect (+1 to avoid divide-by-zero in Process_fx_queue)
		else									// FX_SPOT or FX_LOOP:
 			fxq[j].delay = params[2];			//  'delay' is no. frames to wait before playing

		fxq[j].volume	= params[3];			// 0..16
		fxq[j].pan		= params[4];			// -16..16


		if (fxq[j].type == FX_SPOT)						// spot fx
		{
			#ifdef _SWORD2_DEBUG
 			data = res_man.Res_open(fxq[j].resource);	// "pre-load" the sample; this gets it into memory
			header = (_standardHeader*)data;
			if (header->fileType != WAV_FILE)
				Con_fatal_error("FN_play_fx given invalid resource (%s line %u)",__FILE__,__LINE__);
			#else
			res_man.Res_open(fxq[j].resource);			// "pre-load" the sample; this gets it into memory
			#endif
			res_man.Res_close(fxq[j].resource);			// but then releases it to "age" out if the space is needed
		}
		else											// random & looped fx
		{
			id = (uint32)j+1;							// because 0 is not a valid id

			data = res_man.Res_open(fxq[j].resource);	// load in the sample

			#ifdef _SWORD2_DEBUG
			header = (_standardHeader*)data;
			if (header->fileType != WAV_FILE)
				Con_fatal_error("FN_play_fx given invalid resource (%s line %u)",__FILE__,__LINE__);
			#endif

			data += sizeof(_standardHeader);
			rv = g_sword2->_sound->OpenFx(id,data);							// copy it to sound memory, using position in queue as 'id'

			#ifdef _SWORD2_DEBUG
			if (rv)
				Zdebug("SFX ERROR: OpenFx() returned %.8x (%s line %u)", rv, __FILE__, __LINE__);
			#endif

			res_man.Res_close(fxq[j].resource);			// release the sample
		}
	}


	//---------------------------------------------
	// (James07uag97)
	if (fxq[j].type == FX_LOOP)		// looped fx
		Trigger_fx(j);				// play now, rather than in Process_fx_queue where it was getting played again & again!
 	//---------------------------------------------


	RESULT = j;	// in case we want to call FN_stop_fx() later, to kill this fx (mainly for FX_LOOP & FX_RANDOM)

	return(IR_CONT);	//	continue script
}

//--------------------------------------------------------------------------------------
int32 FN_sound_fetch(int32 *params)
{
	return (IR_CONT);
}
//--------------------------------------------------------------------------------------
// to alter the volume and pan of a currently playing fx
int32 FN_set_fx_vol_and_pan(int32 *params)
{
//	params	0	id of fx (ie. the id returned in 'result' from FN_play_fx
//			1	new volume (0..16)
//			2	new pan (-16..16)

//	SetFxVolumePan(int32 id, uint8 vol, uint8 pan);
	g_sword2->_sound->SetFxVolumePan(1+params[0], params[1], params[2]);	// driver fx_id is 1+<pos in queue>
//	Zdebug("%d",params[2]);

	return (IR_CONT);
}
//--------------------------------------------------------------------------------------
// to alter the volume  of a currently playing fx
int32 FN_set_fx_vol(int32 *params)
{
//	params	0	id of fx (ie. the id returned in 'result' from FN_play_fx
//			1	new volume (0..16)

//	SetFxIdVolume(int32 id, uint8 vol);
	g_sword2->_sound->SetFxIdVolume(1+params[0], params[1]);

	return (IR_CONT);
}
//--------------------------------------------------------------------------------------
int32 FN_stop_fx(int32 *params)		// called from script only
{
	// params:	0 position in queue

	// This will stop looped & random fx instantly, and remove the fx from the queue.
	// So although it doesn't stop spot fx, it will remove them from the queue if they haven't yet played

	uint8	j = (uint8) params[0];
	uint32	id;
	uint32	rv;

	if ((fxq[j].type == FX_RANDOM) || (fxq[j].type == FX_LOOP))
	{
		id = (uint32)j+1;	// because 0 is not a valid id
		rv = g_sword2->_sound->CloseFx(id);		// stop fx & remove sample from sound memory

		#ifdef _SWORD2_DEBUG
		if (rv)
			Zdebug("SFX ERROR: CloseFx() returned %.8x (%s line %u)", rv, __FILE__, __LINE__);
		#endif
	}

	fxq[j].resource = 0;	// remove from queue


	return(IR_CONT);		//	continue script
}

//--------------------------------------------------------------------------------------
int32 FN_stop_all_fx(int32 *params)		// called from script only
{
	// Stops all looped & random fx and clears the entire queue
	// NO PARAMS

	Clear_fx_queue();

	return(IR_CONT);		//	continue script
}
//--------------------------------------------------------------------------------------
// Stops all looped & random fx and clears the entire queue

void Clear_fx_queue(void)
{
	g_sword2->_sound->ClearAllFx();			// stop all fx & remove the samples from sound memory
	Init_fx_queue();		// clean out the queue
}

//--------------------------------------------------------------------------------------

//=============================================================================
//	int32 StreamMusic(uint8 *filename, int32 loopFlag)
//
//	Streams music from the file defined by filename.  The loopFlag should
//	be set to RDSE_FXLOOP if the music is to loop back to the start.
//	Otherwise, it should be RDSE_FXSPOT.
//	The return value must be checked for any problems.
//
//	--------------------------------------------------------------------------
//
//	int32 PauseMusic(void)
//
//	Stops the music dead in it's tracks.
//
//	--------------------------------------------------------------------------
//
//	int32 UnpauseMusic(void)
//
//	Re-starts the music from where it was stopped.
//
//=============================================================================
int32 FN_prepare_music(int32 *params)
{
	return (IR_CONT);
}

//--------------------------------------------------------------------------------------
// Start a tune playing, to play once or to loop until stopped or next one played
int32 FN_play_music(int32 *params)		// updated by James on 10apr97
{
	// params	0 tune id
	//			1 loop flag (0 or 1)

	char	filename[128];
	uint32	loopFlag;
	uint32	rv;	// drivers return value


//	Zdebug("FN_play_music(%d)", params[0]);

	if (params[1]==FX_LOOP)				// if it is to loop
	{
		loopFlag = RDSE_FXLOOP;
		looping_music_id = params[0];	// keep a note of the id, for restarting after an interruption to gameplay
	}
	else								// just play once
	{
 		loopFlag = RDSE_FXSPOT;
		looping_music_id = 0;			// don't need to restart this tune after control panel or restore
	}


	// add the appropriate file extension & play it

	if (g_sword2->_gameId == GID_SWORD2_DEMO)
		sprintf(filename,"MUSIC.CLU");
	else
		sprintf(filename,"%sCLUSTERS/MUSIC.CLU", res_man.GetCdPath());

	rv = g_sword2->_sound->StreamCompMusic(filename, g_sword2->getGameDataPath(), params[0], loopFlag);

	#ifdef _SWORD2_DEBUG
		if (rv)
	 		Zdebug("ERROR: StreamCompMusic(%s, %d, %d) returned error 0x%.8x", filename, params[0], loopFlag, rv);
	#endif

//	Zdebug("FN_play_music(%d) returning", params[0]);

	return(IR_CONT);	//	continue script
}

//--------------------------------------------------------------------------------------
int32 FN_stop_music(int32 *params)		// called from script only
{
	// params:	none


	looping_music_id=0;		// clear the 'looping' flag

	g_sword2->_sound->StopMusic();

	if (params);

	return(IR_CONT);	//	continue script
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
void Kill_music(void)	// James22aug97
{
	uint8 count;

	looping_music_id=0;		// clear the 'looping' flag
	g_sword2->_sound->StopMusic();

	// THIS BIT CAUSES THE MUSIC TO STOP INSTANTLY!
	for(count=0; count<16; count++)
		g_sword2->_sound->UpdateCompSampleStreaming();
}
//--------------------------------------------------------------------------------------
int32 FN_check_music_playing(int32 *params)		// James (30july97)
{

	// params: none
	// sets result to no. of seconds of current tune remaining
	// or 0 if no music playing

	RESULT = g_sword2->_sound->MusicTimeRemaining();	// in seconds, rounded up to the nearest second

	return(IR_CONT);	//	continue script
}
//--------------------------------------------------------------------------------------
void PauseAllSound(void)	// James25july97
{
	uint32	rv;	// for drivers return value

	rv = g_sword2->_sound->PauseMusic();
	if (rv != RD_OK)
		Zdebug("ERROR: PauseMusic() returned %.8x in PauseAllSound()", rv);

	rv = g_sword2->_sound->PauseSpeech();
	if (rv != RD_OK)
		Zdebug("ERROR: PauseSpeech() returned %.8x in PauseAllSound()", rv);

	rv = g_sword2->_sound->PauseFx();
	if (rv != RD_OK)
		Zdebug("ERROR: PauseFx() returned %.8x in PauseAllSound()", rv);
}
//--------------------------------------------------------------------------------------
void UnpauseAllSound(void)	// James25july97
{
	uint32	rv;	// for drivers return value

	rv = g_sword2->_sound->UnpauseMusic();
	if (rv != RD_OK)
		Zdebug("ERROR: UnpauseMusic() returned %.8x in UnpauseAllSound()", rv);

	rv = g_sword2->_sound->UnpauseSpeech();
	if (rv != RD_OK)
		Zdebug("ERROR: UnpauseSpeech() returned %.8x in UnpauseAllSound()", rv);

	rv = g_sword2->_sound->UnpauseFx();
 	if (rv != RD_OK)
		Zdebug("ERROR: UnpauseFx() returned %.8x in UnpauseAllSound()", rv);
}
//--------------------------------------------------------------------------------------

