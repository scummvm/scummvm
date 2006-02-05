// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "debug.h"
#include "driver_sdl.h"

// NOTE: This is not a complete driver, it needs to be subclassed
//       to provide rendering functionality.

uint32 DriverSDL::getMillis() {
	return SDL_GetTicks();
}

void DriverSDL::delayMillis(uint msecs) {
	SDL_Delay(msecs);
}

void DriverSDL::setTimerCallback(TimerProc callback, int timer) {
	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
}

bool DriverSDL::setSoundCallback(SoundProc proc, void *param) {
	SDL_AudioSpec desired;

	memset(&desired, 0, sizeof(desired));

	desired.freq = 22050;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = proc;
	desired.userdata = param;

	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}

	SDL_PauseAudio(0);
	return true;
}

void DriverSDL::clearSoundCallback() {
	SDL_CloseAudio();
}

int DriverSDL::getOutputSampleRate() const {
	return _samplesPerSec;
}

