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

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"

namespace Sword2 {

// initialise the fxq by clearing all the entries

void Sword2Engine::initFxQueue(void) {
	for (int i = 0; i < FXQ_LENGTH; i++) {
		_fxQueue[i].resource = 0;	// 0 resource means 'empty' slot
		_fxQueue[i].fetchId = 0;	// Not being fetched.
	}
}

// process the fx queue once every game cycle

void Sword2Engine::processFxQueue(void) {
	for (int i = 0; i < FXQ_LENGTH; i++) {
		if (!_fxQueue[i].resource)
			continue;

		switch (_fxQueue[i].type) {
		case FX_RANDOM:
			// 1 in 'delay' chance of this fx occurring
			if (_rnd.getRandomNumber(_fxQueue[i].delay) == 0)
				triggerFx(i);
			break;
		case FX_SPOT:
			if (_fxQueue[i].delay)
				_fxQueue[i].delay--;
			else {
				triggerFx(i);
				_fxQueue[i].type = FX_SPOT2;
			}
			break;
		case FX_SPOT2:
			// Once the Fx has finished remove it from the queue.
			if (_sound->isFxOpen(i + 1))
				_fxQueue[i].resource = 0;
			break;
		}
	}
}

// called from processFxQueue only

void Sword2Engine::triggerFx(uint8 j) {
	uint8 *data;
	int32 id;
	uint32 rv;

	id = (uint32) j + 1;	// because 0 is not a valid id

	if (_fxQueue[j].type == FX_SPOT) {
		// load in the sample
		data = _resman->openResource(_fxQueue[j].resource);
		data += sizeof(_standardHeader);
		// wav data gets copied to sound memory
		rv = _sound->playFx(id, data, _fxQueue[j].volume, _fxQueue[j].pan, RDSE_FXSPOT);
		// release the sample
		_resman->closeResource(_fxQueue[j].resource);
	} else {
		// random & looped fx are already loaded into sound memory
		// by fnPlayFx()
		// - to be referenced by 'j', so pass NULL data

		if (_fxQueue[j].type == FX_RANDOM) {
			// Not looped
			rv = _sound->playFx(id, NULL, _fxQueue[j].volume, _fxQueue[j].pan, RDSE_FXSPOT);
		} else {
			// Looped
			rv = _sound->playFx(id, NULL, _fxQueue[j].volume, _fxQueue[j].pan, RDSE_FXLOOP);
		}
	}

	if (rv)
		debug(5, "SFX ERROR: playFx() returned %.8x", rv);
}

// Stops all looped & random fx and clears the entire queue

void Sword2Engine::clearFxQueue(void) {
	// stop all fx & remove the samples from sound memory
	_sound->clearAllFx();

	// clean out the queue
	initFxQueue();
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

// called from script only

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

	uint8 j = 0;
	uint8 *data;
	uint32 id;
	uint32 rv;

#ifdef _SWORD2_DEBUG
	_standardHeader *header;
#endif

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
		}

		debug(0, "SFX (sample=\"%s\", vol=%d, pan=%d, delay=%d, type=%s)", _vm->fetchObjectName(params[0]), params[3], params[4], params[2], type);
	}

	while (j < FXQ_LENGTH && _vm->_fxQueue[j].resource != 0)
		j++;

	if (j == FXQ_LENGTH)
		return IR_CONT;

	_vm->_fxQueue[j].resource = params[0];	// wav resource id
	_vm->_fxQueue[j].type = params[1];	// FX_SPOT, FX_LOOP or FX_RANDOM

	if (_vm->_fxQueue[j].type == FX_RANDOM) {
		// 'delay' param is the intended average no. seconds between
		// playing this effect
		_vm->_fxQueue[j].delay = params[2] * 12;
	} else {
		// FX_SPOT or FX_LOOP:
		//  'delay' is no. frames to wait before playing
		_vm->_fxQueue[j].delay = params[2];
	}

	_vm->_fxQueue[j].volume = params[3];	// 0..16
	_vm->_fxQueue[j].pan = params[4];	// -16..16

	if (_vm->_fxQueue[j].type == FX_SPOT) {
		// "pre-load" the sample; this gets it into memory
		data = _vm->_resman->openResource(_vm->_fxQueue[j].resource);

#ifdef _SWORD2_DEBUG
		header = (_standardHeader *) data;
		if (header->fileType != WAV_FILE)
			error("fnPlayFx given invalid resource");
#endif

		// but then releases it to "age" out if the space is needed
		_vm->_resman->closeResource(_vm->_fxQueue[j].resource);
	} else {
		// random & looped fx

		id = (uint32) j + 1;	// because 0 is not a valid id

		// load in the sample
		data = _vm->_resman->openResource(_vm->_fxQueue[j].resource);

#ifdef _SWORD2_DEBUG
		header = (_standardHeader *) data;
		if (header->fileType != WAV_FILE)
			error("fnPlayFx given invalid resource");
#endif

		data += sizeof(_standardHeader);

		// copy it to sound memory, using position in queue as 'id'
		rv = _vm->_sound->openFx(id, data);

		if (rv)
			debug(5, "SFX ERROR: openFx() returned %.8x", rv);

		// release the sample
		_vm->_resman->closeResource(_vm->_fxQueue[j].resource);
	}

	if (_vm->_fxQueue[j].type == FX_LOOP) {
		// play now, rather than in Process_fx_queue where it was
		// getting played again & again!
		_vm->triggerFx(j);
	}

	// in case we want to call fnStopFx() later, to kill this fx
	// (mainly for FX_LOOP & FX_RANDOM)

	RESULT = j;
	return IR_CONT;
}

int32 Logic::fnSoundFetch(int32 *params) {
	// params:	0 id of sound to fetch [guess]
	return IR_CONT;
}

// to alter the volume and pan of a currently playing fx

int32 Logic::fnSetFxVolAndPan(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)
	//		2 new pan (-16..16)

	debug(5, "fnSetFxVolAndPan(%d, %d, %d)", params[0], params[1], params[2]);

	// setFxIdVolumePan(int32 id, uint8 vol, uint8 pan);
	// driver fx_id is 1 + <pos in queue>
	_vm->_sound->setFxIdVolumePan(1 + params[0], params[1], params[2]);
	return IR_CONT;
}

// to alter the volume  of a currently playing fx

int32 Logic::fnSetFxVol(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)

	// SetFxIdVolume(int32 id, uint8 vol);
	_vm->_sound->setFxIdVolume(1 + params[0], params[1]);
	return IR_CONT;
}

// called from script only

int32 Logic::fnStopFx(int32 *params) {
	// params:	0 position in queue

	// This will stop looped & random fx instantly, and remove the fx
	// from the queue. So although it doesn't stop spot fx, it will
	// remove them from the queue if they haven't yet played

	uint8 j = (uint8) params[0];
	uint32 id;
	uint32 rv;

	if (_vm->_fxQueue[j].type == FX_RANDOM || _vm->_fxQueue[j].type == FX_LOOP) {
		id = (uint32) j + 1;		// because 0 is not a valid id

		// stop fx & remove sample from sound memory
		rv = _vm->_sound->closeFx(id);

		if (rv)
			debug(5, "SFX ERROR: closeFx() returned %.8x", rv);
	}

	// remove from queue
	_vm->_fxQueue[j].resource = 0;

	return IR_CONT;
}

// called from script only

int32 Logic::fnStopAllFx(int32 *params) {
	// Stops all looped & random fx and clears the entire queue

	// params:	none

	_vm->clearFxQueue();
	return IR_CONT;
}

int32 Logic::fnPrepareMusic(int32 *params) {
	// params:	1 id of music to prepare [guess]
	return IR_CONT;
}

// Start a tune playing, to play once or to loop until stopped or next one
// played

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

	// add the appropriate file extension & play it

	if (_vm->_features & GF_DEMO) {
		// The demo I found didn't come with any music file, but you
		// could use the music from the first CD of the complete game,
		// I suppose...
		strcpy(filename, "music.clu");
	} else {
		File f;

		sprintf(filename, "music%d.clu", _vm->_resman->whichCd());
		if (f.open(filename))
			f.close();
		else
			strcpy(filename, "music.clu");
	}

	rv = _vm->_sound->streamCompMusic(filename, params[0], loopFlag);

	if (rv)
		debug(5, "ERROR: streamCompMusic(%s, %d, %d) returned error 0x%.8x", filename, params[0], loopFlag, rv);

	return IR_CONT;
}

// called from script only

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
	RESULT = _vm->_sound->musicTimeRemaining();

	return IR_CONT;
}

} // End of namespace Sword2
