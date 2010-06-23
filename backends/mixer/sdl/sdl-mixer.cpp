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

#if defined(WIN32) || defined(UNIX) || defined(MACOSX)

#include "backends/mixer/sdl/sdl-mixer.h"

#include "common/config-manager.h"

//#define SAMPLES_PER_SEC 11025
#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100

SdlMixerImpl::SdlMixerImpl(OSystem *system)
	:
#if MIXER_DOUBLE_BUFFERING
	_soundMutex(0), _soundCond(0), _soundThread(0),
	_soundThreadIsRunning(false), _soundThreadShouldQuit(false),
#endif
	MixerImpl(system, getSamplesPerSec()) {
	if (_openAudio) {
		setReady(true);

#if MIXER_DOUBLE_BUFFERING
		initThreadedMixer(_obtainedRate.samples * 4);
#endif

		// start the sound system
		SDL_PauseAudio(0);
	}
	else {
		setReady(false);
	}
}

SdlMixerImpl::~SdlMixerImpl() {
	setReady(false);

	SDL_CloseAudio();

#if MIXER_DOUBLE_BUFFERING
	deinitThreadedMixer();
#endif
}

uint SdlMixerImpl::getSamplesPerSec() {

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	SDL_AudioSpec desired;

	// Determine the desired output sampling frequency.
	uint32 samplesPerSec = 0;
	if (ConfMan.hasKey("output_rate"))
		samplesPerSec = ConfMan.getInt("output_rate");
	if (samplesPerSec <= 0)
		samplesPerSec = SAMPLES_PER_SEC;

	// Determine the sample buffer size. We want it to store enough data for
	// at least 1/16th of a second (though at most 8192 samples). Note
	// that it must be a power of two. So e.g. at 22050 Hz, we request a
	// sample buffer size of 2048.
	uint32 samples = 8192;
	while (samples * 16 > samplesPerSec * 2)
		samples >>= 1;

	memset(&desired, 0, sizeof(desired));
	desired.freq = samplesPerSec;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = (uint16)samples;
	desired.callback = mixSdlCallback;
	desired.userdata = this;

	if (SDL_OpenAudio(&desired, &_obtainedRate) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_openAudio = false;
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		samplesPerSec = _obtainedRate.freq;
		debug(1, "Output sample rate: %d Hz", samplesPerSec);
		_openAudio = true;
	}
	return samplesPerSec;
}

#if MIXER_DOUBLE_BUFFERING

void SdlMixerImpl::mixerProducerThread() {
	byte nextSoundBuffer;

	SDL_LockMutex(_soundMutex);
	while (true) {
		// Wait till we are allowed to produce data
		SDL_CondWait(_soundCond, _soundMutex);

		if (_soundThreadShouldQuit)
			break;

		// Generate samples and put them into the next buffer
		nextSoundBuffer = _activeSoundBuf ^ 1;
		mixCallback(_soundBuffers[nextSoundBuffer], _soundBufSize);

		// Swap buffers
		_activeSoundBuf = nextSoundBuffer;
	}
	SDL_UnlockMutex(_soundMutex);
}

int SDLCALL SdlMixerImpl::mixerProducerThreadEntry(void *arg) {
	SdlMixerImpl *mixer = (SdlMixerImpl *)arg;
	assert(mixer);
	mixer->mixerProducerThread();
	return 0;
}


void SdlMixerImpl::initThreadedMixer(uint bufSize) {
	_soundThreadIsRunning = false;
	_soundThreadShouldQuit = false;

	// Create mutex and condition variable
	_soundMutex = SDL_CreateMutex();
	_soundCond = SDL_CreateCond();

	// Create two sound buffers
	_activeSoundBuf = 0;
	_soundBufSize = bufSize;
	_soundBuffers[0] = (byte *)calloc(1, bufSize);
	_soundBuffers[1] = (byte *)calloc(1, bufSize);

	_soundThreadIsRunning = true;

	// Finally start the thread
	_soundThread = SDL_CreateThread(mixerProducerThreadEntry, this);
}

void SdlMixerImpl::deinitThreadedMixer() {
	// Kill thread?? _soundThread

	if (_soundThreadIsRunning) {
		// Signal the producer thread to end, and wait for it to actually finish.
		_soundThreadShouldQuit = true;
		SDL_CondBroadcast(_soundCond);
		SDL_WaitThread(_soundThread, NULL);

		// Kill the mutex & cond variables.
		// Attention: AT this point, the mixer callback must not be running
		// anymore, else we will crash!
		SDL_DestroyMutex(_soundMutex);
		SDL_DestroyCond(_soundCond);

		_soundThreadIsRunning = false;

		free(_soundBuffers[0]);
		free(_soundBuffers[1]);
	}
}


void SdlMixerImpl::mixSdlCallback(void *arg, byte *samples, int len) {
	SdlMixerImpl *mixer = (SdlMixerImpl *)arg;
	assert(mixer);

	assert((int)mixer->getSoundBufSize() == len);

	// Lock mutex, to ensure our data is not overwritten by the producer thread
	g_system->lockMutex((OSystem::MutexRef)mixer->getSoundMutex());

	// Copy data from the current sound buffer
	memcpy(samples, mixer->getActiveSoundBuf(), len);

	// Unlock mutex and wake up the produced thread
	g_system->unlockMutex((OSystem::MutexRef)mixer->getSoundMutex());
	SDL_CondSignal(mixer->getSoundCond());
}

#else

void SdlMixerImpl::mixSdlCallback(void *sys, byte *samples, int len) {
	Audio::MixerImpl *mixer = (Audio::MixerImpl *)sys;
	assert(mixer);
	mixer->mixCallback(samples, len);
}

#endif

#endif
