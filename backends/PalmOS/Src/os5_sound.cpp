/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2002-2005 Chris Apers - PalmOS Backend
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
 * $Header$
 *
 */

#include "be_os5.h"
#include "common/config-manager.h"

#ifdef PALMOS_68K
static void initRegs(void *addr) {
	asm (
		move.l	addr, a0
		move.l	a4, 0(a0)
		move.l	a5, 4(a0)
	);
}

static Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP) {
	asm (
//		movem.l a4-a5, -(sp)
		move.l UserDataP, a0
		move.l 0(a0), a4
		move.l 4(a0), a5
	);

	SoundDataType *_sound = (SoundDataType *)UserDataP;
	((OSystem::SoundProc)_sound->proc)(_sound->param, (byte *)bufferP, *bufferSizeP);

//	asm ( movem.l (sp)+, a4-a5 );
	return errNone;
}

#else

static SYSTEM_CALLBACK Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP) {
	SoundDataType *_sound = (SoundDataType *)UserDataP;
	((OSystem::SoundProc)_sound->proc)(_sound->param, (byte *)bufferP, *bufferSizeP);
	return errNone;
}
#endif


bool OSystem_PalmOS5::setSoundCallback(SoundProc proc, void *param) {
	Err e;
	Boolean success = false;

	if (!_sound.active) {
		if (gVars->fmQuality != FM_QUALITY_INI) {
			ConfMan.set("FM_medium_quality", (gVars->fmQuality == FM_QUALITY_MED));
			ConfMan.set("FM_high_quality", (gVars->fmQuality == FM_QUALITY_HI));
		}

		_sound.proc = proc;
		_sound.param = param;
		_sound.active = true;		// always true when we call this function, false when sound is off
		_sound.handle = NULL;

		if (ConfMan.hasKey("output_rate"))
			_samplesPerSec = ConfMan.getInt("output_rate");
		else
#ifdef PALMOS_ARM
			_samplesPerSec = 44100;	// default value
#else
			_samplesPerSec = 8000;	// default value
#endif

		// try to create sound stream
		if (1 || OPTIONS_TST(kOptPalmSoundAPI)) {
#ifdef PALMOS_68K
			initRegs(&_sound);
#endif
			e = SndStreamCreateExtended(
						&_sound.handle,
						sndOutput,
						sndFormatPCM,
						_samplesPerSec,
#ifdef PALMOS_ARM
						sndInt16Little,
#else
						sndInt16Big,
#endif
						sndStereo,
						(SndStreamVariableBufferCallback)sndCallback,
						&_sound,
						8192
#ifdef PALMOS_68K
						,false
#endif
						);

			e = e ? e : SndStreamStart(_sound.handle);
			e = e ? e :	SndStreamSetVolume(_sound.handle, (32767L / 16) * gVars->palmVolume / 100);
			success = (e == errNone);
		}
	}
	// if not true some scenes (indy3 256,...) may freeze (ESC to skip)
	return true;
}

void OSystem_PalmOS5::clearSoundCallback() {
	if (_sound.active) {
		if (1 || OPTIONS_TST(kOptPalmSoundAPI)) {
			SndStreamStop(_sound.handle);
			SndStreamDelete(_sound.handle);
		}
	}
	
	_sound.active = false;
	_sound.handle = NULL;
}
