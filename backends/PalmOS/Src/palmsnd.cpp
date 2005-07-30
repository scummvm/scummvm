/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/config-manager.h"

#include "palm.h"
#include "pa1lib.h"

#define	SND_BLOCK	(3072)

#define ADPCM_8_KHZ						1
#define ADPCM_MODE_NONCONTINUOUS_PB		0x01
#define ADPCM_MODE_INTERRUPT_MODE		0x02


int OSystem_PALMOS::getOutputSampleRate() const {
	return _samplesPerSec;
}

static void ClieSoundCallback(UInt32 UserData) {
	SoundDataType *snd = (SoundDataType *)UserData;
	snd->set = false;
}

bool OSystem_PALMOS::setSoundCallback(SoundProc proc, void *param) {
	Boolean success = false;

	if (!_sound.active) {
		_sound.proc = proc;
		_sound.param = param;
		_sound.active = true;	// always true when we call this function
		_sound.dataP = NULL;	// required by sound_handler
		_sound.handle = NULL;

		if (ConfMan.hasKey("output_rate"))
			_samplesPerSec = ConfMan.getInt("output_rate");
		else
			_samplesPerSec = 8000;	// default value

		// try to create sound stream
		if (OPTIONS_TST(kOptPalmSoundAPI)) {
			void *sndFuncP;

			_sound.handle = MemPtrNew(sizeof(SndStreamRef));
			sndFuncP = sndCallback;

			Err e = SndStreamCreateExtended(
						(SndStreamRef *)_sound.handle,
						sndOutput,
						sndFormatPCM,
						_samplesPerSec,
						sndInt16Little,
						sndStereo,
						(SndStreamVariableBufferCallback)sndFuncP,
						&_sound,
						8192,
						false);

			e = e ? e : SndStreamStart(*((SndStreamRef *)_sound.handle));
			e = e ? e :	SndStreamSetVolume(*((SndStreamRef *)_sound.handle), 32767 / 2);

			_sound.size = 0;		// set by the callback
			_sound.set = false;
			_sound.wait = true;
			success = (e == errNone);

		} else if (OPTIONS_TST(kOptSonyPa1LibAPI)) {
			static CallbackInfoType cbData;
			_sound.handle = MemPtrNew(sizeof(UInt8));

			cbData.funcP = &ClieSoundCallback;
			cbData.dwUserData = (UInt32)&_sound;

			_sound.size		= SND_BLOCK;
			_sound.set		= false;
			_sound.wait		= true;
			_sound.dataP	= MemPtrNew(SND_BLOCK);
			_sound.tmpP		= MemPtrNew(SND_BLOCK / 8);

			success = true;		// don't generate samples

			Pa1Lib_adpcmOpen(ADPCM_8_KHZ, (UInt8 *)_sound.tmpP, MemPtrSize(_sound.tmpP), &cbData, (UInt8 *)_sound.handle);
		}
		// failed or not supported
		if (!success) {
			_sound.size = SND_BLOCK;
			_sound.set = false;
			_sound.wait = false;
			success = false;		// don't generate samples
		}
	}

	return true;	// if not true some scenes (indy3 256,...) may freeze (ESC to skip)
}

void OSystem_PALMOS::clearSoundCallback() {
	if (_sound.active) {

		if (OPTIONS_TST(kOptPalmSoundAPI)) {
			SndStreamStop(*((SndStreamRef *)_sound.handle));
			SndStreamDelete(*((SndStreamRef *)_sound.handle));
		}

		free(_sound.dataP);
		free(_sound.tmpP);
		free(_sound.handle);
	}


	_sound.active = false;
	_sound.dataP = NULL;
	_sound.tmpP = NULL;
	_sound.handle = NULL;
}

void OSystem_PALMOS::sound_handler() {
	if (_sound.active) {
		if (_sound.size && (!_sound.set || !_sound.wait)) {

			if (!_sound.dataP)
				_sound.dataP = MemPtrNew(_sound.size);

			((SoundProc)_sound.proc)(_sound.param, (byte *)_sound.dataP, _sound.size);

			if (OPTIONS_TST(kOptSonyPa1LibAPI)) {
				pcm2adpcm((Int16 *)_sound.dataP, (UInt8 *)_sound.tmpP, _sound.size);
				_sound.set = Pa1Lib_adpcmStart(*((UInt8 *)_sound.handle), ADPCM_MODE_NONCONTINUOUS_PB|ADPCM_MODE_INTERRUPT_MODE);
			} else {
				_sound.set = true;
			}
		}
	}
}

bool OSystem_PALMOS::openCD(int drive) {
	// TODO : add a function to CDAudio to init the MP3 driver (?)
	return false;
}

void OSystem_PALMOS::stopCD() {
	if (!_cdPlayer)
		return;

	_cdPlayer->stop();
}

void OSystem_PALMOS::playCD(int track, int num_loops, int start_frame, int duration) {
	if (!_cdPlayer)
		return;

	_cdPlayer->play(track, num_loops, start_frame, duration);
}

bool OSystem_PALMOS::pollCD() {
	if (!_cdPlayer)
		return false;

	return _cdPlayer->poll();
}

void OSystem_PALMOS::updateCD() {
	if (!_cdPlayer)
		return;

	_cdPlayer->update();
}
