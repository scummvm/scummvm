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

#if defined(SDL_BACKEND) && defined(__EMSCRIPTEN__)

#include "backends/mixer/emscriptensdl/emscriptensdl-mixer.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/timer.h"

#if SDL_VERSION_ATLEAST(3, 0, 0)
EmscriptenSdlMixerManager::EmscriptenSdlMixerManager() : SdlMixerManager() {
	_dataBuffers = new Common::Queue<Common::Pair<int, Uint8 *> >();
}

EmscriptenSdlMixerManager::~EmscriptenSdlMixerManager() {
	// Clean up any remaining buffers
	while (_dataBuffers && !_dataBuffers->empty()) {
		Common::Pair<int, Uint8 *> dataBuffer = _dataBuffers->pop();
		SDL_free(dataBuffer.second);
	}
	delete _dataBuffers;
}
#endif

void EmscriptenSdlMixerManager::startAudio() {
	// Start the sound system
#if SDL_VERSION_ATLEAST(3, 0, 0)
	_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_obtained, emscriptenSdl3Callback, this);
	if (_stream)
		SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(_stream));
	if (!_timerStarted)
		startTimer();
#else
	SDL_PauseAudio(0);
#endif
}

void EmscriptenSdlMixerManager::suspendAudio() {
#if SDL_VERSION_ATLEAST(3, 0, 0)
	SDL_CloseAudioDevice(SDL_GetAudioStreamDevice(_stream));
	SDL_DestroyAudioStream(_stream);
	if (_timerStarted)
		stopTimer();
#else
	SDL_CloseAudio();
#endif
	_audioSuspended = true;
}

int EmscriptenSdlMixerManager::resumeAudio() {
	if (!_audioSuspended)
		return -2;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_obtained, emscriptenSdl3Callback, this);
	if (!_stream)
		return -1;
	if (SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(_stream)))
		return -1;
	if (!_timerStarted)
		startTimer();
#else
	if (SDL_OpenAudio(&_obtained, nullptr) < 0) {
		return -1;
	}
	SDL_PauseAudio(0);
#endif
	_audioSuspended = false;
	return 0;
}

#if SDL_VERSION_ATLEAST(3, 0, 0)
void EmscriptenSdlMixerManager::startTimer(int interval) {
	Common::TimerManager *manager = g_system->getTimerManager();
	if (manager->installTimerProc(updateMixer, interval, this, "EmscriptenSdlMixerManager's Timer")) {
		_timerStarted = true;
	} else {
		warning("Failed to install Networking::ConnectionManager's timer");
	}
}

void EmscriptenSdlMixerManager::stopTimer() {
	debug(9, "timer stopped");
	Common::TimerManager *manager = g_system->getTimerManager();
	manager->removeTimerProc(updateMixer);
	_timerStarted = false;
}

void EmscriptenSdlMixerManager::emscriptenSdl3Callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
	static int emptyBufferCount = 0;
	static int lastEmptyBufferPrint = 0;
	EmscriptenSdlMixerManager *manager = (EmscriptenSdlMixerManager *)userdata;

	if (manager->_dataBuffers->size() > 0) {
		// Put the data into the stream
		Common::Pair<int, Uint8 *> dataBuffer = manager->_dataBuffers->pop();
		SDL_PutAudioStreamData(manager->_stream, dataBuffer.second, dataBuffer.first);
		SDL_free(dataBuffer.second);
	} else {
		emptyBufferCount++;
	}
	if (g_system->getMillis() - 5000 > lastEmptyBufferPrint && emptyBufferCount > 0) {
		debug(5, "EmscriptenSdlMixerManager::emscriptenSdl3Callback called %d times in last 5 second with empty buffer", emptyBufferCount);
		emptyBufferCount = 0;
		lastEmptyBufferPrint = g_system->getMillis();
	}
	// Just store the requested amount, updateMixer will process it
	manager->_lastRequestedAmount = additional_amount;
}
#endif

void EmscriptenSdlMixerManager::updateMixer(void *refCon) {
	EmscriptenSdlMixerManager *manager = (EmscriptenSdlMixerManager *)refCon;
#if SDL_VERSION_ATLEAST(3, 0, 0)
	static int fullBufferCount = 0;
	static uint lastFullBufferPrint = 0;
	if (manager->_dataBuffers->size() < 2) {
		int amount = manager->_lastRequestedAmount > 0 ? manager->_lastRequestedAmount : 8 * 1024;
		if (manager->_stream) {
			Uint8 *dataBuffer = (Uint8 *)SDL_malloc(sizeof(Uint8) * (amount));
			if (dataBuffer) {
				// Generate the audio data
				sdlCallback(manager, dataBuffer, amount);
				manager->_dataBuffers->push({amount, dataBuffer});
			}
		}
		manager->_lastRequestedAmount = -1; // Reset to -1 after reading
	} else {
		fullBufferCount++;
	}
	if (g_system->getMillis() - 5000 > lastFullBufferPrint && fullBufferCount > 0) {
		debug(5, "EmscriptenSdlMixerManager::updateMixer called %d times in last 5 second with full buffer", fullBufferCount);
		fullBufferCount = 0;
		lastFullBufferPrint = g_system->getMillis();
	}
#endif
	// For non-SDL3 versions, this method does nothing
}

#endif
