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

// ---------------------------------------------------------------------------
//								BROKEN SWORD 2
//
//	SOUND.CPP	Contains the sound engine, fx & music functions
//			Some very 'sound' code in here ;)
//
//	(16Dec96 JEL)
//
// ---------------------------------------------------------------------------

#include <stdio.h>

#include "stdafx.h"
#include "console.h"
#include "defs.h"		// for RESULT
#include "interpreter.h"
#include "protocol.h"		// for FetchObjectName() for debugging FN_play_fx
#include "resman.h"
#include "sound.h"
#include "sword2.h"

typedef struct {
	uint32 resource;	// resource id of sample
	uint32 fetchId;		// Id of resource in PSX CD queue. :)
	uint16 delay;		// cycles to wait before playing (or 'random chance' if FX_RANDOM)
	uint8 volume;		// 0..16
	int8 pan;		// -16..16
	uint8 type;		// FX_SPOT, FX_RANDOM or FX_LOOP
} _fxq_entry;

// max number of fx in queue at once [DO NOT EXCEED 255]
#define FXQ_LENGTH 32

_fxq_entry fxq[FXQ_LENGTH];

// used to store id of tunes that loop, for save & restore
uint32 looping_music_id = 0;

char musicDirectory[120];

void Trigger_fx(uint8 j);

// initialise the fxq by clearing all the entries

void Init_fx_queue(void) {
	for (int j = 0; j < FXQ_LENGTH; j++) {
		fxq[j].resource = 0;	// 0 resource means 'empty' slot
		fxq[j].fetchId = 0;	// Not being fetched.
	}
}

// process the fxq once every game cycle

void Process_fx_queue(void) {
	for (int j = 0; j < FXQ_LENGTH; j++) {
		if (!fxq[j].resource)
			continue;

		switch (fxq[j].type) {
		case FX_RANDOM:
			// 1 in 'delay' chance of this fx occurring
			if (rand() % fxq[j].delay == 0)
				Trigger_fx(j);
			break;
		case FX_SPOT:
			if (fxq[j].delay)
				fxq[j].delay--;
			else {
				Trigger_fx(j);
				fxq[j].type = FX_SPOT2;
			}
			break;
		case FX_SPOT2:
			// Once the Fx has finished remove it from the queue.
			if (g_sound->IsFxOpen(j + 1))
				fxq[j].resource = 0;
			break;
		}
	}
}

// called from Process_fx_queue only

void Trigger_fx(uint8 j) {
	uint8 *data;
	int32 id;
	uint32 rv;

	id = (uint32) j + 1;	// because 0 is not a valid id

	if (fxq[j].type == FX_SPOT) {
		// load in the sample
		data = res_man.open(fxq[j].resource);
		data += sizeof(_standardHeader);
		// wav data gets copied to sound memory
		rv = g_sound->PlayFx(id, data, fxq[j].volume, fxq[j].pan, RDSE_FXSPOT);
		// release the sample
		res_man.close(fxq[j].resource);
	} else {
		// random & looped fx are already loaded into sound memory
		// by FN_play_fx()
		// - to be referenced by 'j', so pass NULL data

		if (fxq[j].type == FX_RANDOM) {
			// Not looped
			rv = g_sound->PlayFx(id, NULL, fxq[j].volume, fxq[j].pan, RDSE_FXSPOT);
		} else {
			// Looped
			rv = g_sound->PlayFx(id, NULL, fxq[j].volume, fxq[j].pan, RDSE_FXLOOP);
		}
	}

	if (rv)
		debug(5, "SFX ERROR: PlayFx() returned %.8x", rv);
}

// called from script only

int32 FN_play_fx(int32 *params) {
	// params:	0 sample resource id
	//		1 type		(FX_SPOT, FX_RANDOM, FX_LOOP)
	//		2 delay		(0..65535)
	//		3 volume	(0..16)
	//		4 pan		(-16..16)

	// example script:
	//		FN_play_fx (FXWATER, FX_LOOP, 0, 10, 15);
	//		// fx_water is just a local script flag
	//		fx_water = result;
	//		.
	//		.
	//		.
	//		FN_stop_fx (fx_water);

	uint8 j = 0;
	uint8 *data;
	uint32 id;
	uint32 rv;

#ifdef _SWORD2_DEBUG
	_standardHeader *header;
	char type[10];

	if (wantSfxDebug) {
		switch (params[1]) {	// 'type'
		case FX_SPOT:
			strcpy(type, "SPOT");
			break;
		case FX_LOOP:
			strcpy(type, "LOOPED");
			break;
		case FX_RANDOM:
			strcpy(type, "RANDOM");
			break;
		default:
			strcpy(type, "INVALID");
		}

		debug(5, "SFX (sample=\"%s\", vol=%d, pan=%d, delay=%d, type=%s)", FetchObjectName(params[0]), params[3], params[4], params[2], type);
	}
#endif

	while (j < FXQ_LENGTH && fxq[j].resource != 0)
		j++;

	if (j == FXQ_LENGTH)
		return IR_CONT;

	fxq[j].resource	= params[0];	// wav resource id
	fxq[j].type = params[1];	// FX_SPOT, FX_LOOP or FX_RANDOM

	if (fxq[j].type == FX_RANDOM) {
		// 'delay' param is the intended average no. seconds between
		// playing this effect (+1 to avoid divide-by-zero in
		// Process_fx_queue)
		fxq[j].delay = params[2] * 12 + 1;
	} else {
		// FX_SPOT or FX_LOOP:
		//  'delay' is no. frames to wait before playing
		fxq[j].delay = params[2];
	}

	fxq[j].volume = params[3];	// 0..16
	fxq[j].pan = params[4];		// -16..16

	if (fxq[j].type == FX_SPOT) {
		// "pre-load" the sample; this gets it into memory
		data = res_man.open(fxq[j].resource);

#ifdef _SWORD2_DEBUG
		header = (_standardHeader*) data;
		if (header->fileType != WAV_FILE)
			Con_fatal_error("FN_play_fx given invalid resource");
#endif

		// but then releases it to "age" out if the space is needed
		res_man.close(fxq[j].resource);
	} else {
		// random & looped fx

		id = (uint32) j + 1;	// because 0 is not a valid id

		// load in the sample
		data = res_man.open(fxq[j].resource);

#ifdef _SWORD2_DEBUG
		header = (_standardHeader*)data;
		if (header->fileType != WAV_FILE)
			Con_fatal_error("FN_play_fx given invalid resource");
#endif

		data += sizeof(_standardHeader);

		// copy it to sound memory, using position in queue as 'id'
		rv = g_sound->OpenFx(id, data);

		if (rv)
			debug(5, "SFX ERROR: OpenFx() returned %.8x", rv);

		// release the sample
		res_man.close(fxq[j].resource);
	}

	// (James07uag97)

	if (fxq[j].type == FX_LOOP) {
		// play now, rather than in Process_fx_queue where it was
		// getting played again & again!
		Trigger_fx(j);
	}

	// in case we want to call FN_stop_fx() later, to kill this fx
	// (mainly for FX_LOOP & FX_RANDOM)

	RESULT = j;
	return IR_CONT;
}

int32 FN_sound_fetch(int32 *params) {
	return (IR_CONT);
}

// to alter the volume and pan of a currently playing fx

int32 FN_set_fx_vol_and_pan(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  FN_play_fx
	//		1 new volume (0..16)
	//		2 new pan (-16..16)

	debug(5, "FN_set_fx_vol_and_pan(%d, %d, %d)", params[0], params[1], params[2]);

	// SetFxVolumePan(int32 id, uint8 vol, uint8 pan);
	// driver fx_id is 1 + <pos in queue>
	g_sound->SetFxVolumePan(1 + params[0], params[1], params[2]);
	return IR_CONT;
}

// to alter the volume  of a currently playing fx

int32 FN_set_fx_vol(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  FN_play_fx
	//		1 new volume (0..16)

	// SetFxIdVolume(int32 id, uint8 vol);
	g_sound->SetFxIdVolume(1 + params[0], params[1]);
	return IR_CONT;
}

// called from script only

int32 FN_stop_fx(int32 *params) {
	// params:	0 position in queue

	// This will stop looped & random fx instantly, and remove the fx
	// from the queue. So although it doesn't stop spot fx, it will
	// remove them from the queue if they haven't yet played

	uint8 j = (uint8) params[0];
	uint32 id;
	uint32 rv;

	if (fxq[j].type == FX_RANDOM || fxq[j].type == FX_LOOP) {
		id = (uint32) j + 1;		// because 0 is not a valid id

		// stop fx & remove sample from sound memory
		rv = g_sound->CloseFx(id);

		if (rv)
			debug(5, "SFX ERROR: CloseFx() returned %.8x", rv);
	}

	// remove from queue
	fxq[j].resource = 0;

	return IR_CONT;
}

// called from script only

int32 FN_stop_all_fx(int32 *params) {
	// Stops all looped & random fx and clears the entire queue
	// params:	none

	Clear_fx_queue();
	return IR_CONT;
}

// Stops all looped & random fx and clears the entire queue

void Clear_fx_queue(void) {
	// stop all fx & remove the samples from sound memory
	g_sound->ClearAllFx();

	// clean out the queue
	Init_fx_queue();
}

// ===========================================================================
//	int32 StreamMusic(uint8 *filename, int32 loopFlag)
//
//	Streams music from the file defined by filename.  The loopFlag should
//	be set to RDSE_FXLOOP if the music is to loop back to the start.
//	Otherwise, it should be RDSE_FXSPOT.
//	The return value must be checked for any problems.
//
// ---------------------------------------------------------------------------
//
//	int32 PauseMusic(void)
//
//	Stops the music dead in it's tracks.
//
// ---------------------------------------------------------------------------
//
//	int32 UnpauseMusic(void)
//
//	Re-starts the music from where it was stopped.
//
// ===========================================================================

int32 FN_prepare_music(int32 *params) {
	return IR_CONT;
}

// Start a tune playing, to play once or to loop until stopped or next one
// played

int32 FN_play_music(int32 *params) {
	// params:	0 tune id
	//		1 loop flag (0 or 1)

	char filename[128];
	bool loopFlag;
	uint32 rv;

	debug(5, "FN_play_music(%d, %d)", params[0], params[1]);

	if (params[1] == FX_LOOP) {
		loopFlag = true;

		// keep a note of the id, for restarting after an
		// interruption to gameplay
		looping_music_id = params[0];
	} else {
 		loopFlag = false;

		// don't need to restart this tune after control panel or
		// restore
		looping_music_id = 0;
	}

	// add the appropriate file extension & play it

	if (g_sword2->_gameId == GID_SWORD2_DEMO) {
		// The demo I found didn't come with any music file, but you
		// could use the music from the first CD of the complete game,
		// I suppose...
		strcpy(filename, "music.clu");
	} else {
		File f;

		sprintf(filename, "music%d.clu", res_man.whichCd());
		if (f.open(filename))
			f.close();
		else
			strcpy(filename, "music.clu");
	}

	rv = g_sound->StreamCompMusic(filename, params[0], loopFlag);

	if (rv)
		debug(5, "ERROR: StreamCompMusic(%s, %d, %d) returned error 0x%.8x", filename, params[0], loopFlag, rv);

	return IR_CONT;
}

int32 FN_stop_music(int32 *params) {	// called from script only
	// params:	none

	looping_music_id = 0;		// clear the 'looping' flag
	g_sound->StopMusic();
	return IR_CONT;
}

void Kill_music(void) {			// James22aug97
	looping_music_id = 0;		// clear the 'looping' flag
	g_sound->StopMusic();
}

int32 FN_check_music_playing(int32 *params) {
	// params:	none

	// sets result to no. of seconds of current tune remaining
	// or 0 if no music playing

	// in seconds, rounded up to the nearest second
	RESULT = g_sound->MusicTimeRemaining();

	return IR_CONT;
}

void PauseAllSound(void) {
	uint32	rv;

	rv = g_sound->PauseMusic();
	if (rv != RD_OK)
		debug(5, "ERROR: PauseMusic() returned %.8x in PauseAllSound()", rv);

	rv = g_sound->PauseSpeech();
	if (rv != RD_OK)
		debug(5, "ERROR: PauseSpeech() returned %.8x in PauseAllSound()", rv);

	rv = g_sound->PauseFx();
	if (rv != RD_OK)
		debug(5, "ERROR: PauseFx() returned %.8x in PauseAllSound()", rv);
}

void UnpauseAllSound(void) {
	uint32	rv;

	rv = g_sound->UnpauseMusic();
	if (rv != RD_OK)
		debug(5, "ERROR: UnpauseMusic() returned %.8x in UnpauseAllSound()", rv);

	rv = g_sound->UnpauseSpeech();
	if (rv != RD_OK)
		debug(5, "ERROR: UnpauseSpeech() returned %.8x in UnpauseAllSound()", rv);

	rv = g_sound->UnpauseFx();
 	if (rv != RD_OK)
		debug(5, "ERROR: UnpauseFx() returned %.8x in UnpauseAllSound()", rv);
}
