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
 */

#if defined(SDL_BACKEND)

#include "backends/mixer/sdl/sdl-mixer.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#ifdef GP2X
#define SAMPLES_PER_SEC 11025
#else
#define SAMPLES_PER_SEC 22050
#endif
//#define SAMPLES_PER_SEC 44100

SdlMixerManager::SdlMixerManager()
	:
	_mixer(0),
	_audioSuspended(false) {

}

SdlMixerManager::~SdlMixerManager() {
	_mixer->setReady(false);

	SDL_CloseAudio();

	delete _mixer;
}

void SdlMixerManager::init() {
	// Start SDL Audio subsystem
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	// Get the desired audio specs
	SDL_AudioSpec desired = getAudioSpec(SAMPLES_PER_SEC);

	// Start SDL audio with the desired specs
	if (SDL_OpenAudio(&desired, &_obtainedRate) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());

		_mixer = new Audio::MixerImpl(g_system, desired.freq);
		assert(_mixer); 
		_mixer->setReady(false);
	} else {
		debug(1, "Output sample rate: %d Hz", _obtainedRate.freq);

		_mixer = new Audio::MixerImpl(g_system, _obtainedRate.freq);
		assert(_mixer); 
		_mixer->setReady(true);

		startAudio();
	}
}

SDL_AudioSpec SdlMixerManager::getAudioSpec(uint32 outputRate) {
	SDL_AudioSpec desired;

	// Determine the desired output sampling frequency.
	uint32 samplesPerSec = 0;
	if (ConfMan.hasKey("output_rate"))
		samplesPerSec = ConfMan.getInt("output_rate");
	if (samplesPerSec <= 0)
		samplesPerSec = outputRate;

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
	desired.callback = sdlCallback;
	desired.userdata = this;

	return desired;
}

void SdlMixerManager::startAudio() {
	// Start the sound system
	SDL_PauseAudio(0);
}

void SdlMixerManager::callbackHandler(byte *samples, int len) {
	assert(_mixer);
	_mixer->mixCallback(samples, len);
}

void SdlMixerManager::sdlCallback(void *this_, byte *samples, int len) {
	SdlMixerManager *manager = (SdlMixerManager *)this_;
	assert(manager);

	manager->callbackHandler(samples, len);
}

void SdlMixerManager::suspendAudio() {
	SDL_CloseAudio();
	_audioSuspended = true;
}

int SdlMixerManager::resumeAudio() {
	if (!_audioSuspended)
		return -2;
	if (SDL_OpenAudio(&_obtainedRate, NULL) < 0){
		return -1;
	}
	SDL_PauseAudio(0);
	_audioSuspended = false;
	return 0;
}

#endif
