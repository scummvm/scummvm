/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/driver/d_sound.h"

namespace Sword2 {

struct FxQueueEntry {
	uint32 resource;	// resource id of sample
	byte *data;		// pointer to WAV data
	uint16 delay;		// cycles to wait before playing (or 'random chance' if FX_RANDOM)
	uint8 volume;		// 0..16
	int8 pan;		// -16..16
	uint8 type;		// FX_SPOT, FX_RANDOM or FX_LOOP
};

// FIXME: Should be in one of the classes, I guess...

static FxQueueEntry fxQueue[FXQ_LENGTH];

/**
 * Initialise the FX queue by clearing all the entries. This is only used at
 * the start of the game. Later when we need to clear the queue we must also
 * stop the sound and close the resource.
 */

void Sword2Engine::initFxQueue(void) {
	for (int i = 0; i < FXQ_LENGTH; i++)
		fxQueue[i].resource = 0;
}

/**
 * Stop all sounds, close their resources and clear the FX queue.
 */

void Sword2Engine::clearFxQueue(void) {
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (fxQueue[i].resource) {
			_sound->stopFx(i + 1);
			_resman->closeResource(fxQueue[i].resource);
			fxQueue[i].resource = 0;
		}
	}
}

/**
 * Process the FX queue once every game cycle
 */

void Sword2Engine::processFxQueue(void) {
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (!fxQueue[i].resource)
			continue;

		switch (fxQueue[i].type) {
		case FX_RANDOM:
			// 1 in 'delay' chance of this fx occurring
			if (_rnd.getRandomNumber(fxQueue[i].delay) == 0)
				triggerFx(i);
			break;
		case FX_SPOT:
			if (fxQueue[i].delay)
				fxQueue[i].delay--;
			else {
				triggerFx(i);
				fxQueue[i].type = FX_SPOT2;
			}
			break;
		case FX_LOOP:
			triggerFx(i);
			fxQueue[i].type = FX_LOOPING;
			break;
		case FX_SPOT2:
			// Once the FX has finished remove it from the queue.
			if (!_sound->isFxPlaying(i + 1)) {
				_sound->stopFx(i + 1);
				_resman->closeResource(fxQueue[i].resource);
				fxQueue[i].resource = 0;
			}
			break;
		case FX_LOOPING:
			// Once the looped FX has started we can ignore it,
			// but we can't close it since the WAV data is in use.
			break;
		}
	}
}

void Sword2Engine::triggerFx(uint8 i) {
	int type;

	if (fxQueue[i].type == FX_LOOP)
		type = RDSE_FXLOOP;
	else
		type = RDSE_FXSPOT;

	uint32 len = _resman->fetchLen(fxQueue[i].resource) - sizeof(StandardHeader);
	uint32 rv = _sound->playFx(i + 1, len, fxQueue[i].data,	fxQueue[i].volume, fxQueue[i].pan, type);

	if (rv)
		debug(5, "SFX ERROR: playFx() returned %.8x", rv);
}

void Sword2Engine::killMusic(void) {
	_loopingMusicId = 0;		// clear the 'looping' flag
	_sound->stopMusic();
}

void Sword2Engine::pauseAllSound(void) {
	_sound->pauseMusic();
	_sound->pauseSpeech();
	_sound->pauseFx();
}

void Sword2Engine::unpauseAllSound(void) {
	_sound->unpauseMusic();
	_sound->unpauseSpeech();
	_sound->unpauseFx();
}

int32 Logic::fnPlayFx(int32 *params) {
	// params:	0 sample resource id
	//		1 type		(FX_SPOT, FX_RANDOM, FX_LOOP)
	//		2 delay		(0..65535)
	//		3 volume	(0..16)
	//		4 pan		(-16..16)

	// example script:
	//		fnPlayFx (FXWATER, FX_LOOP, 0, 10, 15);
	//		// fx_water is just a local script flag
	//		fx_water = result;
	//		.
	//		.
	//		.
	//		fnStopFx (fx_water);

	if (_vm->_wantSfxDebug) {
		char type[10];

		switch (params[1]) {
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
			break;
		}

		byte buf[NAME_LEN];

		debug(0, "SFX (sample=\"%s\", vol=%d, pan=%d, delay=%d, type=%s)", _vm->fetchObjectName(params[0], buf), params[3], params[4], params[2], type);
	}

	int i;

	// Find a free slot in the FX queue

	for (i = 0; i < FXQ_LENGTH; i++) {
		if (!fxQueue[i].resource)
			break;
	}

	if (i == FXQ_LENGTH) {
		warning("No free slot in FX queue");
		return IR_CONT;
	}

	fxQueue[i].resource = params[0];
	fxQueue[i].type = params[1];
	fxQueue[i].delay = params[2];

	if (fxQueue[i].type == FX_RANDOM) {
		// For spot effects and loops the dela is the number of frames
		// to wait. For random effects, however, it's the average
		// number of seconds between playing the sound, so we have to
		// multiply by the frame rate.
		fxQueue[i].delay *= 12;
	}

	fxQueue[i].volume = params[3];
	fxQueue[i].pan = params[4];

	byte *data = _vm->_resman->openResource(params[0]);
	StandardHeader *header = (StandardHeader *) data;

	assert(header->fileType == WAV_FILE);

	fxQueue[i].data = data + sizeof(StandardHeader);

	// Keep track of the index in the loop so that fnStopFx() can be used
	// later to kill this sound. Mainly for FX_LOOP and FX_RANDOM.

	_scriptVars[RESULT] = i;
	return IR_CONT;
}

int32 Logic::fnSoundFetch(int32 *params) {
	// params:	0 id of sound to fetch [guess]
	return IR_CONT;
}

/**
 * Alter the volume and pan of a currently playing FX
 */

int32 Logic::fnSetFxVolAndPan(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)
	//		2 new pan (-16..16)

	debug(5, "fnSetFxVolAndPan(%d, %d, %d)", params[0], params[1], params[2]);

	_vm->_sound->setFxIdVolumePan(params[0] + 1, params[1], params[2]);
	return IR_CONT;
}

/**
 * Alter the volume of a currently playing FX
 */

int32 Logic::fnSetFxVol(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)

	_vm->_sound->setFxIdVolume(params[0] + 1, params[1]);
	return IR_CONT;
}

int32 Logic::fnStopFx(int32 *params) {
	// params:	0 position in queue

	int32 i = params[0];
	uint32 rv = _vm->_sound->stopFx(i + 1);

	if (rv)
		debug(5, "SFX ERROR: closeFx() returned %.8x", rv);

	// Remove from queue
	if (fxQueue[i].resource) {
		_vm->_resman->closeResource(fxQueue[i].resource);
		fxQueue[i].resource = 0;
	}

	return IR_CONT;
}

/**
 * Stops all FX and clears the entire FX queue.
 */

int32 Logic::fnStopAllFx(int32 *params) {
	// params:	none

	_vm->clearFxQueue();
	return IR_CONT;
}

int32 Logic::fnPrepareMusic(int32 *params) {
	// params:	1 id of music to prepare [guess]
	return IR_CONT;
}

/**
 * Start a tune playing, to play once or to loop until stopped or next one
 * played.
 */

int32 Logic::fnPlayMusic(int32 *params) {
	// params:	0 tune id
	//		1 loop flag (0 or 1)

	char filename[128];
	bool loopFlag;
	uint32 rv;

	if (params[1] == FX_LOOP) {
		loopFlag = true;

		// keep a note of the id, for restarting after an
		// interruption to gameplay
		_vm->_loopingMusicId = params[0];
	} else {
		loopFlag = false;

		// don't need to restart this tune after control panel or
		// restore
		_vm->_loopingMusicId = 0;
	}

	rv = _vm->_sound->streamCompMusic(params[0], loopFlag);

	if (rv)
		debug(5, "ERROR: streamCompMusic(%s, %d, %d) returned error 0x%.8x", filename, params[0], loopFlag, rv);

	return IR_CONT;
}

int32 Logic::fnStopMusic(int32 *params) {
	// params:	none

	_vm->_loopingMusicId = 0;		// clear the 'looping' flag
	_vm->_sound->stopMusic();
	return IR_CONT;
}

int32 Logic::fnCheckMusicPlaying(int32 *params) {
	// params:	none

	// sets result to no. of seconds of current tune remaining
	// or 0 if no music playing

	// in seconds, rounded up to the nearest second
	_scriptVars[RESULT] = _vm->_sound->musicTimeRemaining();

	return IR_CONT;
}

} // End of namespace Sword2
