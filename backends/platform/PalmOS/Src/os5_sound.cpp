/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "be_os5.h"
#include "common/config-manager.h"
#include "sound/mixer_intern.h"


#ifdef PALMOS_ARM
#	ifdef COMPILE_ZODIAC
#		define SAMPLES_PER_SEC 44100
#	else
#		define SAMPLES_PER_SEC 22050
#	endif
#else
#	define SAMPLES_PER_SEC 8000
#endif


SoundExType _soundEx;

static Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP) {
	SoundExType *_soundEx = (SoundExType *)UserDataP;
	SoundType *_sound = _soundEx->sound;

	if (_soundEx->set && _soundEx->size) {
		MemMove(bufferP, _soundEx->dataP, _soundEx->size);
		*bufferSizeP = _soundEx->size;
		_soundEx->set = false;

	} else {
		_soundEx->size = *bufferSizeP;
		MemSet(bufferP, 128, 0);
		*bufferSizeP = 128;
	}

	return errNone;
}

void OSystem_PalmOS5::sound_handler() {
	if (_sound.active) {
		if (_soundEx.size && !_soundEx.set) {
			if (!_soundEx.dataP)
				_soundEx.dataP = MemPtrNew(_soundEx.size);

			_mixerMgr->mixCallback((byte *)_soundEx.dataP, _soundEx.size);
			_soundEx.set = true;
		}
	}// TODO : no Sound API case
}

SndStreamVariableBufferCallback OSystem_PalmOS5::sound_callback() {
	return sndCallback;
}

bool OSystem_PalmOS5::setupMixer() {
	Err e;
	Boolean success = false;

	uint32 samplesPerSec;
	if (ConfMan.hasKey("output_rate"))
		samplesPerSec = ConfMan.getInt("output_rate");
	else
		samplesPerSec = SAMPLES_PER_SEC;

	_mixerMgr = new Audio::MixerImpl(this, samplesPerSec);

	if (!_sound.active) {
		if (gVars->fmQuality != FM_QUALITY_INI) {
			ConfMan.setBool("FM_medium_quality", (gVars->fmQuality == FM_QUALITY_MED));
			ConfMan.setBool("FM_high_quality", (gVars->fmQuality == FM_QUALITY_HI));
		}

#if defined (COMPILE_OS5)
		CALLBACK_INIT(_soundEx);
#endif
		_sound.proc = 0;
		_sound.param = _mixerMgr;
		_sound.active = true;	// always true when we call this function, false when sound is off

		_soundEx.handle = 0;
		_soundEx.size = 0;		// set by the callback
		_soundEx.set = false;
		_soundEx.dataP = NULL;	// set by the handler

		// try to create sound stream
		if (OPTIONS_TST(kOptPalmSoundAPI)) {
			e = SndStreamCreateExtended(
						&_soundEx.handle,
						sndOutput,
						sndFormatPCM,
						samplesPerSec,
						sndInt16Little,
						sndStereo,
						sound_callback(),
						&_soundEx,
						8192
#ifdef COMPILE_OS5
						,true
#endif
						);

			e = e ? e : SndStreamStart(_soundEx.handle);
			e = e ? e :	SndStreamSetVolume(_soundEx.handle, 1024L * gVars->palmVolume / 100);
			success = (e == errNone);

		// no Sound API
		} else {
			_soundEx.size = 512;
			_soundEx.dataP = MemPtrNew(_soundEx.size);
		}
	}
	// if not true some scenes (indy3 256,...) may freeze (ESC to skip)

	_mixerMgr->setReady(true);

	return true;
}

void OSystem_PalmOS5::clearSoundCallback() {
	if (_sound.active) {
		if (OPTIONS_TST(kOptPalmSoundAPI)) {
			SndStreamStop(_soundEx.handle);
			SndStreamDelete(_soundEx.handle);
		}

		if (_soundEx.dataP)
			MemPtrFree(_soundEx.dataP);
	}

	_sound.active = false;
	_soundEx.handle = NULL;
	_soundEx.dataP = NULL;
}
