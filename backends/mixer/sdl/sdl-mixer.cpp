/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"

#if defined(SDL_BACKEND)

#include "backends/mixer/sdl/sdl-mixer.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#if defined(PLAYSTATION3) || defined(PSP2) || defined(NINTENDO_SWITCH)
#define SAMPLES_PER_SEC 48000
#elif defined(__MINT__)
#define SAMPLES_PER_SEC 49170
#else
#define SAMPLES_PER_SEC 44100
#endif

SdlMixerManager::SdlMixerManager() : _isSubsystemInitialized(false), _isAudioOpen(false) {
}

SdlMixerManager::~SdlMixerManager() {
	if (_mixer)
		_mixer->setReady(false);

#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(_stream));
	SDL_DestroyAudioStream(_stream);
#else
	if (_isAudioOpen)
		SDL_CloseAudio();
#endif

	if (_isSubsystemInitialized)
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void SdlMixerManager::init() {
	bool isDisabled = ConfMan.hasKey("disable_sdl_audio") && ConfMan.getBool("disable_sdl_audio");

	if (isDisabled) {
		warning("SDL audio subsystem was forcibly disabled by disable_sdl_audio option");
		return;
	}

	// Get the desired audio specs
	SDL_AudioSpec desired = getAudioSpec(SAMPLES_PER_SEC);

	// Start SDL Audio subsystem
#if SDL_VERSION_ATLEAST(3, 0, 0)
	if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
#else
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
#endif
		warning("Could not initialize SDL audio subsystem: %s", SDL_GetError());
		return;
	}

	_isSubsystemInitialized = true;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	const char *sdlDriverName = SDL_GetCurrentAudioDriver();
#else
	const int maxNameLen = 20;
	char sdlDriverName[maxNameLen];
	sdlDriverName[0] = '\0';
	SDL_AudioDriverName(sdlDriverName, maxNameLen);
#endif
	debug(1, "Using SDL Audio Driver \"%s\"", sdlDriverName);

#if SDL_VERSION_ATLEAST(3, 0, 0)
	_isAudioOpen = true;
	_obtained = desired;
#else
	// Needed as SDL_OpenAudio as of SDL-1.2.14 mutates fields in
	// "desired" if used directly.
	SDL_AudioSpec fmt = desired;

	// Start SDL audio with the desired specs
	if (SDL_OpenAudio(&fmt, &_obtained) != 0) {
		warning("Could not open audio device: %s", SDL_GetError());
		return;
	}

	_isAudioOpen = true;

	// The obtained sample format is not supported by the mixer, call
	// SDL_OpenAudio again with NULL as the second argument to force
	// SDL to do resampling to the desired audio spec.
	if (_obtained.format != desired.format) {
		debug(1, "SDL mixer sound format: %d differs from desired: %d", _obtained.format, desired.format);
		SDL_CloseAudio();

		if (SDL_OpenAudio(&fmt, nullptr) != 0) {
			warning("Could not open audio device: %s", SDL_GetError());
			return;
		}

		_obtained = desired;
	}
#endif

	debug(1, "Output sample rate: %d Hz", _obtained.freq);
	if (_obtained.freq != desired.freq)
		warning("SDL mixer output sample rate: %d differs from desired: %d", _obtained.freq, desired.freq);

#if !SDL_VERSION_ATLEAST(3, 0, 0)
	debug(1, "Output buffer size: %d samples", _obtained.samples);
	if (_obtained.samples != desired.samples)
		warning("SDL mixer output buffer size: %d differs from desired: %d", _obtained.samples, desired.samples);

	debug(1, "Output channels: %d", _obtained.channels);
	if (_obtained.channels != 1 && _obtained.channels != 2)
		error("SDL mixer output requires mono or stereo output device");
#endif

	int desiredSamples = 0;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_obtained, &desiredSamples);
#else
	desiredSamples = desired.samples;
#endif

	_mixer = new Audio::MixerImpl(_obtained.freq, _obtained.channels >= 2, desiredSamples);
	assert(_mixer);
	_mixer->setReady(true);

	startAudio();
}

#if !SDL_VERSION_ATLEAST(3, 0, 0)
static uint32 roundDownPowerOfTwo(uint32 samples) {
	// Public domain code from Sean Eron Anderson
	// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	uint32 rounded = samples;
	--rounded;
	rounded |= rounded >> 1;
	rounded |= rounded >> 2;
	rounded |= rounded >> 4;
	rounded |= rounded >> 8;
	rounded |= rounded >> 16;
	++rounded;

	if (rounded != samples)
		rounded >>= 1;

	return rounded;
}
#endif

SDL_AudioSpec SdlMixerManager::getAudioSpec(uint32 outputRate) {
	SDL_AudioSpec desired;

	// There was once a GUI option for this, which was removed. Configurability
	// is retained for advanced users only who wish to use the commandline
	// option (--output-rate) or modify their ScummVM config file directly.
	uint32 freq = 0;
	if (ConfMan.hasKey("output_rate"))
		freq = ConfMan.getInt("output_rate");
	if (freq <= 0)
		freq = outputRate;

	uint32 channels = 2;
	if (ConfMan.hasKey("output_channels"))
		channels = ConfMan.getInt("output_channels");
	if (channels > 2 || channels <= 0)
		channels = 2;

	// One SDL "sample" is a complete audio frame (i.e. all channels = 1 sample)
	uint32 samples = 0;

	// Different games and host systems have different performance
	// characteristics which are not easily measured, so allow advanced users to
	// tweak their audio buffer size if they are experience excess latency or
	// drop-outs by setting this value in their ScummVM config file directly
	if (ConfMan.hasKey("audio_buffer_size", Common::ConfigManager::kApplicationDomain))
		samples = ConfMan.getInt("audio_buffer_size", Common::ConfigManager::kApplicationDomain);

	// 256 is an arbitrary minimum; 32768 is the largest power-of-two value
	// representable with uint16
	if (samples < 256 || samples > 32768)
		// By default, hold no more than 45ms worth of samples to avoid
		// perceptable audio lag (ATSC IS-191). For reference, DOSBox (as of Sep
		// 2017) uses a buffer size of 1024 samples by default for a 16-bit
		// stereo 44kHz mixer, which happens to be the next lowest power of two
		// below 45ms.
		samples = freq / (1000.0 / 45);

	memset(&desired, 0, sizeof(desired));
	desired.freq = freq;
	desired.channels = channels;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	desired.format = SDL_AUDIO_S16;
#else
	desired.format = AUDIO_S16SYS;
	desired.samples = roundDownPowerOfTwo(samples);
	desired.callback = sdlCallback;
	desired.userdata = this;
#endif

	return desired;
}

void SdlMixerManager::startAudio() {
	// Start the sound system
#if SDL_VERSION_ATLEAST(3, 0, 0)
	_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_obtained, sdl3Callback, this);
	if (_stream)
		SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(_stream));
#else
	SDL_PauseAudio(0);
#endif
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

#if SDL_VERSION_ATLEAST(3, 0, 0)
void SdlMixerManager::sdl3Callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
	if (additional_amount > 0) {
		Uint8 *data = SDL_stack_alloc(Uint8, additional_amount);
		if (data) {
			SdlMixerManager *manager = (SdlMixerManager *)userdata;
			manager->sdlCallback(userdata, data, additional_amount);
			SDL_PutAudioStreamData(stream, data, additional_amount);
			SDL_stack_free(data);
		}
	}
}
#endif

void SdlMixerManager::suspendAudio() {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(_stream));
	SDL_DestroyAudioStream(_stream);
#else
	SDL_CloseAudio();
#endif
	_audioSuspended = true;
}

int SdlMixerManager::resumeAudio() {
	if (!_audioSuspended)
		return -2;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_obtained, sdl3Callback, this);
	if(!_stream)
		return -1;
	if (SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(_stream)))
		return -1;
#else
	if (SDL_OpenAudio(&_obtained, nullptr) < 0) {
		return -1;
	}
	SDL_PauseAudio(0);
#endif
	_audioSuspended = false;
	return 0;
}

#endif
