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

#ifndef BACKENDS_MIXER_SDL_H
#define BACKENDS_MIXER_SDL_H

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

#include "sound/mixer_intern.h"

#if defined(MACOSX)
// On Mac OS X, we need to double buffer the audio buffer, else anything
// which produces sampled data with high latency (like the MT-32 emulator)
// will sound terribly.
// This could be enabled for more / most ports in the future, but needs some
// testing.
#define MIXER_DOUBLE_BUFFERING 1
#endif

class SdlMixerImpl : public Audio::MixerImpl {
public:

	SdlMixerImpl(OSystem *system);
	~SdlMixerImpl();

protected:
	SDL_AudioSpec _obtainedRate;
	bool _openAudio;

	uint getSamplesPerSec();

	static void mixSdlCallback(void *s, byte *samples, int len);

#ifdef MIXER_DOUBLE_BUFFERING
	SDL_mutex *_soundMutex;
	SDL_cond *_soundCond;
	SDL_Thread *_soundThread;
	bool _soundThreadIsRunning;
	bool _soundThreadShouldQuit;

	byte _activeSoundBuf;
	uint _soundBufSize;
	byte *_soundBuffers[2];

	void mixerProducerThread();
	static int SDLCALL mixerProducerThreadEntry(void *arg);
	void initThreadedMixer(uint bufSize);
	void deinitThreadedMixer();

public:
	SDL_mutex *getSoundMutex() { return _soundMutex; }
	SDL_cond *getSoundCond() { return _soundCond; }
	uint getSoundBufSize() { return _soundBufSize; }
	byte *getActiveSoundBuf() { return _soundBuffers[_activeSoundBuf]; }
#endif
};

#endif
