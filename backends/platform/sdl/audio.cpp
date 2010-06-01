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

#include "backends/platform/sdl/audio.h"
#include "sound/mixer_intern.h"
#include "common/config-manager.h"

//#define SAMPLES_PER_SEC 11025
#define SAMPLES_PER_SEC 22050
//#define SAMPLES_PER_SEC 44100

SdlSubSys_Audio::SdlSubSys_Audio()
	:
	_inited(false),
	_cdrom(0),
#if MIXER_DOUBLE_BUFFERING
	_soundMutex(0), _soundCond(0), _soundThread(0),
	_soundThreadIsRunning(false), _soundThreadShouldQuit(false),
#endif
	_mixer(0) {

}

SdlSubSys_Audio::~SdlSubSys_Audio() {
	if (_inited) {
		audioDone();
	}
}

void SdlSubSys_Audio::audioInit() {
	if (_inited) {
		return;
	}

	// Create and hook up the mixer, if none exists yet (we check for this to
	// allow subclasses to provide their own).
	if (_mixer == 0) {
		setupMixer();
	}

	_inited = true;
}

void SdlSubSys_Audio::audioDone() {
	if (_cdrom) {
		SDL_CDStop(_cdrom);
		SDL_CDClose(_cdrom);
	}

	closeMixer();

	_inited = false;
}

bool SdlSubSys_Audio::hasFeature(Feature f) {
	return false;
}

void SdlSubSys_Audio::setFeatureState(Feature f, bool enable) {
	
}

bool SdlSubSys_Audio::getFeatureState(Feature f) {
	return false;
}

#if MIXER_DOUBLE_BUFFERING

void SdlSubSys_Audio::mixerProducerThread() {
	byte nextSoundBuffer;

	SDL_LockMutex(_soundMutex);
	while (true) {
		// Wait till we are allowed to produce data
		SDL_CondWait(_soundCond, _soundMutex);

		if (_soundThreadShouldQuit)
			break;

		// Generate samples and put them into the next buffer
		nextSoundBuffer = _activeSoundBuf ^ 1;
		_mixer->mixCallback(_soundBuffers[nextSoundBuffer], _soundBufSize);

		// Swap buffers
		_activeSoundBuf = nextSoundBuffer;
	}
	SDL_UnlockMutex(_soundMutex);
}

int SDLCALL SdlSubSys_Audio::mixerProducerThreadEntry(void *arg) {
	OSystem_SDL *this_ = (OSystem_SDL *)arg;
	assert(this_);
	this_->mixerProducerThread();
	return 0;
}


void SdlSubSys_Audio::initThreadedMixer(Audio::MixerImpl *mixer, uint bufSize) {
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

void SdlSubSys_Audio::deinitThreadedMixer() {
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


void SdlSubSys_Audio::mixCallback(void *arg, byte *samples, int len) {
	OSystem_SDL *this_ = (OSystem_SDL *)arg;
	assert(this_);
	assert(this_->_mixer);

	assert((int)this_->_soundBufSize == len);

	// Lock mutex, to ensure our data is not overwritten by the producer thread
	SDL_LockMutex(this_->_soundMutex);

	// Copy data from the current sound buffer
	memcpy(samples, this_->_soundBuffers[this_->_activeSoundBuf], len);

	// Unlock mutex and wake up the produced thread
	SDL_UnlockMutex(this_->_soundMutex);
	SDL_CondSignal(this_->_soundCond);
}

#else

void SdlSubSys_Audio::mixCallback(void *sys, byte *samples, int len) {
	SdlSubSys_Audio *this_ = (SdlSubSys_Audio *)sys;
	assert(this_);
	assert(this_->_mixer);

	this_->_mixer->mixCallback(samples, len);
}

#endif

void SdlSubSys_Audio::setupMixer() {
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
	desired.callback = mixCallback;
	desired.userdata = this;

	assert(!_mixer);
	if (SDL_OpenAudio(&desired, &_obtainedRate) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		_mixer = new Audio::MixerImpl(this, samplesPerSec);
		assert(_mixer);
		_mixer->setReady(false);
	} else {
		// Note: This should be the obtained output rate, but it seems that at
		// least on some platforms SDL will lie and claim it did get the rate
		// even if it didn't. Probably only happens for "weird" rates, though.
		samplesPerSec = _obtainedRate.freq;
		debug(1, "Output sample rate: %d Hz", samplesPerSec);

		// Create the mixer instance and start the sound processing
		_mixer = new Audio::MixerImpl(this, samplesPerSec);
		assert(_mixer);
		_mixer->setReady(true);

#if MIXER_DOUBLE_BUFFERING
		initThreadedMixer(_mixer, _obtainedRate.samples * 4);
#endif

		// start the sound system
		SDL_PauseAudio(0);
	}
}

void SdlSubSys_Audio::closeMixer() {
	if (_mixer)
		_mixer->setReady(false);

	SDL_CloseAudio();

	delete _mixer;
	_mixer = 0;

#if MIXER_DOUBLE_BUFFERING
	deinitThreadedMixer();
#endif

}

Audio::Mixer *SdlSubSys_Audio::getMixer() {
	assert(_mixer);
	return _mixer;
}

bool SdlSubSys_Audio::openCD(int drive) {
	if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
		_cdrom = NULL;
	else {
		_cdrom = SDL_CDOpen(drive);
		// Did it open? Check if _cdrom is NULL
		if (!_cdrom) {
			warning("Couldn't open drive: %s", SDL_GetError());
		} else {
			_cdNumLoops = 0;
			_cdStopTime = 0;
			_cdEndTime = 0;
		}
	}

	return (_cdrom != NULL);
}

void SdlSubSys_Audio::stopCD() {	/* Stop CD Audio in 1/10th of a second */
	_cdStopTime = SDL_GetTicks() + 100;
	_cdNumLoops = 0;
}

void SdlSubSys_Audio::playCD(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	if (!_cdrom)
		return;

	if (duration > 0)
		duration += 5;

	_cdTrack = track;
	_cdNumLoops = num_loops;
	_cdStartFrame = start_frame;

	SDL_CDStatus(_cdrom);
	if (start_frame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, start_frame, 0, duration);
	_cdDuration = duration;
	_cdStopTime = 0;
	_cdEndTime = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;
}

bool SdlSubSys_Audio::pollCD() {
	if (!_cdrom)
		return false;

	return (_cdNumLoops != 0 && (SDL_GetTicks() < _cdEndTime || SDL_CDStatus(_cdrom) == CD_PLAYING));
}

void SdlSubSys_Audio::updateCD() {
	if (!_cdrom)
		return;

	if (_cdStopTime != 0 && SDL_GetTicks() >= _cdStopTime) {
		SDL_CDStop(_cdrom);
		_cdNumLoops = 0;
		_cdStopTime = 0;
		return;
	}

	if (_cdNumLoops == 0 || SDL_GetTicks() < _cdEndTime)
		return;

	if (_cdNumLoops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		_cdEndTime += 1000;
		return;
	}

	if (_cdNumLoops > 0)
		_cdNumLoops--;

	if (_cdNumLoops != 0) {
		if (_cdStartFrame == 0 && _cdDuration == 0)
			SDL_CDPlayTracks(_cdrom, _cdTrack, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, _cdTrack, _cdStartFrame, 0, _cdDuration);
		_cdEndTime = SDL_GetTicks() + _cdrom->track[_cdTrack].length * 1000 / CD_FPS;
	}
}
